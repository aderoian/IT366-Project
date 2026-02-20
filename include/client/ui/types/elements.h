#ifndef ELEMENTS_H
#define ELEMENTS_H

#include "common/def.h"

#include "client/gf2d_sprite.h"
#include "client/ui/widget.h"

/**
 * @brief Macro to define UI element types, their enum values, string representations, and JSON loading functions.
 */
#define ELEMENTS(X)       \
X(container, CONTAINER, "container") \
X(button, BUTTON, "button")

// Container element

/**
 * @brief Data structure for a container element, which can hold multiple child elements and has a background sprite.
 */
typedef struct elm_container_data_s {
    Sprite *background;
    widget_t **elements;
    size_t numElements;
} elm_container_data_t;

/**
 * @brief Initializes a container widget with a background sprite and allocates space for child elements.
 *
 * @param widget The widget to be initialized as a container.
 * @param spritePath The file path to the background sprite for the container.
 * @param maxElements The maximum number of child elements that the container can hold.
 * @return 0 on success, -1 on failure.
 */
int elm_container_create(widget_t *widget, const char *spritePath, size_t maxElements);

// Button element

/**
 * @brief Data structure for a button element, which has text and a background sprite.
 */
typedef struct elm_button_data_s {
    char text[64];
    Sprite *background;
} elm_button_data_t;

/**
 * @brief Initializes a button widget with specified text and a background sprite.
 *
 * @param widget The widget to be initialized as a button.
 * @param text The text to be displayed on the button.
 * @param spritePath The file path to the background sprite for the button.
 * @return 0 on success, -1 on failure.
 */
int elm_button_init(widget_t *widget, const char *text, const char *spritePath);


/**
 * Generated information for element types, including enum values, string representations, and JSON loading functions.
 */
#define ENUM_FIELD(name, cap_name, str) ELEMENT_TYPE_##cap_name,
typedef enum element_type_e {
    ELEMENT_TYPE_NONE = 0,
    ELEMENTS(ENUM_FIELD)
} element_type_t;
#undef ENUM_FIELD

#define STRING_FIELD(name, cap_name, str) [ELEMENT_TYPE_##cap_name] = str,
static const char *element_type_strings[] = {
    [ELEMENT_TYPE_NONE] = "none",
    ELEMENTS(STRING_FIELD)
};
#undef STRING_FIELD

#define JSON_FIELD(name, cap_name, str) int elm_##name##_json_load(def_data_t *json, widget_t *widget);
ELEMENTS(JSON_FIELD)
#undef JSON_FIELD

#define JSON_LOAD_FIELD(name, cap_name, str) elm_##name##_json_load,
static int (*element_json_loaders[])(def_data_t *json, widget_t *widget) = {
    [ELEMENT_TYPE_NONE] = NULL,
    ELEMENTS(JSON_LOAD_FIELD)
};

/**
 * @brief Converts a string representation of an element type to its corresponding enum value.
 *
 * @param typeStr The string representation of the element type (e.g., "button").
 * @return The corresponding enum value of the element type, or ELEMENT_TYPE_NONE if not found.
 */
element_type_t element_type_from_string(const char *typeStr);

#endif /* ELEMENTS_H */