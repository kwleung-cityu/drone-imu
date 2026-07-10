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
#define MPU6050_INT_PIN_CFG  0x37  // Interrupt pin configuration register
#define MPU6050_ACCEL_XOUT_H  0x3B  //register address for accelerometer X-axis high byte
#define MPU6050_ACCEL_YOUT_H  0x3D  //register address for accelerometer Y-axis high byte
#define MPU6050_ACCEL_ZOUT_H  0x3F  //register address for accelerometer Z-axis high byte
#define MPU6050_TEMP_OUT_H    0x41  //register address for temperature high byte
#define MPU6050_GYRO_XOUT_H   0x43  //register address for gyroscope X-axis high byte
#define MPU6050_GYRO_YOUT_H   0x45  //register address for gyroscope Y-axis high byte
#define MPU6050_GYRO_ZOUT_H   0x47  //register address for gyroscope Z-axis high byte

namespace imu {
    ringBuffer_t    imuRingBuffer;          // Global instance of the ring buffer for IMU data
    imuConfig_t     imuConfig;              // Global configuration for the IMU
    MicroBitPin*    imuIntPin = nullptr;    // Pointer to the micro:bit pin used for IMU interrupt

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

        int divider = baseRateHz / rateHz - 1;
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

    // Helper function to map pin ID to MicroBitPin pointer
    static MicroBitPin* getPinFromId(int pinId) {
        switch (pinId) {
        case 0: return &uBit.io.P0;
        case 1: return &uBit.io.P1;
        case 2: return &uBit.io.P2;
        case 3: return &uBit.io.P3;
        case 4: return &uBit.io.P4;
        case 5: return &uBit.io.P5;
        case 6: return &uBit.io.P6;
        case 7: return &uBit.io.P7;
        case 8: return &uBit.io.P8;
        case 9: return &uBit.io.P9;
        case 10: return &uBit.io.P10;
        case 11: return &uBit.io.P11; 
        case 12: return &uBit.io.P12;
        case 13: return &uBit.io.P13;
        case 14: return &uBit.io.P14;
        case 15: return &uBit.io.P15;
        case 16: return &uBit.io.P16;
        case 19: return &uBit.io.P19;
        case 20: return &uBit.io.P20;
        default: return nullptr; // Invalid pin ID
        }
    }

    // Helper function to map pin ID to MicroBitEvent ID
    static uint16_t getPinEventId(int pinId) {
        switch (pinId) {
        case 0: return MICROBIT_ID_IO_P0;
        case 1: return MICROBIT_ID_IO_P1;
        case 2: return MICROBIT_ID_IO_P2;
        case 3: return MICROBIT_ID_IO_P3;
        case 4: return MICROBIT_ID_IO_P4;
        case 5: return MICROBIT_ID_IO_P5;
        case 6: return MICROBIT_ID_IO_P6;
        case 7: return MICROBIT_ID_IO_P7;
        case 8: return MICROBIT_ID_IO_P8;
        case 9: return MICROBIT_ID_IO_P9;
        case 10: return MICROBIT_ID_IO_P10;
        case 11: return MICROBIT_ID_IO_P11; 
        case 12: return MICROBIT_ID_IO_P12;
        case 13: return MICROBIT_ID_IO_P13;
        case 14: return MICROBIT_ID_IO_P14;
        case 15: return MICROBIT_ID_IO_P15;
        case 16: return MICROBIT_ID_IO_P16;
        case 19: return MICROBIT_ID_IO_P19;
        case 20: return MICROBIT_ID_IO_P20;
        default: return pinId; // already an internal ID (for enum inputs)
        }
    }

    // Helper function to handle IMU data ready interrupt
    static void onImuDataReady() {
        imuRawData sample;
        //burst read 14 bytes starting from ACCEL_XOUT_H
        imuBurstRead(MPU6050_ADDR, MPU6050_ACCEL_XOUT_H, (uint8_t*)&sample, sizeof(sample));

        writeToBuffer(&imuRingBuffer, &sample);
    }

    // Helper function to handle pin interrupt for IMU data ready
    static void onPinInterrupt(MicroBitEvent e) {
        if(e.source == getPinEventId(MPU6050_INT_PIN) && e.value == MICROBIT_PIN_EVT_PULSE_LO) {
            onImuDataReady();
        }
    }

    // Helper function to initialize the IMU with settings from imu.h
    //%
    void imuInit() {
        initRingBuffer(&imuRingBuffer);

        // Wake up the MPU6050 (clear sleep bit).
        imuRegWrite(MPU6050_ADDR, MPU6050_PWR_MGMT_1, 0x00);    // Clear sleep bit to wake up the device
        uBit.sleep(100); // Wait for the device to stabilize

        const uint8_t dlpfCfg = sanitizeDlpfMode(imuConfig.dlpfMode);
        const uint8_t accelCfg = static_cast<uint8_t>(sanitizeAccelRange(imuConfig.accelRange) << 3);
        const uint8_t gyroCfg = static_cast<uint8_t>(sanitizeGyroRange(imuConfig.gyroRange) << 3);
        const uint8_t sampleRateDivider = computeSampleRateDivider(dlpfCfg, imuConfig.sampleRate);
        const uint8_t intEnable = imuConfig.enableInterrupts ? 0x01 : 0x00;

        // Set DLPF to imuConfig.dlpfMode
        imuRegWrite(MPU6050_ADDR, MPU6050_CONFIG, dlpfCfg);
        // Set Gyroscope Full Scale Range
        imuRegWrite(MPU6050_ADDR, MPU6050_GYRO_CONFIG, gyroCfg);
        // Set Accelerometer Full Scale Range
        imuRegWrite(MPU6050_ADDR, MPU6050_ACCEL_CONFIG, accelCfg);
        // Critical: Set Sample Rate Divider
        // If we want 166Hz sample rate, we need to set SMPLRT_DIV to 5 (1kHz / (1 + 5) = 166.67Hz)
        imuRegWrite(MPU6050_ADDR, MPU6050_SMPLRT_DIV, sampleRateDivider);
        // Configure the interrupt pin to be active high, push-pull, and clear on read
        imuRegWrite(MPU6050_ADDR, MPU6050_INT_PIN_CFG, 0x00); // set active high

        // ========== Configure micro:bit GPIO for interrupt handling (DATA_RDY) ===========
        imuIntPin = getPinFromId(MPU6050_INT_PIN);
        if (imuIntPin == nullptr) {
            //alert message printing to console 
            uBit.serial.printf("Error: Invalid pin configuration for IMU interrupt\r\n");
        }   
        imuIntPin->getDigitalValue(); // Clear any pending interrupt
        // Register the interrupt handler for IMU data ready
        uBit.messageBus.listen(getPinEventId(MPU6050_INT_PIN), MICROBIT_PIN_EVT_PULSE_LO, onPinInterrupt);
        // Enable edge detection for the interrupt pin
        imuIntPin->eventOn(MICROBIT_PIN_EVT_PULSE_LO);

        // Enable interrupt for DATA_RDY if imuConfig.enableInterrupts is true
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
