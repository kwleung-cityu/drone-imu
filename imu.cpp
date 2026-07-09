#include "imu.h"
#include "ringBuffer.h"

#if MICROBIT_CODAL
#define BUFFER_TYPE uint8_t *
#else
#define BUFFER_TYPE char *
#endif

// I2C address configuration
#define MPU6050_ADDR       0x68     // Default I2C address
#define MPU6050_ADDR_MASK  0x7F     // 7-bit I2C address mask
// MPU6050 specific constants
#define MPU6050_SMPLRT_DIV   0x19  // Sample rate divider register
#define MPU6050_CONFIG       0x1A  // DLPF configuration register
#define MPU6050_GYRO_CONFIG  0x1B  // Gyroscope range register
#define MPU6050_ACCEL_CONFIG 0x1C  // Accelerometer range register
#define MPU6050_INT_ENABLE   0x38  // Interrupt enable register
#define MPU6050_PWR_MGMT_1   0x6B  // Power management register
#define MPU6050_ACCEL_XOUT_H  0x3B  //register address for accelerometer X-axis high byte
#define MPU6050_ACCEL_YOUT_H  0x3D  //register address for accelerometer Y-axis high byte
#define MPU6050_ACCEL_ZOUT_H  0x3F  //register address for accelerometer Z-axis high byte
#define MPU6050_TEMP_OUT_H    0x41  //register address for temperature high byte
#define MPU6050_GYRO_XOUT_H   0x43  //register address for gyroscope X-axis high byte
#define MPU6050_GYRO_YOUT_H   0x45  //register address for gyroscope Y-axis high byte
#define MPU6050_GYRO_ZOUT_H   0x47  //register address for gyroscope Z-axis high byte

namespace imu {
    ringBuffer_t imuRingBuffer; // Global instance of the ring buffer for IMU data
    imuConfig_t imuConfig;      // Global configuration for the IMU

    /**
     * @brief Write a single register on the MPU6050
     * @param devAddr Device I2C address (7-bit)
     * @param regAddr Register address to write
     * @param data Data byte to write
     */
    static void __inline imuRegWrite(uint8_t devAddr, uint8_t regAddr, uint8_t data) {
        uint8_t buffer[2] = {regAddr, data};
        uBit.i2c.write((uint16_t)(devAddr << 1), (BUFFER_TYPE)&buffer[0], 2, false);
    }

    /**
     * @brief Read a single register from the MPU6050
     * @param devAddr Device I2C address (7-bit)
     * @param regAddr Register address to read
     * @return Value read from the register
     */
    static uint8_t __inline imuRegRead(uint8_t devAddr, uint8_t regAddr) {
        uint8_t r = (uint8_t)regAddr;
        uint8_t v = 0;
        uBit.i2c.write((uint16_t)(devAddr << 1), (BUFFER_TYPE)&r, 1, true);
        uBit.i2c.read((uint16_t)(devAddr << 1), (BUFFER_TYPE)&v, 1, false);
        return v;
    }

    /**
     * @brief Burst read multiple registers from the MPU6050
     * @param devAddr Device I2C address (7-bit)
     * @param startReg Starting register address to read
     * @param buffer Pointer to the buffer to store read data
     * @param length Number of bytes to read
     * @return Number of bytes read
     */
    static uint8_t __inline imuBurstRead(uint8_t devAddr, uint8_t startReg, uint8_t *buffer, uint8_t length) {
        if (buffer == NULL || length == 0) {
        return 0;  // Error: invalid parameters
        }

        uint8_t r = (uint8_t)startReg;
        uBit.i2c.write((uint16_t)(devAddr << 1), (BUFFER_TYPE)&r, 1, true);
        uBit.i2c.read((uint16_t)(devAddr << 1), (BUFFER_TYPE)buffer, length, false);
        return length;
    }

    static uint8_t sanitizeDlpfMode(DLPFMode mode) {
        uint8_t value = static_cast<uint8_t>(mode);
        return value <= 6 ? value : static_cast<uint8_t>(DLPF_94_HZ);
    }

    static uint8_t sanitizeAccelRange(AccelRange range) {
        uint8_t value = static_cast<uint8_t>(range);
        return value <= 3 ? value : static_cast<uint8_t>(ACCEL_RANGE_2G);
    }

    static uint8_t sanitizeGyroRange(GyroRange range) {
        uint8_t value = static_cast<uint8_t>(range);
        return value <= 3 ? value : static_cast<uint8_t>(GYRO_RANGE_250DPS);
    }

    static uint8_t computeSampleRateDivider(uint8_t dlpfCfg, int targetRateHz) {
        // MPU6050 internal sample clock: 8kHz when DLPF is disabled (cfg=0), else 1kHz.
        const int baseRateHz = (dlpfCfg == 0) ? 8000 : 1000;
        int rateHz = targetRateHz;

        if (rateHz < 1) rateHz = 1;
        if (rateHz > baseRateHz) rateHz = baseRateHz;

        int divider = (baseRateHz + (rateHz / 2)) / rateHz - 1;
        if (divider < 0) divider = 0;
        if (divider > 255) divider = 255;

        return static_cast<uint8_t>(divider);
    }

