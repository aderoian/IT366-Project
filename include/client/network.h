#ifndef CLIENT_NETWORK_H
#define CLIENT_NETWORK_H

#include "common/network/udp.h"

typedef struct client_network_config_s {
    size_t channelLimit;
    uint32_t inBandwidth;
    uint32_t outBandwidth;
    uint32_t connectionTimeout;
} client_network_config_t;

typedef struct client_network_s {
    client_network_config_t config;
    net_udp_host_t *udpHost;
    net_udp_peer_t *serverPeer; // represents a connection to a server.
    uint8_t connected;
    uint8_t running;
} client_network_t;

client_network_t *client_network_create(const client_network_config_t *config);
void client_network_destroy(client_network_t *network);

int client_network_start(client_network_t *network, const char *serverIP, const char *serverPort);
void client_network_stop(client_network_t *network);
void client_network_tick(client_network_t *network);

void client_network_send(client_network_t *network, uint8_t packetID, void *context);

#endif /* CLIENT_NETWORK_H */