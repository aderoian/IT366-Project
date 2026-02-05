#include <stdio.h>

#include "common/logger.h"
#include "common/types.h"
#include "common/network/packet/handler.h"
#include "server/network/network.h"

void server_network_handle_data(net_udp_packet_t *rawPacket, net_udp_peer_t *peer);

server_network_t *server_network_create(const server_network_config_t *config) {
    server_network_t *network = malloc(sizeof(server_network_t));
    if (!network) {
        goto fail;
    }

    network->config = *config;
    network->udpHost = NULL;

    network->sessions = malloc(sizeof(server_session_t) * config->maxSessions);
    if (!network->sessions) {
        goto fail;
    }

    network->maxSessions = config->maxSessions;
    network->currentSessionCount = 0;

    return network;

    fail:
        if (network) {
            free(network);
        }
        log_error("Failed to allocate memory.");
        return NULL;
}

void server_network_destroy(server_network_t *network) {
    if (!network) {
        return;
    }

    if (network->running) {
        server_network_stop(network);
    }

    free(network->sessions);
    free(network);
}

int server_network_start(server_network_t *network) {
    net_udp_host_config_t udpConfig;
    char portStr[6];
    if (!network || network->running) {
        return 0;
    }

    snprintf(portStr, 6, "%u", network->config.bindPort);
    net_udp_host_server_config(&udpConfig, network->config.bindIP, portStr, network->config.maxSessions,
                               network->config.channelLimit, network->config.inBandwidth,
                               network->config.outBandwidth, 0);

    network->udpHost = net_udp_host_create(&udpConfig);
    if (!network->udpHost) {
        log_error("Failed to create UDP host for server network");
        return 0;
    }

    network->running = 1;
    return 1;
}

void server_network_stop(server_network_t *network) {
    if (!network || !network->running) {
        return;
    }

    net_udp_host_destroy(network->udpHost);
    network->udpHost = NULL;
    network->running = 0;
}

void server_network_tick(server_network_t *network) {
    net_udp_event_t event;
    if (!network || !network->running) {
        return;
    }

    while (net_udp_host_check_events(network->udpHost, &event) > 0) {
        switch (event.type) {
            case NET_UDP_EVENT_TYPE_CONNECT:
                log_info("New client connected");
                // Handle new connection
                break;
            case NET_UDP_EVENT_TYPE_RECEIVE:
                server_network_handle_data(event.packet, event.peer);
                break;
            case NET_UDP_EVENT_TYPE_DISCONNECT:
                log_info("Client disconnected");
                // Handle disconnection
                break;
            default:
                break;
        }
    }
}

void server_network_handle_data(net_udp_packet_t *rawPacket, net_udp_peer_t *peer) {
    buffer_offset_t offset;
    uint8_t packetID;
    size_t bytes = rawPacket->dataLength;
    buffer_t buffer = rawPacket->data;
    if (!buffer || bytes == 0) {
        log_error("Network received NULL or empty packet.");
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