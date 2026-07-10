# VPN Server Architecture

## High-Level Overview

The VPN server is built as a multi-threaded hybrid protocol server that handles both UDP and TCP connections for VPN clients.

```
┌─────────────────────────────────────────────────────────┐
│                   VPN Server                             │
├─────────────────────────────────────────────────────────┤
│                                                          │
│  ┌──────────────────┐         ┌──────────────────┐    │
│  │  UDP Listener    │         │  TCP Listener    │    │
│  │  (Port 8080)     │         │  (Port 8080)     │    │
│  └────────┬─────────┘         └────────┬─────────┘    │
│           │                            │               │
│  ┌────────▼────────────────────────────▼──────────┐   │
│  │        Connection Handler Thread Pool          │   │
│  │  (Handles multiple clients simultaneously)     │   │
│  └────────┬─────────────────────────────────────────┐ │
│           │                                         │ │
│  ┌────────▼──────────────┐  ┌────────────────────┐│ │
│  │  Crypto Engine        │  │  Protocol Engine   ││ │
│  │  - AES-256            │  │  - Key Exchange    ││ │
│  │  - Encryption         │  │  - Packet Format   ││ │
│  │  - Decryption         │  │  - Routing         ││ │
│  └───────────────────────┘  └────────────────────┘│ │
│                                                    │ │
└────────────────────────────────────────────────────┘ │
         │                              │
         ▼                              ▼
    ┌─────────────┐          ┌──────────────────┐
    │ VPN Clients │          │ Encrypted Tunnel │
    └─────────────┘          └──────────────────┘
```

## Component Breakdown

### 1. **Main Server (main.c)**
- Entry point for the application
- Parses command-line arguments
- Initializes server
- Manages shutdown

```c
int main(int argc, char *argv[]) {
    // Parse arguments
    // Initialize server
    // Start listening
    // Accept connections
    // Cleanup
}
```

### 2. **Server Core (server.c/h)**
- Creates UDP and TCP listeners
- Manages thread pool
- Distributes incoming connections to handlers
- Maintains client registry

**Key Functions:**
```c
vpn_server_t* vpn_server_create(uint16_t port);
int vpn_server_start(vpn_server_t* server);
void vpn_server_shutdown(vpn_server_t* server);
void handle_client(client_t* client);
```

### 3. **Socket Utilities (socket_utils.c/h)**
Core socket operations for networking.

**Key Functions:**
```c
// UDP Operations
int create_udp_socket(uint16_t port);
int send_udp_packet(int sock, const char* data, size_t len, 
                    struct sockaddr_in* addr);
int recv_udp_packet(int sock, char* buffer, size_t max_len,
                    struct sockaddr_in* addr);

// TCP Operations
int create_tcp_socket(uint16_t port);
int accept_tcp_connection(int listen_sock);
int send_tcp_data(int sock, const char* data, size_t len);
int recv_tcp_data(int sock, char* buffer, size_t max_len);

// Utilities
void set_nonblocking(int sock);
int close_socket(int sock);
```

### 4. **Cryptography Engine (crypto.c/h)**
Handles all encryption/decryption using OpenSSL.

**Key Functions:**
```c
// Key Management
unsigned char* generate_key(size_t key_len);
void derive_key(const char* password, unsigned char* key);

// Encryption
int encrypt_aes256(const unsigned char* plaintext, int plaintext_len,
                   const unsigned char* key, unsigned char* ciphertext);

// Decryption
int decrypt_aes256(const unsigned char* ciphertext, int ciphertext_len,
                   const unsigned char* key, unsigned char* plaintext);

// Hashing
void hash_data(const unsigned char* data, size_t len, 
               unsigned char* hash);
```

### 5. **Protocol Handler (protocol.c/h)**
Implements the VPN protocol (packet structure, handshake, routing).

**Packet Structure:**
```
┌──────────┬──────────┬──────────┬──────────────┬────────────┐
│ Type (1) │ Seq (4)  │ Flags(1) │ Length (2)   │ Payload    │
├──────────┼──────────┼──────────┼──────────────┼────────────┤
│ Byte     │ Bytes    │ Byte     │ Bytes        │ Variable   │
└──────────┴──────────┴──────────┴──────────────┴────────────┘
```

