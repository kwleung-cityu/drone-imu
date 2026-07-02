#include "pxt.h"

namespace droneIMUV3 {
    const uint8_t MPU_ADDR = 0x68 << 1;
    const uint8_t REG_SMPLRT_DIV = 0x19;
    const uint8_t REG_CONFIG = 0x1A;
    const uint8_t REG_GYRO_CONFIG = 0x1B;
    const uint8_t REG_ACCEL_CONFIG = 0x1C;
    const uint8_t REG_PWR_MGMT_1 = 0x6B;

    inline void writeReg(uint8_t reg, uint8_t value) {
        uint8_t buf[2] = {reg, value};
        uBit.i2c.write(MPU_ADDR, buf, 2);
    }

    //%
    void initMPU6050() {
        writeReg(REG_PWR_MGMT_1, 0x01);
        fiber_sleep(10);
        writeReg(REG_CONFIG, 0x03);
        writeReg(REG_SMPLRT_DIV, 0x09);
        writeReg(REG_GYRO_CONFIG, 0x08);
        writeReg(REG_ACCEL_CONFIG, 0x08);
    }
}
