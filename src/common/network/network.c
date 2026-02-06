#include <gfc_types.h>

#include "common/logger.h"
#include "common/types.h"

#include "common/network/packet/definitions.h"
#include "common/network/packet/handler.h"
#include "common/network/network.h"

void network_handle_receive(network_t *network, const net_udp_event_t *context);

int network_init(network_t *network, const network_settings_t *settings, void *networkAdapter) {
    if (!network) {
        return -1;
    }

    network->settings = *settings;
    network->udpHost = NULL;
    network->running = 0;
    network->networkAdapter = networkAdapter;
    return 0;
}

void network_deinit(network_t *network) {
    if (!network) {
        return;
    }

    if (network->running) {
        net_udp_host_destroy(network->udpHost);
        network->udpHost = NULL;
        network->running = 0;
    }
}

void network_tick(network_t *network) {
    net_udp_event_t event;
    if (!network || !network->running) {
        return;
    }

    while (net_udp_host_check_events(network->udpHost, &event) > 0) {
        switch (event.type) {
            case NET_UDP_EVENT_TYPE_CONNECT:
                if (network->settings.onConnect) {
                    network->settings.onConnect(network, &event);
                }
                break;
            case NET_UDP_EVENT_TYPE_RECEIVE:
                network_handle_receive(network, &event);
                break;
            case NET_UDP_EVENT_TYPE_DISCONNECT:
                if (network->settings.onDisconnect) {
                    network->settings.onDisconnect(network, &event);
                }
                break;
            default:
                break;
        }
    }
}

void network_send(network_t *network, net_udp_peer_t peer, void *pkt);

void network_handle_receive(network_t *network, const net_udp_event_t *context) {
    net_udp_packet_t *rawPacket;
    net_udp_peer_t *peer;
    buffer_offset_t offset;
    uint8_t packetID;
    size_t bytes;
    buffer_t buffer;

    if (!network || !context->packet || !context->peer) {
        return;
    }

    rawPacket = context->packet;
    peer = context->peer;
    bytes = rawPacket->dataLength;
    buffer = rawPacket->data;
    if (!peer || !buffer || bytes == 0) {
        log_debug("Received NULL or empty packet.");
        return;
    }

    offset = 0;
    while (offset < bytes) {
        packetID = buffer[offset];
        if (packetID >= PACKET_COUNT) {
            log_info("Received invalid packet ID: %d", packetID);
            break;
        }

        packet_dispatch_table[packetID](buffer, &offset, peer);
    }

    net_udp_packet_destroy(rawPacket);
}