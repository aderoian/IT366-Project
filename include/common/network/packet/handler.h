#ifndef NETWORK_PACKET_HANDLER_H
#define NETWORK_PACKET_HANDLER_H

#include "common/types.h"
#include "common/network/udp.h"
#include "common/network/packet/io.h"

#define PACKET_HANDLER(name, id, fields) \
void handle_##name(const name##_packet_t*, net_udp_peer_t*);

PACKET_LIST(PACKET_HANDLER)
#undef PACKET_HANDLER

#define PACKET_DISPATCH(name, id, fields) \
static void dispatch_##name(packet_buffer_t buf, buffer_offset_t* off, net_udp_peer_t* c) { \
name##_packet_t pkt; \
if (deserialize_##name(buf, off, &pkt)) \
handle_##name(&pkt, c); \
}

PACKET_LIST(PACKET_DISPATCH)
#undef PACKET_DISPATCH

typedef void (*packet_dispatch_fn)(
    packet_buffer_t,
    buffer_offset_t*,
    net_udp_peer_t*
);

extern packet_dispatch_fn packet_dispatch_table[PACKET_COUNT];

#endif /* NETWORK_PACKET_HANDLER_H */