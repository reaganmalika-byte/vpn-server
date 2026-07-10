#ifndef VPN_COMMON_H
#define VPN_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

/* Platform-specific includes */
#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <windows.h>
    #pragma comment(lib, "ws2_32.lib")
    typedef int socklen_t;
    #define SHUT_RDWR SD_BOTH
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <pthread.h>
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
    #define closesocket close
#endif

#include <openssl/aes.h>
#include <openssl/rand.h>
#include <openssl/sha.h>
#include <openssl/evp.h>

/* Logging macros */
#define LOG_INFO(fmt, ...) printf("[INFO] " fmt "\n", ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) fprintf(stderr, "[ERROR] " fmt "\n", ##__VA_ARGS__)
#define LOG_DEBUG(fmt, ...) printf("[DEBUG] " fmt "\n", ##__VA_ARGS__)
#define LOG_WARN(fmt, ...) printf("[WARN] " fmt "\n", ##__VA_ARGS__)

/* Return codes */
#define VPN_SUCCESS 0
#define VPN_ERROR -1
#define VPN_ERROR_CRYPTO -2
#define VPN_ERROR_SOCKET -3
#define VPN_ERROR_PROTOCOL -4

/* Sizes */
#define MAX_PACKET_SIZE 65536
#define MAX_PAYLOAD_SIZE 65000
#define KEY_SIZE 32  /* AES-256 */
#define IV_SIZE 16
#define NONCE_SIZE 16
#define MAC_SIZE 32
#define SESSION_ID_SIZE 32
#define MAC_TAG_SIZE 16
#define MAX_CLIENTS 100

/* Packet types */
#define PKT_HANDSHAKE 0x01
#define PKT_SERVER_HELLO 0x02
#define PKT_KEY_EXCHANGE 0x03
#define PKT_SESSION_START 0x04
#define PKT_DATA 0x05
#define PKT_ACK 0x06
#define PKT_PING 0x07
#define PKT_PONG 0x08
#define PKT_DISCONNECT 0x09
#define PKT_ERROR 0x0A

/* Packet flags */
#define FLAG_REQUIRE_ACK 0x01
#define FLAG_FRAGMENTED 0x02
#define FLAG_TCP_MODE 0x04
#define FLAG_UDP_MODE 0x08
#define FLAG_COMPRESSION 0x10

/* Error codes */
#define ERR_INVALID_PACKET 0x01
#define ERR_AUTH_FAILED 0x02
#define ERR_VERSION_MISMATCH 0x03
#define ERR_UNSUPPORTED_CIPHER 0x04
#define ERR_DECRYPTION_FAILED 0x05
#define ERR_REPLAY_ATTACK 0x06
#define ERR_SESSION_EXPIRED 0x07
#define ERR_INTERNAL_ERROR 0xFF

/* Packet header structure */
typedef struct {
    uint8_t type;           /* Packet type */
    uint32_t sequence;      /* Sequence number */
    uint8_t flags;          /* Control flags */
    uint16_t payload_len;   /* Payload length */
    uint32_t timestamp;     /* Unix timestamp */
} vpn_packet_header_t;

/* Full packet structure */
typedef struct {
    vpn_packet_header_t header;
    unsigned char payload[MAX_PAYLOAD_SIZE];
    unsigned char mac_tag[MAC_TAG_SIZE];
} vpn_packet_t;

/* Client session info */
typedef struct {
    uint8_t session_id[SESSION_ID_SIZE];
    unsigned char session_key[KEY_SIZE];
    uint32_t client_sequence;
    uint32_t server_sequence;
    time_t created_at;
    time_t last_activity;
    int is_encrypted;
} session_t;

/* Utility functions */
void* safe_malloc(size_t size);
void safe_free(void* ptr);
int bytes_to_hex(const unsigned char* bytes, int len, char* hex_str);
int hex_to_bytes(const char* hex_str, unsigned char* bytes);

#endif /* VPN_COMMON_H */
