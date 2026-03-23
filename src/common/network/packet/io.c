#include "common/network/packet/io.h"

#include <stdlib.h>
#include <string.h>

#include "common/game/game.h"

void write_uint8(buffer_t buffer, buffer_offset_t *offset, const uint8_t value) {
    buffer[(*offset)++] = value;
}

void write_uint16(buffer_t buffer, buffer_offset_t *offset, const uint16_t value) {
    for (int i = 1; i >= 0; i--) {
        buffer[(*offset)++] = (value >> (i * 8)) & 0xFF;
    }
}

void write_uint32(buffer_t buffer, buffer_offset_t *offset, uint32_t value) {
    for (int i = 3; i >= 0; i--) {
        buffer[(*offset)++] = (value >> (i * 8)) & 0xFF;
    }
}

void write_uint64(buffer_t buffer, buffer_offset_t *offset, uint64_t value) {
    for (int i = 7; i >= 0; i--) {
        buffer[(*offset)++] = (value >> (i * 8)) & 0xFF;
    }
}

void write_int8(buffer_t buffer, buffer_offset_t *offset, int64_t value) {
    buffer[(*offset)++] = (uint8_t)(value & 0xFF);
}

void write_int16(buffer_t buffer, buffer_offset_t *offset, int16_t value) {
    for (int i = 1; i >= 0; i--) {
        buffer[(*offset)++] = (uint8_t)((value >> (i * 8)) & 0xFF);
    }
}

void write_int32(buffer_t buffer, buffer_offset_t *offset, int32_t value) {
    for (int i = 3; i >= 0; i--) {
        buffer[(*offset)++] = (uint8_t)((value >> (i * 8)) & 0xFF);
    }
}

void write_int64(buffer_t buffer, buffer_offset_t *offset, int64_t value) {
    for (int i = 7; i >= 0; i--) {
        buffer[(*offset)++] = (uint8_t)((value >> (i * 8)) & 0xFF);
    }
}

void write_float(buffer_t buffer, buffer_offset_t *offset, float value) {
    union {
        float f;
        uint32_t u;
    } v;

    v.f = value;
    write_uint32(buffer, offset, v.u);
}

void write_double(buffer_t buffer, buffer_offset_t *offset, double value) {
    union {
        double d;
        uint64_t u;
    } v;

    v.d = value;
    write_uint64(buffer, offset, v.u);
}

void write_string(buffer_t buffer, buffer_offset_t *offset, const char *str, uint16_t maxLen) {
    size_t len = strnlen(str, maxLen);
    write_uint16(buffer, offset, (uint16_t)len);
    memcpy(buffer + *offset, str, len);
    *offset += len;
}

uint8_t read_uint8(buffer_t buffer, buffer_offset_t *offset) {
    return buffer[(*offset)++];
}

uint16_t read_uint16(buffer_t buffer, buffer_offset_t *offset) {
    uint16_t value = 0;
    for (int i = 1; i >= 0; i--) {
        value |= ((uint16_t)buffer[(*offset)++] << (i * 8));
    }
    return value;
}

uint32_t read_uint32(buffer_t buffer, buffer_offset_t *offset) {
    uint32_t value = 0;
    for (int i = 3; i >= 0; i--) {
        value |= ((uint32_t)buffer[(*offset)++] << (i * 8));
    }
    return value;
}

uint64_t read_uint64(const buffer_t buffer, buffer_offset_t *offset) {
    uint64_t value = 0;
    for (int i = 7; i >= 0; i--) {
        value |= ((uint64_t)buffer[(*offset)++] << (i * 8));
    }
    return value;
}

int8_t read_int8(buffer_t buffer, buffer_offset_t *offset) {
    return (int8_t)buffer[(*offset)++];
}

int16_t read_int16(buffer_t buffer, buffer_offset_t *offset) {
    int16_t value = 0;
    for (int i = 1; i >= 0; i--) {
        value |= ((uint16_t)buffer[(*offset)++] << (i * 8));
    }
    return value;
}

