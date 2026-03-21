#include "server/network/network_session.h"

#include "common/logger.h"
#include "common/game/inventory.h"
#include "common/network/network.h"
#include "common/network/packet/definitions.h"
#include "common/network/packet/io.h"
#include "server/server.h"

void send_inv_transaction(network_session_t *session, inventory_transaction_t *transaction);

void network_session_create(network_session_t *session, net_udp_peer_t *peer, const uint32_t sessionID) {
    session->peer = peer;
    session->sessionID = sessionID;
    session->player = NULL;
    session->dirtyFlags = 0;
    session->packetQueueSize = 0;

    memset(session->pendingTransactions, 0, sizeof(session->pendingTransactions));

    peer->data = session;
}

void network_session_destroy(network_session_t *session) {
    if (session->player) {
        server_destroy_player(session->player);
        session->player = NULL;
    }

    session->peer->data = NULL;
}

void network_session_send(network_session_t *session, void *context, const uint32_t flags) {
    if (!session || !session->peer) {
        log_error("Invalid session or peer.");
        return;
    }

    network_send(session->peer, context, flags);
}

void network_session_send_batch(network_session_t *session, void *context) {
    if (!session || !session->peer) {
        log_error("Invalid session or peer.");
        return;
    }

    if (session->packetQueueSize >= MAX_PACKET_QUEUE_SIZE) {
        log_error("Packet queue overflow for session ID: %u", session->sessionID);
        return;
    }

    session->packetQueue[session->packetQueueSize++] = context;
}

void network_session_sync(network_session_t *session) {
    if (!session || !session->peer) {
        log_error("Invalid session or peer.");
        return;
    }

    if (session->dirtyFlags & SESSION_DIRTY_INVENTORY) {
        for (uint32_t i = 0; i < MAX_INV_TRANSACTIONS; i++) {
            if (session->pendingTransactions[i]) {
                send_inv_transaction(session, session->pendingTransactions[i]);
                inventory_transaction_destroy(session->pendingTransactions[i]);
                session->pendingTransactions[i] = NULL;
            }
        }
        session->dirtyFlags &= ~SESSION_DIRTY_INVENTORY;
    }

    if (session->packetQueueSize == 0) {
        return; // No packets to send
    }

    network_send_batch(session->peer, session->packetQueue, session->packetQueueSize);

    // Clear the packet queue after sending
    session->packetQueueSize = 0;
}

void network_session_add_transaction(network_session_t *session, const inventory_transaction_t *transaction) {
    if (!session || !transaction) {
        log_error("Invalid session or transaction.");
        return;
    }

    for (uint32_t i = 0; i < MAX_INV_TRANSACTIONS; i++) {
        if (session->pendingTransactions[i] == NULL) {
            session->pendingTransactions[i] = transaction;
            session->dirtyFlags |= SESSION_DIRTY_INVENTORY;
            return;
        }
    }

    log_error("Inventory transaction queue overflow for session ID: %u", session->sessionID);
}

void send_inv_transaction(network_session_t *session, inventory_transaction_t *transaction) {
    if (!session || !transaction) {
        log_error("Invalid session or transaction.");
        return;
    }

    s2c_inventory_update_packet_t pkt;
    create_s2c_inventory_update(&pkt, session->player->id, transaction);
    network_session_send(session, &pkt, NET_UDP_FLAG_RELIABLE);
}
