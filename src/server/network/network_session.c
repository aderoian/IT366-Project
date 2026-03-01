#include "server/network/network_session.h"

#include "common/logger.h"
#include "common/network/network.h"
#include "server/server.h"

void network_session_create(network_session_t *session, net_udp_peer_t *peer, const uint32_t sessionID) {
    session->peer = peer;
    session->sessionID = sessionID;
    session->player = NULL;
    session->dirtyFlags = 0;

    peer->data = session;
}

void network_session_destroy(network_session_t *session) {
    if (session->player) {
        server_destroy_player(session->player);
        session->player = NULL;
    }

    session->peer->data = NULL;
}

void network_session_send(network_session_t *session, const uint8_t packetID, void *context, const uint32_t flags) {
    if (!session || !session->peer) {
        log_error("Invalid session or peer.");
        return;
    }

    network_send(session->peer, packetID, context, flags);
}

void network_session_sync(network_session_t *session) {

}

