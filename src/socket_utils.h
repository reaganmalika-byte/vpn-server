#ifndef VPN_SOCKET_UTILS_H
#define VPN_SOCKET_UTILS_H

#include "common.h"

/* Socket creation */
int create_udp_socket(uint16_t port);
int create_tcp_socket(uint16_t port);

/* UDP operations */
int send_udp_packet(int sock, const unsigned char* data, size_t len,
                    struct sockaddr_in* dest_addr);
int recv_udp_packet(int sock, unsigned char* buffer, size_t max_len,
                    struct sockaddr_in* src_addr);

/* TCP operations */
int send_tcp_data(int sock, const unsigned char* data, size_t len);
int recv_tcp_data(int sock, unsigned char* buffer, size_t max_len);
int accept_tcp_connection(int listen_sock, struct sockaddr_in* client_addr);

/* Socket utilities */
void set_nonblocking(int sock);
void set_blocking(int sock);
int close_socket(int sock);
int get_socket_error(int sock);

#endif /* VPN_SOCKET_UTILS_H */
