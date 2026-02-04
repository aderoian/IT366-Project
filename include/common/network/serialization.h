#ifndef NETWORK_SERIALIZATION_H
#define NETWORK_SERIALIZATION_H

#include <stdint.h>

#include "common/types.h"

/**
 * @brief Primitive data serialization and deserialization functions.
 */
void write_uint8_t(packet_buffer_t buffer, buffer_offset_t *offset, uint8_t value);
void write_uint16_t(packet_buffer_t buffer, buffer_offset_t *offset, uint16_t value);
void write_uint32_t(packet_buffer_t buffer, buffer_offset_t *offset, uint32_t value);
void write_uint64_t(packet_buffer_t buffer, buffer_offset_t *offset, uint64_t value);
void write_int8_t(packet_buffer_t buffer, buffer_offset_t *offset, int64_t value);
void write_int16_t(packet_buffer_t buffer, buffer_offset_t *offset, int16_t value);
void write_int32_t(packet_buffer_t buffer, buffer_offset_t *offset, int32_t value);
void write_int64_t(packet_buffer_t buffer, buffer_offset_t *offset, int64_t value);
void write_float(packet_buffer_t buffer, buffer_offset_t *offset, float value);
void write_double(packet_buffer_t buffer, buffer_offset_t *offset, double value);

uint8_t read_uint8_t(packet_buffer_t buffer, buffer_offset_t *offset);
uint16_t read_uint16_t(packet_buffer_t buffer, buffer_offset_t *offset);
uint32_t read_uint32_t(packet_buffer_t buffer, buffer_offset_t *offset);
uint64_t read_uint64_t(packet_buffer_t buffer, buffer_offset_t *offset);
int8_t read_int8_t(packet_buffer_t buffer, buffer_offset_t *offset);
int16_t read_int16_t(packet_buffer_t buffer, buffer_offset_t *offset);
int32_t read_int32_t(packet_buffer_t buffer, buffer_offset_t *offset);
int64_t read_int64_t(packet_buffer_t buffer, buffer_offset_t *offset);
float read_float(packet_buffer_t buffer, buffer_offset_t *offset);
double read_double(packet_buffer_t buffer, buffer_offset_t *offset);

#endif /* NETWORK_SERIALIZATION_H */