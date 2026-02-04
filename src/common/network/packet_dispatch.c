#include "common/network/packet/handler.h"

packet_dispatch_fn packet_dispatch_table[PACKET_COUNT] = {
#define PACKET_TABLE(name, id, fields) [PACKET_##id] = dispatch_##name,
    PACKET_LIST(PACKET_TABLE)
#undef PACKET_TABLE
};
