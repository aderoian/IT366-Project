#ifndef SERVER_NETWORK_H
#define SERVER_NETWORK_H

#include "common/network/udp.h"

typedef struct server_session_s {
    net_udp_peer_t *peer;
    uint32_t sessionID;
} server_session_t;

typedef struct server_network_config_s {
    char *bindIP;
    uint16_t bindPort;
    size_t maxSessions;
    size_t channelLimit;
    uint32_t inBandwidth;
    uint32_t outBandwidth;
} server_network_config_t;

typedef struct server_network_s {
    server_network_config_t config;
    net_udp_host_t *udpHost;

    server_session_t *sessions;
    size_t maxSessions;
    size_t currentSessionCount;

    uint8_t running;
} server_network_t;

server_network_t *server_network_create(const server_network_config_t *config);
void server_network_destroy(server_network_t *network);

int server_network_start(server_network_t *network);
void server_network_stop(server_network_t *network);

void server_network_tick(server_network_t *network);

void server_network_send(server_network_t *network, net_udp_peer_t *peer, uint8_t packetID, void *context);

#endif /* SERVER_NETWORK_H */