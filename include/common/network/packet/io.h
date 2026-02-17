#ifndef NETWORK_PACKET_IO_H
#define NETWORK_PACKET_IO_H

#include "common/network/packet/definitions.h"
#include "common/serialization.h"

#define FIELD(type, name, field_type) field_type##_FIELD(type, name)
#define PRIMITIVE_FIELD(type, name) ,type name
#define CUSTOM_FIELD(type, name) ,type *name
#define PACKET_IO(name, id, fields) \
void serialize_##name(buffer_t, buffer_offset_t*, const name##_packet_t*); \
void deserialize_##name(buffer_t, buffer_offset_t*, name##_packet_t*); \
void create_##name(name##_packet_t* pkt fields(FIELD));

PACKET_LIST(PACKET_IO)

#undef PACKET_IO
#undef PRIMITIVE_FIELD
#undef CUSTOM_FIELD

#define PRIMITIVE_FIELD(type, name) sizeof(type) +
#define CUSTOM_FIELD(type, name) sizeof(type) +
#define PACKET_SIZE(name, id, fields) \
static inline size_t size_of_##name(void) { \
    return fields(FIELD) + sizeof(net_uint8_t); \
}

PACKET_LIST(PACKET_SIZE)

typedef size_t (*size_fn_t)(void);
extern size_fn_t g_packet_sizes[PACKET_COUNT];

#undef PACKET_SIZE
#undef FIELD
#undef PRIMITIVE_FIELD
#undef CUSTOM_FIELD

#endif /* NETWORK_PACKET_IO_H */