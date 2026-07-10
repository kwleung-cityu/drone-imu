/**
 * @file imu.h
 * @brief Header file for the IMU (Inertial Measurement Unit) interface.
 * This header defines the data structures, enumerations, and function prototypes 
 * for interacting with the MPU6050 IMU sensor. 
 * It includes configuration options for accelerometer and gyroscope ranges, 
 * as well as digital low-pass filter (DLPF) settings.
 */
#ifndef _IMU_H
#define _IMU_H

#include "pxt.h"

// Pin definition for INT on micro:bit
#define MPU6050_INT_PIN      MICROBIT_PIN_P0  // Change as needed e.g. MICROBIT_PIN_P1, MICROBIT_PIN_P2, etc.

// DLPF Configuration modes for MPU6050
enum DLPFMode : uint8_t {
        DLPF_260_HZ = 0,   // Accel: 260Hz, Gyro: 256Hz, Delay: 0ms
        DLPF_184_HZ = 1,   // Accel: 184Hz, Gyro: 188Hz, Delay: 2ms
        DLPF_94_HZ  = 2,   // Accel: 94Hz,  Gyro: 98Hz,  Delay: 3ms
        DLPF_44_HZ  = 3,   // Accel: 44Hz,  Gyro: 42Hz,  Delay: 4.9ms
        DLPF_21_HZ  = 4,   // Accel: 21Hz,  Gyro: 20Hz,  Delay: 8.3ms
        DLPF_10_HZ  = 5,   // Accel: 10Hz,  Gyro: 10Hz,  Delay: 13.4ms
        DLPF_5_HZ   = 6,   // Accel: 5Hz,   Gyro: 5Hz,   Delay: 18.6ms
        DLPF_RESERVED = 7  // Do not use
};
    
// Accelerometer Full Scale Range
enum AccelRange : uint8_t {
    ACCEL_RANGE_2G  = 0,
    ACCEL_RANGE_4G  = 1,
    ACCEL_RANGE_8G  = 2,
    ACCEL_RANGE_16G = 3
};

// Gyroscope Full Scale Range
enum GyroRange : uint8_t {
    GYRO_RANGE_250DPS  = 0,
    GYRO_RANGE_500DPS  = 1,
    GYRO_RANGE_1000DPS = 2,
    GYRO_RANGE_2000DPS = 3
};

struct imuConfig_t {
    AccelRange accelRange = ACCEL_RANGE_2G;
    GyroRange gyroRange = GYRO_RANGE_250DPS;
    DLPFMode dlpfMode = DLPF_94_HZ;    // Default: 94Hz bandwith
    int sampleRate = 166;              // set SMPRT_DIV to achieve this sample rate (in Hz)
                                       // We will use dt=0.006 (1/166) in Kalman filter because of this sample rate.
                                       // Why sampleRate=166 and dt=0.006? Read `Configuring DLPF and Sample Rate on MPU6050.md`
    bool enableInterrupts = true;      // Enable DATA_RDY interrupt for IMU data ready
    // Calculated field based on DLPF mode (read-only in practice)
    float getAccelBandwidth() const {
        switch(dlpfMode) {
            case DLPF_260_HZ: return 260.0f;
            case DLPF_184_HZ: return 184.0f;
            case DLPF_94_HZ:  return 94.0f;
            case DLPF_44_HZ:  return 44.0f;
            case DLPF_21_HZ:  return 21.0f;
            case DLPF_10_HZ:  return 10.0f;
            case DLPF_5_HZ:   return 5.0f;
            default:          return 0.0f;
        }
    }   

    float getGyroBandwidth() const {
        switch(dlpfMode) {
            case DLPF_260_HZ: return 256.0f;
            case DLPF_184_HZ: return 188.0f;
            case DLPF_94_HZ:  return 98.0f;
            case DLPF_44_HZ:  return 42.0f;
            case DLPF_21_HZ:  return 20.0f;
            case DLPF_10_HZ:  return 10.0f;
            case DLPF_5_HZ:   return 5.0f;
            default:          return 0.0f;
        }
    }  

    float getDelay() const {
        switch(dlpfMode) {
            case DLPF_260_HZ: return 0.0f;
            case DLPF_184_HZ: return 2.0f;
            case DLPF_94_HZ:  return 3.0f;
            case DLPF_44_HZ:  return 4.9f;
            case DLPF_21_HZ:  return 8.3f;
            case DLPF_10_HZ:  return 13.4f;
            case DLPF_5_HZ:   return 18.6f;
            default:          return 0.0f;
        }
    }   
};

#endif
