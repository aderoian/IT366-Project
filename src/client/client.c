#include "gfc_config_def.h"
#include "gfc_input.h"

#include "common/logger.h"
#include "common/thread/mutex.h"
#include "common/entity.h"
#include "common/tower.h"

#include "common/player_entity.h"

#include "client/animation.h"
#include "client/gf2d_graphics.h"
#include "client/gf2d_sprite.h"

#include "client/client.h"
#include "client/camera.h"
#include "common/def.h"

void client_tickLoop(Client* client);
void client_render(Client *client, uint64_t alpha);

Client g_client = {0};

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
    def_init(32);
    animation_manager_init(256);
    entity_init(1024);
    phys_init(1024);
    tower_init(32);
    tower_load_defs("def/towers.json");

    camera_init(&g_camera);

    // Init network
    g_client.network = client_network_create(&(network_settings_t){
        .channelLimit = 4,
        .inBandwidth = 0,
        .outBandwidth = 0,
        .connectionTimeout = 5000,
    });

    if (!g_client.network) {
        log_error("Failed to create client network");
        return -1;
    }

    mutex_lock(&g_client.lock);
    g_client.state = CLIENT_RUNNING;
    mutex_unlock(&g_client.lock);

    g_client.renderState.background = gf2d_sprite_load_image("images/backgrounds/bg_flat.png");

    Entity *ent = player_spawn(gfc_vector2d(100, 100), "images/pointer.png");
    player_spawn_immobile(gfc_vector2d(300, 300), "images/pointer.png");

    camera_set_target(&g_camera, ent);

    log_info("Client running");

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
        frameTime = currentTime - lastTime;
        accumulator += frameTime;
        lastTime = currentTime;

        while (accumulator >= dt) {
            gfc_input_update();

            network_tick(&client->network->baseNetwork);

            deltaUpdate = (float) dt / 1000.0f;
            entity_think_all();
            entity_update_all(deltaUpdate);

            camera_update(&g_camera);

            // c2s_player_input_snapshot_packet_t packet;
            // player_input_command_t inputCommand = {
            //     .clientTick = SDL_GetTicks64(),
            //     .lastServerTick = 0, // This would be updated with the last known server tick
            //     .axisX = 1,
            //     .axisY = -1,
            // };
            // create_c2s_player_input_snapshot(&packet, &inputCommand);
            // client_network_send(client->network, PACKET_C2S_PLAYER_INPUT_SNAPSHOT, &packet);

            //phys_step(deltaUpdate);
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

    gf2d_sprite_draw_image(client->renderState.background, gfc_vector2d(0, 0));

    entity_draw_all();
}