    static uint16_t accelLsbPerGFromRange(AccelRange range) {
        switch (sanitizeAccelRange(range)) {
        case ACCEL_RANGE_2G:  return 16384;
        case ACCEL_RANGE_4G:  return 8192;
        case ACCEL_RANGE_8G:  return 4096;
        case ACCEL_RANGE_16G: return 2048;
        default:              return 16384;
        }
    }

    static float gyroLsbPerDpsFromRange(GyroRange range) {
        switch (sanitizeGyroRange(range)) {
        case GYRO_RANGE_250DPS:  return 131.0f;
        case GYRO_RANGE_500DPS:  return 65.5f;
        case GYRO_RANGE_1000DPS: return 32.8f;
        case GYRO_RANGE_2000DPS: return 16.4f;
        default:                 return 131.0f;
        }
    }

    // Helper function to initialize the IMU with settings from imu.h
    //%
    void imuInit() {
        initRingBuffer(&imuRingBuffer);

        // Wake up the MPU6050 (clear sleep bit).
        imuRegWrite(MPU6050_ADDR, MPU6050_PWR_MGMT_1, 0x00);
        uBit.sleep(100); // Wait for the device to stabilize

        const uint8_t dlpfCfg = sanitizeDlpfMode(imuConfig.dlpfMode);
        const uint8_t accelCfg = static_cast<uint8_t>(sanitizeAccelRange(imuConfig.accelRange) << 3);
        const uint8_t gyroCfg = static_cast<uint8_t>(sanitizeGyroRange(imuConfig.gyroRange) << 3);
        const uint8_t sampleRateDivider = computeSampleRateDivider(dlpfCfg, imuConfig.sampleRate);
        const uint8_t intEnable = imuConfig.enableInterrupts ? 0x01 : 0x00;

        imuRegWrite(MPU6050_ADDR, MPU6050_CONFIG, dlpfCfg);
        imuRegWrite(MPU6050_ADDR, MPU6050_GYRO_CONFIG, gyroCfg);
        imuRegWrite(MPU6050_ADDR, MPU6050_ACCEL_CONFIG, accelCfg);
        imuRegWrite(MPU6050_ADDR, MPU6050_SMPLRT_DIV, sampleRateDivider);
        imuRegWrite(MPU6050_ADDR, MPU6050_INT_ENABLE, intEnable);
    } 

    //%
    int imuGetAccelLsbPerG() {
        return accelLsbPerGFromRange(imuConfig.accelRange);
    }

    //%
    float imuGetGyroLsbPerDps() {
        return gyroLsbPerDpsFromRange(imuConfig.gyroRange);
    }

    //% 
    int16_t imuReadAccelX() {
        uint8_t buffer[2];
        imuBurstRead(MPU6050_ADDR, MPU6050_ACCEL_XOUT_H, buffer, 2);
        return (int16_t)((buffer[0] << 8) | buffer[1]);
    }
    
    //% 
    int16_t imuReadAccelY() {
        uint8_t buffer[2];
        imuBurstRead(MPU6050_ADDR, MPU6050_ACCEL_YOUT_H, buffer, 2);
        return (int16_t)((buffer[0] << 8) | buffer[1]);
    }
    
    //% 
    int16_t imuReadAccelZ() {
        uint8_t buffer[2];
        imuBurstRead(MPU6050_ADDR, MPU6050_ACCEL_ZOUT_H, buffer, 2);
        return (int16_t)((buffer[0] << 8) | buffer[1]);
    }

    //%
    int16_t imuReadGyroX() {
        uint8_t buffer[2];
        imuBurstRead(MPU6050_ADDR, MPU6050_GYRO_XOUT_H, buffer, 2);
        return (int16_t)((buffer[0] << 8) | buffer[1]);
    }

    //%
    int16_t imuReadGyroY() {
        uint8_t buffer[2];
        imuBurstRead(MPU6050_ADDR, MPU6050_GYRO_YOUT_H, buffer, 2);
        return (int16_t)((buffer[0] << 8) | buffer[1]);
    }

    //%
    int16_t imuReadGyroZ() {
        uint8_t buffer[2];
        imuBurstRead(MPU6050_ADDR, MPU6050_GYRO_ZOUT_H, buffer, 2);
        return (int16_t)((buffer[0] << 8) | buffer[1]);
    }   

    //%
    int16_t imuReadTemperature() {
        uint8_t buffer[2];
        imuBurstRead(MPU6050_ADDR, MPU6050_TEMP_OUT_H, buffer, 2);
        return (int16_t)((buffer[0] << 8) | buffer[1]);
    } 
  
} //namespace imu
