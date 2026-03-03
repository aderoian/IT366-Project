#ifndef COMMON_GAME_H
#define COMMON_GAME_H

#include <stdint.h>

typedef struct game_s {
    uint64_t tickNumber;
    float deltaTime;
} game_t;

#endif /* COMMON_GAME_H */