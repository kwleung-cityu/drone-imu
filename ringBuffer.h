/*
    * ringBuffer.h
    *
    * Ring buffer implementation for IMU data logging.
    *
    * This header defines the data structures and configuration parameters for a ring buffer
    * that stores raw IMU samples. The buffer is designed to handle overflow situations and
    * provide recovery mechanisms to ensure data integrity.
    *
*/
#ifndef _RINGBUFFER_H_
#define _RINGBUFFER_H_

#include "pxt.h"

// ============ CONFIGURATION ============
#define BUF_SIZE 64  // Power of 2 for optimal modulo; reduced to fit micro:bit v1 RAM

// ============ DATA STRUCTURES ============
// MPU6050 raw sample structure
struct imuRawData {
    int16_t accel[3];       // ±2g/±4g/±8g/±16g scale
    int16_t temperature;    // Raw temperature value
    int16_t gyro[3];        // ±250/±500/±1000/±2000 °/s scale
};

typedef struct {
    volatile imuRawData buffer[BUF_SIZE];  // Circular buffer for IMU samples
    volatile uint32_t head;                 // Index for the next write
    volatile uint32_t tail;                 // Index for the next read
    volatile bool overflow;                 // Flag indicating if an overflow has occurred
} ringBuffer_t;

// Function prototypes
void initRingBuffer(ringBuffer_t *rb);
void writeToBuffer(ringBuffer_t *rb, imuRawData *sample);
int readFromBuffer(ringBuffer_t *rb, imuRawData *sample);
int isBufferEmpty(ringBuffer_t *rb);
int isBufferFull(ringBuffer_t *rb);

// Inline helper macros (optional, for performance)
#define RB_IS_EMPTY(rb) ((rb)->head == (rb)->tail)
#define RB_IS_FULL(rb)  (((rb)->head + 1) % BUF_SIZE == (rb)->tail)
#define RB_COUNT(rb)    (((rb)->head - (rb)->tail) % BUF_SIZE)

// ============ GLOBAL INSTANCE ============
extern ringBuffer_t imuRingBuffer;

#endif // _RINGBUFFER_H_
