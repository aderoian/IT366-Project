#include "common/network/packet/handler.h"

#define PACKET_RECEIVE(name, id, fields) \
void receive_##name(buffer_t buf, buffer_offset_t* off, void* c) { \
name##_packet_t pkt; \
deserialize_##name(buf, off, &pkt); \
handle_##name(&pkt, c); \
}

PACKET_LIST(PACKET_RECEIVE)

packet_receive_fn packet_dispatch_table[PACKET_COUNT] = {
#define PACKET_RECEIVE_TABLE(name, id, fields) [PACKET_##id] = receive_##name,
    PACKET_LIST(PACKET_RECEIVE_TABLE)
};

#undef PACKET_RECEIVE
#undef PACKET_RECEIVE_TABLE

#define PACKET_SEND(name, id, fields) \
void prepare_send_##name(void *context, buffer_t buf, buffer_offset_t* off) { \
    serialize_##name(buf, off, (name##_packet_t *)context); \
}

PACKET_LIST(PACKET_SEND)

packet_send_fn packet_send_table[PACKET_COUNT] = {
#define PACKET_SEND_TABLE(name, id, fields) [PACKET_##id] = prepare_send_##name,
    PACKET_LIST(PACKET_SEND_TABLE)
};

#undef PACKET_SEND
#undef PACKET_SEND_TABLE