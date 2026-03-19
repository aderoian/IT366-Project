#ifndef SERVER_NETWORK_SESSION_H
#define SERVER_NETWORK_SESSION_H

#include "common/network/udp.h"

#define MAX_PACKET_QUEUE_SIZE 1024

#define SESSION_DIRTY_INVENTORY (1 << 0)
#define SESSION_DIRTY_PACKET_QUEUE (1 << 1)

typedef struct network_session_s {
    net_udp_peer_t *peer;
    uint32_t sessionID;
    struct player_s *player;

    uint32_t dirtyFlags;

    void *packetQueue[MAX_PACKET_QUEUE_SIZE];
    uint32_t packetQueueSize;
} network_session_t;

void network_session_create(network_session_t *session, net_udp_peer_t *peer, uint32_t sessionID);

void network_session_destroy(network_session_t *session);

void network_session_send(network_session_t *session, void *context, uint32_t flags);
void network_session_send_batch(network_session_t *session, void *context);

void network_session_sync(network_session_t *session);

#endif /* SERVER_NETWORK_SESSION_H */