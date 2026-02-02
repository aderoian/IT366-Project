#include "gfc_config_def.h"
#include "gfc_input.h"

#include "common/logger.h"
#include "common/thread/thread.h"
#include "common/thread/mutex.h"
#include "common/entity.h"

#include "client/animation.h"
#include "client/gf2d_graphics.h"
#include "client/gf2d_sprite.h"

#include "client/client.h"

#include "common/player_entity.h"

typedef enum ClientState_E {
    CLIENT_IDLE = 0,
    CLIENT_RUNNING = 1,
    CLIENT_SHUTDOWN_REQUESTED = 2,
    CLIENT_SHUTTING_DOWN = 3,
    CLIENT_STOPPED = 4,
} ClientState;

struct Client_S {
    ClientState state;
    mutex_t lock;
    thread_t thread;
};

struct Client_S client = {0};

void client_run(void);

int client_main(void) {
    log_info("Initializing client...");
    gfc_input_init("def/input.json");
    gfc_config_def_init();

    client.state = CLIENT_IDLE;
    mutex_init(&client.lock);

    gf2d_graphics_initialize(
        "gf2d",
        1200,
        720,
        1200,
        720,
        gfc_vector4d(0,0,0,255),
        0);
    gf2d_graphics_set_frame_delay(16);
    gf2d_sprite_init(1024);
    animation_manager_init(256);
    entity_init(1024);
    phys_init(1024);

    client_run();

    mutex_destroy(&client.lock);

    return 0;
}

void client_close(void) {
    mutex_lock(&client.lock);
    if (client.state == CLIENT_RUNNING) {
        client.state = CLIENT_SHUTDOWN_REQUESTED;
    }
    mutex_unlock(&client.lock);

    thread_join(&client.thread);
    mutex_destroy(&client.lock);
}

void client_run(void) {
    float now, then;
    Sprite *bg;
    uint8_t shutDownRequested = 0;

    mutex_lock(&client.lock);
    client.state = CLIENT_RUNNING;
    mutex_unlock(&client.lock);

    log_info("Client running");

    bg = gf2d_sprite_load_image("images/backgrounds/bg_flat.png");

    player_spawn_immobile(gfc_vector2d(300, 200), "images/pointer.png");

    then = (float) SDL_GetTicks() * 0.001f;
    while (1) {
        mutex_lock(&client.lock);
        if (client.state != CLIENT_RUNNING) {
            mutex_unlock(&client.lock);
            break;
        }

        if (client.state == CLIENT_SHUTDOWN_REQUESTED) {
            client.state = CLIENT_SHUTTING_DOWN;
            shutDownRequested = 1;
        }
        mutex_unlock(&client.lock);

        if (shutDownRequested) {
            // perform shutdown tasks
            shutDownRequested = 0;
            mutex_lock(&client.lock);
            client.state = CLIENT_STOPPED;
            mutex_unlock(&client.lock);
            break;
        }

        gfc_input_update();

        now = (float) SDL_GetTicks() * 0.001f;
        entity_think_all();
        entity_update_all(now - then);

        phys_step(now - then);
        then = now;

        gf2d_graphics_clear_screen();
            gf2d_sprite_draw_image(bg,gfc_vector2d(0,0));
            entity_draw_all();

            //UI elements last

        gf2d_graphics_next_frame();

        if (gfc_input_command_down("exit")) {
            client_close();
        }
    }
}