int32_t read_int32(buffer_t buffer, buffer_offset_t *offset) {
    int32_t value = 0;
    for (int i = 3; i >= 0; i--) {
        value |= ((uint32_t)buffer[(*offset)++] << (i * 8));
    }
    return value;
}

int64_t read_int64(buffer_t buffer, buffer_offset_t *offset) {
    int64_t value = 0;
    for (int i = 7; i >= 0; i--) {
        value |= ((uint64_t)buffer[(*offset)++] << (i * 8));
    }
    return value;
}

float read_float(buffer_t buffer, buffer_offset_t *offset) {
    union {
        float f;
        uint32_t u;
    } v;

    v.u = read_uint32(buffer, offset);
    return v.f;
}

double read_double(buffer_t buffer, buffer_offset_t *offset) {
    union {
        double d;
        uint64_t u;
    } v;

    v.u = read_uint64(buffer, offset);
    return v.d;
}

char *read_string(buffer_t buffer, buffer_offset_t *offset, uint16_t *outCount, const uint16_t maxLen) {
    uint16_t len = read_uint16(buffer, offset);
    if (len > maxLen) {
        len = maxLen - 1; // Prevent overflow
    }

    char *str = malloc(len + 1);
    if (!str) {
        if (outCount) {
            *outCount = 0;
        }
        return NULL; // Allocation failed
    }

    memcpy(str, buffer + *offset, len);
    str[len] = '\0'; // Null-terminate the string
    *offset += len;

    if (outCount) {
        *outCount = len;
    }
    return str;
}

void write_game_state(buffer_t buffer, buffer_offset_t *offset, const game_state_t *state) {
    write_uint8(buffer, offset, state->phase);
    write_float(buffer, offset, state->cycleTime);
    write_uint64(buffer, offset, state->waveNumber);
}

void write_player_input_command(buffer_t buffer, buffer_offset_t *offset, const player_input_command_t *cmd) {
    write_uint64(buffer, offset, cmd->tickNumber);
    write_int8(buffer, offset, cmd->axisX);
    write_int8(buffer, offset, cmd->axisY);
    write_int8(buffer, offset, cmd->attack);
}

void write_item(buffer_t buffer, buffer_offset_t *offset, const item_t *item) {
    write_uint32(buffer, offset, item->def->index);
    write_uint32(buffer, offset, item->quantity);
}

void write_item_array(buffer_t buffer, buffer_offset_t *offset, const item_t *items, size_t count) {
    write_uint16(buffer, offset, (uint16_t)count);
    for (size_t i = 0; i < count; i++) {
        write_item(buffer, offset, &items[i]);
    }
}

void write_inventory_transaction(buffer_t buffer, buffer_offset_t *offset, const inventory_transaction_t *transaction) {
    write_uint8(buffer, offset, transaction->isAddition);
    write_item_array(buffer, offset, transaction->items, transaction->numItems);
}

void read_game_state(buffer_t buffer, buffer_offset_t *offset, game_state_t *state) {
    state->phase = read_uint8(buffer, offset);
    state->cycleTime = read_float(buffer, offset);
    state->waveNumber = read_uint64(buffer, offset);
}

void read_player_input_command(buffer_t buffer, buffer_offset_t *offset, player_input_command_t *cmd) {
    cmd->tickNumber = read_uint64(buffer, offset);
    cmd->axisX = read_int8(buffer, offset);
    cmd->axisY = read_int8(buffer, offset);
    cmd->attack = read_int8(buffer, offset);
}

void read_item(buffer_t buffer, buffer_offset_t *offset, item_t *item) {
    item->def = item_def_get_by_index(g_game.itemDefManager, read_uint32(buffer, offset));
    item->quantity = read_uint32(buffer, offset);;
}

