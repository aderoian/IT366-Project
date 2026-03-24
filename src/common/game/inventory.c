#include "common/game/inventory.h"

#include "gfc_types.h"
#include "common/game/item.h"

void inventory_init(inventory_t *inventory, uint32_t capacity) {
    inventory->items = gfc_allocate_array(sizeof(item_t), capacity);
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
    item_t *newItems;
    if (!inventory) {
        return;
    }

    if (newCapacity < inventory->capacity) {
        return;
    }

    newItems = gfc_allocate_array(sizeof(item_t), newCapacity);
    memcpy(newItems, inventory->items, sizeof(item_t) * inventory->capacity);
    free(inventory->items);
    inventory->items = newItems;
    inventory->capacity = newCapacity;
}

uint8_t inventory_has_item(const inventory_t *inventory, const item_t *item) {
        uint32_t i;
        if (!inventory) {
            return 0;
        }

        for (i = 0; i < inventory->numItems; i++) {
            if (item_compare(&inventory->items[i], item) && inventory->items[i].quantity >= item->quantity) {
                return 1;
            }
        }

        return 0;
}

void inventory_add_item(inventory_t *inventory, const item_t *item) {
    uint32_t i;
    if (!inventory) {
        return;
    }

    for (i = 0; i < inventory->numItems; i++) {
        if (item_compare(&inventory->items[i], item)) {
            inventory->items[i].quantity += item->quantity;
            return;
        }
    }

    if (inventory->numItems >= inventory->capacity) {
        inventory_resize(inventory, inventory->capacity * 2);
    }

    item_clone_to(item, &inventory->items[inventory->numItems]);
    inventory->numItems++;
}

void inventory_remove_item(inventory_t *inventory, const item_t *item) {
    uint32_t i;
    if (!inventory || !item) {
        return;
    }

    for (i = 0; i < inventory->numItems; i++) {
        if (item_compare(&inventory->items[i], item)) {
            if (inventory->items[i].quantity > item->quantity) {
                inventory->items[i].quantity -= item->quantity;
            } else {
                inventory->items[i].quantity = 0;
            }
            return;
        }
    }
}

const item_t * inventory_get_item(const inventory_t *inventory, const item_def_t *def) {
    uint32_t i;
    if (!inventory || !def) {
        return NULL;
    }

    for (i = 0; i < inventory->numItems; i++) {
        if (inventory->items[i].def == def) {
            return &inventory->items[i];
        }
    }

    return NULL;
}

void inventory_set_item_quantity(inventory_t *inventory, const item_t *item) {
    uint32_t i;
    if (!inventory || !item) {
        return;
    }

    for (i = 0; i < inventory->numItems; i++) {
        if (item_compare(&inventory->items[i], item)) {
            inventory->items[i].quantity = item->quantity;
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

inventory_transaction_t * inventory_transaction_create(uint32_t numItems, uint8_t isAddition) {
    inventory_transaction_t *transaction = gfc_allocate_array(sizeof(inventory_transaction_t), 1);
    if (!transaction) {
        return NULL;
    }

    transaction->items = gfc_allocate_array(sizeof(item_t), numItems);
    transaction->numItems = 0;
    transaction->capacity = numItems;
    transaction->isAddition = isAddition;

    return transaction;
}

void inventory_transaction_destroy(inventory_transaction_t *transaction) {
    if (transaction) {
        if (transaction->items) {
            free(transaction->items);
        }
        free(transaction);
    }
}

void inventory_transaction_add_item(inventory_transaction_t *transaction, const item_t *item) {
    uint32_t newCapacity;
    item_t *newItems;
    if (!transaction || !item) {
        return;
    }

    if (transaction->numItems >= transaction->capacity) {
        newCapacity = transaction->capacity * 2;
        newItems = gfc_allocate_array(sizeof(item_t), newCapacity);
        memcpy(newItems, transaction->items, sizeof(item_t) * transaction->capacity);
        free(transaction->items);
        transaction->items = newItems;
        transaction->capacity = newCapacity;
    }

    item_clone_to(item, &transaction->items[transaction->numItems]);
    transaction->numItems++;
}

int inventory_transaction_try(inventory_t *inventory, const inventory_transaction_t *transaction) {
    uint32_t i;
    if (!inventory || !transaction) {
        return 0;
    }

    for (i = 0; i < transaction->numItems; i++) {
        if (transaction->isAddition) {
            /* For additions, we can always add the item */
            continue;
        }

        /* For removals, check if the inventory has enough of the item */
        if (!inventory_has_item(inventory, &transaction->items[i])) {
            return 0; // Transaction cannot be applied
        }
    }

    return 1; // Transaction can be applied
}

void inventory_transaction_apply(inventory_t *inventory, const inventory_transaction_t *transaction) {
    uint32_t i;
    if (!inventory || !transaction) {
        return;
    }

    for (i = 0; i < transaction->numItems; i++) {
        if (transaction->isAddition) {
            inventory_add_item(inventory, &transaction->items[i]);
        } else {
            inventory_remove_item(inventory, &transaction->items[i]);
        }
    }
}

void inventory_transaction_revert(inventory_t *inventory, const inventory_transaction_t *transaction) {
    uint32_t i;
    if (!inventory || !transaction) {
        return;
    }

    for (i = 0; i < transaction->numItems; i++) {
        if (transaction->isAddition) {
            inventory_remove_item(inventory, &transaction->items[i]);
        } else {
            inventory_add_item(inventory, &transaction->items[i]);
        }
    }
}
