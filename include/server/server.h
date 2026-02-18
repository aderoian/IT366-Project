#ifndef SERVER_H
#define SERVER_H

#include <stdint.h>

#include "common/game/entity.h"
#include "common/thread/mutex.h"
#include "common/thread/thread.h"

extern uint8_t __DEBUG;
extern uint8_t _dedicatedServer;

struct server_session_s;
struct player_s;
struct player_manager_s;

#define SERVER_TARGET_TICKRATE 30
#define SERVER_TARGET_SECONDS_PER_TICK (1.0 / SERVER_TARGET_TICKRATE)
#define SERVER_TARGET_TICK_TIME_MS (1000.0 / SERVER_TARGET_TICKRATE)

typedef enum ServerState_E {
    SERVER_IDLE = 0,
    SERVER_RUNNING = 1,
    SERVER_SHUTDOWN_REQUESTED = 2,
    SERVER_SHUTTING_DOWN = 3,
    SERVER_STOPPED = 4,
} ServerState;

typedef struct Server_S {
    ServerState state;
    mutex_t lock;
    thread_t thread;

    struct server_network_s *network;
    struct player_manager_s *playerManager;

    double currentTps;
    double currentUse;
    double averageTps[20];
    double averageUse[20];
} Server;

extern Server g_server;

int server_main(void);
void server_close(void);

struct player_s *server_create_player(Server *server, struct server_session_s *session);
void server_destroy_player(struct player_s *player);

Entity *server_spawn_player_entity(struct player_s *player, GFC_Vector2D pos);

void server_send_packet(Server* server, const struct player_s *player, uint8_t packetID, void *context, uint32_t flags);
void server_broadcast_packet(Server* server, uint8_t packetID, void *context, uint32_t flags);

#endif /* SERVER_H */