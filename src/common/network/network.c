#include <gfc_types.h>

#include "common/logger.h"
#include "common/types.h"

#include "common/network/packet/definitions.h"
#include "common/network/packet/handler.h"
#include "common/network/network.h"

#include "common/game/game.h"

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

int network_send(net_udp_peer_t *peer, void *pkt, const uint32_t flags) {
    size_t numBytes, length;
    uint8_t *buffer;
    buffer_offset_t offset = 0;
    uint8_t pktId;
    if (!pkt) {
        return -1;
    }

    // TODO: Avoid heap alloc here by using a packet pool and reusing buffers.
    pktId = *((uint8_t *)pkt);
    length = *((uint64_t *)pkt + 1);
    numBytes = length + PACKET_HEADER_SIZE;
    buffer = malloc(numBytes);
    packet_send_table[pktId](buffer, &offset, pkt);
    net_udp_packet_t *packet = net_udp_packet_create(buffer, numBytes, flags);
    if (!packet) {
        log_error("Failed to create packet for sending.");
        return -1;
    }

    return net_udp_peer_send(peer, 0, packet);
}

int network_send_batch(net_udp_peer_t *peer, void **pkts, const uint32_t count) {
    size_t numBytes = 0, length = 0;
    uint8_t *buffer;
    buffer_offset_t offset = 0;
    uint8_t pktId;
    uint32_t i;
    if (!peer || !pkts || count == 0) {
        return -1;
    }

    // Calculate total buffer size needed for all packets
    for (i = 0; i < count; i++) {
        length = *((uint64_t *)pkts[i] + 1);
        numBytes += length + PACKET_HEADER_SIZE;
    }

    buffer = malloc(numBytes + PACKET_HEADER_SIZE);
    if (!buffer) {
        log_error("Failed to allocate buffer for batch sending.");
        return -1;
    }

    // Serialize each packet into the buffer
    for (i = 0; i < count; i++) {
        pktId = *((uint8_t *)pkts[i]);
        packet_send_table[pktId](buffer, &offset, pkts[i]);
        free(pkts[i]);
    }

    net_udp_packet_t *packet = net_udp_packet_create(buffer, numBytes, 0);
    if (!packet) {
        net_udp_packet_destroy(packet);
        log_error("Failed to create packet for sending.");
        return -1;
    }

    return net_udp_peer_send(peer, 0, packet);
}

void network_handle_receive(network_t *network, const net_udp_event_t *context) {
    net_udp_packet_t *rawPacket;
    net_udp_peer_t *peer;
    buffer_offset_t offset;
    uint8_t packetID;
    size_t length, bytes;
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
        length = be64toh(*((uint64_t *)(buffer + offset + sizeof(uint8_t)))) + PACKET_HEADER_SIZE;
        if (packetID >= PACKET_COUNT) {
            log_info("Received invalid packet ID: %d", packetID);
            break;
        }

        if (offset + length > bytes) {
            log_info("Received incomplete packet with ID: %d", packetID);
            break;
        }

        packet_dispatch_table[packetID](buffer, &offset, peer);
    }

    net_udp_packet_destroy(rawPacket);
}