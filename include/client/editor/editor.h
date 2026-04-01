#ifndef CLIENT_EDITOR_H
#define CLIENT_EDITOR_H

#include "client/client.h"

int editor_main(int argc, char *argv[]);

void editor_tick_loop(Client* client);

void editor_render(Client* client, uint64_t alpha);

#endif /* CLIENT_EDITOR_H */