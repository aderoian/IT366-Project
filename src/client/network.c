#include "client/network.h"

#include <stdio.h>
#include <string.h>

#include "common/types.h"
#include "common/network/packet/handler.h"

client_network_t *client_network_create(const client_network_config_t *config) {
    client_network_t *network;
    net_udp_host_config_t hostConfig;

    if (!config) {
        return NULL;
    }

    network = (client_network_t *)malloc(sizeof(client_network_t));
    if (!network) {
        return NULL;
    }

    network->config = *config;
    network->connected = 0;
    network->running = 0;

    net_udp_host_client_config(&hostConfig,
                               "127.0.0.1",
                               "12345",
                               network->config.channelLimit,
                               network->config.inBandwidth,
                               network->config.outBandwidth,
                               network->config.connectionTimeout);

    network->udpHost = net_udp_host_create(&hostConfig);
    if (!network->udpHost) {
        free(network);
        return NULL;
    }

    return network;
}

void client_network_destroy(client_network_t *network);

int client_network_start(client_network_t *network, const char *serverIP, const char *serverPort) {
    if (!network || network->connected || network->running) {
        return -1;
    }

    if (net_addr_resolve(&network->udpHost->address, serverIP, serverPort, NET_SOCK_DGRAM) < 0) {
        return -1;
    }

    network->serverPeer = net_udp_host_client_connect(network->udpHost);
    if (!network->serverPeer) {
        return -1;
    }

    network->running = 1;
    network->connected = 1;
    return 0;
}

void client_network_stop(client_network_t *network) {
    if (!network || !network->running) {
        return;
    }

    net_tcp_host_client_disconnect(network->udpHost);
    network->running = 0;
}

void client_network_tick(client_network_t *network) {
    net_udp_event_t event;
    if (!network || !network->running) {
        return;
    }

    while (net_udp_host_check_events(network->udpHost, &event) > 0) {
        switch (event.type) {
            case NET_UDP_EVENT_TYPE_CONNECT:
                network->connected = 1;
                printf("Connected to server.\n");
                break;
            case NET_UDP_EVENT_TYPE_RECEIVE:
                // Handle received packet
                printf("Received packet on channel %u with length %zu.\n",
                       event.chanelId, event.packet->dataLength);
                net_udp_packet_destroy(event.packet);
                break;
            case NET_UDP_EVENT_TYPE_DISCONNECT:
                network->connected = 0;
                printf("Disconnected from server.\n");
                break;
            default:
                break;
        }
    }
}

void client_network_send(client_network_t *network, uint8_t packetID, void *context) {
    buffer_t buf = (buffer_t) malloc(1024); // FIXME: packet pooling
    buffer_offset_t off = 0;
    net_udp_packet_t *packet;

    if (!network || !network->running || !network->connected) {
        free(buf);
        return;
    }

    packet_send_table[packetID](packetID, context, buf, &off);
    packet = net_udp_packet_create(buf, off, NET_UDP_FLAG_RELIABLE);
    net_tcp_host_client_send(network->udpHost, 0, packet);
}