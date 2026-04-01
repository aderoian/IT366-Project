#include <string.h>

#include "common/logger.h"
#include "client/client.h"
#include "server/server.h"

uint8_t _dedicatedServer;

// development flags
uint8_t __DEBUG = 0, __DEBUG_LINES = 0, __INF_RESOURCES = 0, __INF_DAMAGE = 0;

void parse_arguments(int argc,char *argv[]);

int main(int argc, char * argv[]) {
    /*program initializtion*/
    parse_arguments(argc,argv);
    logger_init("gf2d.log", LOG_INFO, LOG_DEBUG);

    log_info("---==== BEGIN ====---");

    if (_dedicatedServer) {
        log_info("Starting in SERVER mode");
        server_main();
    } else {
        log_info("Starting in CLIENT mode");
        client_main(argc, argv);
    }

    log_info("---==== END ====---");
    return 0;
}

void parse_arguments(int argc,char *argv[]) {
    int a;
    for (a = 1; a < argc;a++) {
        if (strcmp(argv[a],"--debug") == 0) {
            __DEBUG = 1;
        }
        if (strcmp(argv[a],"--debug-lines") == 0) {
            __DEBUG_LINES = 1;
        }
        if (strcmp(argv[a],"--server") == 0) {
            _dedicatedServer = 1;
        }
        if (strcmp(argv[a],"--inf-resources") == 0) {
            __INF_RESOURCES = 1;
        }
        if (strcmp(argv[a],"--inf-damage") == 0) {
            __INF_DAMAGE = 1;
        }
    }
}
/*eol@eof*/