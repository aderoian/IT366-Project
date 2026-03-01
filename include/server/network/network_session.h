#ifndef SERVER_NETWORK_SESSION_H
#define SERVER_NETWORK_SESSION_H

#include "common/network/udp.h"

#define SESSION_DIRTY_INVENTORY (1 << 0)

typedef struct network_session_s {
    net_udp_peer_t *peer;
    uint32_t sessionID;
    struct player_s *player;

    uint32_t dirtyFlags;
} network_session_t;

void network_session_create(network_session_t *session, net_udp_peer_t *peer, uint32_t sessionID);

void network_session_destroy(network_session_t *session);

void network_session_send(network_session_t *session, uint8_t packetID, void *context, uint32_t flags);

void network_session_sync(network_session_t *session);

#endif /* SERVER_NETWORK_SESSION_H */