item_t *read_item_array(buffer_t buffer, buffer_offset_t *offset, uint16_t *outCount, uint16_t maxCount) {
    item_t *items;
    uint16_t numItems = read_uint16(buffer, offset);
    if (numItems > maxCount) {
        numItems = maxCount; // Prevent overflow
    }

    items = malloc(sizeof(item_t) * numItems);
    if (!items) {
        if (outCount) {
            *outCount = 0;
        }
        return NULL; // Allocation failed
    }

    for (uint16_t i = 0; i < numItems; i++) {
        read_item(buffer, offset, &items[i]);
    }

    if (outCount) {
        *outCount = numItems;
    }
    return items;
}

void read_inventory_transaction(buffer_t buffer, buffer_offset_t *offset, inventory_transaction_t *transaction) {
    uint16_t numItems;
    transaction->isAddition = read_uint8(buffer, offset);
    transaction->items = read_item_array(buffer, offset, &numItems, MAX_ITEMS_LENGTH);
    transaction->numItems = numItems;
    transaction->capacity = transaction->numItems; // Set capacity to match the number of items read
}

void write_c2s_player_join_request(buffer_t buf, buffer_offset_t *off,
                                   const c2s_player_join_request_packet_t *pkt) {
    write_uint8(buf, off, pkt->packetID);
    write_uint64(buf, off, pkt->length);
    write_string(buf, off, pkt->name, MAX_STRING_LENGTH);
}

void write_s2c_player_join_response(buffer_t buf, buffer_offset_t *off,
                                        const s2c_player_join_response_packet_t *pkt) {
    write_uint8(buf, off, pkt->packetID);
    write_uint64(buf, off, pkt->length);
    write_uint8(buf, off, pkt->success);
    write_uint32(buf, off, pkt->playerID);
    write_int64(buf, off, pkt->entityID);
    write_int32(buf, off, pkt->worldL);
    write_int32(buf, off, pkt->worldW);
    write_float(buf, off, pkt->spawnX);
    write_float(buf, off, pkt->spawnY);
    write_game_state(buf, off, &pkt->initialGameState);
}

void write_c2s_player_input_snapshot(buffer_t buf, buffer_offset_t *off,
                                         const c2s_player_input_snapshot_packet_t *pkt) {
    write_uint8(buf, off, pkt->packetID);
    write_uint64(buf, off, pkt->length);
    write_player_input_command(buf, off, &pkt->inputCommand);
}

void write_s2c_player_state_snapshot(buffer_t buf, buffer_offset_t *off,
                                         const s2c_player_state_snapshot_packet_t *pkt) {
    write_uint8(buf, off, pkt->packetID);
    write_uint64(buf, off, pkt->length);
    write_uint64(buf, off, pkt->tickNumber);
    write_float(buf, off, pkt->xPos);
    write_float(buf, off, pkt->yPos);
}

void write_s2c_player_create(buffer_t buf, buffer_offset_t *off, const s2c_player_create_packet_t *pkt) {
    write_uint8(buf, off, pkt->packetID);
    write_uint64(buf, off, pkt->length);
    write_uint32(buf, off, pkt->playerID);
    write_float(buf, off, pkt->spawnX);
    write_float(buf, off, pkt->spawnY);
}

void write_c2s_tower_request(buffer_t buf, buffer_offset_t *off,
                                       const c2s_tower_request_packet_t *pkt) {
    write_uint8(buf, off, pkt->packetID);
    write_uint64(buf, off, pkt->length);
    write_uint8(buf, off, pkt->requestID);
    if (pkt->requestID == TOWER_REQUEST_BUILD) {
        write_float(buf, off, pkt->requestData.buildData.xPos);
        write_float(buf, off, pkt->requestData.buildData.yPos);
        write_uint32(buf, off, pkt->requestData.buildData.towerDefIndex);
    } else if (pkt->requestID == TOWER_REQUEST_UPGRADE) {
        write_uint32(buf, off, pkt->requestData.upgradeData.towerID);
    } else if (pkt->requestID == TOWER_REQUEST_SELL) {
        write_uint32(buf, off, pkt->requestData.sellData.towerID);
    }
}

