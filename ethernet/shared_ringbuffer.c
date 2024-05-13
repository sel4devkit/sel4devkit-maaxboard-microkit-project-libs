/* This work is Crown Copyright NCSC, 2023. */
/*
 * Copyright 2022, UNSW
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <shared_ringbuffer.h>

void ring_init(ring_handle_t *ring, ring_buffer_t *free, ring_buffer_t *used, notify_fn notify, int buffer_init)
{
    ring->free_ring = free;
    printf("ring->free_ring\n");
    ring->used_ring = used;
    printf("ring->used_ring\n");
    ring->notify = notify;
    printf("ring->notify\n");

    if (buffer_init) {
        ring->free_ring->write_idx = 0;
        ring->free_ring->read_idx = 0;
        ring->used_ring->write_idx = 0;
        ring->used_ring->read_idx = 0;
    }
}