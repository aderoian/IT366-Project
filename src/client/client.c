#include "gfc_config_def.h"
#include "gfc_input.h"

#include "common/logger.h"
#include "common/thread/mutex.h"
#include "common/game/entity.h"
#include "common/game/tower.h"
#include "common/game/game.h"
#include "common/def.h"
#include "common/network/packet/definitions.h"
#include "common/network/packet/io.h"

#include "client/animation.h"
#include "client/gf2d_graphics.h"
#include "client/gf2d_sprite.h"
#include "client/client.h"
#include "client/camera.h"
#include "client/ui/overlay.h"
#include "client/ui/window.h"

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
    window_init(32, 256, 256);
    overlay_init(&g_client.overlay, 32, "def/overlay.json");
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

    tower_create_by_name("Basic Tower", gfc_vector2d(500, 500));

    client_connect(&g_client, "127.0.0.1", "12345");
    c2s_player_join_request_packet_t pkt;
    create_c2s_player_join_request(&pkt);

    g_client.state = CLIENT_JOINING;
    client_send_to_server(&g_client, PACKET_C2S_PLAYER_JOIN_REQUEST, &pkt, ENET_PACKET_FLAG_RELIABLE);

    log_info("Client running");

    g_game.tickNumber = 0;
    g_game.deltaTime = 0.0f;
    g_game.isLocal = 1;
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

int client_connect(Client* client, const char *serverIP, const char *serverPort) {
    if (!client || !client->network) {
        return -1;
    }

    return client_network_start(client->network, serverIP, serverPort);
}

void client_disconnect(Client* client) {
    if (!client || !client->network) {
        return;
    }

    client_network_stop(client->network);
}

int client_send_to_server(Client *client, const uint8_t pktId, void *pkt, const uint32_t flags) {
    if (!client || !pkt || !client->network) {
        return -1;
    }

    return client_network_send(client->network, pktId, pkt, flags);
}

void client_tickLoop(Client* client) {
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
            network_tick(&client->network->baseNetwork);

            window_handle_event_all();
            window_update_all(g_game.deltaTime);
            overlay_update(&g_client.overlay, g_game.deltaTime);

            entity_think_all();
            entity_update_all(g_game.deltaTime);

            camera_update(&g_camera);

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
    overlay_draw(&g_client.overlay);
    window_draw_all();
}