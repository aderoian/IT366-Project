#ifndef TYPES_H
#define TYPES_H

#include <stddef.h>
#include <stdint.h>

typedef uint8_t *buffer_t;
typedef size_t buffer_offset_t;

/**
 * @brief Generates a list of all  types that may be serialized/deserialized.
 * @param X Generation function which takes (name, type, def) as parameters.
 *
 * Packet List Entry:
 * name - the name of the type (omitting the _t suffix)
 * type - either PRIMITIVE or CUSTOM
 * def  - for PRIMITIVE types, the underlying C type; for CUSTOM types, the list of fields (field generation macro)
 *
 * Field List:
 * A field list defines all the fields in a CUSTOM type. Each field can either be PRIMITIVE or CUSTOM.
 * Each field entry takes (type, type_kind, name) as parameters.
 * type - the type of the field (omitting the _t suffix)
 * type_kind - either PRIMITIVE or CUSTOM
 * name - the name of the field
 *
 * Note: if a CUSTOM type contains another CUSTOM type as a field, that type must be defined earlier in the PACKET_TYPE_LIST.
 *
 * Example:
 * Primitive Type: X(net_uint32, PRIMITIVE, uint32_t)
 *
 * Custom Type:
 * X(vector3, CUSTOM, VECTOR3_FIELDS)
 * where VECTOR3_FIELDS is defined as:
 * #define VECTOR3_FIELDS(F) \
 * F(net_float, PRIMITIVE, x) \
 * F(net_float, PRIMITIVE, y) \
 * F(net_float, PRIMITIVE, z)
 *
 * X(player_state, CUSTOM, EXAMPLE_NESTED_FIELDS)
 * where PLAYER_STATE_FIELDS is defined as:
 * #define PLAYER_STATE_FIELDS(F) \
 * F(net_uint64, PRIMITIVE, playerID) \
 * F(vector3, CUSTOM, position) \
 * F(vector3, CUSTOM, velocity)
 */
#define PACKET_TYPE_LIST(X)         \
X(net_uint8_t,  PRIMITIVE, uint8_t )  \
X(net_uint16_t, PRIMITIVE, uint16_t)  \
X(net_uint32_t, PRIMITIVE, uint32_t)  \
X(net_uint64_t, PRIMITIVE, uint64_t)  \
X(net_int8_t,   PRIMITIVE, int8_t )   \
X(net_int16_t,  PRIMITIVE, int16_t)   \
X(net_int32_t,  PRIMITIVE, int32_t)   \
X(net_int64_t,  PRIMITIVE, int64_t)   \
X(net_float_t,  PRIMITIVE, float )    \
X(net_double_t, PRIMITIVE, double)    \
\
X(player_input_command_t, CUSTOM, PLAYER_INPUT_COMMAND_FIELDS)

#define PLAYER_INPUT_COMMAND_FIELDS(F)     \
F(net_uint64_t, PRIMITIVE, tickNumber) \
F(net_int32_t,  PRIMITIVE, axisX)          \
F(net_int32_t,  PRIMITIVE, axisY)

/**
 * @internal Below are macros to generate type definitions based on the PACKET_TYPE_LIST.
 */

/**
 * Generates the struct definition for a CUSTOM type.
 */
#define FIELD(type, _, name) type name;
#define CUSTOM_STRUCT_DEF(name, fields) \
struct name##_s { \
fields(FIELD) \
}

/**
 * Generates the type definition for a type based on its kind (PRIMITIVE or CUSTOM).
 */
#define GEN_TYPE_DEF(name, type, def) type##_TYPE_DEF(name, def)
#define PRIMITIVE_TYPE_DEF(name, def) typedef def name;
#define CUSTOM_TYPE_DEF(name, def) typedef CUSTOM_STRUCT_DEF(name, def) name;

/**
 * Generate all type definitions.
 */
PACKET_TYPE_LIST(GEN_TYPE_DEF)

#undef FIELD
#undef GEN_TYPE_DEF
#undef PRIMITIVE_TYPE_DEF
#undef CUSTOM_TYPE_DEF


typedef enum {
 TYPE_PRIMITIVE,
 TYPE_CUSTOM
} type_kind_t;

#define GEN_TYPE_TRAIT(name, kind, def) \
enum { name##_TYPE_KIND = TYPE_##kind };
PACKET_TYPE_LIST(GEN_TYPE_TRAIT)

#undef GEN_TYPE_TRAIT

#endif /* TYPES_H */