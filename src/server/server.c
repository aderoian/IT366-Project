#include <stdio.h>
#include <string.h>

#include "common/logger.h"

#include "server/server.h"

#include <math.h>

#include "common/time.h"
#include "../../include/server/server_network.h"
#include "common/game/player.h"
#include "server/game/player_manager.h"

Server g_server = {0};

void *server_run(void *arg);
void server_runCommandLoop(void);

uint64_t server_tick(Server *server, uint64_t tickTimeMs);
void server_tickProcessor(Server *server);

int server_main(void) {
    log_info("Initializing server...");

    g_server.state = SERVER_IDLE;
    mutex_init(&g_server.lock);

    if (thread_create(&g_server.thread, server_run, &g_server) < 0) {
        log_fatal("Failed to create server thread");
        mutex_destroy(&g_server.lock);
        return -1;
    }

    if (_dedicatedServer) {
        server_runCommandLoop();
    }

    return 0;
}

int server_startup(Server *server) {
    log_info("Starting server...");

    // FIXME: Load configuration from file or arguments
    network_settings_t settings = {
        .bindIP = "",
        .bindPort = 12345,
        .maxSessions = 1024,
        .channelLimit = 4,
        .inBandwidth = 0,
        .outBandwidth = 0,
    };

    server->playerManager = player_manager_create(8);
    if (!server->playerManager) {
        log_error("Failed to create player manager");
        return 0;
    }

    // Initialize server subsystems here (networking, database, etc.)
    server->network = server_network_create(&settings);
    if (!server->network) {
        log_error("Failed to create server network");
        return 0;
    }

    log_info("Starting server network on port %u...", settings.bindPort);
    if (!server_network_start(server->network)) {
        log_error("Failed to start server network");
        server_network_destroy(server->network);
        return 0;
    }
    log_info("Server network started successfully.");

    log_info("Server started successfully.");
    return 1;
}

int server_shutdown(Server *server) {
    if (!server) {
        return 0;
    }

    log_info("Stopping server network...");
    server_network_stop(server->network);
    server_network_destroy(server->network);
    server->network = NULL;
    log_info("Server network stopped.");

    return 1;
}

void server_close(void) {
    log_info("Shutting down server...");

    // Request shutdown
    mutex_lock(&g_server.lock);
    if (g_server.state == SERVER_RUNNING) {
        g_server.state = SERVER_SHUTDOWN_REQUESTED;
    }
    mutex_unlock(&g_server.lock);

    // Wait for server thread to finish
    thread_join(&g_server.thread);
    mutex_destroy(&g_server.lock);
}

void *server_run(void *arg) {
    Server *server = (Server *) arg;

    if (!server_startup(server)) {
        log_fatal("Server startup failed! Aborting...");
        abort();
    }

    // Set server state to running
    mutex_lock(&g_server.lock);
    g_server.state = SERVER_RUNNING;
    mutex_unlock(&g_server.lock);

    // Main server loop
    server_tickProcessor(server);
    log_info("Server stopped!");

    return NULL;
}

uint64_t server_tick(Server *server, const uint64_t tickTimeMs) {
    uint64_t tickDuration;
    size_t index;

    server->tickCounter++;

    network_tick(&server->network->baseNetwork);

    tickDuration = time_now_ms() - tickTimeMs;
    server->currentTps = fminf(SERVER_TARGET_TICKRATE, 1000.0f / (float)tickDuration);
    server->currentUse = fminf(1.0f, (float)tickDuration / SERVER_TARGET_TICK_TIME_MS);

    // Update average TPS and use
    index = server->tickCounter % 20;
    server->averageTps[index] = server->currentTps;
    server->averageUse[index] = server->currentUse;

    return tickDuration;
}

void server_tickProcessor(Server *server) {
    uint8_t shutdownRequested = 0;
    uint64_t tickTime, tickDur;
    int sleepTime;

    while (1) {
        // Check for shutdown request
        mutex_lock(&server->lock);
        if (server->state == SERVER_SHUTDOWN_REQUESTED) {
            server->state = SERVER_SHUTTING_DOWN;
            shutdownRequested = 1;
        }
        mutex_unlock(&server->lock);

        // Handle shutdown if requested
        if (shutdownRequested) {
            server_shutdown(server);

            mutex_lock(&server->lock);
            server->state = SERVER_STOPPED;
            mutex_unlock(&server->lock);
            break;
        }

        // Perform server tick
        tickTime = time_now_ms();
        tickDur = server_tick(server, tickTime);
        if (tickDur < SERVER_TARGET_TICK_TIME_MS) {
            sleepTime = SERVER_TARGET_TICK_TIME_MS - tickDur;
            if (sleepTime > 0) {
                thread_sleepMs(sleepTime);
            } else {
                log_warn("Server overloaded! Running behind by %d ms", -sleepTime);
            }
        }
    }
}

void server_runCommandLoop(void) {
    char command[256];
    while (1) {
        printf("server> ");
        if (fgets(command, sizeof(command), stdin) == NULL) {
            break;
        }

        if (strncmp(command, "stop", 4) == 0) {
            server_close();
            break;
        }
        if (strncmp(command, "status", 6) == 0) {
            mutex_lock(&g_server.lock);
            log_info("Server State: %d", g_server.state);
            log_info("Current TPS: %.2f", g_server.currentTps);
            log_info("Current CPU Use: %.2f%%", g_server.currentUse * 100.0f);
            mutex_unlock(&g_server.lock);
        } else {
            printf("Unknown command: %s", command);
        }
    }
}

player_t *server_create_player(Server *server, struct server_session_s *session) {
    player_t *player;
    if (!server || !session) {
        return NULL;
    }

    player = player_manager_add(server->playerManager, session->sessionID, "");
    if (!player) {
        log_error("Failed to create player for session ID %u", session->sessionID);
        return NULL;
    }

    player->data = session;
    log_info("Created player with ID %u for session ID %u", player->id, session->sessionID);
    return player;
}

void server_destroy_player(struct player_s *player) {
    if (!player) {
        return;
    }

    player_manager_remove(g_server.playerManager, player->id);
    log_info("Destroyed player with ID %u", player->id);
}

void server_send_packet(Server* server, const player_t *player, const uint8_t packetID, void *context, const uint32_t flags) {
    server_session_t *session;
    if (!server || !player) {
        return;
    }

    session = (server_session_t *) player->data;
    if (!session->peer) {
        log_warn("Player ID %u does not have a valid session", player->id);
        return;
    }

    server_network_send(server->network, session, packetID, context, flags);
}

void server_broadcast_packet(Server* server, const uint8_t packetID, void *context, const uint32_t flags) {
    size_t i, playerCount;
    const player_t **players;
    if (!server) {
        return;
    }

    players = player_manager_get_all(server->playerManager, &playerCount);
    for (i = 0; i < playerCount; ++i) {
        server_send_packet(server, players[i], packetID, context, flags);
    }
}