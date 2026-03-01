#include "common/def.h"
#include "common/game/item.h"

struct item_def_manager_s {
    item_def_t *itemDefs;
    int numItemDefs;
};

static item_def_manager_t g_itemDefManager = {0};

void item_init(const char *def) {
    def_data_t *data, *itemsDef, *itemDef;
    item_def_t *item;
    const char *str;
    int i;
    if (!def) {
        return;
    }

    data = def_load(def);
    if (!data) {
        return;
    }

    itemsDef = def_data_get_array(data, "items");
    if (!itemsDef) {
        return;
    }

    def_data_array_get_count(itemsDef, &g_itemDefManager.numItemDefs);
    g_itemDefManager.itemDefs = gfc_allocate_array(sizeof(item_def_t), g_itemDefManager.numItemDefs);

    for (i = 0; i < g_itemDefManager.numItemDefs; i++) {
        itemDef = def_data_array_get_nth(itemsDef, i);
        item = &g_itemDefManager.itemDefs[i];

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
}

void item_close(void) {
    if (g_itemDefManager.itemDefs) {
        free(g_itemDefManager.itemDefs);
        g_itemDefManager.itemDefs = NULL;
        g_itemDefManager.numItemDefs = 0;
    }
}

const item_def_t * item_def_get(const char *name) {
    int i;
    for (i = 0; i < g_itemDefManager.numItemDefs; i++) {
        if (strcmp(g_itemDefManager.itemDefs[i].name, name) == 0) {
            return &g_itemDefManager.itemDefs[i];
        }
    }
    return NULL; // Not found
}

const item_def_t * item_def_get_by_index(int index) {
    if (index < 0 || index >= g_itemDefManager.numItemDefs) {
        return NULL; // Index out of bounds
    }
    return &g_itemDefManager.itemDefs[index];
}

int item_get_count(void) {
    return g_itemDefManager.numItemDefs;
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
