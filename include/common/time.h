#ifndef TIME_H
#define TIME_H

#include <stdint.h>
#include <time.h>

static inline uint64_t time_now_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t) ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}

static inline uint64_t time_now_ms(void) {
    return time_now_ns() / 1000000ULL;
}

#endif /* TIME_H */