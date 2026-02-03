#ifndef UDP_H
#define UDP_H

#include <sys/socket.h>

#include <enet/enet.h>

#include "common/buffer/ring.h"
#include "common/thread/mutex.h"
#include "common/thread/thread.h"

/**
 * @brief Structure representing a network address (IPv4 or IPv6).
 */
typedef struct net_addr_s {
    struct sockaddr_storage storage;
    socklen_t len;
} net_addr_t;

#define NET_OK          0
#define NET_ERROR      (-1)
#define NET_WOULDBLOCK (-2)  // for non-blocking operations

#define NET_AF_UNSPEC AF_UNSPEC
#define NET_AF_INET   AF_INET
#define NET_AF_INET6  AF_INET6

#define NET_SOCK_STREAM SOCK_STREAM
#define NET_SOCK_DGRAM  SOCK_DGRAM

/** @def NET_HOST_DEFAULT_SHUTDOWN_TIMEOUT
 * @brief Default shutdown timeout in seconds for network hosts.
 */
#define NET_HOST_DEFAULT_SHUTDOWN_TIMEOUT 15

/** @def NET_HOST_SHUTDOWN_TIMEOUT_MS
 * @brief Default shutdown timeout in milliseconds for network hosts.
 */
#define NET_HOST_SHUTDOWN_TIMEOUT_MS (NET_HOST_DEFAULT_SHUTDOWN_TIMEOUT * 1000ULL)

/** @def NET_HOST_SHUTDOWN_TIMEOUT_NS
 * @brief Default shutdown timeout in nanoseconds for network hosts.
 */
#define NET_HOST_SHUTDOWN_TIMEOUT_NS (NET_HOST_DEFAULT_SHUTDOWN_TIMEOUT * 1000000000ULL)

/**
 * @brief Resolve a hostname and service to a network address.
 *
 * @param out Pointer to net_addr_t structure to store the resolved address.
 * @param host Hostname or IP address as a string.
 * @param service Service name or port number as a string.
 * @param socktype Socket type (e.g., SOCK_STREAM, SOCK_DGRAM).
 * @return 1 on success, 0 on failure.
 */
int net_addr_resolve(net_addr_t *out, const char *host, const char *service, int socktype);

/**
 * @brief Get the address family of the network address.
 *
 * @param addr Pointer to the net_addr_t structure.
 * @return Address family (NET_AF_INET or NET_AF_INET6).
 */
int net_addr_family(const net_addr_t *addr);

/**
 * @brief Get the port number from the network address.
 *
 * @param addr Pointer to the net_addr_t structure.
 * @return Port number in host byte order.
 */
uint16_t net_addr_port(const net_addr_t *addr);

/**
 * @brief Convert the network address to a string representation.
 *
 * @param addr Pointer to the net_addr_t structure.
 * @param buffer Buffer to store the string representation.
 * @param buffer_size Size of the buffer.
 * @return 1 on success, 0 on failure.
 */
int net_addr_toString(const net_addr_t *addr, char *buffer, size_t buffer_size);

/**
 * @brief UDP Packet Flags
 * These flags can be used when creating UDP packets to specify their behavior.
 * They map directly to ENet packet flags.
 */

/** @def NET_UDP_FLAG_RELIABLE
 * @brief Flag indicating that the packet should be sent reliably.
 */
#define NET_UDP_FLAG_RELIABLE ENET_PACKET_FLAG_RELIABLE

/** @def NET_UDP_FLAG_UNSEQUENCED
 * @brief Flag indicating that the packet should be sent unsequenced.
 */
#define NET_UDP_FLAG_UNSEQUENCED ENET_PACKET_FLAG_UNSEQUENCED

/** @def NET_UDP_FLAG_NO_ALLOCATE
 * @brief Flag indicating that no memory allocation should be performed for the packet.
 */
#define NET_UDP_FLAG_NO_ALLOCATE ENET_PACKET_FLAG_NO_ALLOCATE

/** @def NET_UDP_FLAG_UNRELIABLE_FRAGMENT
 * @brief Flag indicating that the packet is an unreliable fragment.
 */
#define NET_UDP_FLAG_UNRELIABLE_FRAGMENT ENET_PACKET_FLAG_UNRELIABLE_FRAGMENT

/** @def NET_UDP_FLAG_SENT
 * @brief Flag indicating that the packet has been sent.
 * @internal This flag is set by the ENet library and should not be set manually.
 */
