#ifndef SERVER_NETWORK_H
#define SERVER_NETWORK_H

#include "common/network/network.h"
#include "common/network/udp.h"

typedef struct server_session_s {
    net_udp_peer_t *peer;
    uint32_t sessionID;
} server_session_t;

typedef struct server_network_s {
    network_t baseNetwork;

    server_session_t *sessions;
    size_t maxSessions;
    size_t currentSessionCount;
} server_network_t;

server_network_t *server_network_create(const network_settings_t *settings);
void server_network_destroy(server_network_t *network);

int server_network_start(server_network_t *network);
void server_network_stop(server_network_t *network);

#endif /* SERVER_NETWORK_H */