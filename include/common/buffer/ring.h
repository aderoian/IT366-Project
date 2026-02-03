#ifndef RING_H
#define RING_H

#include "common/thread/atomic.h"

/**
 * @brief Single-producer single-consumer ring buffer structure.
 */
typedef struct buf_spsc_ring {
    void* buffer;
    uint32_t capacity;
    uint32_t item_size;
    atomic_u32_t writeIdx;
    atomic_u32_t readIdx;
} buf_spsc_ring_t;

/**
 * @brief Initialize a single-producer single-consumer ring buffer.
 *
 * @param ring Pointer to the buf_spsc_ring_t to initialize.
 * @param capacity The capacity of the ring buffer (number of items).
 * @param item_size The size of each item in the buffer.
 * @return 1 on success, 0 on failure.
 */
int buf_spsc_ring_init(buf_spsc_ring_t* ring, uint32_t capacity, uint32_t item_size);

/**
 * @brief Destroy a single-producer single-consumer ring buffer.
 *
 * @param ring Pointer to the buf_spsc_ring_t to destroy.
 */
void buf_spsc_ring_destroy(buf_spsc_ring_t* ring);

/**
 * @brief Push an item into the ring buffer.
 *
 * @param ring Pointer to the buf_spsc_ring_t.
 * @param item Pointer to the item to push.
 * @return 1 on success, 0 if the buffer is full.
 */
int buf_spsc_ring_push(buf_spsc_ring_t* ring, const void* item);

/**
 * @brief Pop an item from the ring buffer.
 *
 * @param ring Pointer to the buf_spsc_ring_t.
 * @param item Pointer to the buffer where the popped item will be stored.
 * @return 1 on success, 0 if the buffer is empty.
 */
int buf_spsc_ring_pop(buf_spsc_ring_t* ring, void* item);

/**
 * @brief Check if the ring buffer is empty.
 *
 * @param ring Pointer to the buf_spsc_ring_t.
 * @return 1 if the buffer is empty, 0 otherwise.
 */
int buf_spsc_ring_is_empty(const buf_spsc_ring_t* ring);

/**
 * @brief Check if the ring buffer is full.
 *
 * @param ring Pointer to the buf_spsc_ring_t.
 * @return 1 if the buffer is full, 0 otherwise.
 */
int buf_spsc_ring_is_full(const buf_spsc_ring_t* ring);

#endif /* RING_H */