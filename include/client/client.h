#ifndef CLIENT_H
#define CLIENT_H

#include "common/game/game.h"
#include "common/game/player.h"
#include "client/client_network.h"
#include "client/ui/overlay.h"
#include "../common/game/world/world.h"

typedef enum client_state_e {
    CLIENT_IDLE = 0,
    CLIENT_RUNNING = 1,
    CLIENT_JOINING = 2,
    CLIENT_PLAYING = 3,
    CLIENT_SHUTDOWN_REQUESTED = 4,
    CLIENT_SHUTTING_DOWN = 5,
    CLIENT_STOPPED = 6,
} client_state_t;

typedef enum client_mode_e {
    CLIENT_MODE_NONE = 0,
    CLIENT_MODE_SINGLEPLAYER = 1,
    CLIENT_MODE_MULTIPLAYER = 2,
} client_mode_t;

typedef struct client_render_state_s {
    struct Sprite_S* background;
} client_render_state_t;

typedef struct Client_S {
    struct def_manager_s *defManager;
    struct entity_manager_s *entityManager;
    struct tower_manager_s *towerManager;
    struct enemy_def_manager_s *enemyManager;

    client_mode_t mode;
    client_state_t state;
    mutex_t lock;

    char playerName[32];
    client_network_t *network;
    player_t *player;

    client_render_state_t renderState;
    float deltaTime;

    overlay_t overlay;
} Client;

extern Client g_client;

int client_main(void);
void client_close(void);

int client_connect(Client* client, const char *serverIP, const char *serverPort);
void client_disconnect(Client* client);

int client_begin_singleplayer(Client* client);

int client_send_to_server(Client *client, void *pkt, uint32_t flags);

#endif /* CLIENT_H */