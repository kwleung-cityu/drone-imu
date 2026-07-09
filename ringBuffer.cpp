#include "ringBuffer.h"

void initRingBuffer(ringBuffer_t *rb) {
    rb->head = 0;
    rb->tail = 0;
    rb->overflow = false;
}

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