#define NET_UDP_FLAG_SENT ENET_PACKET_FLAG_SENT

/**
 * @brief Structure representing a UDP packet.
 */
typedef struct net_udp_packet_s {
 /** @internal ENet packet used internally. */
 ENetPacket *_internalPacket;
 /** Length of the data in the packet. */
 size_t dataLength;
 /** Pointer to the packet data. (points to the data stored in _internalPacket) */
 void *data;
} net_udp_packet_t;

/**
 * @brief Create a new UDP packet.
 *
 * @param data Pointer to the data to be included in the packet.
 * @param dataSize Size of the data in bytes.
 * @param flags Flags for the packet (e.g., reliability).
 * @return Pointer to the created net_udp_packet_t, or NULL on failure.
 */
net_udp_packet_t *net_udp_packet_create(void *data, size_t dataSize, uint32_t flags);

/**
 * @brief Destroy a UDP packet.
 *
 * @param packet Pointer to the net_udp_packet_t to be destroyed.
 * @note Frees memory stored in the @code data @endcode field.
 */
void net_udp_packet_destroy(net_udp_packet_t *packet);

/**
 * @brief Resize the data buffer of a UDP packet.
 *
 * @param packet Pointer to the net_udp_packet_t to be resized.
 * @param dataLength New size of the data buffer in bytes.
 * @return 1 on success, 0 on failure.
 * @note This will reallocate the internal ENet packet and copy existing data if any.
 */
int net_udp_packet_resize(net_udp_packet_t *packet, size_t dataLength);

/**
 * @brief UDP Peer structure.
 */
typedef struct _ENetPeer net_udp_peer_t;

/**
 * @brief UDP Peer Constants mapped from ENet.
 */
#define NET_UDP_HOST_RECEIVE_BUFFER_SIZE ENET_HOST_RECEIVE_BUFFER_SIZE
#define NET_UDP_HOST_SEND_BUFFER_SIZE ENET_HOST_SEND_BUFFER_SIZE
#define NET_UDP_HOST_BANDWIDTH_THROTTLE_INTERVAL ENET_HOST_BANDWIDTH_THROTTLE_INTERVAL
#define NET_UDP_HOST_DEFAULT_MTU ENET_HOST_DEFAULT_MTU
#define NET_UDP_HOST_DEFAULT_MAXIMUM_PACKET_SIZE ENET_HOST_DEFAULT_MAXIMUM_PACKET_SIZE
#define NET_UDP_HOST_DEFAULT_MAXIMUM_WAITING_DATA ENET_HOST_DEFAULT_MAXIMUM_WAITING_DATA
#define NET_UDP_PEER_DEFAULT_ROUND_TRIP_TIME ENET_PEER_DEFAULT_ROUND_TRIP_TIME
#define NET_UDP_PEER_DEFAULT_PACKET_THROTTLE ENET_PEER_DEFAULT_PACKET_THROTTLE
#define NET_UDP_PEER_PACKET_THROTTLE_SCALE ENET_PEER_PACKET_THROTTLE_SCALE
#define NET_UDP_PEER_PACKET_THROTTLE_COUNTER ENET_PEER_PACKET_THROTTLE_COUNTER
#define NET_UDP_PEER_PACKET_THROTTLE_ACCELERATION ENET_PEER_PACKET_THROTTLE_ACCELERATION
#define NET_UDP_PEER_PACKET_THROTTLE_DECELERATION ENET_PEER_PACKET_THROTTLE_DECELERATION
#define NET_UDP_PEER_PACKET_THROTTLE_INTERVAL ENET_PEER_PACKET_THROTTLE_INTERVAL
#define NET_UDP_PEER_PACKET_LOSS_SCALE ENET_PEER_PACKET_LOSS_SCALE
#define NET_UDP_PEER_PACKET_LOSS_INTERVAL ENET_PEER_PACKET_LOSS_INTERVAL
#define NET_UDP_PEER_WINDOW_SIZE_SCALE ENET_PEER_WINDOW_SIZE_SCALE
#define NET_UDP_PEER_TIMEOUT_LIMIT ENET_PEER_TIMEOUT_LIMIT
#define NET_UDP_PEER_TIMEOUT_MINIMUM ENET_PEER_TIMEOUT_MINIMUM
#define NET_UDP_PEER_TIMEOUT_MAXIMUM ENET_PEER_TIMEOUT_MAXIMUM
#define NET_UDP_PEER_PING_INTERVAL ENET_PEER_PING_INTERVAL
#define NET_UDP_PEER_UNSEQUENCED_WINDOWS ENET_PEER_UNSEQUENCED_WINDOWS
#define NET_UDP_PEER_UNSEQUENCED_WINDOW_SIZE ENET_PEER_UNSEQUENCED_WINDOW_SIZE
#define NET_UDP_PEER_FREE_UNSEQUENCED_WINDOWS ENET_PEER_FREE_UNSEQUENCED_WINDOWS
#define NET_UDP_PEER_RELIABLE_WINDOWS ENET_PEER_RELIABLE_WINDOWS
#define NET_UDP_PEER_RELIABLE_WINDOW_SIZE ENET_PEER_RELIABLE_WINDOW_SIZE
#define NET_UDP_PEER_FREE_RELIABLE_WINDOWS ENET_PEER_FREE_RELIABLE_WINDOWS

