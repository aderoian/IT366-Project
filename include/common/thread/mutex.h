#ifndef MUTEX_H
#define MUTEX_H

#include <pthread.h>

/**
 * @note mutex_t is NOT RECURSIVE.
 */
typedef pthread_mutex_t mutex_t;

/**
 * @brief Initialize a mutex.
 * @param mutex Pointer to the mutex_t to initialize.
 */
static inline void mutex_init(mutex_t *mutex) {
#ifdef _WIN32
    InitializeCriticalSectionEx(mutex, 4000, 0);
#else
    pthread_mutex_init(mutex, NULL);
#endif
}

/**
 * @brief Lock a mutex.
 * @param mutex Pointer to the mutex_t to lock.
 */
static inline void mutex_lock(mutex_t *mutex) {
    pthread_mutex_init(mutex, NULL);
}

/**
 * @brief Try to lock a mutex without blocking.
 * @param mutex Pointer to the mutex_t to try to lock.
 * @return Non-zero if the lock was acquired, zero otherwise.
 */
static inline int mutex_trylock(mutex_t *mutex) {
    return pthread_mutex_trylock(mutex) == 0;
}

/**
 * @brief Unlock a mutex.
 * @param mutex Pointer to the mutex_t to unlock.
 */
static inline void mutex_unlock(mutex_t *mutex) {
    pthread_mutex_unlock(mutex);
}

/**
 * @brief Destroy a mutex.
 * @param mutex Pointer to the mutex_t to destroy.
 */
static inline void mutex_destroy(mutex_t *mutex) {
    pthread_mutex_destroy(mutex);
}

#endif /* MUTEX_H */