#include "common/game/inventory.h"

#include "gfc_types.h"
#include "common/game/item.h"

void inventory_init(inventory_t *inventory, uint32_t capacity) {
    inventory->items = gfc_allocate_array(sizeof(inventory_item_t), capacity);
    inventory->numItems = 0;
    inventory->capacity = capacity;
}

void inventory_free(inventory_t *inventory) {
    if (inventory->items) {
        free(inventory->items);
        inventory->items = NULL;
    }
    inventory->numItems = 0;
    inventory->capacity = 0;
}

void inventory_resize(inventory_t *inventory, const uint32_t newCapacity) {
    inventory_item_t *newItems;
    if (!inventory) {
        return;
    }

    if (newCapacity < inventory->capacity) {
        return;
    }

    newItems = gfc_allocate_array(sizeof(inventory_item_t), newCapacity);
    memcpy(newItems, inventory->items, sizeof(inventory_item_t) * inventory->capacity);
    free(inventory->items);
    inventory->items = newItems;
    inventory->capacity = newCapacity;
}

uint8_t inventory_has_item(const inventory_t *inventory, const struct item_def_s *itemDef, const uint32_t quantity) {
        uint32_t i;
        if (!inventory || !itemDef) {
            return 0;
        }

        for (i = 0; i < inventory->numItems; i++) {
            if (item_compare(inventory->items[i].def, itemDef) && inventory->items[i].quantity >= quantity) {
                return 1;
            }
        }

        return 0;
}

void inventory_add_item(inventory_t *inventory, const struct item_def_s *itemDef, const uint32_t quantity) {
    uint32_t i;
    if (!inventory || !itemDef || quantity == 0) {
        return;
    }

    for (i = 0; i < inventory->numItems; i++) {
        if (item_compare(inventory->items[i].def, itemDef)) {
            inventory->items[i].quantity += quantity;
            return;
        }
    }

    if (inventory->numItems >= inventory->capacity) {
        inventory_resize(inventory, inventory->capacity * 2);
    }

    inventory->items[inventory->numItems].def = itemDef;
    inventory->items[inventory->numItems].quantity = quantity;
    inventory->numItems++;
}

void inventory_remove_item(inventory_t *inventory, const struct item_def_s *itemDef, const uint32_t quantity) {
    uint32_t i;
    if (!inventory || !itemDef || quantity == 0) {
        return;
    }

    for (i = 0; i < inventory->numItems; i++) {
        if (item_compare(inventory->items[i].def, itemDef)) {
            if (inventory->items[i].quantity > quantity) {
                inventory->items[i].quantity -= quantity;
            } else {
                inventory->items[i].quantity = 0;
            }
            return;
        }
    }
}

void inventory_set_item_quantity(inventory_t *inventory, const struct item_def_s *itemDef, const uint32_t quantity) {
    uint32_t i;
    if (!inventory || !itemDef) {
        return;
    }

    for (i = 0; i < inventory->numItems; i++) {
        if (item_compare(inventory->items[i].def, itemDef)) {
            inventory->items[i].quantity = quantity;
            return;
        }
    }
}

void inventory_clear(inventory_t *inventory) {
    if (!inventory) {
        return;
    }

    inventory->numItems = 0;
}
