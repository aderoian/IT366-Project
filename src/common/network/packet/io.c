#include "common/network/packet/io.h"

#define FIELD(type, name, field_type) field_type##_FIELD(type, name)
#define PRIMITIVE_FIELD(type, name) serialize_##type(buf, off, pkt->name);
#define CUSTOM_FIELD(type, name) serialize_##type(buf, off, &pkt->name);
#define PACKET_SERIALIZE(name, id, fields) \
void serialize_##name(buffer_t buf, buffer_offset_t* off, const name##_packet_t* pkt) { \
PRIMITIVE_FIELD(net_uint8, packetID) \
fields(FIELD) \
}

PACKET_LIST(PACKET_SERIALIZE)

#undef PRIMITIVE_FIELD
#undef CUSTOM_FIELD
#undef PACKET_SERIALIZE

#define PRIMITIVE_FIELD(type, name) deserialize_##type(buf, off, &pkt->name);
#define CUSTOM_FIELD(type, name) deserialize_##type(buf, off, &pkt->name);
#define PACKET_DESERIALIZE(name, id, fields) \
void deserialize_##name(buffer_t buf, buffer_offset_t* off, name##_packet_t* pkt) { \
PRIMITIVE_FIELD(net_uint8, packetID) \
fields(FIELD) \
}

PACKET_LIST(PACKET_DESERIALIZE)

#undef FIELD
#undef PACKET_DESERIALIZE
#undef PRIMITIVE_FIELD
#undef CUSTOM_FIELD

#define FIELD(type, name, field_type) field_type##_FIELD(type, name)
#define PRIMITIVE_FIELD(type, name) ,type name
#define CUSTOM_FIELD(type, name) ,type##_t *name

#define INIT_FIELD(type, name, field_type) field_type##_INIT_FIELD(type, name)
#define PRIMITIVE_INIT_FIELD(type, name) pkt->name = name;
#define CUSTOM_INIT_FIELD(type, name) pkt->name = *name;
#define PACKET_CREATE(name, id, fields) \
void create_##name(name##_packet_t* pkt fields(FIELD)) { \
pkt->packetID = PACKET_##id; \
fields(INIT_FIELD) \
}

PACKET_LIST(PACKET_CREATE)

#undef FIELD
#undef PRIMITIVE_FIELD
#undef CUSTOM_FIELD
#undef INIT_FIELD
#undef PRIMITIVE_INIT_FIELD
#undef CUSTOM_INIT_FIELD
#undef PACKET_CREATE