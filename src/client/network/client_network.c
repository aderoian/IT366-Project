#include <stdio.h>

#include "common/logger.h"

#include "client/client_network.h"

void client_network_client_connect(struct network_s *network, const net_udp_event_t *context);
void client_network_client_disconnect(struct network_s *network, const net_udp_event_t *context);

client_network_t *client_network_create(const network_settings_t *settings) {
    client_network_t *network;
    net_udp_host_config_t hostConfig;
    if (!settings) {
        return NULL;
    }

    network = (client_network_t *)malloc(sizeof(client_network_t));
    if (!network) {
        return NULL;
    }

    network_init(&network->baseNetwork, settings, network);
    network->baseNetwork.settings.onConnect = client_network_client_connect;
    network->baseNetwork.settings.onDisconnect = client_network_client_disconnect;
    net_udp_host_client_config(&hostConfig,
                               "127.0.0.1",
                               "12345",
                               settings->channelLimit,
                               settings->inBandwidth,
                               settings->outBandwidth,
                               settings->connectionTimeout);

    network->baseNetwork.udpHost = net_udp_host_create(&hostConfig);
    if (!network->baseNetwork.udpHost) {
        free(network);
        return NULL;
    }

    network->connected = 0;
    network->baseNetwork.running = 1;
    return network;
}

void client_network_destroy(client_network_t *network) {
    if (!network) {
        return;
    }

    if (network->connected) {
        client_network_stop(network);
    }

    network_deinit(&network->baseNetwork);
    free(network);
}

int client_network_start(client_network_t *network, const char *serverIP, const char *serverPort) {
    if (!network || network->connected || !network->baseNetwork.running) {
        return -1;
    }

    if (net_addr_resolve(&network->baseNetwork.udpHost->address, serverIP, serverPort, NET_SOCK_DGRAM) < 0) {
        return -1;
    }

    network->serverPeer = net_udp_host_client_connect(network->baseNetwork.udpHost);
    if (!network->serverPeer) {
        return -1;
    }

    network->connected = 1;
    return 0;
}

void client_network_stop(client_network_t *network) {
    if (!network || !network->baseNetwork.running) {
        return;
    }

    net_tcp_host_client_disconnect(network->baseNetwork.udpHost);
    network->connected = 0;
}

void client_network_client_connect(struct network_s *network, const net_udp_event_t *context) {
    log_info("Connected to server.");
}

void client_network_client_disconnect(struct network_s *network, const net_udp_event_t *context) {
    log_info("Disconnected from server.");
}