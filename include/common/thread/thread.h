#ifndef THREAD_H
#define THREAD_H

#include <stdlib.h>

#include <pthread.h>
#include <unistd.h>
#include <time.h>

typedef pthread_t thread_t;

/** Function signature for thread entry functions */
typedef void *(*thread_func_t)(void *userData);

/**
 * @brief Create a new thread.
 * @param outThread Pointer to thread_t to receive the created thread handle.
 * @param fn Function pointer to the thread entry function.
 * @param userData Pointer to user data to pass to the thread function.
 * @return Zero on success, negative value on failure.
 */
static inline int thread_create(thread_t *outThread, const thread_func_t fn, void *userData) {
    if (pthread_create(outThread, NULL, fn, userData) == 0) {
        return 0;
    }

    return -1;
}

/**
 * @brief Join a thread, blocking until it completes.
 * @param thread Pointer to the thread_t to join.
 * @return Zero on success, negative value on failure.
 */
static inline void *thread_join(const thread_t *thread) {
    void *ret;
    if (pthread_join(*thread, &ret) != 0) {
        return NULL;
    }
    return ret;
}

/**
 * @brief Detach a thread, allowing it to run independently.
 * @param thread Pointer to the thread_t to detach.
 */
static inline void thread_detach(const thread_t *thread) {
    pthread_detach(*thread);
}

/**
 * @brief Yield execution of the current thread.
 */
static inline void thread_yield(void) {
    sched_yield();
}

/**
 * @brief Sleep the current thread for a specified number of milliseconds.
 * @param ms Number of milliseconds to sleep.
 */
static inline void thread_sleepMs(unsigned ms) {
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000;
    nanosleep(&ts, NULL);
}

#endif /* THREAD_H */