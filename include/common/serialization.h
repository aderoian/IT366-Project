#ifndef SERIALIZATION_H
#define SERIALIZATION_H

#include <stdint.h>

#include "common/types.h"

/**
 * @brief Primitive data serialization and deserialization functions.
 */
void write_uint8_t(buffer_t buffer, buffer_offset_t *offset, uint8_t value);
void write_uint16_t(buffer_t buffer, buffer_offset_t *offset, uint16_t value);
void write_uint32_t(buffer_t buffer, buffer_offset_t *offset, uint32_t value);
void write_uint64_t(buffer_t buffer, buffer_offset_t *offset, uint64_t value);
void write_int8_t(buffer_t buffer, buffer_offset_t *offset, int64_t value);
void write_int16_t(buffer_t buffer, buffer_offset_t *offset, int16_t value);
void write_int32_t(buffer_t buffer, buffer_offset_t *offset, int32_t value);
void write_int64_t(buffer_t buffer, buffer_offset_t *offset, int64_t value);
void write_float(buffer_t buffer, buffer_offset_t *offset, float value);
void write_double(buffer_t buffer, buffer_offset_t *offset, double value);

uint8_t read_uint8_t(buffer_t buffer, buffer_offset_t *offset);
uint16_t read_uint16_t(buffer_t buffer, buffer_offset_t *offset);
uint32_t read_uint32_t(buffer_t buffer, buffer_offset_t *offset);
uint64_t read_uint64_t(buffer_t buffer, buffer_offset_t *offset);
int8_t read_int8_t(buffer_t buffer, buffer_offset_t *offset);
int16_t read_int16_t(buffer_t buffer, buffer_offset_t *offset);
int32_t read_int32_t(buffer_t buffer, buffer_offset_t *offset);
int64_t read_int64_t(buffer_t buffer, buffer_offset_t *offset);
float read_float(buffer_t buffer, buffer_offset_t *offset);
double read_double(buffer_t buffer, buffer_offset_t *offset);

#define SERIALIZE_PRIMITIVE(name, def)                                                          \
static inline void serialize_##name(buffer_t buffer, buffer_offset_t *offset, name##_t value) { \
    write_##def(buffer, offset, value);                                                         \
}

#define DESERIALIZE_PRIMITIVE(name, def) \
static inline void deserialize_##name(   \
buffer_t buffer,                         \
buffer_offset_t *offset,                 \
name##_t *value) {                       \
*value = read_##def(buffer, offset);     \
}

#define SERIALIZE_CUSTOM(name, fields) \
static inline void serialize_##name(   \
buffer_t buffer,                       \
buffer_offset_t *offset,               \
const name##_t *value) {                     \
fields(GEN_SERIALIZE_FIELD)            \
}

#define DESERIALIZE_CUSTOM(name, fields) \
static inline void deserialize_##name(   \
buffer_t buffer,                         \
buffer_offset_t *offset,                 \
name##_t *value) {                       \
fields(GEN_DESERIALIZE_FIELD)            \
}

#define GEN_SERIALIZE_FIELD(type, field_type, field) SERIALIZE_##field_type##_FIELD(type, field_type, field)
#define SERIALIZE_PRIMITIVE_FIELD(type, field_type, field) serialize_##type(buffer, offset, value->field);
#define SERIALIZE_CUSTOM_FIELD(type, field_type, field) serialize_##type(buffer, offset, &value->field);

#define GEN_DESERIALIZE_FIELD(type, field_type, field) DESERIALIZE_##field_type##_FIELD(type, field_type, field)
#define DESERIALIZE_PRIMITIVE_FIELD(type, field_type, field) deserialize_##type(buffer, offset, &value->field);
#define DESERIALIZE_CUSTOM_FIELD(type, field_type, field) deserialize_##type(buffer, offset, &value->field);

#define GEN_SERIALIZE(name, kind, payload) GEN_SERIALIZE_##kind(name, payload)
#define GEN_SERIALIZE_PRIMITIVE(name, def) SERIALIZE_PRIMITIVE(name, def)
#define GEN_SERIALIZE_CUSTOM(name, fields) SERIALIZE_CUSTOM(name, fields)

#define GEN_DESERIALIZE(name, kind, payload) GEN_DESERIALIZE_##kind(name, payload)
#define GEN_DESERIALIZE_PRIMITIVE(name, def) DESERIALIZE_PRIMITIVE(name, def)
#define GEN_DESERIALIZE_CUSTOM(name, fields) DESERIALIZE_CUSTOM(name, fields)

PACKET_TYPE_LIST(GEN_SERIALIZE)
PACKET_TYPE_LIST(GEN_DESERIALIZE)

#endif /* SERIALIZATION_H */