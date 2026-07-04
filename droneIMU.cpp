#include "pxt.h"

#if MICROBIT_CODAL
#define BUFFER_TYPE uint8_t *
#else
#define BUFFER_TYPE char *
#endif

namespace droneIMUV3 {
    static int readRegInternal(int addr, int reg) {
        uint8_t r = (uint8_t)reg;
        uint8_t v = 0;
        uBit.i2c.write(addr << 1, (BUFFER_TYPE)&r, 1, true);
        uBit.i2c.read(addr << 1, (BUFFER_TYPE)&v, 1, false);
        return v;
    }

    static int readWordInternal(int addr, int regHigh) {
        uint8_t r = (uint8_t)regHigh;
        uint8_t d[2] = {0, 0};
        uBit.i2c.write(addr << 1, (BUFFER_TYPE)&r, 1, true);
        uBit.i2c.read(addr << 1, (BUFFER_TYPE)&d[0], 2, false);
        int v = (d[0] << 8) | d[1];
        if (v & 0x8000)
            v -= 0x10000;
        return v;
    }

    //%
    int nativeConst123() {
        return 123;
    }

    //%
    int imuReadReg(int addr, int reg) {
        return readRegInternal(addr, reg);
    }

    //%
    void imuWriteReg(int addr, int reg, int value) {
        uint8_t b[2];
        b[0] = (uint8_t)reg;
        b[1] = (uint8_t)value;
        uBit.i2c.write(addr << 1, (BUFFER_TYPE)&b[0], 2, false);
    }

    //%
    int imuReadWord(int addr, int regHigh) {
        return readWordInternal(addr, regHigh);
    }

    //%
    Buffer imuReadSensorPacket14(int addr) {
        Buffer buf = pins::createBuffer(14);
        uint8_t reg = 0x3B;
        uBit.i2c.write(addr << 1, (BUFFER_TYPE)&reg, 1, true);
        uBit.i2c.read(addr << 1, (BUFFER_TYPE)buf->data, 14, false);
        return buf;
    }

    //%
    int imuRun100HzToggleTest(int addr, int pinId, int cycles, bool includeRead) {
        if (cycles < 1)
            cycles = 200;

        MicroBitPin *pin = pxt::getPin(pinId);
        if (!pin)
            return -1;

        for (int i = 0; i < cycles; i++) {
            if (includeRead) {
                volatile int gx = readWordInternal(addr, 0x43);
                (void)gx;
            }

            pin->setDigitalValue(1);
            pxt::sleep_us(5000);
            pin->setDigitalValue(0);
            pxt::sleep_us(5000);
        }

        return cycles;
    }
}
