#ifndef WIDGIT_H
#define WIDGIT_H

#include <stdint.h>

#include "gfc_shape.h"
#include "gfc_vector.h"

struct window_s;
struct window_event_s;

/**
 * Widget structure representing a UI element.
 */
typedef struct widget_s {
    char id[32];
    const struct window_s *parent;
    GFC_Rect rect;
    struct widget_s *child;
    uint32_t _refCount;
    uint8_t visible;
    uint8_t type;
    void (*draw)(struct widget_s *widget);
    void (*update)(struct widget_s *widget, float deltaTime);
    int (*handle_event)(struct widget_s *widget, const struct window_event_s *event);
    void (*destroy)(struct widget_s *widget);
    void *data;
} widget_t;

/**
 * @brief Initializes the widget system with a specified maximum number of widgets.
 *
 * @param maxWidgets The maximum number of widgets that can be created.
 */
void widget_init(size_t maxWidgets);

/**
 * @brief Closes the widget system and frees all associated resources.
 */
void widget_close(void);

/**
 * @brief Creates a new widget with the specified name.
 *
 * @param name The unique identifier for the widget.
 * @return A pointer to the newly created widget, or NULL if creation failed.
 */
widget_t *widget_create(const char *name);

/**
 * @brief Destroys a widget and frees its resources.
 *
 * @param widget The widget to be destroyed.
 */
void widget_destroy(widget_t *widget);

/**
 * @brief Sets the child widget for a parent widget.
 *
 * @param parent The parent widget to which the child will be attached.
 * @param child The child widget to be attached to the parent.
 */
void widget_set_child(widget_t *parent, widget_t *child);

/**
 * @brief Sets the position of a widget.
 *
 * @param widget The widget whose position is to be set.
 * @param position The new position for the widget.
 */
void widget_set_position(widget_t *widget, GFC_Vector2D position);

/**
 * @brief Sets the offset of a widget.
 *
 * @param widget The widget whose offset is to be set.
 * @param offset The new offset for the widget.
 */
void widget_set_offset(widget_t *widget, GFC_Vector2D offset);

/**
 * @brief Sets the size of a widget.
 *
 * @param widget The widget whose size is to be set.
 * @param width The new width for the widget.
 * @param height The new height for the widget.
 */
void widget_set_size(widget_t *widget, int width, int height);

/**
 * @brief Sets the visibility of a widget.
 *
 * @param widget The widget whose visibility is to be set.
 * @param visible A boolean value indicating whether the widget should be visible (1) or hidden (0).
 */
void widget_set_visible(widget_t *widget, uint8_t visible);

/**
 * @brief Draws a widget and its child widgets recursively.
 *
 * @param widget The widget to be drawn.
 */
void widget_draw(widget_t *widget);

/**
 * @brief Updates a widget and its child widgets recursively.
 *
 * @param widget The widget to be updated.
 * @param deltaTime The time elapsed since the last update, in seconds.
 */
void widget_update(widget_t *widget, float deltaTime);

/**
 * @brief Handles an event for a widget and its child widgets recursively.
 *
 * @param widget The widget to handle the event.
 * @param event The event to be handled.
 * @return 1 if the event was handled by the widget or its children, 0 otherwise.
 */
int widget_handle_event(widget_t *widget, const struct window_event_s *event);

/**
 * @brief Loads a widget from a JSON definition.
 *
 * @param json The JSON data containing the widget definition.
 * @param parent The parent widget to which the loaded widget will be attached.
 * @return A pointer to the loaded widget, or NULL if loading failed.
 */
widget_t *widget_load_from_json(def_data_t *json, widget_t *parent);

#endif /* WIDGIT_H */