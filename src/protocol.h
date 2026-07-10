#ifndef VPN_PROTOCOL_H
#define VPN_PROTOCOL_H

#include "common.h"

/* Protocol version */
#define PROTOCOL_VERSION 0x01

/* Handshake structures */
typedef struct {
    uint8_t client_id[16];
    uint8_t protocol_version;
    uint8_t supported_ciphers;
    uint8_t preferred_mode;  /* 1=UDP, 2=TCP, 3=Both */
} handshake_request_t;

typedef struct {
    uint8_t server_id[16];
    uint8_t server_nonce[NONCE_SIZE];
    uint8_t supported_ciphers;
    uint8_t selected_mode;
} handshake_response_t;

/* Key exchange structures */
typedef struct {
    uint8_t client_nonce[NONCE_SIZE];
    unsigned char ephemeral_pubkey[32];
    unsigned char hmac[MAC_SIZE];
} key_exchange_request_t;

typedef struct {
    uint8_t session_token[32];
    unsigned char server_pubkey[32];
    unsigned char hmac[MAC_SIZE];
} key_exchange_response_t;

/* Protocol functions */
int pack_packet(vpn_packet_t* packet, unsigned char* buffer, size_t max_len);
int unpack_packet(const unsigned char* buffer, size_t len, vpn_packet_t* packet);

int send_packet(int sock, vpn_packet_t* packet, int is_tcp,
                struct sockaddr_in* dest_addr);
int recv_packet(int sock, vpn_packet_t* packet, int is_tcp,
                struct sockaddr_in* src_addr);

/* Handshake */
int send_handshake_request(int sock, int is_tcp, struct sockaddr_in* dest_addr);
int recv_handshake_request(int sock, int is_tcp, struct sockaddr_in* src_addr,
                           handshake_request_t* req);
int send_server_hello(int sock, int is_tcp, struct sockaddr_in* dest_addr);
int recv_server_hello(int sock, int is_tcp, struct sockaddr_in* src_addr,
                      handshake_response_t* resp);

/* Key exchange */
int send_key_exchange(int sock, int is_tcp, struct sockaddr_in* dest_addr,
                      const key_exchange_request_t* req);
int recv_key_exchange(int sock, int is_tcp, struct sockaddr_in* src_addr,
                      key_exchange_request_t* req);

/* Data transmission */
int send_encrypted_data(int sock, int is_tcp, struct sockaddr_in* dest_addr,
                        const unsigned char* data, int data_len,
                        session_t* session);
int recv_encrypted_data(int sock, int is_tcp, struct sockaddr_in* src_addr,
                        unsigned char* data, int max_len,
                        session_t* session);

#endif /* VPN_PROTOCOL_H */
