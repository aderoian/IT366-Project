#include "common/logger.h"
#include "common/network/packet/handler.h"

#include "client/client.h"

void handle_s2c_player_join_response(const s2c_player_join_response_packet_t *pkt, void *client) {
    if (!pkt) {
        return;
    }

    if (pkt->success) {
        if (g_client.state != CLIENT_JOINING) {
            log_warn("Received join response while not joining. Current state: %d", g_client.state);
            return;
        }

        log_info("Joined server successfully with Player ID: %u", pkt->playerID);

        g_client.player = player_create(pkt->playerID, "Player"); // FIXME: Use actual player name (needs array serialization)
        g_client.state = CLIENT_PLAYING;
    } else {
        log_info("Failed to join server.");
    }
}

void handle_s2c_player_create(const s2c_player_create_packet_t *pkt, void *client) {
    log_info("Player created with ID: %u at position (%f, %f)", pkt->playerID, pkt->spawnX, pkt->spawnY);
}