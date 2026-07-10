# VPN Socket Programming Guide

This document explains the socket programming concepts used in the VPN server.

## Socket Programming Basics

### What is a Socket?

A socket is an endpoint for network communication. Think of it as a virtual "port" through which your program communicates with other programs over a network.

```
Your App ←→ Socket ←→ Network ←→ Remote App
```

### Two Main Socket Types

#### 1. UDP (User Datagram Protocol)

**Characteristics:**
- Fast (no connection overhead)
- Unreliable (packets can be lost)
- No ordering guarantee
- Low latency
- Good for: video, audio, real-time data

**Basic Flow:**
```
Server:                          Client:
1. Create UDP socket            1. Create UDP socket
2. Bind to port                 2. Send packet to server
3. Wait for packets             3. Wait for response
4. Receive packet
5. Send response
```

**Code Example:**
```c
// Server
int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
struct sockaddr_in addr;
addr.sin_family = AF_INET;
addr.sin_addr.s_addr = INADDR_ANY;
addr.sin_port = htons(8080);
bind(sock, (struct sockaddr*)&addr, sizeof(addr));

// Receive packet
unsigned char buffer[1024];
struct sockaddr_in client_addr;
socklen_t addr_len = sizeof(client_addr);
int received = recvfrom(sock, buffer, 1024, 0,
                        (struct sockaddr*)&client_addr, &addr_len);

// Send response
sendto(sock, response, len, 0, (struct sockaddr*)&client_addr, sizeof(client_addr));
```

#### 2. TCP (Transmission Control Protocol)

**Characteristics:**
- Slow (connection setup overhead)
- Reliable (packets guaranteed)
- Ordered delivery
- Connection-based
- Good for: file transfers, web, reliable data

**Basic Flow:**
```
Server:                          Client:
1. Create TCP socket            1. Create TCP socket
2. Bind to port                 2. Connect to server
3. Listen for connections       3. Send data
4. Accept connection            4. Wait for response
5. Receive data
6. Send response
7. Close connection
```

**Code Example:**
```c
// Server
int server_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
struct sockaddr_in addr;
addr.sin_family = AF_INET;
addr.sin_addr.s_addr = INADDR_ANY;
addr.sin_port = htons(8080);
bind(server_sock, (struct sockaddr*)&addr, sizeof(addr));
listen(server_sock, 5);  // Listen for up to 5 connections

// Accept connection
struct sockaddr_in client_addr;
socklen_t addr_len = sizeof(client_addr);
int client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &addr_len);

// Receive data
unsigned char buffer[1024];
int received = recv(client_sock, buffer, 1024, 0);

// Send response
send(client_sock, response, len, 0);

// Close
close(client_sock);
close(server_sock);
```

---

## VPN Server Socket Architecture

### Multi-Protocol Support

The VPN server supports both UDP and TCP:

```c
// In server.c
int udp_socket = create_udp_socket(8080);  // UDP listener
int tcp_socket = create_tcp_socket(8080);  // TCP listener

// Both listen on the same port
// Clients choose which protocol to use
```

### Socket States

```
┌─────────────────────────────────────────────────┐
│                Socket Lifecycle                 │
├─────────────────────────────────────────────────┤
│                                                 │
│  CREATE → BIND → LISTEN → ACCEPT → RECEIVE    │
│                                      ↓          │
│                                    SEND        │
│                                      ↓          │
│                                    CLOSE       │
│                                                 │
└─────────────────────────────────────────────────┘
```

### Creating a UDP Socket

```c
int create_udp_socket(uint16_t port) {
    // 1. Create socket
    int sock = socket(AF_INET,      // IPv4
                      SOCK_DGRAM,   // UDP
                      IPPROTO_UDP); // UDP protocol
    
    // 2. Allow socket reuse
    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    // 3. Bind to port
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);  // Listen on all interfaces
    addr.sin_port = htons(port);                // Convert port to network byte order
    
    bind(sock, (struct sockaddr*)&addr, sizeof(addr));
    
    return sock;
}
```

### Creating a TCP Socket

```c
int create_tcp_socket(uint16_t port) {
    // 1. Create socket
    int sock = socket(AF_INET,       // IPv4
                      SOCK_STREAM,   // TCP
                      IPPROTO_TCP);  // TCP protocol
    
    // 2. Allow socket reuse
    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    // 3. Bind to port
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);
    bind(sock, (struct sockaddr*)&addr, sizeof(addr));
    
    // 4. Listen for connections
    listen(sock, 5);  // Queue up to 5 pending connections
    
    return sock;
}
```

---

## Network Byte Order

**Important Concept:** Computers store numbers in different byte orders!

