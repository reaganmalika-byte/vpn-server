#include "client_handler.h"

/* Create client handler */
client_handler_t* client_handler_create(int client_id, int socket, int is_tcp,
                                        struct sockaddr_in* addr) {
    client_handler_t* handler = (client_handler_t*)safe_malloc(sizeof(client_handler_t));
    
    handler->client_id = client_id;
    handler->socket = socket;
    handler->is_tcp = is_tcp;
    memcpy(&handler->client_addr, addr, sizeof(struct sockaddr_in));
    handler->state = 0;  /* NEW */
    
    /* Initialize session */
    if (generate_random_bytes(handler->session.session_id, SESSION_ID_SIZE) < 0) {
        LOG_ERROR("Failed to generate session ID");
        safe_free(handler);
        return NULL;
    }
    
    handler->session.created_at = time(NULL);
    handler->session.client_sequence = 0;
    handler->session.server_sequence = 0;
    handler->session.is_encrypted = 0;
    
    LOG_INFO("Client handler created (ID: %d)", client_id);
    return handler;
}

/* Destroy client handler */
void client_handler_destroy(client_handler_t* handler) {
    if (!handler) return;
    
    /* Secure clear session key */
    secure_memzero(handler->session.session_key, KEY_SIZE);
    secure_memzero(handler->server_nonce, NONCE_SIZE);
    secure_memzero(handler->client_nonce, NONCE_SIZE);
    
    safe_free(handler);
}

/* Handle client handshake */
int client_handler_handshake(client_handler_t* handler) {
    handshake_request_t client_req;
    handshake_response_t server_resp;
    
    if (!handler) return VPN_ERROR;
    
    LOG_INFO("[Client %d] Starting handshake", handler->client_id);
    
    /* Receive handshake request from client */
    if (recv_handshake_request(handler->socket, handler->is_tcp,
                               &handler->client_addr, &client_req) < 0) {
        LOG_ERROR("[Client %d] Failed to receive handshake", handler->client_id);
        return VPN_ERROR;
    }
    
    LOG_INFO("[Client %d] Handshake received", handler->client_id);
    
    /* Generate server nonce */
    if (generate_random_bytes(handler->server_nonce, NONCE_SIZE) < 0) {
        LOG_ERROR("[Client %d] Failed to generate nonce", handler->client_id);
        return VPN_ERROR;
    }
    
    /* Send server hello */
    if (send_server_hello(handler->socket, handler->is_tcp,
                          &handler->client_addr) < 0) {
        LOG_ERROR("[Client %d] Failed to send server hello", handler->client_id);
        return VPN_ERROR;
    }
    
    handler->state = 1;  /* HANDSHAKE_COMPLETE */
    LOG_INFO("[Client %d] Handshake complete", handler->client_id);
    
    return VPN_SUCCESS;
}

/* Handle key exchange */
int client_handler_key_exchange(client_handler_t* handler) {
    key_exchange_request_t client_req;
    key_exchange_response_t server_resp;
    unsigned char shared_secret[KEY_SIZE];
    unsigned char salt[32];
    
    if (!handler) return VPN_ERROR;
    
    LOG_INFO("[Client %d] Starting key exchange", handler->client_id);
    
    /* Receive key exchange from client */
    if (recv_key_exchange(handler->socket, handler->is_tcp,
                          &handler->client_addr, &client_req) < 0) {
        LOG_ERROR("[Client %d] Failed to receive key exchange", handler->client_id);
        return VPN_ERROR;
    }
    
    memcpy(handler->client_nonce, client_req.client_nonce, NONCE_SIZE);
    
    /* For this implementation, use simple key derivation */
    /* In production, use ECDH for ephemeral keys */
    
    /* Create salt from nonces */
    memcpy(salt, handler->server_nonce, NONCE_SIZE);
    memcpy(salt + NONCE_SIZE, handler->client_nonce, NONCE_SIZE);
    
    /* Derive session key */
    if (derive_key_from_password("vpn_session_key", 15,
                                 salt, sizeof(salt),
                                 handler->session.session_key, KEY_SIZE) < 0) {
        LOG_ERROR("[Client %d] Failed to derive session key", handler->client_id);
        return VPN_ERROR;
    }
    
    /* Generate session token */
    if (generate_random_bytes((unsigned char*)server_resp.session_token, 32) < 0) {
        LOG_ERROR("[Client %d] Failed to generate session token", handler->client_id);
        return VPN_ERROR;
    }
    
    /* Generate server ephemeral key */
    if (generate_random_bytes(server_resp.server_pubkey, 32) < 0) {
        LOG_ERROR("[Client %d] Failed to generate server key", handler->client_id);
        return VPN_ERROR;
    }
    
    /* Compute HMAC */
    hmac_sha256(handler->session.session_key, KEY_SIZE,
                server_resp.session_token, 32,
                server_resp.hmac);
    
    /* Create and send session start packet */
    vpn_packet_t packet;
    memset(&packet, 0, sizeof(packet));
    packet.header.type = PKT_SESSION_START;
    packet.header.sequence = 3;
    packet.header.flags = FLAG_REQUIRE_ACK;
    packet.header.payload_len = sizeof(key_exchange_response_t);
    packet.header.timestamp = (uint32_t)time(NULL);
    
    memcpy(packet.payload, &server_resp, sizeof(server_resp));
    memset(packet.mac_tag, 0, MAC_TAG_SIZE);
    
    if (send_packet(handler->socket, &packet, handler->is_tcp,
                    &handler->client_addr) < 0) {
        LOG_ERROR("[Client %d] Failed to send session start", handler->client_id);
        return VPN_ERROR;
    }
    
    handler->session.is_encrypted = 1;
    handler->state = 3;  /* CONNECTED */
    
    LOG_INFO("[Client %d] Key exchange complete - Session encrypted", handler->client_id);
    return VPN_SUCCESS;
}

/* Process encrypted data from client */
int client_handler_process_data(client_handler_t* handler) {
    unsigned char data[MAX_PAYLOAD_SIZE];
    int data_len;
    
    if (!handler) return VPN_ERROR;
    
    /* Receive and decrypt data */
    data_len = recv_encrypted_data(handler->socket, handler->is_tcp,
                                   &handler->client_addr, data,
                                   sizeof(data), &handler->session);
    
    if (data_len < 0) {
        LOG_ERROR("[Client %d] Failed to receive encrypted data", handler->client_id);
        return VPN_ERROR;
    }
    
    LOG_DEBUG("[Client %d] Received %d bytes of decrypted data",
              handler->client_id, data_len);
    
    /* TODO: Process/route the decrypted data */
    /* In a real VPN, this would tunnel the data through a TUN device */
    
    return VPN_SUCCESS;
}

/* Handle client disconnect */
int client_handler_disconnect(client_handler_t* handler) {
    if (!handler) return VPN_ERROR;
    
    LOG_INFO("[Client %d] Disconnecting", handler->client_id);
    
    if (handler->socket != INVALID_SOCKET) {
        close_socket(handler->socket);
        handler->socket = INVALID_SOCKET;
    }
    
    return VPN_SUCCESS;
}
