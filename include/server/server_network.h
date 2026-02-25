#ifndef SERVER_NETWORK_H
#define SERVER_NETWORK_H

#include "common/network/network.h"
#include "common/network/udp.h"

typedef struct server_session_s {
    net_udp_peer_t *peer;
    uint32_t sessionID;
    struct player_s *player;
} server_session_t;

typedef struct server_network_s {
    network_t baseNetwork;

    server_session_t *sessions;
    size_t maxSessions;
    size_t currentSessionCount;
    uint64_t nextSessionID;
} server_network_t;

server_network_t *server_network_create(const network_settings_t *settings);
void server_network_destroy(server_network_t *network);

int server_network_start(server_network_t *network);
void server_network_stop(server_network_t *network);

int server_network_send(server_network_t *network, server_session_t *session, uint8_t packetID, void *context, uint32_t flags);

#endif /* SERVER_NETWORK_H */