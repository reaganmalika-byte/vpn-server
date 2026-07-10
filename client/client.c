#include "common.h"
#include "socket_utils.h"
#include "crypto.h"
#include "protocol.h"

/* VPN Client structure */
typedef struct {
    char* server_host;
    uint16_t server_port;
    int socket;
    int is_tcp;
    struct sockaddr_in server_addr;
    session_t session;
} vpn_client_t;

/* Create VPN client */
vpn_client_t* vpn_client_create(const char* host, uint16_t port) {
    vpn_client_t* client = (vpn_client_t*)safe_malloc(sizeof(vpn_client_t));
    
    client->server_host = (char*)safe_malloc(strlen(host) + 1);
    strcpy(client->server_host, host);
    client->server_port = port;
    client->socket = INVALID_SOCKET;
    client->is_tcp = 1;  /* Use TCP by default */
    
    /* Initialize session */
    if (generate_random_bytes(client->session.session_id, SESSION_ID_SIZE) < 0) {
        LOG_ERROR("Failed to generate session ID");
        safe_free(client->server_host);
        safe_free(client);
        return NULL;
    }
    
    client->session.created_at = time(NULL);
    client->session.client_sequence = 0;
    client->session.server_sequence = 0;
    client->session.is_encrypted = 0;
    
    return client;
}

/* Connect to server */
int vpn_client_connect(vpn_client_t* client) {
    struct hostent* host;
    
    if (!client) return VPN_ERROR;
    
    LOG_INFO("Connecting to %s:%u", client->server_host, client->server_port);
    
    /* Resolve hostname */
    host = gethostbyname(client->server_host);
    if (!host) {
        LOG_ERROR("Failed to resolve hostname: %s", client->server_host);
        return VPN_ERROR_SOCKET;
    }
    
    /* Setup server address */
    memset(&client->server_addr, 0, sizeof(client->server_addr));
    client->server_addr.sin_family = AF_INET;
    client->server_addr.sin_port = htons(client->server_port);
    memcpy(&client->server_addr.sin_addr, host->h_addr, host->h_length);
    
    /* Create and connect TCP socket */
    client->socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (client->socket == INVALID_SOCKET) {
        LOG_ERROR("Failed to create socket");
        return VPN_ERROR_SOCKET;
    }
    
    if (connect(client->socket, (struct sockaddr*)&client->server_addr,
                sizeof(client->server_addr)) < 0) {
        LOG_ERROR("Failed to connect to server");
        closesocket(client->socket);
        return VPN_ERROR_SOCKET;
    }
    
    LOG_INFO("Connected to server");
    return VPN_SUCCESS;
}

/* Perform handshake with server */
int vpn_client_handshake(vpn_client_t* client) {
    handshake_response_t server_resp;
    
    if (!client) return VPN_ERROR;
    
    LOG_INFO("Starting handshake");
    
    /* Send handshake request */
    if (send_handshake_request(client->socket, client->is_tcp,
                               &client->server_addr) < 0) {
        LOG_ERROR("Failed to send handshake");
        return VPN_ERROR;
    }
    
    /* Receive server hello */
    if (recv_server_hello(client->socket, client->is_tcp,
                          &client->server_addr, &server_resp) < 0) {
        LOG_ERROR("Failed to receive server hello");
        return VPN_ERROR;
    }
    
    LOG_INFO("Handshake complete");
    return VPN_SUCCESS;
}

/* Perform key exchange with server */
int vpn_client_key_exchange(vpn_client_t* client,
                            const uint8_t* server_nonce) {
    key_exchange_request_t req;
    vpn_packet_t packet;
    unsigned char salt[32];
    
    if (!client || !server_nonce) return VPN_ERROR;
    
    LOG_INFO("Starting key exchange");
    
    /* Generate client nonce */
    if (generate_random_bytes(req.client_nonce, NONCE_SIZE) < 0) {
        LOG_ERROR("Failed to generate nonce");
        return VPN_ERROR;
    }
    
    /* Generate ephemeral key */
    if (generate_random_bytes(req.ephemeral_pubkey, 32) < 0) {
        LOG_ERROR("Failed to generate ephemeral key");
        return VPN_ERROR;
    }
    
    /* Compute HMAC */
    hmac_sha256((unsigned char*)"client_secret", 13,
                req.client_nonce, NONCE_SIZE,
                req.hmac);
    
    /* Send key exchange */
    if (send_key_exchange(client->socket, client->is_tcp,
                          &client->server_addr, &req) < 0) {
        LOG_ERROR("Failed to send key exchange");
        return VPN_ERROR;
    }
    
    /* Receive session start */
    if (recv_packet(client->socket, &packet, client->is_tcp,
                    &client->server_addr) < 0) {
        LOG_ERROR("Failed to receive session start");
        return VPN_ERROR;
    }
    
    if (packet.header.type != PKT_SESSION_START) {
        LOG_ERROR("Expected session start packet");
        return VPN_ERROR;
    }
    
    /* Derive session key (same as server) */
    memcpy(salt, server_nonce, NONCE_SIZE);
    memcpy(salt + NONCE_SIZE, req.client_nonce, NONCE_SIZE);
    
    if (derive_key_from_password("vpn_session_key", 15,
                                 salt, sizeof(salt),
                                 client->session.session_key, KEY_SIZE) < 0) {
        LOG_ERROR("Failed to derive session key");
        return VPN_ERROR;
    }
    
    client->session.is_encrypted = 1;
    LOG_INFO("Key exchange complete - Tunnel encrypted");
    
    return VPN_SUCCESS;
}

