#ifndef COMMON_GAME_H
#define COMMON_GAME_H

#include <stdint.h>

#define GAME_ROLE_NONE 0
#define GAME_ROLE_SERVER 1
#define GAME_ROLE_CLIENT 2

typedef struct game_s {
    uint64_t tickNumber;
    float deltaTime;
    uint8_t role; // 0 = none, 1 = server, 2 = client
    uint8_t isLocal; // 1 if this instance is running in the same process as the server (for client), 0 otherwise

    struct item_def_manager_s *itemDefManager;
    struct world_s *world; // Pointer to the game world (server and client will have their own instances)
} game_t;

extern __thread game_t g_game;

#endif /* COMMON_GAME_H */