#ifndef COMMON_INVENTORY_H
#define COMMON_INVENTORY_H
#include <stdint.h>

#include "item.h"

struct item_def_s;

typedef struct inventory_s {
    item_t *items;
    uint32_t numItems;
    uint32_t capacity;
} inventory_t;

typedef struct inventory_transaction_s {
    /* items to be added or removed */
    item_t *items;
    /* number of items in the transaction */
    uint32_t numItems;
    /* capacity of the transaction's items array (for resizing) */
    uint32_t capacity;
    /* whether this transaction is an addition (1) or removal (0) */
    uint8_t isAddition;
} inventory_transaction_t;

void inventory_init(inventory_t *inventory, uint32_t capacity);

void inventory_free(inventory_t *inventory);

void inventory_resize(inventory_t *inventory, uint32_t newCapacity);

uint8_t inventory_has_item(const inventory_t *inventory, const item_t *item);

void inventory_add_item(inventory_t *inventory, const item_t *item);

void inventory_remove_item(inventory_t *inventory, const item_t *item);

const item_t *inventory_get_item(const inventory_t *inventory, const item_def_t *def);

void inventory_set_item_quantity(inventory_t *inventory, const item_t *item);

void inventory_clear(inventory_t *inventory);

inventory_transaction_t *inventory_transaction_create(uint32_t numItems, uint8_t isAddition);

void inventory_transaction_destroy(inventory_transaction_t *transaction);

void inventory_transaction_add_item(inventory_transaction_t *transaction, const item_t *item);

int inventory_transaction_try(inventory_t *inventory, const inventory_transaction_t *transaction);

void inventory_transaction_apply(inventory_t *inventory, const inventory_transaction_t *transaction);

void inventory_transaction_revert(inventory_t *inventory, const inventory_transaction_t *transaction);

#endif /* COMMON_INVENTORY_H */