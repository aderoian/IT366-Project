#include <stdio.h>

#include "common/logger.h"
#include "server/server_network.h"
#include "server/server.h"

server_network_t g_serverNetwork = {0};

void server_network_client_connect(struct network_s *network, const net_udp_event_t *context);
void server_network_client_disconnect(struct network_s *network, const net_udp_event_t *context);

server_network_t *server_network_create(const network_settings_t *settings) {
    server_network_t *network = malloc(sizeof(server_network_t));
    if (!network) {
        goto fail;
    }

    network_init(&network->baseNetwork, settings, network);
    network->baseNetwork.settings.onConnect = server_network_client_connect;
    network->baseNetwork.settings.onDisconnect = server_network_client_disconnect;

    network->sessions = malloc(sizeof(server_session_t) * settings->maxSessions);
    if (!network->sessions) {
        goto fail;
    }
    network->maxSessions = settings->maxSessions;
    network->currentSessionCount = 0;
    network->nextSessionID = 0;

    return network;

    fail:
        if (network) {
            network_deinit(&network->baseNetwork);
            free(network);
        }
        log_error("Failed to allocate memory.");
        return NULL;
}

void server_network_destroy(server_network_t *network) {
    if (!network) {
        return;
    }

    if (network->baseNetwork.running) {
        server_network_stop(network);
    }

    network_deinit(&network->baseNetwork);
    free(network->sessions);
    free(network);
}

int server_network_start(server_network_t *network) {
    network_settings_t *settings;
    net_udp_host_config_t udpConfig;
    char portStr[6];
    if (!network || network->baseNetwork.running) {
        return 0;
    }

    settings = &network->baseNetwork.settings;
    snprintf(portStr, 6, "%u", settings->bindPort);
    net_udp_host_server_config(&udpConfig, NULL, portStr, settings->maxSessions,
                               settings->channelLimit, settings->inBandwidth,
                               settings->outBandwidth, 0);

    network->baseNetwork.udpHost = net_udp_host_create(&udpConfig);
    if (!network->baseNetwork.udpHost) {
        log_error("Failed to create UDP host for server network");
        return 0;
    }

    network->baseNetwork.running = 1;
    return 1;
}

void server_network_stop(server_network_t *network) {
    network_t *baseNetwork;
    if (!network) {
        return;
    }

    baseNetwork = &network->baseNetwork;
    if (!baseNetwork->running) {
        return;
    }

    net_udp_host_destroy(baseNetwork->udpHost);
    baseNetwork->udpHost = NULL;
    baseNetwork->running = 0;
}

void server_network_client_connect(struct network_s *network, const net_udp_event_t *context) {
    server_network_t *serverNetwork = network->networkAdapter;
    if (serverNetwork->currentSessionCount >= serverNetwork->maxSessions) {
        log_warn("Max sessions reached. Rejecting new connection.");
        net_udp_peer_disconnect(context->peer, 0);
        return;
    }

    server_session_t *session = &serverNetwork->sessions[serverNetwork->currentSessionCount++];
    session->peer = context->peer;
    context->peer->data = session;
    session->sessionID = serverNetwork->nextSessionID++;

    log_info("New client connected. Session ID: %u", session->sessionID);
}

void server_network_client_disconnect(struct network_s *network, const net_udp_event_t *context) {
    server_network_t *serverNetwork = network->networkAdapter;
    server_session_t *session;
    for (size_t i = 0; i < serverNetwork->currentSessionCount; ++i) {
        if (serverNetwork->sessions[i].peer == context->peer) {
            session = &serverNetwork->sessions[i];
            if (session->player) {
                server_destroy_player(session->player);
            }
            log_info("Client disconnected. Session ID: %u", session->sessionID);

            // Shift remaining sessions down
            for (size_t j = i; j < serverNetwork->currentSessionCount - 1; ++j) {
                serverNetwork->sessions[j] = serverNetwork->sessions[j + 1];
            }
            --serverNetwork->currentSessionCount;
            break;
        }
    }
}

int server_network_send(server_network_t *network, server_session_t *session, const uint8_t packetID, void *context, const uint32_t flags) {
    if (!network || !session || !network->baseNetwork.running) {
        return -1;
    }

    return network_send(session->peer, packetID, context, flags);
}