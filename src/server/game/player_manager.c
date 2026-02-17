#include "common/logger.h"

#include "server/game/player_manager.h"

struct player_manager_s {
    player_t **players;
    size_t *idToIndexMap;
    size_t playerCount;
    size_t capacity;
};

player_manager_t *player_manager_create(const size_t initialCapacity) {
    player_manager_t *manager = malloc(sizeof(player_manager_t));
    if (!manager) {
        log_error("Failed to allocate memory for player manager");
        return NULL;
    }

    manager->players = gfc_allocate_array(sizeof(player_t *), initialCapacity);
    manager->idToIndexMap = gfc_allocate_array(sizeof(uint32_t), initialCapacity);
    if (!manager->players || !manager->idToIndexMap) {
        if (manager->players) free(manager->players);
        if (manager->idToIndexMap) free(manager->idToIndexMap);
        manager->capacity = 0;
        manager->playerCount = 0;
        return NULL; // Allocation failed
    }

    manager->capacity = initialCapacity;
    manager->playerCount = 0;
    return manager;
}

void player_manager_destroy(player_manager_t *manager) {
    size_t i;
    if (!manager) {
        return;
    }

    for (i = 0; i < manager->playerCount; i++) {
        player_destroy(manager->players[i]);
    }
    free(manager->players);
    manager->players = NULL;
    manager->capacity = 0;
    manager->playerCount = 0;
}

int player_manager_resize(player_manager_t *manager, const size_t newCapacity) {
    player_t **newPlayers;
    size_t *newIdToIndexMap;
    if (newCapacity <= manager->capacity) {
        return 0; // No need to resize
    }

    newPlayers = gfc_allocate_array(sizeof(player_t *), newCapacity);
    newIdToIndexMap = gfc_allocate_array(sizeof(uint32_t), newCapacity);
    if (!newPlayers || !newIdToIndexMap) {
        if (newPlayers) free(newPlayers);
        if (newIdToIndexMap) free(newIdToIndexMap);
        return -1; // Allocation failed
    }

    memcpy(newPlayers, manager->players, sizeof(player_t *) * manager->playerCount);
    memcpy(newIdToIndexMap, manager->idToIndexMap, sizeof(uint32_t) * manager->playerCount);

    free(manager->players);
    free(manager->idToIndexMap);

    manager->players = newPlayers;
    manager->idToIndexMap = newIdToIndexMap;
    manager->capacity = newCapacity;

    return 0; // Success
}

player_t *player_manager_add(player_manager_t *manager, const uint32_t id, const char *name) {
    player_t *newPlayer;
    if (!manager || id >= manager->capacity) {
        return NULL; // Invalid parameters
    }

    if (manager->playerCount >= manager->capacity) {
        if (player_manager_resize(manager, manager->capacity * 2) < 0) {
            return NULL; // Resize failed
        }
    }

    newPlayer = player_create(id, name);
    if (!newPlayer) {
        return NULL; // Player creation failed
    }

    manager->players[manager->playerCount] = newPlayer;
    manager->idToIndexMap[manager->playerCount++] = id;
    return newPlayer; // Success
}

void player_manager_remove(player_manager_t *manager, const uint32_t id) {
    size_t idx, toMoveIdx;
    if (!manager) {
        return;
    }

    idx = manager->idToIndexMap[id];
    if (idx >= manager->playerCount || manager->players[idx]->id != id) {
        return; // Player not found
    }

    player_destroy(manager->players[idx]);
    toMoveIdx = manager->playerCount - 1;
    manager->players[idx] = manager->players[toMoveIdx];
    manager->idToIndexMap[idx] = manager->idToIndexMap[toMoveIdx];
    manager->players[toMoveIdx] = NULL;
    manager->idToIndexMap[toMoveIdx] = UINT32_MAX;
    manager->playerCount--;
}

player_t *player_manager_get(player_manager_t *manager, uint32_t id) {
    size_t idx;
    if (!manager || id >= manager->capacity) {
        return NULL; // Invalid parameters
    }

    idx = manager->idToIndexMap[id];
    if (idx >= manager->playerCount || manager->players[idx]->id != id) {
        return NULL; // Player not found
    }

    return manager->players[idx];
}

const player_t **player_manager_get_all(player_manager_t *manager, size_t *outCount) {
    if (!manager || !outCount) {
        return NULL; // Invalid parameters
    }

    *outCount = manager->playerCount;
    return (const player_t **)manager->players;
}