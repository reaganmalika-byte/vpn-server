#ifndef VPN_SERVER_H
#define VPN_SERVER_H

#include "common.h"
#include "socket_utils.h"
#include "crypto.h"
#include "protocol.h"

/* Server state */
struct vpn_server;
typedef struct vpn_server vpn_server_t;

/* Client info */
typedef struct {
    int client_id;
    int socket;
    int is_tcp;
    struct sockaddr_in client_addr;
    session_t session;
    time_t last_ping;
    int active;
} client_info_t;

/* Server creation and lifecycle */
vpn_server_t* vpn_server_create(uint16_t port);
int vpn_server_start(vpn_server_t* server);
int vpn_server_run(vpn_server_t* server);
void vpn_server_shutdown(vpn_server_t* server);
void vpn_server_destroy(vpn_server_t* server);

/* Client management */
int add_client(vpn_server_t* server, int socket, int is_tcp,
               struct sockaddr_in* addr);
void remove_client(vpn_server_t* server, int client_id);
client_info_t* get_client(vpn_server_t* server, int client_id);

/* Handler threads */
void* handle_client_connection(void* arg);
void* handle_udp_listener(void* arg);
void* handle_tcp_listener(void* arg);

#endif /* VPN_SERVER_H */
