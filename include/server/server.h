#ifndef SERVER_H
#define SERVER_H

#include <stdint.h>

#include "common/thread/mutex.h"
#include "common/thread/thread.h"

extern uint8_t __DEBUG;
extern uint8_t _dedicatedServer;

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
} Server;

extern Server g_server;

int server_main(void);
void server_close(void);

#endif /* SERVER_H */