#ifndef ATOMIC_H
#define ATOMIC_H

#include <stdint.h>
#include <stdatomic.h>

/**
 * @brief Atomic unsigned 32-bit integer type.
 */
typedef atomic_uint atomic_u32_t;

/**
 * @brief Initialize an atomic unsigned 32-bit integer.
 *
 * @param a Pointer to the atomic_u32_t to initialize.
 * @param v The initial value.
 */
static inline void atomic_u32_init(atomic_u32_t *a, uint32_t v) {
    atomic_init(a, v);
}

/**
 * @brief Load the value of an atomic unsigned 32-bit integer with relaxed memory order.
 *
 * @param a Pointer to the atomic_u32_t to load from.
 * @return The loaded value.
 */
static inline uint32_t atomic_u32_load_relaxed(const atomic_u32_t *a) {
    return atomic_load_explicit(a, memory_order_relaxed);
}

/**
 * @brief Load the value of an atomic unsigned 32-bit integer with acquire memory order.
 *
 * @param a Pointer to the atomic_u32_t to load from.
 * @return The loaded value.
 */
static inline uint32_t atomic_u32_load_acquire(const atomic_u32_t *a) {
    return atomic_load_explicit(a, memory_order_acquire);
}

/**
 * @brief Store a value into an atomic unsigned 32-bit integer with release memory order.
 *
 * @param a Pointer to the atomic_u32_t to store to.
 * @param v The value to store.
 */
static inline void atomic_u32_store_release(atomic_u32_t *a, uint32_t v) {
    atomic_store_explicit(a, v, memory_order_release);
}

#endif /* ATOMIC_H */