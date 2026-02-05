#ifndef NETWORK_PACKET_HANDLER_H
#define NETWORK_PACKET_HANDLER_H

#include "common/types.h"
#include "common/network/packet/io.h"

#define PACKET_HANDLER(name, id, fields) \
void handle_##name(const name##_packet_t*, void*);

PACKET_LIST(PACKET_HANDLER)

#undef PACKET_HANDLER

#define PACKET_RECEIVE(name, id, fields) \
void receive_##name(buffer_t buf, buffer_offset_t* off, void* c);

PACKET_LIST(PACKET_RECEIVE)

#undef PACKET_RECEIVE

#define PACKET_SEND(name, id, fields) \
void send_##name(uint8_t, void *, buffer_t, buffer_offset_t*);

PACKET_LIST(PACKET_SEND)

#undef PACKET_SEND

typedef void (*packet_receive_fn)(
    buffer_t,
    buffer_offset_t*,
    void*
);

extern packet_receive_fn packet_dispatch_table[PACKET_COUNT];

typedef void (*packet_send_fn) (
    uint8_t,
    void *,
    buffer_t,
    buffer_offset_t*
);

extern packet_send_fn packet_send_table[PACKET_COUNT];

#endif /* NETWORK_PACKET_HANDLER_H */