#ifndef VPN_CLIENT_HANDLER_H
#define VPN_CLIENT_HANDLER_H

#include "common.h"
#include "protocol.h"

/* Client handler structure */
typedef struct {
    int client_id;
    int socket;
    int is_tcp;
    struct sockaddr_in client_addr;
    session_t session;
    uint8_t server_nonce[NONCE_SIZE];
    uint8_t client_nonce[NONCE_SIZE];
    int state;  /* 0=new, 1=handshake, 2=key_exchange, 3=connected */
} client_handler_t;

/* Client handler functions */
client_handler_t* client_handler_create(int client_id, int socket, int is_tcp,
                                        struct sockaddr_in* addr);
void client_handler_destroy(client_handler_t* handler);

int client_handler_handshake(client_handler_t* handler);
int client_handler_key_exchange(client_handler_t* handler);
int client_handler_process_data(client_handler_t* handler);
int client_handler_disconnect(client_handler_t* handler);

#endif /* VPN_CLIENT_HANDLER_H */
