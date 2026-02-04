#ifndef NETWORK_PACKET_IO_H
#define NETWORK_PACKET_IO_H

#include "common/network/packet/definitions.h"
#include "common/network/serialization.h"

#define PACKET_IO(name, id, fields) \
void serialize_##name(packet_buffer_t, buffer_offset_t*, const name##_packet_t*); \
int deserialize_##name(packet_buffer_t, buffer_offset_t*, name##_packet_t*);

PACKET_LIST(PACKET_IO)
#undef PACKET_IO

#endif /* NETWORK_PACKET_IO_H */