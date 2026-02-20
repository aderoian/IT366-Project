#ifndef WINDOW_EVENT_H
#define WINDOW_EVENT_H

#include <stddef.h>
#include <stdint.h>

#include <SDL_mouse.h>

/**
 * @brief Enumeration for different types of events that can occur in the window system.
 */
typedef enum event_type_e {
    EVENT_TYPE_NONE = 0,
    EVENT_TYPE_KEYBOARD = 1,
    EVENT_TYPE_MOUSE = 2,
    EVENT_TYPE_COMMAND = 3,
} event_type_e;

/**
 * @brief Enumeration for different types of keyboard events.
 */
typedef enum keyboard_event_type_e {
    KEYBOARD_EVENT_KEY_DOWN = 0,
    KEYBOARD_EVENT_KEY_UP = 1,
} keyboard_event_type_e;

/**
 * @brief Enumeration for different types of mouse events.
 */
typedef enum mouse_event_type_e {
    MOUSE_EVENT_BUTTON_DOWN = 0,
    MOUSE_EVENT_BUTTON_UP = 1,
    MOUSE_EVENT_MOVE = 2,
} mouse_event_type_e;

/**
 * @brief Structure representing a keyboard event, including the type of event, key code, and whether the key is held down.
 */
typedef struct keyboard_event_s {
    keyboard_event_type_e type;
    int keyCode;
    uint8_t held;
} keyboard_event_t;

/**
 * @brief Bit shifted values for mouse buttons, used in the buttons bitmask of mouse events.
 */
#define EVENT_MOUSE_BUTTON_LEFT SDL_BUTTON_LMASK
#define EVENT_MOUSE_BUTTON_RIGHT SDL_BUTTON_RMASK
#define EVENT_MOUSE_BUTTON_MIDDLE SDL_BUTTON_MMASK
#define EVENT_MOUSE_BUTTON_X1 SDL_BUTTON_X1MASK
#define EVENT_MOUSE_BUTTON_X2 SDL_BUTTON_X2MASK

/**
 * @brief Structure representing a mouse event, including the type of event, x and y coordinates, and a bitmask for mouse buttons.
 */
typedef struct mouse_event_s {
    mouse_event_type_e type;
    int x, y;
    uint32_t buttons; // Bitmask for mouse buttons (e.g., 1 for left, 2 for right, 4 for middle)
} mouse_event_t;

/**
 * @brief Bit shifted values for command event types, allowing for combinations of pressed, released, and held states.
 */
#define EVENT_COMMAND_TYPE_PRESSED 1
#define EVENT_COMMAND_TYPE_RELEASED 2
#define EVENT_COMMAND_TYPE_HELD 4

/**
 * @brief Structure representing a command event, including the command string and the type of event (pressed, released, held).
 */
typedef struct command_event_s {
    char command[16];
    uint8_t type;
} command_event_t;

/**
 * @brief Structure representing a window event, which can be a keyboard event, mouse event, or command event, depending on the type field.
 */
typedef struct window_event_s {
    event_type_e type;
    union {
        keyboard_event_t keyboard;
        mouse_event_t mouse;
        command_event_t command;
    } data;
} window_event_t;

/**
 * @brief Initializes the event buffer with a specified maximum number of events.
 *
 * @param maxEvents The maximum number of events that can be stored in the buffer.
 */
void event_buffer_init(size_t maxEvents);

/**
 * @brief Closes the event buffer and frees all associated resources.
 */
void event_buffer_close(void);

/**
 * @brief Creates a new event in the event buffer and returns a pointer to it for filling in event data.
 *
 * @return A pointer to the newly created event, or NULL if the buffer is full.
 */
window_event_t *event_buffer_create(void);

/**
 * @brief Processes mouse input and adds corresponding mouse events to the event buffer.
 */
void event_process_mouse(void);

/**
 * @brief Processes keyboard input and adds corresponding keyboard events to the event buffer.
 */
void event_process_keyboard(void);

/**
 * @brief Processes a specific command input and adds a corresponding command event to the event buffer if the command is active.
 *
 * @param command The command string to be processed (e.g., "exit", "jump").
 */
void event_process_command(const char *command);

/**
 * @brief Retrieves the current head index of the event buffer.
 *
 * @return The index of the head of the event buffer.
 */
size_t event_buffer_head(void);

/**
 * @brief Retrieves the current tail index of the event buffer.
 *
 * @return The index of the tail of the event buffer.
 */
size_t event_buffer_tail(void);

/**
 * @brief Retrieves a pointer to the event at the specified index in the event buffer.
 *
 * @param index The index of the event to retrieve.
 * @return A pointer to the event at the specified index, or NULL if the index is out of bounds.
 */
const window_event_t *event_buffer_get(size_t index);

/**
 * @brief Retrieves the index of the next event in the buffer after the specified current index.
 *
 * @param current The current index in the event buffer.
 * @return The index of the next event in the buffer, or the tail index if there are no more events.
 */
size_t event_buffer_next(size_t current);

/**
 * @brief Clears the event buffer, resetting the head and tail indices to indicate that the buffer is empty.
 */
void event_buffer_clear(void);

#endif /* WINDOW_EVENT_H */