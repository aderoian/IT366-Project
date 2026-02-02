#ifndef CONDVAR_H
#define CONDVAR_H

#include <stdint.h>

#include <pthread.h>
#include <time.h>

#include "common/thread/mutex.h"

typedef pthread_cond_t cond_t;

/**
 * @brief Initialize a condition variable.
 * @param cv Pointer to the cond_t to initialize.
 */
static inline void condvar_init(cond_t *cv) {
    pthread_cond_init(cv, NULL);
}

/**
 * @brief Destroy a condition variable.
 * @param cv Pointer to the cond_t to destroy.
 */
static inline void condvar_destroy(cond_t *cv) {
    pthread_cond_destroy(cv);
}

/**
 * @brief Wait on a condition variable.
 * @param cv Pointer to the cond_t to wait on.
 * @param mutex Pointer to the mutex_t associated with the condition variable.
 */
static inline void condvar_wait(cond_t *cv, mutex_t *mutex) {
    pthread_cond_wait(cv, mutex);
}

/**
 * @brief Wait on a condition variable with a timeout in milliseconds.
 * @param cv Pointer to the cond_t to wait on.
 * @param mutex Pointer to the mutex_t associated with the condition variable.
 * @param timeoutMs Timeout in milliseconds.
 * @return Non-zero if the wait was successful, zero if it timed out.
 */
static inline int condvar_timedWaitMs(cond_t *cv, mutex_t *mutex, uint32_t timeoutMs) {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);

    ts.tv_sec += timeoutMs / 1000;
    ts.tv_nsec += (timeoutMs % 1000) * 1000000;

    if (ts.tv_nsec >= 1000000000) {
        ts.tv_sec++;
        ts.tv_nsec -= 1000000000;
    }

    return pthread_cond_timedwait(cv, mutex, &ts) == 0;
}

/**
 * @brief Signal a condition variable to wake one waiting thread.
 * @param cv Pointer to the cond_t to signal.
 */
static inline void condvar_signal(cond_t *cv) {
    pthread_cond_signal(cv);
}

/**
 * @brief Broadcast a condition variable to wake all waiting threads.
 * @param cv Pointer to the cond_t to broadcast.
 */
static inline void condvar_broadcast(cond_t *cv) {
    pthread_cond_broadcast(cv);
}

#endif /* CONDVAR_H */