/**
 * @brief Send a UDP packet to a peer on a specific channel.
 *
 * On success, ownership is assumed of the packet, and so net_udp_packet_destroy
 * should not be called on it thereafter. On failure, the caller still must destroy
 * the packet on its own as the packet has not been queued.
 *
 * @param peer Pointer to the net_udp_peer_t.
 * @param channelID The channel ID to send the packet on.
 * @param packet Pointer to the net_udp_packet_t to send.
 * @return 0 on success, or a negative value on failure.
 */
static inline int net_udp_peer_send(net_udp_peer_t *peer, const uint8_t channelID,
                                          const net_udp_packet_t *packet) {
    return enet_peer_send(peer, channelID, packet->_internalPacket);
}

/**
 * @brief Receive a UDP packet from a peer.
 *
 * @param peer Pointer to the net_udp_peer_t.
 * @param channelID Pointer to store the channel ID of the received packet.
 * @return Pointer to the received net_udp_packet_t, or NULL if no packet is available.
 */
net_udp_packet_t *net_udp_peer_receive(net_udp_peer_t *peer, uint8_t *channelID);

/**
 * @brief Ping a UDP peer to check its responsiveness.
 * @note ping requests factor into the mean round trip time as designated by the
 * roundTripTime field in the peer structure. All connected peers are already
 * pinged at regular intervals, however, this function may be called to ensure more
 * frequent ping requests.
 *
 * @param peer Pointer to the net_udp_peer_t.
 */
static inline void net_udp_peer_ping(net_udp_peer_t *peer) {
    enet_peer_ping(peer);
}

/**
 * @brief Set the ping interval for a UDP peer.
 *
 * Pings are used both to monitor the liveness of the connection and also to dynamically
 * adjust the throttle during periods of low traffic so that the throttle has reasonable
 * responsiveness during traffic spikes.
 *
 * @param peer Pointer to the net_udp_peer_t.
 * @param pingInterval The ping interval in milliseconds.
 */
static inline void net_udp_peer_ping_interval(net_udp_peer_t *peer, const uint32_t pingInterval) {
    enet_peer_ping_interval(peer, pingInterval);
}

/**
 * @brief Set the timeout parameters for a UDP peer.
 *
 * The timeout parameter control how and when a peer will timeout from a failure to acknowledge
 * reliable traffic. Timeout values use an exponential backoff mechanism, where if a reliable
 * packet is not acknowledge within some multiple of the average RTT plus a variance tolerance,
 * the timeout will be doubled until it reaches a set limit. If the timeout is thus at this
 * limit and reliable packets have been sent but not acknowledged within a certain minimum time
 * period, the peer will be disconnected. Alternatively, if reliable packets have been sent
 * but not acknowledged for a certain maximum time period, the peer will be disconnected regardless
 * of the current timeout limit value.
 *
 * @param peer Pointer to the net_udp_peer_t.
 * @param timeoutLimit The timeout limit in milliseconds.
 * @param timeoutMinimum The minimum timeout in milliseconds.
 * @param timeoutMaximum The maximum timeout in milliseconds.
 */
static inline void net_udp_peer_timeout(net_udp_peer_t *peer, const uint32_t timeoutLimit,
                                              const uint32_t timeoutMinimum, const uint32_t timeoutMaximum) {
    enet_peer_timeout(peer, timeoutLimit, timeoutMinimum, timeoutMaximum);
}