void write_s2c_tower_snapshot(buffer_t buf, buffer_offset_t *off, const s2c_tower_snapshot_packet_t *pkt) {
    write_uint8(buf, off, pkt->packetID);
    write_uint64(buf, off, pkt->length);
    write_uint32(buf, off, pkt->towerID);
    write_uint8(buf, off, pkt->snapshotID);
    if (pkt->snapshotID == TOWER_SNAPSHOT_CREATE) {
        write_float(buf, off, pkt->snapshotData.createData.xPos);
        write_float(buf, off, pkt->snapshotData.createData.yPos);
        write_uint32(buf, off, pkt->snapshotData.createData.towerDefIndex);
        write_uint32(buf, off, pkt->snapshotData.createData.towerID);
        write_int64(buf, off, pkt->snapshotData.createData.entityID);
    } else if (pkt->snapshotID == TOWER_SNAPSHOT_SHOOT) {
        write_float(buf, off, pkt->snapshotData.shootData.xDir);
        write_float(buf, off, pkt->snapshotData.shootData.yDir);
    } else if (pkt->snapshotID == TOWER_SNAPSHOT_CHANGE) {
        write_int32(buf, off, pkt->snapshotData.changeData.level);
    } else if (pkt->snapshotID == TOWER_SNAPSHOT_DESTROY) {
        // No additional data for destroy snapshot
    }
}

void write_s2c_inventory_update(buffer_t buf, buffer_offset_t *off, const s2c_inventory_update_packet_t *pkt) {
    write_uint8(buf, off, pkt->packetID);
    write_uint64(buf, off, pkt->length);
    write_uint32(buf, off, pkt->playerID);
    write_inventory_transaction(buf, off, &pkt->transaction);
}

void write_s2c_game_state_snapshot(buffer_t buf, buffer_offset_t *off, const s2c_game_state_snapshot_packet_t *pkt) {
    write_uint8(buf, off, pkt->packetID);
    write_uint64(buf, off, pkt->length);
    write_game_state(buf, off, &pkt->gameState);
}

void write_s2c_enemy_snapshot(buffer_t buf, buffer_offset_t *off, const s2c_enemy_snapshot_packet_t *pkt) {
    write_uint8(buf, off, pkt->packetID);
    write_uint64(buf, off, pkt->length);
    write_int64(buf, off, pkt->enemyID);
    write_uint32(buf, off, pkt->eventID);

    if (pkt->eventID == ENEMY_EVENT_SPAWN) {
        write_uint32(buf, off, pkt->eventData.spawnData.enemyDefIndex);
        write_float(buf, off, pkt->eventData.spawnData.xPos);
        write_float(buf, off, pkt->eventData.spawnData.yPos);
        write_float(buf, off, pkt->eventData.spawnData.rotation);
    } else if (pkt->eventID == ENEMY_EVENT_DESPAWN) {
        // No additional data for despawn
    } else if (pkt->eventID == ENEMY_EVENT_MOVE) {
        write_float(buf, off, pkt->eventData.moveData.xPos);
        write_float(buf, off, pkt->eventData.moveData.yPos);
        write_float(buf, off, pkt->eventData.moveData.rotation);
    } else if (pkt->eventID == ENEMY_EVENT_ATTACK) {
        // No additional data for attack
    }
}

void read_c2s_player_join_request(buffer_t buf, buffer_offset_t *off, c2s_player_join_request_packet_t *pkt) {
    pkt->packetID = read_uint8(buf, off);
    pkt->length = read_uint64(buf, off);
    pkt->name = read_string(buf, off, NULL, MAX_STRING_LENGTH);
}

void read_s2c_player_join_response(buffer_t buf, buffer_offset_t *off, s2c_player_join_response_packet_t *pkt) {
    pkt->packetID = read_uint8(buf, off);
    pkt->length = read_uint64(buf, off);
    pkt->success = read_uint8(buf, off);
    pkt->playerID = read_uint32(buf, off);
    pkt->entityID = read_int64(buf, off);
    pkt->worldL = read_int32(buf, off);
    pkt->worldW = read_int32(buf, off);
    pkt->spawnX = read_float(buf, off);
    pkt->spawnY = read_float(buf, off);
    read_game_state(buf, off, &pkt->initialGameState);
}

