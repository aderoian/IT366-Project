#include <stdlib.h>

#include "common/buffer/ring.h"

int buf_spsc_ring_init(buf_spsc_ring_t* ring, const uint32_t capacity, const uint32_t item_size) {
    if (!ring || capacity == 0 || item_size == 0) {
        return 0;
    }

    ring->buffer = malloc(capacity * item_size);
    if (!ring->buffer) {
        return 0;
    }

    ring->capacity = capacity;
    ring->item_size = item_size;
    atomic_u32_init(&ring->writeIdx, 0);
    atomic_u32_init(&ring->readIdx, 0);

    return 1;
}

void buf_spsc_ring_destroy(buf_spsc_ring_t* ring) {
    if (ring && ring->buffer) {
        free(ring->buffer);
        ring->buffer = NULL;
    }
}

int buf_spsc_ring_push(buf_spsc_ring_t* ring, const void* item) {
    uint32_t writeIdx, readIdx, nextWriteIdx;
    void *dest;
    if (!ring || !item) {
        return 0;
    }

    writeIdx = atomic_u32_load_relaxed(&ring->writeIdx);
    readIdx = atomic_u32_load_acquire(&ring->readIdx);
    nextWriteIdx = (writeIdx + 1) % ring->capacity;

    if (nextWriteIdx == readIdx) {
        return 0; // Ring is full
    }

    dest = (uint8_t*)ring->buffer + (writeIdx * ring->item_size);
    memcpy(dest, item, ring->item_size);
    atomic_u32_store_release(&ring->writeIdx, nextWriteIdx);

    return 1;
}

int buf_spsc_ring_pop(buf_spsc_ring_t* ring, void* item) {
    uint32_t writeIdx, readIdx;
    void *src;
    if (!ring || !item) {
        return 0;
    }

    writeIdx = atomic_u32_load_acquire(&ring->writeIdx);
    readIdx = atomic_u32_load_relaxed(&ring->readIdx);

    if (readIdx == writeIdx) {
        return 0; // Ring is empty
    }

    src = (uint8_t*)ring->buffer + (readIdx * ring->item_size);
    memcpy(item, src, ring->item_size);
    atomic_u32_store_release(&ring->readIdx, (readIdx + 1) % ring->capacity);

    return 1;
}

int buf_spsc_ring_is_empty(const buf_spsc_ring_t* ring) {
    uint32_t writeIdx, readIdx;
    if (!ring) {
        return 1; // Consider null ring as empty
    }

    writeIdx = atomic_u32_load_acquire(&ring->writeIdx);
    readIdx = atomic_u32_load_relaxed(&ring->readIdx);

    return (readIdx == writeIdx);
}

int buf_spsc_ring_is_full(const buf_spsc_ring_t* ring) {
    uint32_t writeIdx, readIdx, nextWriteIdx;
    if (!ring) {
        return 0; // Consider null ring as not full
    }

    writeIdx = atomic_u32_load_relaxed(&ring->writeIdx);
    readIdx = atomic_u32_load_acquire(&ring->readIdx);
    nextWriteIdx = (writeIdx + 1) % ring->capacity;

    return (nextWriteIdx == readIdx);
}