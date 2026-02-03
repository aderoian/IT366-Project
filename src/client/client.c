#include "gfc_config_def.h"
#include "gfc_input.h"

#include "common/logger.h"
#include "common/thread/mutex.h"
#include "common/entity.h"

#include "client/animation.h"
#include "client/gf2d_graphics.h"
#include "client/gf2d_sprite.h"

#include "client/client.h"

#include "common/player_entity.h"

void client_tickLoop(Client* client);
void client_render(Client *client, uint64_t alpha);

static Client g_client = {0};

void client_run(void);

int client_main(void) {
    log_info("Initializing client...");
    gfc_input_init("def/input.json");
    gfc_config_def_init();

    g_client.state = CLIENT_IDLE;
    mutex_init(&g_client.lock);

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

    mutex_lock(&g_client.lock);
    g_client.state = CLIENT_RUNNING;
    mutex_unlock(&g_client.lock);

    log_info("Client running");

    player_spawn_immobile(gfc_vector2d(300, 200), "images/pointer.png");

    client_tickLoop(&g_client);

    return 0;
}

void client_close(void) {
    mutex_lock(&g_client.lock);
    if (g_client.state == CLIENT_RUNNING) {
        g_client.state = CLIENT_SHUTDOWN_REQUESTED;
    }
    mutex_unlock(&g_client.lock);

    mutex_destroy(&g_client.lock);
}

void client_tickLoop(Client* client) {
    uint8_t shutDownRequested = 0;

    uint64_t dt = 1000ULL / 60ULL; // 60 FPS
    uint64_t accumulator = 0, currentTime, frameTime, lastTime = SDL_GetTicks64();
    float deltaUpdate;

    while (1) {
        mutex_lock(&g_client.lock);
        if (g_client.state == CLIENT_STOPPED) {
            mutex_unlock(&g_client.lock);
            break;
        }

        if (g_client.state == CLIENT_SHUTDOWN_REQUESTED) {
            g_client.state = CLIENT_SHUTTING_DOWN;
            shutDownRequested = 1;
        }
        mutex_unlock(&g_client.lock);

        if (shutDownRequested) {
            // perform shutdown tasks
            mutex_lock(&g_client.lock);
            g_client.state = CLIENT_STOPPED;
            mutex_unlock(&g_client.lock);
            break;
        }

        currentTime = SDL_GetTicks64();
        uint64_t frameTime = currentTime - lastTime;
        accumulator += frameTime;
        lastTime = currentTime;

        while (accumulator >= dt) {
            gfc_input_update();

            deltaUpdate = (float) dt / 1000.0f;
            entity_think_all();
            entity_update_all(deltaUpdate);

            phys_step(deltaUpdate);
            accumulator -= dt;

            if (gfc_input_command_down("exit")) {
                client_close();
            }
        }


        client_render(client, accumulator / dt);
        gf2d_graphics_next_frame();
    }
}

void client_render(Client* client, uint64_t alpha) {
    gf2d_graphics_clear_screen();

    entity_draw_all();
}