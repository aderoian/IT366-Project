#include "common/logger.h"
#include "common/network/packet/handler.h"

void handle_c2s_player_input_snapshot(const c2s_player_input_snapshot_packet_t *packet, void *peer) {
    log_info("Received player input packet: %llu, %llu, %d, %d", packet->inputCommand.clientTick, packet->inputCommand.lastServerTick, packet->inputCommand.axisX, packet->inputCommand.axisY);
}