/**
 * @brief Forcefully reset a UDP peer's connection.
 *
 * @note No NET_UDP_EVENT_TYPE_DISCONNECT event will be generated. The foreign peer is not
 * guaranteed to receive the disconnect notification, and is reset immediately upon
 * return from this function.
 *
 * @param peer Pointer to the net_udp_peer_t.
 */
static inline void net_udp_peer_reset(net_udp_peer_t *peer) {
    enet_peer_reset(peer);
}

/**
 * @brief Disconnect a UDP peer gracefully.
 *
 * @note An NET_UDP_EVENT_TYPE_DISCONNECT event will be generated once the disconnection is complete.
 *
 * @param peer Pointer to the net_udp_peer_t.
 * @param data User-defined data for the disconnection.
 */
static inline void net_udp_peer_disconnect(net_udp_peer_t *peer, const uint32_t data) {
    enet_peer_disconnect(peer, data);
}

/**
 * @brief Immediately disconnect a UDP peer.
 *
 * @param peer Pointer to the net_udp_peer_t.
 * @param data User-defined data for the disconnection.
 */
static inline void net_udp_peer_disconnect_now(net_udp_peer_t *peer, const uint32_t data) {
    enet_peer_disconnect_now(peer, data);
}

/**
 * @brief Disconnect a UDP peer after all queued outgoing packets are sent.
 * @note An NET_UDP_EVENT_TYPE_DISCONNECT event will be generated by enet_host_service()
 * once the disconnection is complete.
 *
 * @param peer Pointer to the net_udp_peer_t.
 * @param data User-defined data for the disconnection.
 */
static inline void net_udp_peer_disconnect_later(net_udp_peer_t *peer, const uint32_t data) {
    enet_peer_disconnect_later(peer, data);
}

/**
 * @brief Configure the packet throttle parameters for a UDP peer.
 *
 * Unreliable packets are dropped by ENet in response to the varying conditions
 * of the Internet connection to the peer.  The throttle represents a probability
 * that an unreliable packet should not be dropped and thus sent by ENet to the peer.
 * The lowest mean round trip time from the sending of a reliable packet to the
 * receipt of its acknowledgement is measured over an amount of time specified by
 * the interval parameter in milliseconds.  If a measured round trip time happens to
 * be significantly less than the mean round trip time measured over the interval,
 * then the throttle probability is increased to allow more traffic by an amount
 * specified in the acceleration parameter, which is a ratio to the NET_UDP_PEER_PACKET_THROTTLE_SCALE
 * constant.  If a measured round trip time happens to be significantly greater than
 * the mean round trip time measured over the interval, then the throttle probability
 * is decreased to limit traffic by an amount specified in the deceleration parameter, which
 * is a ratio to the NET_UDP_PEER_PACKET_THROTTLE_SCALE constant.  When the throttle has
 * a value of NET_UDP_PEER_PACKET_THROTTLE_SCALE, no unreliable packets are dropped by
 * ENet, and so 100% of all unreliable packets will be sent.  When the throttle has a
 * value of 0, all unreliable packets are dropped by ENet, and so 0% of all unreliable
 * packets will be sent.  Intermediate values for the throttle represent intermediate
 * probabilities between 0% and 100% of unreliable packets being sent.  The bandwidth
 * limits of the local and foreign hosts are taken into account to determine a
 * sensible limit for the throttle probability above which it should not raise even in
 * the best of conditions.
 *
 * @param peer Pointer to the net_udp_peer_t.
 * @param interval Interval in milliseconds over which to measure lowest mean RTT.
 * @param acceleration Rate at which to increase the throttle probability as mean RTT declines.
 * @param deceleration Rate at which to decrease the throttle probability as mean RTT increases.
 */
static inline void net_udp_peer_throttle_configure(net_udp_peer_t *peer, const uint32_t interval,
                                                         const uint32_t acceleration, const uint32_t deceleration) {
    enet_peer_throttle_configure(peer, interval, acceleration, deceleration);
}

/**
 * @brief Enumeration representing the state of a host.
 */
