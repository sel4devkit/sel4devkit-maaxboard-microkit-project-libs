#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <circular_buffer.h>
#include <microkit.h>

circular_buffer_t* circular_buffer_init(uintptr_t circular_buffer, size_t size, uintptr_t data_buffer) {
    circular_buffer_t *cb = (circular_buffer_t *)circular_buffer;
    cb->buffer = (char *)data_buffer;
    cb->max = size;
    circular_buffer_reset(cb);
    return cb;
}

void circular_buffer_free(circular_buffer_t *cb) {
    free(cb->buffer);
    free(cb);
}

void circular_buffer_reset(circular_buffer_t *cb) {
    cb->head = 0;
    cb->tail = 0;
    cb->full = false;
}

bool circular_buffer_full(circular_buffer_t *cb) {
    return cb->full;
}

bool circular_buffer_empty(circular_buffer_t *cb) {
    return (!cb->full && (cb->head == cb->tail));
}

void circular_buffer_put(circular_buffer_t *cb, uintptr_t data_buffer, size_t data_size, char data) {
    // Invalidate the cache
    seL4_ARM_VSpace_Invalidate_Data(3, data_buffer, data_buffer + data_size);

    cb->buffer[cb->head] = data;

    // Ensure buffer written before updating head
    __sync_synchronize(); 

    // Flush the cache
    seL4_ARM_VSpace_CleanInvalidate_Data(3, data_buffer, data_buffer + data_size);

    if(cb->full) {
        // Increment tail to make space for new data
        cb->tail = (cb->tail + 1) % cb->max;
    }

    cb->head = (cb->head + 1) % cb->max;
    cb->full = (cb->head == cb->tail);

    // Flush the cache
    seL4_ARM_VSpace_CleanInvalidate_Data(3, data_buffer, data_buffer + data_size);
}

char circular_buffer_get(circular_buffer_t *cb, uintptr_t data_buffer, size_t data_size) {
    // Invalidate the cache
    seL4_ARM_VSpace_Invalidate_Data(3, data_buffer, data_buffer + data_size);

    if (circular_buffer_empty(cb)) {
        // Buffer is empty
        return -1; 
    }

    int data = cb->buffer[cb->tail];

    // Ensure buffer read before updating tail
    __sync_synchronize(); 

    cb->full = false;
    cb->tail = (cb->tail + 1) % cb->max;

    // Flush the cache
    seL4_ARM_VSpace_CleanInvalidate_Data(3, data_buffer, data_buffer + data_size);

    return data;
}

