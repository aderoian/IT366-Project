#include "gfc_types.h"
#include "gfc_input.h"

#include "client/ui/event.h"

typedef struct event_buffer_s {
    window_event_t *events;
    size_t maxEvents;
    size_t head;
    size_t tail;
} event_buffer_t;

event_buffer_t event_buffer;

void event_buffer_init(const size_t maxEvents) {
    event_buffer.events = gfc_allocate_array(sizeof(window_event_t), maxEvents);
    event_buffer.maxEvents = maxEvents;
    event_buffer.head = 0;
    event_buffer.tail = 0;
    atexit(event_buffer_close);
}

void event_buffer_close(void) {
    if (event_buffer.events) {
        free(event_buffer.events);
        event_buffer.events = NULL;
    }
    event_buffer.maxEvents = 0;
    event_buffer.head = 0;
    event_buffer.tail = 0;
}

window_event_t *event_buffer_create(void) {
    window_event_t *event;
    if (!event_buffer.events) {
        return NULL;
    }

    event_buffer.head = (event_buffer.head + 1) % event_buffer.maxEvents;
    if (event_buffer.head == event_buffer.tail) {
        // Buffer is full, move tail to overwrite oldest event
        event_buffer.tail = (event_buffer.tail + 1) % event_buffer.maxEvents;
    }

    event = &event_buffer.events[event_buffer.head];
    memset(event, 0, sizeof(window_event_t));
    return event;
}

void event_process_mouse(void) {
    uint8_t buttons;
    int x = 0, y = 0;

    buttons = SDL_GetRelativeMouseState(&x, &y);
    if (buttons || x || y) {
        window_event_t *event = event_buffer_create();
        if (event) {
            event->type = EVENT_TYPE_MOUSE;
            event->data.mouse.x = x;
            event->data.mouse.y = y;
            event->data.mouse.buttons = buttons;
        }
    }
}

void event_process_keyboard(void) {
    // TODO: Implement keyboard processing. Cannot use SDL_PumpEvents
}

void event_process_command(const char *command) {
    if (gfc_input_command_down(command)) {
        window_event_t *event = event_buffer_create();
        if (event) {
            event->type = EVENT_TYPE_COMMAND;
            strncpy(event->data.command.command, command, sizeof(event->data.command.command) - 1);
            event->data.command.command[sizeof(event->data.command.command) - 1] = '\0';
            event->data.command.type = EVENT_COMMAND_TYPE_PRESSED;
            if (gfc_input_command_held(command)) {
                event->data.command.type |= EVENT_COMMAND_TYPE_HELD;
            }
        }
    } else if (gfc_input_command_released(command)) {
        window_event_t *event = event_buffer_create();
        if (event) {
            event->type = EVENT_TYPE_COMMAND;
            strncpy(event->data.command.command, command, sizeof(event->data.command.command) - 1);
            event->data.command.command[sizeof(event->data.command.command) - 1] = '\0';
            event->data.command.type = EVENT_COMMAND_TYPE_RELEASED;
        }
    }
}

size_t event_buffer_head(void) {
    return event_buffer.head;
}

size_t event_buffer_tail(void) {
    return event_buffer.tail;
}

const window_event_t * event_buffer_get(const size_t index) {
    if (!event_buffer.events || index >= event_buffer.maxEvents) {
        return NULL;
    }
    return &event_buffer.events[index];
}

size_t event_buffer_next(const size_t current) {
    size_t next;
    if (event_buffer.head == event_buffer.tail) {
        return event_buffer.tail; // Buffer is empty
    }

    next = (current + 1) % event_buffer.maxEvents;
    if (next == event_buffer.tail) {
        return event_buffer.tail; // Reached the end of the buffer
    }

    return next;
}

void event_buffer_clear(void) {
    event_buffer.head = 0;
    event_buffer.tail = 0;
}
