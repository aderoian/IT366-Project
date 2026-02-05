#include "common/serialization.h"

void write_uint8_t(buffer_t buffer, buffer_offset_t *offset, const uint8_t value) {
    buffer[(*offset)++] = value;
}

void write_uint16_t(buffer_t buffer, buffer_offset_t *offset, const uint16_t value) {
    for (int i = 1; i >= 0; i--) {
        buffer[(*offset)++] = (value >> (i * 8)) & 0xFF;
    }
}

void write_uint32_t(buffer_t buffer, buffer_offset_t *offset, uint32_t value) {
    for (int i = 3; i >= 0; i--) {
        buffer[(*offset)++] = (value >> (i * 8)) & 0xFF;
    }
}

void write_uint64_t(buffer_t buffer, buffer_offset_t *offset, uint64_t value) {
    for (int i = 7; i >= 0; i--) {
        buffer[(*offset)++] = (value >> (i * 8)) & 0xFF;
    }
}

void write_int8_t(buffer_t buffer, buffer_offset_t *offset, int64_t value) {
    buffer[(*offset)++] = (uint8_t)(value & 0xFF);
}

void write_int16_t(buffer_t buffer, buffer_offset_t *offset, int16_t value) {
    for (int i = 1; i >= 0; i--) {
        buffer[(*offset)++] = (uint8_t)((value >> (i * 8)) & 0xFF);
    }
}

void write_int32_t(buffer_t buffer, buffer_offset_t *offset, int32_t value) {
    for (int i = 3; i >= 0; i--) {
        buffer[(*offset)++] = (uint8_t)((value >> (i * 8)) & 0xFF);
    }
}

void write_int64_t(buffer_t buffer, buffer_offset_t *offset, int64_t value) {
    for (int i = 7; i >= 0; i--) {
        buffer[(*offset)++] = (uint8_t)((value >> (i * 8)) & 0xFF);
    }
}

void write_float(buffer_t buffer, buffer_offset_t *offset, float value) {
    union {
        float f;
        uint32_t u;
    } v;

    v.f = value;
    write_uint32_t(buffer, offset, v.u);
}

void write_double(buffer_t buffer, buffer_offset_t *offset, double value) {
    union {
        double d;
        uint64_t u;
    } v;

    v.d = value;
    write_uint64_t(buffer, offset, v.u);
}

uint8_t read_uint8_t(buffer_t buffer, buffer_offset_t *offset) {
    return buffer[(*offset)++];
}

uint16_t read_uint16_t(buffer_t buffer, buffer_offset_t *offset) {
    uint16_t value = 0;
    for (int i = 1; i >= 0; i--) {
        value |= ((uint16_t)buffer[(*offset)++] << (i * 8));
    }
    return value;
}

uint32_t read_uint32_t(buffer_t buffer, buffer_offset_t *offset) {
    uint32_t value = 0;
    for (int i = 3; i >= 0; i--) {
        value |= ((uint32_t)buffer[(*offset)++] << (i * 8));
    }
    return value;
}

uint64_t read_uint64_t(const buffer_t buffer, buffer_offset_t *offset) {
    uint64_t value = 0;
    for (int i = 7; i >= 0; i--) {
        value |= ((uint64_t)buffer[(*offset)++] << (i * 8));
    }
    return value;
}

int8_t read_int8_t(buffer_t buffer, buffer_offset_t *offset) {
    return (int8_t)buffer[(*offset)++];
}

int16_t read_int16_t(buffer_t buffer, buffer_offset_t *offset) {
    int16_t value = 0;
    for (int i = 1; i >= 0; i--) {
        value |= ((uint16_t)buffer[(*offset)++] << (i * 8));
    }
    return value;
}

int32_t read_int32_t(buffer_t buffer, buffer_offset_t *offset) {
    int32_t value = 0;
    for (int i = 3; i >= 0; i--) {
        value |= ((uint32_t)buffer[(*offset)++] << (i * 8));
    }
    return value;
}

int64_t read_int64_t(buffer_t buffer, buffer_offset_t *offset) {
    int64_t value = 0;
    for (int i = 7; i >= 0; i--) {
        value |= ((uint64_t)buffer[(*offset)++] << (i * 8));
    }
    return value;
}

float read_float(buffer_t buffer, buffer_offset_t *offset) {
    union {
        float f;
        uint32_t u;
    } v;

    v.u = read_uint32_t(buffer, offset);
    return v.f;
}

double read_double(buffer_t buffer, buffer_offset_t *offset) {
    union {
        double d;
        uint64_t u;
    } v;

    v.u = read_uint64_t(buffer, offset);
    return v.d;
}