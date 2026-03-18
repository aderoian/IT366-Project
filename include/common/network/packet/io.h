#ifndef NETWORK_PACKET_IO_H
#define NETWORK_PACKET_IO_H

#include "common/network/packet/definitions.h"
#include "common/serialization.h"

#define FIELD(type, name, field_type) field_type##_FIELD(type, name)
#define PRIMITIVE_FIELD(type, name) ,type name
#define CUSTOM_FIELD(type, name) ,type *name
#define LIST_PRIMITIVE_FIELD(type, name) ,type *name
#define LIST_CUSTOM_FIELD(type, name) ,type *name
#define PACKET_IO(name, id, fields) \
void serialize_##name(buffer_t, buffer_offset_t*, const name##_packet_t*); \
void deserialize_##name(buffer_t, buffer_offset_t*, name##_packet_t*); \
void create_##name(name##_packet_t* pkt fields(FIELD));

PACKET_LIST(PACKET_IO)

#undef PACKET_IO
#undef PRIMITIVE_FIELD
#undef CUSTOM_FIELD
#undef LIST_PRIMITIVE_FIELD
#undef LIST_CUSTOM_FIELD

#endif /* NETWORK_PACKET_IO_H */