```c
// Example: Port 8080 in decimal
// Binary: 0001 1111 1001 1000
// But which byte comes first?

// htons = Host TO Network Short (16-bit)
unit16_t port_net = htons(8080);

// ntohs = Network TO Host Short
uint16_t port_host = ntohs(port_net);

// For 32-bit values:
htonl()  // Host TO Network Long
ntohl()  // Network TO Host Long
```

Always use these functions when dealing with network values!

---

## Data Transmission

### UDP Send/Receive

```c
// SEND
struct sockaddr_in dest;
dest.sin_family = AF_INET;
dest.sin_addr.s_addr = inet_aton("127.0.0.1");
dest.sin_port = htons(8080);

int sent = sendto(udp_sock,           // Socket
                  data,               // Data to send
                  len,                // Data length
                  0,                  // Flags
                  (struct sockaddr*)&dest,  // Destination address
                  sizeof(dest));      // Address size

// RECEIVE
struct sockaddr_in src;
socklen_t src_len = sizeof(src);
int received = recvfrom(udp_sock,     // Socket
                        buffer,       // Receive buffer
                        max_len,      // Buffer size
                        0,            // Flags
                        (struct sockaddr*)&src,  // Source address (filled by kernel)
                        &src_len);    // Address size
```

### TCP Send/Receive

```c
// SEND (guaranteed all data sent)
int total_sent = 0;
while (total_sent < len) {
    int sent = send(tcp_sock, data + total_sent, len - total_sent, 0);
    if (sent < 0) break;  // Error
    if (sent == 0) break; // Connection closed
    total_sent += sent;
}

// RECEIVE
int received = recv(tcp_sock,  // Socket
                    buffer,    // Receive buffer
                    max_len,   // Buffer size
                    0);        // Flags

if (received == 0) {
    // Connection closed by peer
}
if (received < 0) {
    // Error
}
```

---

## Important Socket Functions Reference

### Creating Sockets
```c
int socket(int domain, int type, int protocol);
// Returns: socket file descriptor, or -1 on error

// Examples:
socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);  // UDP
socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); // TCP
socket(AF_INET6, SOCK_STREAM, 0);          // IPv6 TCP
```

### Binding
```c
int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
// Bind socket to local address/port
```

### TCP Server Functions
```c
int listen(int sockfd, int backlog);
// Start listening for connections

int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
// Accept a connection, returns new socket

int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
// Connect to remote address (client side)
```

### Sending/Receiving
```c
// TCP
int send(int sockfd, const void *buf, size_t len, int flags);
int recv(int sockfd, void *buf, size_t len, int flags);

// UDP
int sendto(int sockfd, const void *buf, size_t len, int flags,
           const struct sockaddr *dest_addr, socklen_t addrlen);
int recvfrom(int sockfd, void *buf, size_t len, int flags,
             struct sockaddr *src_addr, socklen_t *addrlen);
```

### Closing
```c
int close(int sockfd);           // Close socket
int shutdown(int sockfd, int how); // Shutdown communication
// how: SHUT_RD (read), SHUT_WR (write), SHUT_RDWR (both)
```

---

## Error Handling

```c
// Check return values!
int sock = socket(AF_INET, SOCK_STREAM, 0);
if (sock < 0) {
    perror("socket");
    return -1;
}

// For Windows, check INVALID_SOCKET
#ifdef _WIN32
if (sock == INVALID_SOCKET) {
    printf("Error: %d\n", WSAGetLastError());
}
#endif
```

---

## Common Mistakes to Avoid

1. ❌ Forgetting to bind before listen
   ```c
   // Wrong!
   listen(sock, 5);
   bind(sock, ...);
   
   // Right!
   bind(sock, ...);
   listen(sock, 5);
   ```

2. ❌ Forgetting network byte order
   ```c
   // Wrong!
   addr.sin_port = 8080;  // Host byte order
   
   // Right!
   addr.sin_port = htons(8080);
   ```

3. ❌ Not closing sockets
   ```c
   // Wrong! Resource leak
   int sock = socket(...);
   // ... do stuff ...
   // forgot to close!
   
   // Right!
   int sock = socket(...);
   // ... do stuff ...
   close(sock);
   ```

4. ❌ Not checking return values
   ```c
   // Wrong!
   bind(sock, ...);
   listen(sock, 5);  // What if bind failed?
   
   // Right!
   if (bind(sock, ...) < 0) {
       perror("bind");
       close(sock);
       return -1;
   }
   ```

---

## Practice Exercises

1. **Echo Server**: Build a simple echo server that receives UDP packets and sends them back
2. **Chat Application**: Create a chat program using TCP sockets
3. **Port Scanner**: Write a program that scans open TCP ports
4. **DNS Resolver**: Implement a simple DNS client using UDP

---

**Now you understand VPN socket programming!** 🎓
