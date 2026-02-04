#include "common/network/serialization.h"

#include "common/network/packet/io.h"

void write_uint8_t(packet_buffer_t buffer, buffer_offset_t *offset, const uint8_t value) {
    buffer[(*offset)++] = value;
}

void write_uint16_t(packet_buffer_t buffer, buffer_offset_t *offset, const uint16_t value) {
    for (int i = 1; i >= 0; i--) {
        buffer[(*offset)++] = (value >> (i * 8)) & 0xFF;
    }
}

void write_uint32_t(packet_buffer_t buffer, buffer_offset_t *offset, uint32_t value) {
    for (int i = 3; i >= 0; i--) {
        buffer[(*offset)++] = (value >> (i * 8)) & 0xFF;
    }
}

void write_uint64_t(packet_buffer_t buffer, buffer_offset_t *offset, uint64_t value) {
    for (int i = 7; i >= 0; i--) {
        buffer[(*offset)++] = (value >> (i * 8)) & 0xFF;
    }
}

void write_int8_t(packet_buffer_t buffer, buffer_offset_t *offset, int64_t value) {
    buffer[(*offset)++] = (uint8_t)(value & 0xFF);
}

void write_int16_t(packet_buffer_t buffer, buffer_offset_t *offset, int16_t value) {
    for (int i = 1; i >= 0; i--) {
        buffer[(*offset)++] = (uint8_t)((value >> (i * 8)) & 0xFF);
    }
}

void write_int32_t(packet_buffer_t buffer, buffer_offset_t *offset, int32_t value) {
    for (int i = 3; i >= 0; i--) {
        buffer[(*offset)++] = (uint8_t)((value >> (i * 8)) & 0xFF);
    }
}

void write_int64_t(packet_buffer_t buffer, buffer_offset_t *offset, int64_t value) {
    for (int i = 7; i >= 0; i--) {
        buffer[(*offset)++] = (uint8_t)((value >> (i * 8)) & 0xFF);
    }
}

void write_float(packet_buffer_t buffer, buffer_offset_t *offset, float value) {
    union {
        float f;
        uint32_t u;
    } v;

    v.f = value;
    write_uint32_t(buffer, offset, v.u);
}

void write_double(packet_buffer_t buffer, buffer_offset_t *offset, double value) {
    union {
        double d;
        uint64_t u;
    } v;

    v.d = value;
    write_uint64_t(buffer, offset, v.u);
}

uint8_t read_uint8_t(packet_buffer_t buffer, buffer_offset_t *offset) {
    return buffer[(*offset)++];
}

uint16_t read_uint16_t(packet_buffer_t buffer, buffer_offset_t *offset) {
    uint16_t value = 0;
    for (int i = 1; i >= 0; i--) {
        value |= ((uint16_t)buffer[(*offset)++] << (i * 8));
    }
    return value;
}

uint32_t read_uint32_t(packet_buffer_t buffer, buffer_offset_t *offset) {
    uint32_t value = 0;
    for (int i = 3; i >= 0; i--) {
        value |= ((uint32_t)buffer[(*offset)++] << (i * 8));
    }
    return value;
}

uint64_t read_uint64_t(const packet_buffer_t buffer, buffer_offset_t *offset) {
    uint64_t value = 0;
    for (int i = 7; i >= 0; i--) {
        value |= ((uint64_t)buffer[(*offset)++] << (i * 8));
    }
    return value;
}

int8_t read_int8_t(packet_buffer_t buffer, buffer_offset_t *offset) {
    return (int8_t)buffer[(*offset)++];
}

int16_t read_int16_t(packet_buffer_t buffer, buffer_offset_t *offset) {
    int16_t value = 0;
    for (int i = 1; i >= 0; i--) {
        value |= ((uint16_t)buffer[(*offset)++] << (i * 8));
    }
    return value;
}

int32_t read_int32_t(packet_buffer_t buffer, buffer_offset_t *offset) {
    int32_t value = 0;
    for (int i = 3; i >= 0; i--) {
        value |= ((uint32_t)buffer[(*offset)++] << (i * 8));
    }
    return value;
}

int64_t read_int64_t(packet_buffer_t buffer, buffer_offset_t *offset) {
    int64_t value = 0;
    for (int i = 7; i >= 0; i--) {
        value |= ((uint64_t)buffer[(*offset)++] << (i * 8));
    }
    return value;
}

float read_float(packet_buffer_t buffer, buffer_offset_t *offset) {
    union {
        float f;
        uint32_t u;
    } v;

    v.u = read_uint32_t(buffer, offset);
    return v.f;
}

double read_double(packet_buffer_t buffer, buffer_offset_t *offset) {
    union {
        double d;
        uint64_t u;
    } v;

    v.u = read_uint64_t(buffer, offset);
    return v.d;
}

#define FIELD(type, name) write_##type(buf, off, pkt->name);

#define PACKET_SERIALIZE(name, id, fields) \
void serialize_##name(packet_buffer_t buf, buffer_offset_t* off, const name##_packet_t* pkt) { \
write_int8_t(buf, off, PACKET_##id); \
fields(FIELD) \
}

PACKET_LIST(PACKET_SERIALIZE)
#undef FIELD
#undef PACKET_SERIALIZE

#define FIELD(type, name) pkt->name = read_##type(buf, off);

#define PACKET_DESERIALIZE(name, id, fields) \
int deserialize_##name(packet_buffer_t buf, buffer_offset_t* off, name##_packet_t* pkt) { \
FIELD(uint8_t, packetID) \
fields(FIELD) \
return 1; \
}

PACKET_LIST(PACKET_DESERIALIZE)

#undef FIELD
#undef PACKET_DESERIALIZE

#define FIELD(type, name) ,type name
#define INIT_FIELD(type, name) pkt->name = name;

#define PACKET_CREATE(name, id, fields) \
void create_##name(name##_packet_t* pkt fields(FIELD)) { \
pkt->packetID = PACKET_##id; \
fields(INIT_FIELD) \
}

PACKET_LIST(PACKET_CREATE)
#undef FIELD
#undef INIT_FIELD
#undef PACKET_CREATE