#include "pxt.h"

namespace droneIMU {
    // MPU6050 Default I2C Address (when AD0 pin is tied to GND)
    const uint8_t MPU_ADDR = 0x68 << 1; 

    // MPU6050 Internal Register Map
    const uint8_t REG_SMPLRT_DIV   = 0x19;
    const uint8_t REG_CONFIG       = 0x1A;
    const uint8_t REG_GYRO_CONFIG  = 0x1B;
    const uint8_t REG_ACCEL_CONFIG = 0x1C;
    const uint8_t REG_ACCEL_XOUT_H = 0x3B;
    const uint8_t REG_PWR_MGMT_1   = 0x6B;

    // Local helper function to write 1 byte over I2C
    void writeReg(uint8_t reg, uint8_t value) {
        uint8_t buf[2] = {reg, value};
        uBit.i2c.write(MPU_ADDR, buf, 2);
    }

    //%
    void initMPU6050() {
        // 1. Wake up MPU6050 and set clock source to X-axis Gyro PLL
        writeReg(REG_PWR_MGMT_1, 0x01);
        fiber_sleep(10); // Short delay for clock stabilization

        // 2. Set DLPF (Digital Low Pass Filter) to 42Hz Bandwidth (Removes drone frame vibrations)
        writeReg(REG_CONFIG, 0x03);

        // 3. Set Sample Rate Divider to 9 -> 1kHz / (1 + 9) = 100Hz output data rate
        writeReg(REG_SMPLRT_DIV, 0x09);

        // 4. Configure Gyro Full Scale Range to +/- 500 deg/s (Standard for mini drones)
        writeReg(REG_GYRO_CONFIG, 0x08);

        // 5. Configure Accel Full Scale Range to +/- 4g
        writeReg(REG_ACCEL_CONFIG, 0x08);
    }

    //%
    Buffer getSensorData() {
        uint8_t reg = REG_ACCEL_XOUT_H;
        uint8_t data[14] = {0};

        // Point to the first data register (Accel X High)
        uBit.i2c.write(MPU_ADDR, &reg, 1, true); 
        // Read 14 sequential bytes (6 Accel, 2 Temp, 6 Gyro)
        uBit.i2c.read(MPU_ADDR, data, 14);

        // Allocate a MakeCode managed buffer to pass back safely to TypeScript/Python
        return mkBuffer(data, 14);
    }
}