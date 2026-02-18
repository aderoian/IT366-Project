#ifndef COMMON_GAME_H
#define COMMON_GAME_H

#include <stdint.h>

typedef struct game_s {
    uint64_t tickNumber;
    float deltaTime;
    uint8_t isLocal;
} game_t;

extern game_t g_game;

#define IS_CLIENT() (g_game.isLocal)
#define IS_SERVER() (!g_game.isLocal)

#endif /* COMMON_GAME_H */