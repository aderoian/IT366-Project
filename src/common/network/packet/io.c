#include "common/network/packet/io.h"

#define FIELD(type, name, field_type) field_type##_FIELD(type, name)
#define PRIMITIVE_FIELD(type, name) serialize_##type(buf, off, pkt->name);
#define CUSTOM_FIELD(type, name) serialize_##type(buf, off, &pkt->name);
#define LIST_PRIMITIVE_FIELD(type, name) serialize_##type(buf, off, &pkt->name);
#define LIST_CUSTOM_FIELD(type, name) serialize_##type(buf, off, &pkt->name);
#define PACKET_SERIALIZE(name, id, fields) \
void serialize_##name(buffer_t buf, buffer_offset_t* off, const name##_packet_t* pkt) { \
PRIMITIVE_FIELD(net_uint8_t, packetID) \
PRIMITIVE_FIELD(net_uint64_t, length) \
fields(FIELD) \
}

PACKET_LIST(PACKET_SERIALIZE)

#undef PRIMITIVE_FIELD
#undef CUSTOM_FIELD
#undef LIST_PRIMITIVE_FIELD
#undef LIST_CUSTOM_FIELD
#undef PACKET_SERIALIZE

#define PRIMITIVE_FIELD(type, name) deserialize_##type(buf, off, &pkt->name);
#define CUSTOM_FIELD(type, name) deserialize_##type(buf, off, &pkt->name);
#define LIST_PRIMITIVE_FIELD(type, name) deserialize_##type(buf, off, &pkt->name);
#define LIST_CUSTOM_FIELD(type, name) deserialize_##type(buf, off, &pkt->name);
#define PACKET_DESERIALIZE(name, id, fields) \
void deserialize_##name(buffer_t buf, buffer_offset_t* off, name##_packet_t* pkt) { \
PRIMITIVE_FIELD(net_uint8_t, packetID) \
PRIMITIVE_FIELD(net_uint64_t, length) \
fields(FIELD) \
}

PACKET_LIST(PACKET_DESERIALIZE)

#undef FIELD
#undef PACKET_DESERIALIZE
#undef PRIMITIVE_FIELD
#undef CUSTOM_FIELD
#undef LIST_PRIMITIVE_FIELD
#undef LIST_CUSTOM_FIELD

#define FIELD(type, name, field_type) field_type##_FIELD(type, name)
#define PRIMITIVE_FIELD(type, name) ,type name
#define CUSTOM_FIELD(type, name) ,type *name
#define LIST_PRIMITIVE_FIELD(type, name) ,type *name
#define LIST_CUSTOM_FIELD(type, name) ,type *name

#define INIT_FIELD(type, name, field_type) field_type##_INIT_FIELD(type, name)
#define PRIMITIVE_INIT_FIELD(type, name) pkt->name = name; \
    pkt->length += sizeof(name);
#define CUSTOM_INIT_FIELD(type, name) pkt->name = *name; \
    pkt->length += sizeof(*name);

#define LIST_PRIMITIVE_INIT_FIELD(type, name) \
    pkt->name.elements = malloc(sizeof(*((type *)0)->elements) * name->count); \
    pkt->name.count = name->count; \
    for (size_t i = 0; i < name->count; ++i) { \
        pkt->name.elements[i] = name->elements[i]; \
    } \
    pkt->length += sizeof(uint64_t) + sizeof(*((type *)0)->elements) * name->count;

#define LIST_CUSTOM_INIT_FIELD(type, name) \
pkt->name.elements = malloc(sizeof(*((type *)0)->elements) * name->count); \
pkt->name.count = name->count; \
for (size_t i = 0; i < name->count; ++i) { \
pkt->name.elements[i] = name->elements[i]; \
} \
pkt->length += sizeof(uint64_t) + sizeof(*((type *)0)->elements) * name->count;

#define PACKET_CREATE(name, id, fields) \
void create_##name(name##_packet_t* pkt fields(FIELD)) { \
    pkt->packetID = PACKET_##id; \
    pkt->length = 0; \
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