typedef enum net_host_state_e {
    /** Host is idle and not running. */
    NET_HOST_IDLE = 0,
    /** Host is running and operational. */
    NET_HOST_RUNNING = 1,
    /** Host shutdown has been requested. */
    NET_HOST_SHUTDOWN_REQUESTED = 2,
    /** Host is in the process of shutting down. */
    NET_HOST_SHUTTING_DOWN = 3,
    /** Host has been stopped. */
    NET_HOST_STOPPED = 4,
} net_host_state_t;

/**
 * @brief UDP Host structure.
 *
 * Thread-safety: All operations (public methods) on net_udp_host_t are thread-safe.
 * Fields marked as internal should not be accessed directly and are not thread-safe.
 * Fields not marked as internal are safe to read assuming a lock is acquired.
 */
typedef struct net_udp_host_s {
    /** Network address of the host. */
    net_addr_t address;
    /** Maximum number of channels supported by the host. */
    size_t channelLimit;
    /** Connection timeout in milliseconds. Used for a client host only. */
    uint32_t connectTimeout;
    /** @internal Event buffer for transporting events between threads. */
    buf_spsc_ring_t eventBuffer;
    /** @internal Thread handling ENet operations. */
    thread_t hostThread;
    /** Mutex for synchronizing access to the host state. */
    mutex_t hostLock;
    /** Current state of the host. */
    net_host_state_t state;
    /** @internal Timestamp marking the start of shutdown. */
    uint64_t shutdownStartTime;
    /** @internal Flag indicating if the host thread is running. */
    uint8_t threadRunning;
    /** @internal Flag indicating if the host is a server. */
    uint8_t isServer;
    /** @internal Pointer to the ENet host. */
    ENetHost *enetHost;
    /** @internal Pointer to the server peer when the host is a client. */
    net_udp_peer_t *serverPeer; // Used when the host is a client.
} net_udp_host_t;

/**
 * @brief Enumeration of UDP event types.
 */
typedef enum net_udp_event_type_e {
    NET_UDP_EVENT_TYPE_CONNECT = 1,
    NET_UDP_EVENT_TYPE_DISCONNECT = 2,
    NET_UDP_EVENT_TYPE_RECEIVE = 3
} net_udp_event_type_t;

/**
 * @brief Structure representing a UDP event.
 * @note Some fields are only valid depending on the event type.
 */
typedef struct net_udp_event_s {
    /** Type of the event. */
    net_udp_event_type_t type;
    /** Pointer to the peer associated with the event. */
    net_udp_peer_t *peer;
    /** Channel ID associated with the event (for receive events). */
    uint8_t chanelId;
    /** User-defined data associated with the event (for connect/disconnect events). */
    uint32_t data;
    /** Pointer to the received packet (for receive events). */
    struct net_udp_packet_s *packet;
} net_udp_event_t;

/**
 * @brief Opaque structure representing a UDP compressor.
 */
typedef struct _ENetCompressor net_udp_compressor_t;

/**
 * @brief Configuration structure for creating a UDP host.
 *
 * Packets will be strategically dropped on specific sides of a connection between hosts
 * to ensure the host's bandwidth is not overwhelmed.  The bandwidth parameters also determine
 * the window size of a connection which limits the amount of reliable packets that may be in transit
 * at any given time.
 */
typedef struct net_udp_host_config_s {
    /** Bind or connect address for the host. */
    net_addr_t address;
    /** Maximum number of peers (for server hosts) or 1 (for client hosts). */
    size_t peerCount;
    /** Maximum number of channels supported by the host. */
    size_t channelLimit;
    /** Incoming bandwidth limit in bytes/second. Zero assumes unlimited bandwidth. */
    uint64_t incomingBandwidth;
    /** Outgoing bandwidth limit in bytes/second. Zero assumes unlimited bandwidth. */
    uint64_t outgoingBandwidth;
    /** Connection timeout in milliseconds. Used for a client host only. */
    uint64_t connectTimeout;
    /** Flag indicating if the host is a server (1) or client (0). */
    uint8_t isServer;
} net_udp_host_config_t;

/**
 * @brief Convert a net_addr_t to an ENetAddress.
 * @note Only IPv4 addresses are supported; IPv6 addresses will result in NULL.
 *
 * @param address Pointer to the net_addr_t to convert.
 * @param out Pointer to the ENetAddress to populate.
 * @return Pointer to the populated ENetAddress, or NULL if conversion failed (e.g., unsupported address family).
 */