void read_c2s_player_input_snapshot(buffer_t buf, buffer_offset_t *off,
                                           c2s_player_input_snapshot_packet_t *pkt) {
    pkt->packetID = read_uint8(buf, off);
    pkt->length = read_uint64(buf, off);
    read_player_input_command(buf, off, &pkt->inputCommand);
}

void read_s2c_player_state_snapshot(buffer_t buf, buffer_offset_t *off,
                                           s2c_player_state_snapshot_packet_t *pkt) {
    pkt->packetID = read_uint8(buf, off);
    pkt->length = read_uint64(buf, off);
    pkt->tickNumber = read_uint64(buf, off);
    pkt->xPos = read_float(buf, off);
    pkt->yPos = read_float(buf, off);
}

void read_s2c_player_create(buffer_t buf, buffer_offset_t *off, s2c_player_create_packet_t *pkt) {
    pkt->packetID = read_uint8(buf, off);
    pkt->length = read_uint64(buf, off);
    pkt->playerID = read_uint32(buf, off);
    pkt->spawnX = read_float(buf, off);
    pkt->spawnY = read_float(buf, off);
}

void read_c2s_tower_request(buffer_t buf, buffer_offset_t *off, c2s_tower_request_packet_t *pkt) {
    pkt->packetID = read_uint8(buf, off);
    pkt->length = read_uint64(buf, off);
    pkt->requestID = read_uint8(buf, off);
    if (pkt->requestID == TOWER_REQUEST_BUILD) {
        pkt->requestData.buildData.xPos = read_float(buf, off);
        pkt->requestData.buildData.yPos = read_float(buf, off);
        pkt->requestData.buildData.towerDefIndex = read_uint32(buf, off);
    } else if (pkt->requestID == TOWER_REQUEST_UPGRADE) {
        pkt->requestData.upgradeData.towerID = read_uint32(buf, off);
    } else if (pkt->requestID == TOWER_REQUEST_SELL) {
        pkt->requestData.sellData.towerID = read_uint32(buf, off);
    }
}

void read_s2c_tower_snapshot(buffer_t buf, buffer_offset_t *off, s2c_tower_snapshot_packet_t *pkt) {
    pkt->packetID = read_uint8(buf, off);
    pkt->length = read_uint64(buf, off);
    pkt->towerID = read_uint32(buf, off);
    pkt->snapshotID = read_uint8(buf, off);

    if (pkt->snapshotID == TOWER_SNAPSHOT_CREATE) {
        pkt->snapshotData.createData.xPos = read_float(buf, off);
        pkt->snapshotData.createData.yPos = read_float(buf, off);
        pkt->snapshotData.createData.towerDefIndex = read_uint32(buf, off);
        pkt->snapshotData.createData.towerID = read_uint32(buf, off);
        pkt->snapshotData.createData.entityID = read_int64(buf, off);
    } else if (pkt->snapshotID == TOWER_SNAPSHOT_SHOOT) {
        pkt->snapshotData.shootData.xDir = read_float(buf, off);
        pkt->snapshotData.shootData.yDir = read_float(buf, off);
    } else if (pkt->snapshotID == TOWER_SNAPSHOT_CHANGE) {
        pkt->snapshotData.changeData.level = read_int32(buf, off);
    } else if (pkt->snapshotID == TOWER_SNAPSHOT_DESTROY) {
        // No additional data for destroy snapshot
    }
}

void read_s2c_inventory_update(buffer_t buf, buffer_offset_t *off, s2c_inventory_update_packet_t *pkt) {
    pkt->packetID = read_uint8(buf, off);
    pkt->length = read_uint64(buf, off);
    pkt->playerID = read_uint32(buf, off);
    read_inventory_transaction(buf, off, &pkt->transaction);
}

