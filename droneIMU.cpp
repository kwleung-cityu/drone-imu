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

    // PID state (single loop) for simple educational control tasks.
    static float pidKp = 0.0f;
    static float pidKi = 0.0f;
    static float pidKd = 0.0f;
    static float pidOutMin = -1024.0f;
    static float pidOutMax = 1024.0f;
    static float pidIntegral = 0.0f;
    static float pidPrevError = 0.0f;
    static bool pidHasPrev = false;

    inline float clampf(float v, float lo, float hi) {
        if (v < lo) return lo;
        if (v > hi) return hi;
        return v;
    }

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

    //%
    void pidConfigure(float kp, float ki, float kd, float outMin, float outMax) {
        pidKp = kp;
        pidKi = ki;
        pidKd = kd;

        if (outMin <= outMax) {
            pidOutMin = outMin;
            pidOutMax = outMax;
        } else {
            // Keep limits valid even if user passes reversed bounds.
            pidOutMin = outMax;
            pidOutMax = outMin;
        }

        // Integral clamp uses output bounds for a simple anti-windup scheme.
        pidIntegral = clampf(pidIntegral, pidOutMin, pidOutMax);
    }

    //%
    void pidReset() {
        pidIntegral = 0.0f;
        pidPrevError = 0.0f;
        pidHasPrev = false;
    }

    //%
    float pidUpdate(float setpoint, float measurement, float dtMs) {
        float dt = dtMs / 1000.0f;
        if (dt <= 0.000001f) dt = 0.01f;

        float error = setpoint - measurement;
        pidIntegral += error * dt;
        pidIntegral = clampf(pidIntegral, pidOutMin, pidOutMax);

        float derivative = 0.0f;
        if (pidHasPrev) {
            derivative = (error - pidPrevError) / dt;
        }

        float output = pidKp * error + pidKi * pidIntegral + pidKd * derivative;
        output = clampf(output, pidOutMin, pidOutMax);

        pidPrevError = error;
        pidHasPrev = true;
        return output;
    }
}