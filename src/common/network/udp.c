#include "common/time.h"
#include "common/network/udp.h"

#include <netdb.h>
#include <string.h>

int net_addr_resolve(net_addr_t *out, const char *host, const char *service, const int socktype) {
    struct addrinfo hints, *res = NULL;
    int rc;
    if (!out || !service) {
        return -1;
    }

    memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_UNSPEC; // IPv4 or IPv6
    hints.ai_socktype = socktype;
    hints.ai_flags = AI_ADDRCONFIG; // avoid unusable families
    if (host == NULL) {
        hints.ai_flags |= AI_PASSIVE; // for wildcard IP address
    }

    rc = getaddrinfo(host, service, &hints, &res);
    if (rc != 0 || !res) {
        return -1;
    }

    memcpy(&out->storage, res->ai_addr, res->ai_addrlen);
    out->len = (socklen_t) res->ai_addrlen;

    freeaddrinfo(res);
    return 0;
}

int net_addr_family(const net_addr_t *addr) {
    if (!addr) {
        return -1; // invalid input
    }

    switch (addr->storage.ss_family) {
        case NET_AF_INET: return NET_AF_INET; // IPv4
        case NET_AF_INET6: return NET_AF_INET6; // IPv6
#ifdef _WIN32
        case AF_UNSPEC: return AF_UNSPEC;
#endif
        default: return -1; // unknown/unsupported family
    }
}

uint16_t net_addr_port(const net_addr_t *addr) {
    struct sockaddr_in *in;
    struct sockaddr_in6 *in6;
    if (!addr) {
        return 0;
    }

    switch (addr->storage.ss_family) {
        case NET_AF_INET: {
            in = (struct sockaddr_in *) &addr->storage;
            return ntohs(in->sin_port);
        }
        case NET_AF_INET6: {
            in6 = (struct sockaddr_in6 *) &addr->storage;
            return ntohs(in6->sin6_port);
        }
        default:
            return 0; // Unknown family
    }
}

int net_addr_toString(const net_addr_t *addr, char *buffer, size_t buffer_size) {
    struct sockaddr_in *in;
    struct in_addr ipv4;
    struct sockaddr_in6 *in6;
    uint8_t *bytes;
    char ipv4buf[INET_ADDRSTRLEN];
    if (!addr || !buffer || buffer_size == 0) {
        return -1;
    }

    if (addr->storage.ss_family == NET_AF_INET) {
        in = (struct sockaddr_in *) &addr->storage;
        if (!inet_ntop(AF_INET, &in->sin_addr, buffer, (socklen_t) buffer_size)) {
            return -1;
        }
        return 0;
    }

    if (addr->storage.ss_family == NET_AF_INET6) {
        in6 = (struct sockaddr_in6 *) &addr->storage;

        // Check for IPv4-mapped IPv6 address ::ffff:a.b.c.d
        bytes = in6->sin6_addr.s6_addr;
        if (memcmp(bytes, "\0\0\0\0\0\0\0\0\0\0\xFF\xFF", 12) == 0) {
            memcpy(&ipv4buf, &bytes[12], 4);
            memcpy(&ipv4, &bytes[12], 4);
            if (!inet_ntop(NET_AF_INET, &ipv4, buffer, (socklen_t) buffer_size)) {
                return -1;
            }
            return 0;
        }

        // Otherwise, normal IPv6
        if (!inet_ntop(NET_AF_INET6, &in6->sin6_addr, buffer, (socklen_t) buffer_size)) {
            return -1;
        }
        return 0;
    }

    // Unknown family
    return -1;
}

net_udp_packet_t *net_udp_packet_create(void *data, const size_t dataSize, const uint32_t flags) {
    net_udp_packet_t *packet;
    packet = malloc(sizeof(net_udp_packet_t));
    if (!packet) {
        return NULL;
    }

    packet->_internalPacket = enet_packet_create(data, dataSize, flags | NET_UDP_FLAG_NO_ALLOCATE);
    packet->dataLength = dataSize;
    packet->data = data;

    return packet;
}

void net_udp_packet_destroy(net_udp_packet_t *packet) {
    if (packet) {
        if (packet->_internalPacket->flags & NET_UDP_FLAG_NO_ALLOCATE) {
            free(packet->data);
        }
        enet_packet_destroy(packet->_internalPacket);
        free(packet);
    }
}

int net_udp_packet_resize(net_udp_packet_t *packet, const size_t dataLength) {
    if (enet_packet_resize(packet->_internalPacket, dataLength) < 0) {
        return -1;
    }

    packet->dataLength = dataLength;
    packet->data = packet->_internalPacket->data;
    return 0;
}

net_udp_packet_t *net_udp_peer_receive(net_udp_peer_t *peer, uint8_t *channelID) {
    net_udp_packet_t *packet;
    struct _ENetPacket *internalPacket = enet_peer_receive(peer, channelID);
    if (!internalPacket) {
        return NULL;
    }

    packet = malloc(sizeof(net_udp_packet_t));
    if (!packet) {
        enet_packet_destroy(internalPacket);
        return NULL;
    }

    packet->_internalPacket = internalPacket;
    packet->dataLength = internalPacket->dataLength;
    packet->data = internalPacket->data;

    return packet;
}

