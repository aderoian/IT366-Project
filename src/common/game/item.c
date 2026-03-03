#include "common/def.h"
#include "common/game/item.h"

struct item_def_manager_s {
    item_def_t *itemDefs;
    size_t numItemDefs;
};

item_def_manager_t *item_init(const struct def_manager_s *defManager, const char *defFile) {
    item_def_manager_t *manager;
    def_data_t *data, *itemsDef, *itemDef;
    item_def_t *item;
    const char *str;
    int size;
    size_t i;
    if (!defFile) {
        return NULL;
    }

    data = def_load(defManager, defFile);
    if (!data) {
        return NULL;
    }

    itemsDef = def_data_get_array(data, "items");
    if (!itemsDef) {
        return NULL;
    }

    manager = gfc_allocate_array(sizeof(item_def_manager_t), 1);
    if (!manager) {
        return NULL;
    }

    def_data_array_get_count(itemsDef, &size);
    manager->numItemDefs = size;
    manager->itemDefs = gfc_allocate_array(sizeof(item_def_t), manager->numItemDefs);

    for (i = 0; i < manager->numItemDefs; i++) {
        itemDef = def_data_array_get_nth(itemsDef, i);
        item = &manager->itemDefs[i];

        str = def_data_get_string(itemDef, "name");
        memcpy(item->name, str, sizeof(item->name) - 1);
        item->name[sizeof(item->name) - 1] = '\0';

        str = def_data_get_string(itemDef, "description");
        memcpy(item->description, str, sizeof(item->description) - 1);
        item->description[sizeof(item->description) - 1] = '\0';
        
        str = def_data_get_string(itemDef, "type");
        item->type = item_type_from_string(str);

        str = def_data_get_string(itemDef, "sprite");
        memcpy(item->sprite, str, sizeof(item->sprite) - 1);
        item->sprite[sizeof(item->sprite) - 1] = '\0';

        item->index = i;
    }

    return manager;
}

void item_close(item_def_manager_t *manager) {
    if (manager->itemDefs) {
        free(manager->itemDefs);
        manager->itemDefs = NULL;
        manager->numItemDefs = 0;
    }
}

const item_def_t * item_def_get(const item_def_manager_t *manager, const char *name) {
    size_t i;
    for (i = 0; i < manager->numItemDefs; i++) {
        if (strcmp(manager->itemDefs[i].name, name) == 0) {
            return &manager->itemDefs[i];
        }
    }
    return NULL; // Not found
}

const item_def_t * item_def_get_by_index(const item_def_manager_t *manager, const size_t index) {
    if (index >= manager->numItemDefs) {
        return NULL; // Index out of bounds
    }
    return &manager->itemDefs[index];
}

int item_get_count(const item_def_manager_t *manager) {
    return manager->numItemDefs;
}

uint8_t item_compare(const item_def_t *a, const item_def_t *b) {
    if (!a || !b) {
        return 0; // Consider NULL items as not equal
    }
    return a->index == b->index;
}

#define ITEM_TYPE_STRING(name, enumVal) \
    if (strcmp(str, #name) == 0) { \
        return ITEM_TYPE_##enumVal; \
    }
item_type_t item_type_from_string(const char *str) {
    ITEM_TYPES(ITEM_TYPE_STRING)
    return ITEM_TYPE_COUNT; // Invalid type
}
#undef ITEM_TYPE_STRING
