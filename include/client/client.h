#ifndef CLIENT_H
#define CLIENT_H

#include "../common/game/player.h"
#include "client/client_network.h"

typedef enum ClientState_E {
    CLIENT_IDLE = 0,
    CLIENT_RUNNING = 1,
    CLIENT_JOINING = 2,
    CLIENT_PLAYING = 3,
    CLIENT_SHUTDOWN_REQUESTED = 4,
    CLIENT_SHUTTING_DOWN = 5,
    CLIENT_STOPPED = 6,
} ClientState;

typedef struct client_render_state_s {
    struct Sprite_S* background;
} client_render_state_t;

typedef struct Client_S {
    ClientState state;
    mutex_t lock;

    client_network_t *network;
    player_t *player;

    client_render_state_t renderState;
} Client;

extern Client g_client;

int client_main(void);
void client_close(void);

int client_connect(Client* client, const char *serverIP, const char *serverPort);
void client_disconnect(Client* client);

int client_send_to_server(Client *client, uint8_t pktId, void *pkt, uint32_t flags);

#endif /* CLIENT_H */