#include "common/logger.h"
#include "common/network/packet/handler.h"

void handle_c2s_player_input(const c2s_player_input_packet_t *packet, void *peer) {
    log_info("Received player input packet: %llu, %llu, %d, %d", packet->clientTick, packet->lastServerTick, packet->axisX, packet->axisY);
}
