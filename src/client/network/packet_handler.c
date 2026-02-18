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
        player_entity_spawn(g_client.player, gfc_vector2d(pkt->spawnX, pkt->spawnY), "images/pointer.png");
        g_client.state = CLIENT_PLAYING;
    } else {
        log_info("Failed to join server.");
    }
}

void handle_s2c_player_create(const s2c_player_create_packet_t *pkt, void *client) {
    log_info("Player created with ID: %u at position (%f, %f)", pkt->playerID, pkt->spawnX, pkt->spawnY);
}

void handle_s2c_player_state_snapshot(const s2c_player_state_snapshot_packet_t *pkt, void *client) {
    player_input_process_server(g_client.player, pkt->tickNumber, pkt->xPos, pkt->yPos);
}