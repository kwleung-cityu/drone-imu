#include "ringBuffer.h"

void initRingBuffer(ringBuffer_t *rb) {
    rb->head = 0;
    rb->tail = 0;
    rb->overflow = false;
}

/*
// For reference, here's a simple implementation of writeToBuffer that handles overflow by discarding the oldest sample when the buffer is full. 
// This is a common approach for circular buffers.
void writeToBuffer(ringBuffer_t *rb, imuRawData *sample) {
    rb->buffer[rb->head].accel[0] = sample->accel[0];
    rb->buffer[rb->head].accel[1] = sample->accel[1];
    rb->buffer[rb->head].accel[2] = sample->accel[2];
    rb->buffer[rb->head].temperature = sample->temperature;
    rb->buffer[rb->head].gyro[0] = sample->gyro[0];
    rb->buffer[rb->head].gyro[1] = sample->gyro[1];
    rb->buffer[rb->head].gyro[2] = sample->gyro[2];
    rb->head = (rb->head + 1) % BUF_SIZE;

    // Handle overflow: if head catches up to tail, move tail forward
    if (rb->head == rb->tail) {
        rb->tail = (rb->tail + 1) % BUF_SIZE;  // Discard oldest sample
        rb->overflow = true;
    }
}
*/

// Write a sample to the ring buffer
void writeToBuffer(ringBuffer_t *rb, imuRawData *sample) {
        uint8_t nextHead = (rb->head + 1) % BUF_SIZE;
        
        // Check if buffer is full (next head would overwrite tail)
        if (nextHead == rb->tail) {
            rb->overflow = true;  // Buffer full, overwrite oldest or drop
        }
        
        __disable_irq();
        rb->buffer[rb->head] = *sample;
        rb->head = nextHead;
        __enable_irq();
}

bool readFromBuffer(ringBuffer_t *rb, imuRawData *sample) {
    if (rb->head == rb->tail) {
        return false;  // Buffer empty
    }
    
    __disable_irq();
    *sample = rb->buffer[rb->tail];
    rb->tail = (rb->tail + 1) % BUF_SIZE;
    __enable_irq();
    
    return true;
}

uint8_t getBufferCount(ringBuffer_t *rb) {
    return RB_COUNT(rb);
}

bool isImuDataAvailable() {
    return !RB_IS_EMPTY(&imuRingBuffer);
}