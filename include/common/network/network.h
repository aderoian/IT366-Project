#ifndef COMMON_NETWORK_H
#define COMMON_NETWORK_H

#include "common/network/udp.h"

struct network_s;

typedef void (*network_event_callback_t)(struct network_s *network, const net_udp_event_t *context);

typedef struct network_settings_s {
    char bindIP[16];
    uint16_t bindPort;
    size_t maxSessions;
    size_t channelLimit;
    uint32_t inBandwidth;
    uint32_t outBandwidth;
    uint32_t connectionTimeout;

    network_event_callback_t onConnect;
    network_event_callback_t onDisconnect;
} network_settings_t;

typedef struct network_s {
    network_settings_t settings;
    net_udp_host_t *udpHost;
    uint8_t running;
    void *networkAdapter;
} network_t;

int network_init(network_t *network, const network_settings_t *settings, void *networkAdapter);
void network_deinit(network_t *network);

void network_tick(network_t *network);
int network_send(net_udp_peer_t *peer, uint8_t pktId, void *pkt, uint32_t flags);

#endif /* COMMON_NETWORK_H */