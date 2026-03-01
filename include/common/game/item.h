#ifndef COMMON_ITEM_H
#define COMMON_ITEM_H

#include <stdint.h>

#define ITEM_TYPES(X) \
X(material, MATERIAL) \
X(consumable, CONSUMABLE) \
X(equipment, EQUIPMENT)

#define ITEM_TYPE_ENUM(name, _) ITEM_TYPE_##_,
typedef enum item_type_e {
    ITEM_TYPES(ITEM_TYPE_ENUM)
    ITEM_TYPE_COUNT
} item_type_t;
#undef ITEM_TYPE_ENUM

typedef struct item_def_s {
    uint32_t index;
    char name[32];
    char description[256];
    item_type_t type;
    char sprite[64];
} item_def_t;

typedef struct item_def_manager_s item_def_manager_t;

void item_init(const char *def);

void item_close(void);

const item_def_t *item_def_get(const char *name);

const item_def_t *item_def_get_by_index(int index);

int item_get_count(void);

uint8_t item_compare(const item_def_t *a, const item_def_t *b);

item_type_t item_type_from_string(const char *str);

#endif /* COMMON_ITEM_H */