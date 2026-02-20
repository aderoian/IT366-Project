#include <stddef.h>
#include <string.h>

#include "gfc_types.h"

#include "client/ui/window.h"

#include "simple_logger.h"
#include "client/ui/widget.h"

typedef struct window_manager_s {
    window_t *active;
    window_t *windows;
    size_t maxWindows;
} window_manager_t;

window_manager_t g_windowManager;

void basic_window_draw(window_t *window) {
    if (!window) {
        return;
    }

    if (window->root) {
        widget_draw(window->root);
    }
}

void basic_window_update(window_t *window, float deltaTime) {
    if (!window) {
        return;
    }

    if (window->root) {
        widget_update(window->root, deltaTime);
    }
}

int basic_window_handle_event(window_t *window, const window_event_t *event) {
    if (!window || !event) {
        return 0;
    }

    if (window->root) {
        return widget_handle_event(window->root, event);
    }

    return 0;
}

void window_init(const size_t maxWindows, const size_t maxWidgets, const size_t maxEvents) {
    event_buffer_init(maxEvents);
    widget_init(maxWidgets);

    g_windowManager.active = NULL;
    g_windowManager.maxWindows = maxWindows;

    g_windowManager.windows = gfc_allocate_array(sizeof(window_t), maxWindows);
    atexit(window_close);
}

void window_close(void) {
    if (g_windowManager.windows) {
        for (size_t i = 0; i < g_windowManager.maxWindows; i++) {
            if (g_windowManager.windows[i]._refCount) {
                window_destroy(&g_windowManager.windows[i]);
            }
        }
        free(g_windowManager.windows);
        g_windowManager.windows = NULL;
    }
}

window_t *window_create(int x, int y, int width, int height, const char *title) {
    size_t i, j = g_windowManager.maxWindows; // Initialize j to maxWindows to indicate no free slot found
    window_t *window;
    for (i = 0; i < g_windowManager.maxWindows; i++) {
        window = &g_windowManager.windows[i];
        if (window->_refCount && strcmp(window->title, title) == 0) {
            window->_refCount++;
            return window;
        }

        if (!window->_refCount) {
            j = i;
        }
    }

    if (j < g_windowManager.maxWindows) {
        window = &g_windowManager.windows[j];
        window->x = x;
        window->y = y;
        window->width = width;
        window->height = height;
        strncpy(window->title, title, MAX_TITLE_LENGTH - 1);
        window->title[MAX_TITLE_LENGTH - 1] = '\0';
        window->_refCount = 1;

        window->draw = basic_window_draw;
        window->update = basic_window_update;
        window->handle_event = basic_window_handle_event;
        return window;
    }

    slog("Failed to create window: no available slots");
    return NULL; // No available windows
}

void window_destroy(window_t *window) {
    if (window && window->_refCount > 0) {
        window->_refCount--;
        if (window->_refCount == 0) {
            memset(window, 0, sizeof(window_t));
        }
    }
}

void window_show(window_t *window) {
    if (!window || !window->_refCount || window->visible) {
        return;
    }

    window->visible = 1;
    if (!g_windowManager.active) {
        g_windowManager.active = window;
    } else {
        window->parent = g_windowManager.active;
        g_windowManager.active = window;
    }
}

void window_hide(window_t *window) {
    if (!window || !window->_refCount || !window->visible) {
        return;
    }

    window->visible = 0;
    if (g_windowManager.active == window) {
        g_windowManager.active = window->parent;
    }
}

void window_draw(window_t *window) {
    if (window->visible && window->draw) {
        window->draw(window);
    }

    if (window->parent) {
        window_draw(window->parent);
    }
}

void window_update(window_t *window, float deltaTime) {
    if (window->visible && window->update) {
        window->update(window, deltaTime);
    }

    if (window->parent) {
        window_update(window->parent, deltaTime);
    }
}

int window_handle_event(window_t *window, const window_event_t *event) {
    int handled = 0;
    if (window->visible && window->handle_event) {
        handled = window->handle_event(window, event);
    }

    if (!handled && window->parent) {
        return window_handle_event(window->parent, event);
    }

    return handled;
}

void window_draw_all(void) {
    if (g_windowManager.active) {
        window_draw(g_windowManager.active);
    }
}

void window_update_all(const float deltaTime) {
    if (g_windowManager.active) {
        window_update(g_windowManager.active, deltaTime);
    }
}

void window_handle_event_all(void) {
    size_t head, tail;
    const window_event_t *event;
    if (!g_windowManager.active) {
        return;
    }

    event_buffer_clear();
    event_process_mouse();
    event_process_keyboard();

    head = event_buffer_head();
    tail = event_buffer_tail();
    while (head != tail) {
        event = event_buffer_get(head);
        window_handle_event(g_windowManager.active, event);
        head = event_buffer_next(head);
    }
}

window_t * window_load_from_json(def_data_t *json) {
    window_t *window;
    const char *title;
    GFC_Vector2D position, size;
    if (!json) {
        slog("Failed to load window from JSON: null data");
        return NULL;
    }

    title = def_data_get_string(json, "title");
    if (!title) {
        slog("Failed to load window from JSON: missing 'title' field");
        return NULL;
    }

    def_data_get_vector2d(json, "position", &position);
    def_data_get_vector2d(json, "size", &size);

    window = window_create(position.x, position.y, size.x, size.y, title);
    if (!window) {
        slog("Failed to create window from JSON");
        return NULL;
    }

    window->root = widget_load_from_json(json, NULL);
    return window;

}
