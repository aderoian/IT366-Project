#ifndef COMMON_INVENTORY_H
#define COMMON_INVENTORY_H
#include <stdint.h>

struct item_def_s;

typedef struct inventory_item_s {
    const struct item_def_s *def;
    uint32_t quantity;
} inventory_item_t;

typedef struct inventory_s {
    inventory_item_t *items;
    uint32_t numItems;
    uint32_t capacity;
} inventory_t;

void inventory_init(inventory_t *inventory, uint32_t capacity);

void inventory_free(inventory_t *inventory);

void inventory_resize(inventory_t *inventory, uint32_t newCapacity);

uint8_t inventory_has_item(const inventory_t *inventory, const struct item_def_s *itemDef, uint32_t quantity);

void inventory_add_item(inventory_t *inventory, const struct item_def_s *itemDef, uint32_t quantity);

void inventory_remove_item(inventory_t *inventory, const struct item_def_s *itemDef, uint32_t quantity);

void inventory_set_item_quantity(inventory_t *inventory, const struct item_def_s *itemDef, uint32_t quantity);

void inventory_clear(inventory_t *inventory);

#endif /* COMMON_INVENTORY_H */