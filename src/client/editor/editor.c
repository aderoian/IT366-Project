#include "client/editor/editor.h"

#include "gfc_input.h"
#include "client/camera.h"
#include "client/editor/editor_camera.h"
#include "client/editor/editor_overlay.h"
#include "client/ui/window.h"
#include "common/logger.h"
#include "common/render/gf2d_font.h"
#include "common/render/gf2d_graphics.h"

extern uint8_t __DEBUG;

int editor_main(int argc, char *argv[]) {

    editor_overlay_init(&g_editorOverlay, 10, 1);

    mutex_lock(&g_client.lock);
    g_client.state = CLIENT_RUNNING;
    mutex_unlock(&g_client.lock);

    log_info("Editor running");

    g_game.tickNumber = 0;
    g_game.deltaTime = 0.0f;
    g_game.role = GAME_ROLE_CLIENT;
    g_game.isLocal = 1;

    //g_game.world = world_create_empty(5, 5);

    entity_t * cameraEntity = editor_camera_spawn(gfc_vector2d(0, 0));
    camera_set_target(&g_camera, cameraEntity);

    editor_tick_loop(&g_client);

    return 0;
}

void editor_on_mouse(float deltaTime) {

}

void editor_tick_loop(Client* client) {
    uint8_t shutDownRequested = 0;

    uint64_t dt = 1000ULL / 30ULL; // 30 ticks per second
    uint64_t accumulator = 0, currentTime, frameTime, lastTime = SDL_GetTicks64();

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
        frameTime = currentTime - lastTime;
        accumulator += frameTime;
        lastTime = currentTime;

        while (accumulator >= dt) {
            g_game.tickNumber++;
            g_game.deltaTime = (float) dt / 1000.0f;

            gfc_input_update();

            world_update(g_game.world, g_game.deltaTime);

            camera_update(&g_camera);

            entity_think_all(g_game.entityManager);
            entity_update_all(g_game.entityManager, g_game.deltaTime);

            editor_on_mouse(g_game.deltaTime);
            window_handle_keyboard();
            window_update_all(g_game.deltaTime);
            overlay_update(g_editorOverlay, g_game.deltaTime);

            accumulator -= dt;

            if (gfc_input_command_down("exit")) {
                client_close();
            }
        }

        editor_render(client, accumulator / dt);
        gf2d_graphics_next_frame();
    }
}

void editor_render(Client* client, uint64_t alpha) {
    gf2d_graphics_clear_screen();

    if (g_game.world) world_draw(g_game.world);
    entity_draw_all(g_game.entityManager);
    overlay_draw(g_editorOverlay);
    window_draw_all();

    if (__DEBUG) {
        gf2d_font_draw_textf(14, "FPS: %f", gfc_vector2d(10, 10), gf2d_graphics_get_frames_per_second());
    }
}