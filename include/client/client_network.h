#ifndef CLIENT_NETWORK_H
#define CLIENT_NETWORK_H

#include "common/network/network.h"

typedef struct client_network_s {
    network_t baseNetwork;

    net_udp_peer_t *serverPeer;
    uint8_t connected;
} client_network_t;

client_network_t *client_network_create(const network_settings_t *settings);
void client_network_destroy(client_network_t *network);

int client_network_start(client_network_t *network, const char *serverIP, const char *serverPort);
void client_network_stop(client_network_t *network);

#endif /* CLIENT_NETWORK_H */