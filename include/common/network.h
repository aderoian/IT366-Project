#ifndef NETWORK_H
#define NETWORK_H

#include "enet/enet.h"

void network_init(const char *address, uint16_t port);
void network_shutdown(void);

void network_sendPacket(ENetPeer *peer, const void *data, size_t dataSize, enet_uint32 flags);

ENetHost* network_createServer(uint16_t port, size_t maxClients);
void network_destroyServer(ENetHost* server);

ENetHost* network_createClient(uint16_t port, size_t maxClients);
ENetPeer* network_connect_to_server(const char *address, uint16_t port);

#endif /* NETWORK_H */