static inline ENetAddress *net_address_to_enet(const net_addr_t *address, ENetAddress *out) {
    if (address->storage.ss_family != AF_INET) {
        // ENet does not support IPv6
        return NULL;
    }

    const struct sockaddr_in *addr = (const struct sockaddr_in *) &address->storage;
    out->host = addr->sin_addr.s_addr;
    out->port = ntohs(addr->sin_port);
    return out;
}

/**
 * @brief Create a new UDP host based on the provided configuration.
 *
 * If the host is configured as a server, it will bind to the specified address and listen for incoming connections.
 * If configured as a client, net_udp_host_connect must be called to connect to a server.
 *
 * @param config Pointer to the net_udp_host_config_t configuration structure.
 * @return Pointer to the created net_udp_host_t, or NULL on failure.
 */
net_udp_host_t *net_udp_host_create(const net_udp_host_config_t *config);

/**
 * @brief Destroy a UDP host and frees associated resources.
 *
 * @param host Pointer to the net_udp_host_t to destroy.
 */
void net_udp_host_destroy(net_udp_host_t *host);

/**
 * @brief Connect to a remote UDP host (for client hosts).
 * @note The peer returned will have not completed the connection until net_udp_host_check_events()
 * notifies of an NET_UDP_EVENT_TYPE_CONNECT event for the peer.
 *
 * @param host Pointer to the net_udp_host_t.
 * @param address Pointer to the net_addr_t of the remote host to connect to.
 * @param channelCount Number of channels to allocate for the connection.
 * @param data User-defined data to send with the connection request.
 * @return Pointer to the net_udp_peer_t representing the connection, or NULL on failure.
 */
static inline net_udp_peer_t *net_udp_host_connect(const net_udp_host_t *host, const net_addr_t *address,
                                                            const size_t channelCount, const uint32_t data) {
    ENetAddress eAddr;
    net_address_to_enet(address, &eAddr);
    return enet_host_connect(host->enetHost, &eAddr, channelCount, data);
}

/**
 * @brief Check for pending events on the UDP host.
 *
 * @param host Pointer to the net_udp_host_t.
 * @param event Pointer to the net_udp_event_t to populate with event data.
 * @return 1 if an event was retrieved, 0 if no event is available, or a negative value on error.
 */
int net_udp_host_check_events(net_udp_host_t *host, net_udp_event_t *event);

/**
 * @brief Flush any queued outgoing packets for the UDP host.
 *
 * @param host Pointer to the net_udp_host_t.
 */
static inline void net_udp_host_flush(const net_udp_host_t *host) {
    enet_host_flush(host->enetHost);
}

/**
 * @brief Broadcast a UDP packet to all connected peers on a specific channel.
 *
 * On success, ownership is assumed of the packet, and so net_udp_packet_destroy
 * should not be called on it thereafter. On failure, the caller still must destroy
 * the packet on its own as the packet has not been queued.
 *
 * @param host Pointer to the net_udp_host_t.
 * @param channelId The channel ID to broadcast the packet on.
 * @param packet Pointer to the net_udp_packet_t to broadcast.
 */
static inline void net_udp_host_broadcast(const net_udp_host_t *host, const uint8_t channelId,
                                                const struct net_udp_packet_s *packet) {
    enet_host_broadcast(host->enetHost, channelId, packet->_internalPacket);
}

/**
 * @brief Set the packet compressor for the UDP host.
 *
 * @param host Pointer to the net_udp_host_t.
 * @param compressor Pointer to the net_udp_compressor_t to use for compression; if NULL, compression is disabled.
 */
static inline void
net_udp_host_compress(const net_udp_host_t *host, const net_udp_compressor_t *compressor) {
    enet_host_compress(host->enetHost, compressor);
}

/**
 * @brief Enable range coder compression for the UDP host.
 *
 * @param host Pointer to the net_udp_host_t.
 * @return 0 on success, or a negative value on failure.
 */
static inline int net_udp_host_compress_with_range_coder(const net_udp_host_t *host) {
    return enet_host_compress_with_range_coder(host->enetHost);
}

/**
 * @brief Set the channel limit for the UDP host.
 *
 * @param host Pointer to the net_udp_host_t.
 * @param limit The maximum number of channels allowed for connected peers.
 */
static inline void net_udp_host_channel_limit(const net_udp_host_t *host, const size_t limit) {
    enet_host_channel_limit(host->enetHost, limit);
}

