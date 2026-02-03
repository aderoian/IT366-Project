#ifndef SERVER_H
#define SERVER_H

#include <stdint.h>

#include "common/thread/mutex.h"
#include "common/thread/thread.h"

extern uint8_t __DEBUG;
extern uint8_t _dedicatedServer;

#define SERVER_TARGET_TICKRATE 30
#define SERVER_TARGET_SECONDS_PER_TICK (1.0f / SERVER_TARGET_TICKRATE)
#define SERVER_TARGET_TICK_TIME_MS (1000.0f / SERVER_TARGET_TICKRATE)

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

    uint64_t tickCounter;
    float currentTps;
    float currentUse;
    float averageTps[20];
    float averageUse[20];
} Server;

extern Server g_server;

int server_main(void);
void server_close(void);

#endif /* SERVER_H */