void *net_udp_host_thread(void *arg);

net_udp_host_t *net_udp_host_create(const net_udp_host_config_t *config) {
    ENetAddress eAddr = {0};
    char ip[64];
    net_udp_host_t *host = malloc(sizeof(net_udp_host_t));
    if (!host) {
        return NULL;
    }

    if (!buf_spsc_ring_init(&host->eventBuffer, 1024, sizeof(net_udp_event_t))) {
        free(host);
        return NULL;
    }

    net_address_to_enet(&config->address, &eAddr);
    enet_address_get_host_ip(&eAddr, ip, 64);
    host->enetHost = enet_host_create(config->isServer ? &eAddr : NULL, config->peerCount, config->channelLimit,
                                      config->incomingBandwidth, config->outgoingBandwidth);
    if (!host->enetHost) {
        free(host);
        return NULL;
    }

    mutex_init(&host->hostLock);
    host->state = NET_HOST_IDLE;
    host->threadRunning = 1;
    host->shutdownStartTime = 0;
    host->address = config->address;
    host->channelLimit = config->channelLimit;
    host->connectTimeout = config->connectTimeout;
    host->isServer = config->isServer;

    if (config->isServer) {
        if (!thread_create(&host->hostThread, net_udp_host_thread, host)) {
            net_udp_host_destroy(host);
            free(host);
            return NULL;
        }
    }

    return host;
}

void net_udp_host_destroy(net_udp_host_t *host) {
    // Initiate shutdown
    mutex_lock(&host->hostLock);
    host->state = NET_HOST_SHUTDOWN_REQUESTED;
    mutex_unlock(&host->hostLock);

    // Wait for socket to close
    thread_join(&host->hostThread);

    // Destroy
    enet_host_destroy(host->enetHost);
    mutex_destroy(&host->hostLock);
    buf_spsc_ring_destroy(&host->eventBuffer);
}

int net_udp_host_check_events(net_udp_host_t *host, net_udp_event_t *event) {
    return buf_spsc_ring_pop(&host->eventBuffer, event);
}

int net_udp_service(const net_udp_host_t *host, net_udp_event_t *event, const uint32_t timeout) {
    struct _ENetEvent ev;
    int status = enet_host_service(host->enetHost, &ev, timeout);
    switch (status) {
        default:
        case -1:
        case 0:
            return status;
        case 1: {
            if (ev.type == ENET_EVENT_TYPE_NONE) {
                break;
            }

            event->type = ev.type;
            event->peer = ev.peer;
            event->chanelId = ev.channelID;
            event->data = ev.data;

            // FIXME: Packet pool -- avoid heap alloc
            event->packet = malloc(sizeof(net_udp_packet_t));
            event->packet->_internalPacket = ev.packet;
            if (ev.packet) {
                event->packet->dataLength = ev.packet->dataLength;
                event->packet->data = ev.packet->data;
            }

            return 1;
        }
    }

    return -1;
}

void *net_udp_host_thread(void *arg) {
    net_udp_host_t *host = (net_udp_host_t *) arg;
    uint8_t running;
    net_udp_event_t ev;
    size_t i;
    while (1) {
        mutex_lock(&host->hostLock);
        running = host->threadRunning;
        mutex_unlock(&host->hostLock);

        if (!running) {
            break;
        }

        while (net_udp_service(host, &ev, 100) > 0) {
            buf_spsc_ring_push(&host->eventBuffer, &ev);
        }

        // shutdown process if requested
        mutex_lock(&host->hostLock);

        if (host->state == NET_HOST_SHUTDOWN_REQUESTED) {
            host->state = NET_HOST_SHUTTING_DOWN;
            for (i = 0; i < host->enetHost->peerCount; i++) {
                enet_peer_disconnect_later(&host->enetHost->peers[i], 0); //TODO: Disconnect reasons
            }
            host->shutdownStartTime = time_now_ns();
        }

        if (host->state == NET_HOST_SHUTTING_DOWN) {
            if (host->enetHost->peerCount == 0 ||
                host->shutdownStartTime + NET_HOST_SHUTDOWN_TIMEOUT_NS > time_now_ns()) {
                host->threadRunning = 0;
                host->state = NET_HOST_STOPPED;
                break;
            }
        }

        mutex_unlock(&host->hostLock);
    }

    return NULL;
}

net_udp_peer_t *net_udp_host_client_connect(net_udp_host_t *host) {
    ENetEvent ev;
    if (!host || host->isServer) {
        return NULL;
    }

    host->serverPeer = net_udp_host_connect(host, &host->address, host->channelLimit, 0);
    if (!host->serverPeer) {
        return NULL;
    }

    // Attempt to connect with given timeout
    if (enet_host_service(host->enetHost, &ev, host->connectTimeout) != 1 || ev.type != ENET_EVENT_TYPE_CONNECT) {
        // Connect timeout
        net_udp_peer_reset(host->serverPeer);
        return NULL;
    }

    if (!thread_create(&host->hostThread, net_udp_host_thread, host)) {
        net_udp_peer_disconnect_now(host->serverPeer, 0);
        net_udp_host_destroy(host);
        free(host);
        return NULL;
    }

    return host->serverPeer;
}