void read_s2c_game_state_snapshot(buffer_t buf, buffer_offset_t *off, s2c_game_state_snapshot_packet_t *pkt) {
    pkt->packetID = read_uint8(buf, off);
    pkt->length = read_uint64(buf, off);
    read_game_state(buf, off, &pkt->gameState);
}

void read_s2c_enemy_snapshot(buffer_t buf, buffer_offset_t *off, s2c_enemy_snapshot_packet_t *pkt) {
    pkt->packetID = read_uint8(buf, off);
    pkt->length = read_uint64(buf, off);
    pkt->enemyID = read_int64(buf, off);
    pkt->eventID = read_uint32(buf, off);

    if (pkt->eventID == ENEMY_EVENT_SPAWN) {
        pkt->eventData.spawnData.enemyDefIndex = read_uint32(buf, off);
        pkt->eventData.spawnData.xPos = read_float(buf, off);
        pkt->eventData.spawnData.yPos = read_float(buf, off);
        pkt->eventData.spawnData.rotation = read_float(buf, off);
    } else if (pkt->eventID == ENEMY_EVENT_DESPAWN) {
        // No additional data for despawn
    } else if (pkt->eventID == ENEMY_EVENT_MOVE) {
        pkt->eventData.moveData.xPos = read_float(buf, off);
        pkt->eventData.moveData.yPos = read_float(buf, off);
        pkt->eventData.moveData.rotation = read_float(buf, off);
    } else if (pkt->eventID == ENEMY_EVENT_ATTACK) {
        // No additional data for attack
    }
}

void create_c2s_player_join_request(c2s_player_join_request_packet_t *pkt, char *name) {
    pkt->packetID = PACKET_C2S_PLAYER_JOIN_REQUEST;
    pkt->length = sizeof(uint16_t) + strnlen(name, MAX_STRING_LENGTH);
    pkt->name = name;
}

void create_s2c_player_join_response(s2c_player_join_response_packet_t *pkt, uint8_t success, uint32_t playerID, int64_t entityID,
                                     int32_t worldL, int32_t worldW, float spawnX, float spawnY, game_state_t *initialGameState) {
    pkt->packetID = PACKET_S2C_PLAYER_JOIN_RESPONSE;
    pkt->length = sizeof(success) + sizeof(playerID) + sizeof(entityID) + sizeof(worldL) + sizeof(worldW) + sizeof(spawnX) + sizeof(spawnY) + (sizeof(uint8_t) + sizeof(float) + sizeof(uint64_t));
    pkt->success = success;
    pkt->playerID = playerID;
    pkt->entityID = entityID;
    pkt->worldL = worldL;
    pkt->worldW = worldW;
    pkt->spawnX = spawnX;
    pkt->spawnY = spawnY;
    pkt->initialGameState = *initialGameState;
}

void create_c2s_player_input_snapshot(c2s_player_input_snapshot_packet_t *pkt, player_input_command_t *inputCommand) {
    pkt->packetID = PACKET_C2S_PLAYER_INPUT_SNAPSHOT;
    pkt->length = sizeof(inputCommand->tickNumber) + sizeof(inputCommand->axisX) + sizeof(inputCommand->axisY) + sizeof(inputCommand->attack);
    pkt->inputCommand = *inputCommand;
}

void create_s2c_player_state_snapshot(s2c_player_state_snapshot_packet_t *pkt, uint64_t tickNumber,
                                      float xPos, float yPos) {
    pkt->packetID = PACKET_S2C_PLAYER_STATE_SNAPSHOT;
    pkt->length = sizeof(tickNumber) + sizeof(xPos) + sizeof(yPos);
    pkt->tickNumber = tickNumber;
    pkt->xPos = xPos;
    pkt->yPos = yPos;
}

