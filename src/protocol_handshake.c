#include "protocol.h"
#include "crypto.h"
#include "socket_utils.h"

/* Send handshake request */
int send_handshake_request(int sock, int is_tcp, struct sockaddr_in* dest_addr) {
    vpn_packet_t packet;
    handshake_request_t req;
    
    memset(&packet, 0, sizeof(packet));
    memset(&req, 0, sizeof(req));
    
    /* Generate random client ID */
    if (generate_random_bytes(req.client_id, 16) < 0) {
        LOG_ERROR("Failed to generate client ID");
        return VPN_ERROR;
    }
    
    req.protocol_version = PROTOCOL_VERSION;
    req.supported_ciphers = 0x01;  /* AES-256-GCM */
    req.preferred_mode = 3;  /* Both UDP and TCP */
    
    /* Create packet */
    packet.header.type = PKT_HANDSHAKE;
    packet.header.sequence = 0;
    packet.header.flags = FLAG_REQUIRE_ACK;
    packet.header.payload_len = sizeof(handshake_request_t);
    packet.header.timestamp = (uint32_t)time(NULL);
    
    memcpy(packet.payload, &req, sizeof(req));
    memset(packet.mac_tag, 0, MAC_TAG_SIZE);  /* No encryption yet */
    
    LOG_DEBUG("Sending handshake request");
    return send_packet(sock, &packet, is_tcp, dest_addr);
}

/* Receive handshake request */
int recv_handshake_request(int sock, int is_tcp, struct sockaddr_in* src_addr,
                           handshake_request_t* req) {
    vpn_packet_t packet;
    
    if (!req) return VPN_ERROR;
    
    if (recv_packet(sock, &packet, is_tcp, src_addr) < 0) {
        LOG_ERROR("Failed to receive handshake request");
        return VPN_ERROR;
    }
    
    if (packet.header.type != PKT_HANDSHAKE) {
        LOG_ERROR("Expected handshake packet");
        return VPN_ERROR_PROTOCOL;
    }
    
    if (packet.header.payload_len != sizeof(handshake_request_t)) {
        LOG_ERROR("Invalid handshake payload size");
        return VPN_ERROR_PROTOCOL;
    }
    
    memcpy(req, packet.payload, sizeof(handshake_request_t));
    
    if (req->protocol_version != PROTOCOL_VERSION) {
        LOG_ERROR("Protocol version mismatch");
        return VPN_ERROR_PROTOCOL;
    }
    
    LOG_DEBUG("Received handshake request");
    return VPN_SUCCESS;
}

/* Send server hello response */
int send_server_hello(int sock, int is_tcp, struct sockaddr_in* dest_addr) {
    vpn_packet_t packet;
    handshake_response_t resp;
    
    memset(&packet, 0, sizeof(packet));
    memset(&resp, 0, sizeof(resp));
    
    /* Generate server ID and nonce */
    if (generate_random_bytes(resp.server_id, 16) < 0 ||
        generate_random_bytes(resp.server_nonce, NONCE_SIZE) < 0) {
        LOG_ERROR("Failed to generate server parameters");
        return VPN_ERROR;
    }
    
    resp.supported_ciphers = 0x01;  /* AES-256-GCM */
    resp.selected_mode = 3;  /* Support both UDP and TCP */
    
    /* Create packet */
    packet.header.type = PKT_SERVER_HELLO;
    packet.header.sequence = 1;
    packet.header.flags = FLAG_REQUIRE_ACK;
    packet.header.payload_len = sizeof(handshake_response_t);
    packet.header.timestamp = (uint32_t)time(NULL);
    
    memcpy(packet.payload, &resp, sizeof(resp));
    memset(packet.mac_tag, 0, MAC_TAG_SIZE);
    
    LOG_DEBUG("Sending server hello");
    return send_packet(sock, &packet, is_tcp, dest_addr);
}

/* Receive server hello response */
int recv_server_hello(int sock, int is_tcp, struct sockaddr_in* src_addr,
                      handshake_response_t* resp) {
    vpn_packet_t packet;
    
    if (!resp) return VPN_ERROR;
    
    if (recv_packet(sock, &packet, is_tcp, src_addr) < 0) {
        LOG_ERROR("Failed to receive server hello");
        return VPN_ERROR;
    }
    
    if (packet.header.type != PKT_SERVER_HELLO) {
        LOG_ERROR("Expected server hello packet");
        return VPN_ERROR_PROTOCOL;
    }
    
    if (packet.header.payload_len != sizeof(handshake_response_t)) {
        LOG_ERROR("Invalid server hello payload size");
        return VPN_ERROR_PROTOCOL;
    }
    
    memcpy(resp, packet.payload, sizeof(handshake_response_t));
    
    LOG_DEBUG("Received server hello");
    return VPN_SUCCESS;
}

/* Send key exchange request */
int send_key_exchange(int sock, int is_tcp, struct sockaddr_in* dest_addr,
                      const key_exchange_request_t* req) {
    vpn_packet_t packet;
    
    if (!req) return VPN_ERROR;
    
    memset(&packet, 0, sizeof(packet));
    
    packet.header.type = PKT_KEY_EXCHANGE;
    packet.header.sequence = 2;
    packet.header.flags = FLAG_REQUIRE_ACK;
    packet.header.payload_len = sizeof(key_exchange_request_t);
    packet.header.timestamp = (uint32_t)time(NULL);
    
    memcpy(packet.payload, req, sizeof(key_exchange_request_t));
    memset(packet.mac_tag, 0, MAC_TAG_SIZE);
    
    LOG_DEBUG("Sending key exchange");
    return send_packet(sock, &packet, is_tcp, dest_addr);
}

/* Receive key exchange request */
int recv_key_exchange(int sock, int is_tcp, struct sockaddr_in* src_addr,
                      key_exchange_request_t* req) {
    vpn_packet_t packet;
    
    if (!req) return VPN_ERROR;
    
    if (recv_packet(sock, &packet, is_tcp, src_addr) < 0) {
        LOG_ERROR("Failed to receive key exchange");
        return VPN_ERROR;
    }
    
    if (packet.header.type != PKT_KEY_EXCHANGE) {
        LOG_ERROR("Expected key exchange packet");
        return VPN_ERROR_PROTOCOL;
    }
    
    if (packet.header.payload_len != sizeof(key_exchange_request_t)) {
        LOG_ERROR("Invalid key exchange payload size");
        return VPN_ERROR_PROTOCOL;
    }
    
    memcpy(req, packet.payload, sizeof(key_exchange_request_t));
    
    LOG_DEBUG("Received key exchange");
    return VPN_SUCCESS;
}
