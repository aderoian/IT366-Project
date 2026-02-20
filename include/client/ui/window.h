#ifndef WINDOW_H
#define WINDOW_H

#include <stddef.h>
#include <stdint.h>

#include "common/def.h"

#include "client/ui/event.h"
#include "client/ui/widget.h"

#define MAX_TITLE_LENGTH 64

/**
 * Window structure representing a UI window.
 */
typedef struct window_s {
    struct window_s *parent;
    int x, y, width, height;
    uint32_t _refCount;
    char title[MAX_TITLE_LENGTH];
    widget_t *root;
    uint8_t visible;
    void (*draw)(struct window_s *window);
    void (*update)(struct window_s *window, float deltaTime);
    int (*handle_event)(struct window_s *window, const window_event_t *event);

    void (*on_button_click)(struct window_s *window, const char *elementId);
} window_t;

/**
 * @brief Window manager structure to manage multiple windows.
 */
void window_init(size_t maxWindows, size_t maxWidgets, size_t maxEvents);

/**
 * @brief Closes the window system and frees all associated resources.
 */
void window_close(void);

/**
 * @brief Creates a new window with the specified parameters.
 *
 * @param x The x-coordinate of the window's position.
 * @param y The y-coordinate of the window's position.
 * @param width The width of the window.
 * @param height The height of the window.
 * @param title The title of the window.
 * @return A pointer to the newly created window, or NULL if creation failed.
 */
window_t *window_create(int x, int y, int width, int height, const char *title);

/**
 * @brief Destroys a window and frees its resources.
 *
 * @param window The window to be destroyed.
 */
void window_destroy(window_t *window);

/**
 * @brief Shows a window and makes it the active window.
 *
 * @param window The window to be shown.
 */
void window_show(window_t *window);

/**
 * @brief Hides a window and deactivates it if it is the active window.
 *
 * @param window The window to be hidden.
 */
void window_hide(window_t *window);

/**
 * @brief Draws a window and its contents.
 *
 * @param window The window to be drawn.
 */
void window_draw(window_t *window);

/**
 * @brief Updates a window and its contents based on the elapsed time.
 *
 * @param window The window to be updated.
 * @param deltaTime The time elapsed since the last update, in seconds.
 */
void window_update(window_t *window, float deltaTime);

/**
 * @brief Handles an event for a window and its contents.
 *
 * @param window The window to handle the event for.
 * @param event The event to be handled.
 * @return 1 if the event was handled, 0 otherwise.
 */
int window_handle_event(window_t *window, const window_event_t *event);

/**
 * @brief Draws all active windows.
 */
void window_draw_all(void);

/**
 * @brief Updates all active windows based on the elapsed time.
 *
 * @param deltaTime The time elapsed since the last update, in seconds.
 */
void window_update_all(float deltaTime);

/**
 * @brief Handles all pending events for the active windows.
 */
void window_handle_event_all(void);

/**
 * @brief Loads a window from a JSON definition.
 *
 * @param json The JSON data containing the window definition.
 * @return A pointer to the newly created window, or NULL if loading failed.
 */
window_t *window_load_from_json(def_data_t *json);


#endif /* WINDOW_H */