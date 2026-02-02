#include <stdio.h>
#include <string.h>

#include "common/logger.h"

#include "server/server.h"

Server g_server = {0};

void *server_run(void *arg);
void server_runCommandLoop(void);

int server_main(void) {
    log_info("Initializing server...");

    g_server.state = SERVER_IDLE;
    mutex_init(&g_server.lock);

    if (thread_create(&g_server.thread, server_run, NULL) < 0) {
        log_fatal("Failed to create server thread");
        mutex_destroy(&g_server.lock);
        return -1;
    }

    if (_dedicatedServer) {
        server_runCommandLoop();
    }

    return 0;
}

void server_close(void) {
    mutex_lock(&g_server.lock);
    if (g_server.state == SERVER_RUNNING) {
        g_server.state = SERVER_SHUTDOWN_REQUESTED;
    }
    mutex_unlock(&g_server.lock);

    thread_join(&g_server.thread);
    mutex_destroy(&g_server.lock);
}

void *server_run(void *arg) {
    uint8_t shutdownRequested = 0;

    mutex_lock(&g_server.lock);
    g_server.state = SERVER_RUNNING;
    mutex_unlock(&g_server.lock);

    log_info("Server running");

    while (1) {
        mutex_lock(&g_server.lock);
        if (g_server.state == SERVER_SHUTDOWN_REQUESTED) {
            g_server.state = SERVER_SHUTTING_DOWN;
            shutdownRequested = 1;
        }
        mutex_unlock(&g_server.lock);

        if (shutdownRequested) {
            log_info("Stopping server...");
            // perform shutdown tasks
            shutdownRequested = 0;
            mutex_lock(&g_server.lock);
            g_server.state = SERVER_STOPPED;
            mutex_unlock(&g_server.lock);
            log_info("Server stopped");
            break;
        }

        // Server logic goes here
    }

    return NULL;
}

void server_runCommandLoop(void) {
    char command[256];
    while (1) {
        printf("server> ");
        if (fgets(command, sizeof(command), stdin) == NULL) {
            break;
        }

        if (strncmp(command, "shutdown", 8) == 0) {
            server_close();
            break;
        }

        printf("Unknown command: %s", command);
    }
}