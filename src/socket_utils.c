#include "socket_utils.h"

#ifdef _WIN32
    #include <fcntl.h>
#endif

/* Create UDP socket bound to port */
int create_udp_socket(uint16_t port) {
    struct sockaddr_in addr;
    int sock;
    int opt = 1;
    
    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_SOCKET) {
        LOG_ERROR("Failed to create UDP socket");
        return VPN_ERROR_SOCKET;
    }
    
    /* Allow socket reuse */
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR,
                   (const char*)&opt, sizeof(opt)) < 0) {
        LOG_ERROR("Failed to set socket options");
        closesocket(sock);
        return VPN_ERROR_SOCKET;
    }
    
    /* Bind to port */
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);
    
    if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        LOG_ERROR("Failed to bind UDP socket to port %u", port);
        closesocket(sock);
        return VPN_ERROR_SOCKET;
    }
    
    LOG_INFO("UDP socket created and bound to port %u", port);
    return sock;
}

/* Create TCP socket bound to port */
int create_tcp_socket(uint16_t port) {
    struct sockaddr_in addr;
    int sock;
    int opt = 1;
    
    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        LOG_ERROR("Failed to create TCP socket");
        return VPN_ERROR_SOCKET;
    }
    
    /* Allow socket reuse */
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR,
                   (const char*)&opt, sizeof(opt)) < 0) {
        LOG_ERROR("Failed to set socket options");
        closesocket(sock);
        return VPN_ERROR_SOCKET;
    }
    
    /* Bind to port */
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);
    
    if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        LOG_ERROR("Failed to bind TCP socket to port %u", port);
        closesocket(sock);
        return VPN_ERROR_SOCKET;
    }
    
    /* Listen for connections */
    if (listen(sock, 5) < 0) {
        LOG_ERROR("Failed to listen on TCP socket");
        closesocket(sock);
        return VPN_ERROR_SOCKET;
    }
    
    LOG_INFO("TCP socket created and listening on port %u", port);
    return sock;
}

/* Send UDP packet */
int send_udp_packet(int sock, const unsigned char* data, size_t len,
                    struct sockaddr_in* dest_addr) {
    int sent;
    
    if (!data || len == 0) return VPN_ERROR;
    
    sent = sendto(sock, (const char*)data, (int)len, 0,
                  (struct sockaddr*)dest_addr, sizeof(*dest_addr));
    
    if (sent < 0) {
        LOG_ERROR("Failed to send UDP packet");
        return VPN_ERROR_SOCKET;
    }
    
    if (sent != (int)len) {
        LOG_WARN("Partial UDP send: %d of %zu bytes", sent, len);
    }
    
    return sent;
}

/* Receive UDP packet */
int recv_udp_packet(int sock, unsigned char* buffer, size_t max_len,
                    struct sockaddr_in* src_addr) {
    int received;
    socklen_t addr_len = sizeof(*src_addr);
    
    if (!buffer || max_len == 0) return VPN_ERROR;
    
    received = recvfrom(sock, (char*)buffer, (int)max_len, 0,
                        (struct sockaddr*)src_addr, &addr_len);
    
    if (received < 0) {
        LOG_ERROR("Failed to receive UDP packet");
        return VPN_ERROR_SOCKET;
    }
    
    return received;
}

/* Send TCP data */
int send_tcp_data(int sock, const unsigned char* data, size_t len) {
    int sent;
    size_t total_sent = 0;
    
    if (!data || len == 0) return VPN_ERROR;
    
    while (total_sent < len) {
        sent = send(sock, (const char*)data + total_sent,
                   (int)(len - total_sent), 0);
        
        if (sent < 0) {
            LOG_ERROR("Failed to send TCP data");
            return VPN_ERROR_SOCKET;
        }
        
        if (sent == 0) {
            LOG_WARN("TCP connection closed by peer");
            return VPN_ERROR_SOCKET;
        }
        
        total_sent += sent;
    }
    
    return (int)total_sent;
}

/* Receive TCP data */
int recv_tcp_data(int sock, unsigned char* buffer, size_t max_len) {
    int received;
    
    if (!buffer || max_len == 0) return VPN_ERROR;
    
    received = recv(sock, (char*)buffer, (int)max_len, 0);
    
    if (received < 0) {
        LOG_ERROR("Failed to receive TCP data");
        return VPN_ERROR_SOCKET;
    }
    
    if (received == 0) {
        LOG_INFO("TCP connection closed by peer");
        return 0;
    }
    
    return received;
}

/* Accept TCP connection */
int accept_tcp_connection(int listen_sock, struct sockaddr_in* client_addr) {
    socklen_t addr_len = sizeof(*client_addr);
    int client_sock;
    
    if (!client_addr) return VPN_ERROR_SOCKET;
    
    client_sock = accept(listen_sock, (struct sockaddr*)client_addr, &addr_len);
    
    if (client_sock == INVALID_SOCKET) {
        LOG_ERROR("Failed to accept TCP connection");
        return VPN_ERROR_SOCKET;
    }
    
    return client_sock;
}

/* Set socket to non-blocking mode */
void set_nonblocking(int sock) {
#ifdef _WIN32
    unsigned long mode = 1;
    if (ioctlsocket(sock, FIONBIO, &mode) != NO_ERROR) {
        LOG_ERROR("Failed to set non-blocking mode");
    }
#else
    int flags = fcntl(sock, F_GETFL, 0);
    if (fcntl(sock, F_SETFL, flags | O_NONBLOCK) < 0) {
        LOG_ERROR("Failed to set non-blocking mode");
    }
#endif
}

/* Set socket to blocking mode */
void set_blocking(int sock) {
#ifdef _WIN32
    unsigned long mode = 0;
    if (ioctlsocket(sock, FIONBIO, &mode) != NO_ERROR) {
        LOG_ERROR("Failed to set blocking mode");
    }
#else
    int flags = fcntl(sock, F_GETFL, 0);
    if (fcntl(sock, F_SETFL, flags & ~O_NONBLOCK) < 0) {
        LOG_ERROR("Failed to set blocking mode");
    }
#endif
}

/* Close socket */
int close_socket(int sock) {
    if (sock == INVALID_SOCKET) return VPN_ERROR;
    
    shutdown(sock, SHUT_RDWR);
    
    if (closesocket(sock) < 0) {
        LOG_ERROR("Failed to close socket");
        return VPN_ERROR_SOCKET;
    }
    
    return VPN_SUCCESS;
}

/* Get socket error */
int get_socket_error(int sock) {
    int error = 0;
    socklen_t error_len = sizeof(error);
    
    if (getsockopt(sock, SOL_SOCKET, SO_ERROR, (char*)&error, &error_len) < 0) {
        return VPN_ERROR_SOCKET;
    }
    
    return error;
}