/**
 * @brief Set the bandwidth limits for the UDP host.
 *
 * Packets will be strategically dropped on specific sides of a connection between hosts
 * to ensure the host's bandwidth is not overwhelmed.  The bandwidth parameters also determine
 * the window size of a connection which limits the amount of reliable packets that may be in transit
 * at any given time.
 *
 * @param host Pointer to the net_udp_host_t.
 * @param incoming Incoming bandwidth limit in bytes/second. Zero assumes unlimited bandwidth.
 * @param outgoing Outgoing bandwidth limit in bytes/second. Zero assumes unlimited bandwidth.
 */
static inline void net_udp_host_bandwidth_limit(const net_udp_host_t *host, const uint32_t incoming,
                                                      const uint32_t outgoing) {
    enet_host_bandwidth_limit(host->enetHost, incoming, outgoing);
}

/**
 * @brief Send a UDP packet from the client host to the server.
 *
 * On success, ownership is assumed of the packet, and so net_udp_packet_destroy
 * should not be called on it thereafter. On failure, the caller still must destroy
 * the packet on its own as the packet has not been queued.
 *
 * @param host Pointer to the net_udp_host_t client host.
 * @param channelID The channel ID to send the packet on.
 * @param packet Pointer to the net_udp_packet_t to send.
 * @return 0 on success, or a negative value on failure.
 */
#define net_tcp_host_client_send(host, channelID, packet) net_udp_peer_send(host->serverPeer, channelID, packet)

/**
 * @brief Get the server peer for the client host.
 *
 * @param host Pointer to the net_udp_host_t client host.
 * @return Pointer to the net_udp_peer_t representing the server peer.
 */
net_udp_peer_t *net_udp_host_client_connect(net_udp_host_t *host);

/**
 * @brief Disconnect the client host from the server gracefully.
 * @note An NET_UDP_EVENT_TYPE_DISCONNECT event will be generated once the disconnection is complete.
 *
 * @param host Pointer to the net_udp_host_t client host.
 */
#define net_tcp_host_client_disconnect(host) net_udp_peer_disconnect(host->serverPeer, 0)

/**
 * @brief Configure a UDP host as a client.
 *
 * @param config Pointer to the net_udp_host_config_t to populate.
 * @param ip IP address to connect to.
 * @param port Port number to connect to.
 * @param _channelLimit Maximum number of channels supported by the host.
 * @param inBandwidth Incoming bandwidth limit in bytes/second. Zero assumes unlimited bandwidth.
 * @param outBandwidth Outgoing bandwidth limit in bytes/second. Zero assumes unlimited bandwidth.
 * @param connectionTimeout Connection timeout in milliseconds.
 */
#define net_udp_host_client_config(config, ip, port, _channelLimit, inBandwidth, outBandwidth, connectionTimeout) \
    net_addr_resolve(&(config)->address, ip, port, NET_SOCK_DGRAM); \
    (config)->peerCount = 1; \
    (config)->channelLimit = _channelLimit; \
    (config)->incomingBandwidth = inBandwidth; \
    (config)->outgoingBandwidth = outBandwidth; \
    (config)->connectTimeout = connectionTimeout; \
    (config)->isServer = 0

/**
 * @brief Configure a UDP host as a server.
 *
 * @param config Pointer to the net_udp_host_config_t to populate.
 * @param ip IP address to bind to.
 * @param port Port number to bind to.
 * @param _peerCount Maximum number of peers the server can handle.
 * @param _channelLimit Maximum number of channels supported by the host.
 * @param inBandwidth Incoming bandwidth limit in bytes/second. Zero assumes unlimited bandwidth.
 * @param outBandwidth Outgoing bandwidth limit in bytes/second. Zero assumes unlimited bandwidth.
 * @param connectionTimeout Connection timeout in milliseconds.
 */
#define net_udp_host_server_config(config, ip, port, _peerCount, _channelLimit, inBandwidth, outBandwidth, connectionTimeout) \
    net_addr_resolve(&(config)->address, ip, port, NET_SOCK_DGRAM); \
    (config)->peerCount = _peerCount; \
    (config)->channelLimit = _channelLimit; \
    (config)->incomingBandwidth = inBandwidth; \
    (config)->outgoingBandwidth = outBandwidth; \
    (config)->connectTimeout = connectionTimeout; \
    (config)->isServer = 1

#endif /* UDP_H */