void create_s2c_player_create(s2c_player_create_packet_t *pkt, uint32_t playerID, float spawnX,
                              float spawnY) {
    pkt->packetID = PACKET_S2C_PLAYER_CREATE;
    pkt->length = sizeof(playerID) + sizeof(spawnX) + sizeof(spawnY);
    pkt->playerID = playerID;
    pkt->spawnX = spawnX;
    pkt->spawnY = spawnY;
}

void create_c2s_tower_request(c2s_tower_request_packet_t *pkt, tower_request_id_t id, tower_request_data_t *data) {
    pkt->packetID = PACKET_C2S_TOWER_REQUEST;
    pkt->length = sizeof(uint8_t);
    if (id == TOWER_REQUEST_BUILD) {
        pkt->length += sizeof(data->buildData.xPos) + sizeof(data->buildData.yPos) + sizeof(data->buildData.towerDefIndex);
    } else if (id == TOWER_REQUEST_UPGRADE) {
        pkt->length += sizeof(data->upgradeData.towerID);
    } else if (id == TOWER_REQUEST_SELL) {
        pkt->length += sizeof(data->sellData.towerID);
    }

    pkt->requestID = id;
    pkt->requestData = *data;
}

void create_s2c_tower_snapshot(s2c_tower_snapshot_packet_t *pkt, uint32_t towerID, tower_snapshot_id_t snapshotID, tower_snapshot_data_t *eventData) {
    pkt->packetID = PACKET_S2C_TOWER_SNAPSHOT;
    pkt->length = sizeof(towerID) + sizeof(uint8_t);

    if (snapshotID == TOWER_SNAPSHOT_CREATE) {
        pkt->length += sizeof(eventData->createData.xPos) + sizeof(eventData->createData.yPos) + sizeof(eventData->createData.towerDefIndex) + sizeof(eventData->createData.towerID) + sizeof(eventData->createData.entityID);
    } else if (snapshotID == TOWER_SNAPSHOT_SHOOT) {
        pkt->length += sizeof(eventData->shootData.xDir) + sizeof(eventData->shootData.yDir);
    } else if (snapshotID == TOWER_SNAPSHOT_CHANGE) {
        pkt->length += sizeof(eventData->changeData.level);
    } else if (snapshotID == TOWER_SNAPSHOT_DESTROY) {
        // No additional data for destroy snapshot
    }

    pkt->towerID = towerID;
    pkt->snapshotID = snapshotID;
    pkt->snapshotData = *eventData;
}

void create_s2c_inventory_update(s2c_inventory_update_packet_t *pkt, uint32_t playerID,
                                 inventory_transaction_t *transaction) {
    pkt->packetID = PACKET_S2C_INVENTORY_UPDATE;
    pkt->length = sizeof(playerID) + sizeof(transaction->isAddition) + sizeof(uint16_t) + ((sizeof(uint32_t) + sizeof(uint32_t)) * transaction->numItems);
    pkt->playerID = playerID;
    pkt->transaction = *transaction;
}

void create_s2c_game_state_snapshot(s2c_game_state_snapshot_packet_t *pkt, game_state_t *state) {
    pkt->packetID = PACKET_S2C_GAME_STATE_SNAPSHOT;
    pkt->length = sizeof(uint8_t) + sizeof(state->cycleTime) + sizeof(state->waveNumber);
    pkt->gameState = *state;
}

void create_s2c_enemy_snapshot(s2c_enemy_snapshot_packet_t *pkt, int64_t enemyID, uint32_t eventID, enemy_snapshot_data_t *eventData) {
    pkt->packetID = PACKET_S2C_ENEMY_SNAPSHOT;
    pkt->length = sizeof(enemyID) + sizeof(eventID);

    if (eventID == ENEMY_EVENT_SPAWN) {
        pkt->length += sizeof(uint32_t) + sizeof(float) + sizeof(float) + sizeof(float);
    } else if (eventID == ENEMY_EVENT_MOVE) {
        pkt->length += sizeof(float) + sizeof(float) + sizeof(float);
    }

    pkt->enemyID = enemyID;
    pkt->eventID = eventID;
    pkt->eventData = *eventData;
}