**Packet Types:**
```c
#define PKT_HANDSHAKE     0x01  // Initial connection
#define PKT_KEY_EXCHANGE  0x02  // Key agreement
#define PKT_DATA          0x03  // Encrypted tunnel data
#define PKT_ACK           0x04  // Acknowledgment
#define PKT_DISCONNECT    0x05  // Connection close
```

**Key Functions:**
```c
vpn_packet_t* create_packet(uint8_t type, const void* payload, 
                            uint16_t payload_len);
int parse_packet(const unsigned char* data, size_t len, 
                 vpn_packet_t* packet);
int send_packet(int sock, vpn_packet_t* packet, int is_tcp);
int recv_packet(int sock, vpn_packet_t* packet, int is_tcp);
```

### 6. **Client Handler (client_handler.c/h)**
Manages individual client connections and state.

**Client State Machine:**
```
[NEW] → [HANDSHAKE] → [KEY_EXCHANGE] → [CONNECTED] → [DISCONNECT]
         ↓ (on error) ↓                 ↓ (on error)
         └─────────────────[ERROR]─────────────────┘
```

**Key Functions:**
```c
client_t* client_create(int socket, int is_tcp);
int client_handshake(client_t* client);
int client_key_exchange(client_t* client);
int client_process_data(client_t* client);
void client_disconnect(client_t* client);
```

## Data Flow

### Connection Establishment

```
1. Client Connects
   ↓
2. Server Accepts (UDP or TCP)
   ↓
3. Handshake Exchange
   - Server sends server_id, nonce
   - Client responds with client_id
   ↓
4. Key Exchange
   - Generate ephemeral keys
   - Exchange public keys
   - Derive shared secret
   - Establish cipher context
   ↓
5. Ready for Encrypted Data
   - All traffic is now encrypted
```

### Data Transmission

```
Client Data → Plaintext
             ↓
         Encryption (AES-256)
             ↓
        Create VPN Packet
             ↓
        UDP/TCP Send
             ↓
        Server Receive
             ↓
        Parse Packet
             ↓
        Decryption (AES-256)
             ↓
    Process/Route Plaintext
```

## Threading Model

```
Main Thread
├── UDP Listener Thread (blocking accept on UDP socket)
├── TCP Listener Thread (blocking accept on TCP socket)
└── Thread Pool
    ├── Worker Thread 1 ┐
    ├── Worker Thread 2 ├─→ Handle client connections
    ├── Worker Thread 3 │
    └── Worker Thread N ┘
```

## Hybrid Protocol Strategy

### When to Use UDP
- Fast, low-latency applications
- Tolerant of packet loss (video, audio)
- Streaming data

### When to Use TCP
- Reliable data transmission
- File transfers
- When packet order matters

**Client chooses based on needs in handshake.**

## Error Handling

```c
// All functions return:
// 0 or positive value  = success
// -1                   = error
// NULL                 = allocation failure

// Always check return values
if (encrypt_aes256(...) < 0) {
    // handle error
    log_error("Encryption failed");
}
```

## Security Considerations

1. **Key Management**
   - Keys never logged
   - Keys cleared after use (memset)
   - Ephemeral keys for each session

2. **Encryption**
   - AES-256 for confidentiality
   - HMAC for authenticity (future)
   - Random IVs for each packet

3. **Authentication**
   - Password-based key derivation (PBKDF2)
   - Client certificate support (future)

## File Structure Summary

| File | Purpose |
|------|---------|
| main.c | Application entry point |
| server.c/h | Server core logic |
| socket_utils.c/h | Socket operations |
| crypto.c/h | Encryption/decryption |
| protocol.c/h | Packet format & handshake |
| client_handler.c/h | Client state management |

## Compilation Dependencies

- **OpenSSL** - Cryptography
- **pthreads** - Threading (Linux/macOS) / WinThreads (Windows)
- **Standard C Library** - Core functionality

This architecture balances learning value with practical VPN functionality!
