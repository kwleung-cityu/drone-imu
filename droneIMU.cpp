#include "pxt.h"

#if MICROBIT_CODAL
#define BUFFER_TYPE uint8_t *
#else
#define BUFFER_TYPE char *
#endif

namespace droneIMUV3 {
    static int normalizePinId(int pinId) {
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
    int imuRun100HzBurstReadToggleTest(int addr, int pinId, int cycles) {
        if (cycles < 1)
            cycles = 200;

        MicroBitPin *pin = pxt::getPin(normalizePinId(pinId));
        if (!pin)
            return -1;

        uint8_t reg = 0x3B;
        uint8_t data[14];

        for (int i = 0; i < cycles; i++) {
            pin->setDigitalValue(1);
            uBit.i2c.write(addr << 1, (BUFFER_TYPE)&reg, 1, true);
            uBit.i2c.read(addr << 1, (BUFFER_TYPE)&data[0], 14, false);
            pin->setDigitalValue(0);
            pxt::sleep_us(10000);
        }

        return cycles;
    }

    //%
    int imuRun100HzToggleTest(int addr, int pinId, int cycles, bool includeRead) {
        if (cycles < 1)
            cycles = 200;

        MicroBitPin *pin = pxt::getPin(normalizePinId(pinId));
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