/* Send data through tunnel */
int vpn_client_send_data(vpn_client_t* client,
                         const unsigned char* data, int len) {
    if (!client || !data || len <= 0) return VPN_ERROR;
    
    return send_encrypted_data(client->socket, client->is_tcp,
                               &client->server_addr, data, len,
                               &client->session);
}

/* Receive data from tunnel */
int vpn_client_recv_data(vpn_client_t* client,
                         unsigned char* data, int max_len) {
    if (!client || !data || max_len <= 0) return VPN_ERROR;
    
    return recv_encrypted_data(client->socket, client->is_tcp,
                               &client->server_addr, data, max_len,
                               &client->session);
}

/* Disconnect from server */
int vpn_client_disconnect(vpn_client_t* client) {
    if (!client) return VPN_ERROR;
    
    LOG_INFO("Disconnecting from server");
    
    if (client->socket != INVALID_SOCKET) {
        close_socket(client->socket);
        client->socket = INVALID_SOCKET;
    }
    
    return VPN_SUCCESS;
}

/* Destroy client */
void vpn_client_destroy(vpn_client_t* client) {
    if (!client) return;
    
    vpn_client_disconnect(client);
    secure_memzero(client->session.session_key, KEY_SIZE);
    safe_free(client->server_host);
    safe_free(client);
}

/* Main client program */
int main(int argc, char* argv[]) {
    vpn_client_t* client;
    uint16_t port = 8080;
    unsigned char test_data[100];
    int received;
    
    printf("VPN Client - Custom C Implementation\n");
    printf("Version 1.0\n\n");
    
#ifdef _WIN32
    WSADATA wsa_data;
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
        LOG_ERROR("WSAStartup failed");
        return 1;
    }
#endif
    
    if (argc < 2) {
        printf("Usage: %s <server_ip> [port]\n", argv[0]);
        printf("Example: %s 127.0.0.1 8080\n", argv[0]);
        return 1;
    }
    
    if (argc > 2) {
        port = (uint16_t)atoi(argv[2]);
    }
    
    /* Create client */
    client = vpn_client_create(argv[1], port);
    if (!client) {
        LOG_ERROR("Failed to create VPN client");
        return 1;
    }
    
    /* Connect to server */
    if (vpn_client_connect(client) != VPN_SUCCESS) {
        LOG_ERROR("Failed to connect to server");
        vpn_client_destroy(client);
        return 1;
    }
    
    /* Perform handshake */
    if (vpn_client_handshake(client) != VPN_SUCCESS) {
        LOG_ERROR("Handshake failed");
        vpn_client_destroy(client);
        return 1;
    }
    
    /* TODO: Get server nonce from handshake response and pass it */
    /* For now, use a dummy nonce */
    unsigned char dummy_nonce[NONCE_SIZE];
    memset(dummy_nonce, 0, NONCE_SIZE);
    
    /* Perform key exchange */
    if (vpn_client_key_exchange(client, dummy_nonce) != VPN_SUCCESS) {
        LOG_ERROR("Key exchange failed");
        vpn_client_destroy(client);
        return 1;
    }
    
    LOG_INFO("\n=== VPN Tunnel Established ===");
    LOG_INFO("Connection to %s:%u successful", argv[1], port);
    LOG_INFO("Tunnel is encrypted\n");
    
    /* Send test data */
    strcpy((char*)test_data, "Hello from VPN Client!");
    LOG_INFO("Sending: %s", (char*)test_data);
    
    if (vpn_client_send_data(client, test_data, strlen((char*)test_data)) < 0) {
        LOG_ERROR("Failed to send data");
    }
    
    /* Try to receive response */
    memset(test_data, 0, sizeof(test_data));
    received = vpn_client_recv_data(client, test_data, sizeof(test_data) - 1);
    
    if (received > 0) {
        test_data[received] = '\0';
        LOG_INFO("Received: %s", (char*)test_data);
    }
    
    /* Cleanup */
    vpn_client_disconnect(client);
    vpn_client_destroy(client);
    
#ifdef _WIN32
    WSACleanup();
#endif
    
    LOG_INFO("Client closed");
    return 0;
}
