#ifndef CLIENT_H
#define CLIENT_H

#include "client/network.h"

typedef enum ClientState_E {
    CLIENT_IDLE = 0,
    CLIENT_RUNNING = 1,
    CLIENT_SHUTDOWN_REQUESTED = 2,
    CLIENT_SHUTTING_DOWN = 3,
    CLIENT_STOPPED = 4,
} ClientState;

typedef struct Client_S {
    ClientState state;
    mutex_t lock;

    client_network_t *network;
} Client;

int client_main(void);
void client_close(void);

#endif /* CLIENT_H */