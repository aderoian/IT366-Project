#ifndef COMMON_GAME_H
#define COMMON_GAME_H

#include <stdint.h>

#include "gfc_vector.h"
#include "wave.h"

#define GAME_ROLE_NONE 0
#define GAME_ROLE_SERVER 1
#define GAME_ROLE_CLIENT 2

#define HALF_CYCLE_TIME 3.0f // Time in second for day and night cycle

typedef enum game_phase_e {
    GAME_PHASE_EXPLORING = 0,
    GAME_PHASE_BUILDING,
    GAME_PHASE_WAVE,
    GAME_PHASE_PAUSED
} game_phase_t;

typedef struct game_state_t {
    game_phase_t phase;
    uint64_t waveNumber;
    wave_t currentWave;
    float cycleTime;
    GFC_Vector2D stashPosition;

    char world[64];
} game_state_t;

typedef struct game_s {
    uint64_t tickNumber;
    float deltaTime;
    uint8_t role; // 0 = none, 1 = server, 2 = client
    uint8_t isLocal; // 1 if this instance is running in the same process as the server (for client), 0 otherwise

    struct def_manager_s *defManager;
    struct item_def_manager_s *itemDefManager;
    struct tile_manager_s *tileManager;
    struct entity_manager_s *entityManager;
    struct tower_manager_s *towerManager;
    struct enemy_def_manager_s *enemyManager;
    struct world_s *world; // Pointer to the game world (server and client will have their own instances)

    game_state_t state;
} game_t;

extern __thread game_t g_game;

#endif /* COMMON_GAME_H */