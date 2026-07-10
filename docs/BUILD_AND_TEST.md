# Complete Build and Test Guide

## System Requirements

- **Windows 10/11, Linux (Ubuntu 20.04+), or macOS 10.14+**
- **C Compiler**: GCC, Clang, or MSVC
- **CMake**: 3.10+
- **OpenSSL**: 1.1.1 or 3.0+
- **Git**: For cloning the repository

---

## Part 1: Installation and Setup

### Step 1.1: Install OpenSSL

#### Windows
```powershell
# Option A: Using pre-built binaries
# Download from: https://slproweb.com/products/Win32OpenSSL.html
# Run installer and remember installation path

# Option B: Using vcpkg
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg install openssl:x64-windows
```

#### Linux (Ubuntu/Debian)
```bash
sudo apt-get update
sudo apt-get install libssl-dev cmake build-essential
```

#### macOS
```bash
brew install openssl cmake
```

### Step 1.2: Clone Repository

```bash
git clone https://github.com/reaganmalika-byte/vpn-server.git
cd vpn-server
```

### Step 1.3: Build the Project

#### Using CMake (Recommended)

```bash
# Create build directory
mkdir build
cd build

# Configure build
cmake ..

# Build
cmake --build . --config Release

# On Windows with MSVC, use:
# cmake --build . --config Release

# On Linux/macOS:
# make
```

#### Using Makefile (Linux/macOS)

```bash
make clean
make

# Run server
make run
```

### Step 1.4: Verify Build

After building, check for executables:

```bash
# Linux/macOS
ls -la bin/vpn_server
ls -la bin/vpn_client

# Windows
dir bin\vpn_server.exe
dir bin\vpn_client.exe
```

---

## Part 2: Running the Server

### Basic Server Start

```bash
# Linux/macOS
./bin/vpn_server 8080

# Windows
.\bin\vpn_server.exe 8080
```

**Expected Output:**
```
VPN Server - Custom C Implementation
Version 1.0

[INFO] VPN Server created (port 8080)
[INFO] UDP socket created and bound to port 8080
[INFO] TCP socket created and listening on port 8080
[INFO] VPN Server started

=== VPN Server is running ===
Listening on port 8080
Press Ctrl+C to stop
```

### Server with Different Port

```bash
./bin/vpn_server 9090
```

### Graceful Shutdown

Press `Ctrl+C` to stop the server:

```
^C[INFO] Shutdown signal received
[INFO] VPN Server shutdown
Server stopped
```

---

## Part 3: Running the Client

### Basic Client Connection

In another terminal:

```bash
# Linux/macOS
./bin/vpn_client 127.0.0.1 8080

# Windows
.\bin\vpn_client.exe 127.0.0.1 8080
```

**Expected Output:**
```
VPN Client - Custom C Implementation
Version 1.0

[INFO] Connecting to 127.0.0.1:8080
[INFO] Connected to server
[INFO] Starting handshake
[DEBUG] Sending handshake request
[DEBUG] Received server hello
[INFO] Handshake complete
[INFO] Starting key exchange
[DEBUG] Sending key exchange
[DEBUG] Received session start
[INFO] Key exchange complete - Tunnel encrypted

=== VPN Tunnel Established ===
Connection to 127.0.0.1:8080 successful
Tunnel is encrypted

[INFO] Sending: Hello from VPN Client!
[INFO] Received: (response from server)
[INFO] Disconnecting from server
[INFO] Client closed
```

### Connect to Remote Server

```bash
./bin/vpn_client example.com 8080
```

---

## Part 4: Testing the VPN

### Test 1: Basic Connectivity

**Terminal 1 (Server):**
```bash
./bin/vpn_server 8080
```

**Terminal 2 (Client):**
```bash
./bin/vpn_client 127.0.0.1 8080
```

✅ **Success**: Client connects and shows encrypted tunnel established

### Test 2: Multiple Clients

**Terminal 1 (Server):**
```bash
./bin/vpn_server 8080
```

**Terminal 2 (Client 1):**
```bash
./bin/vpn_client 127.0.0.1 8080
```

**Terminal 3 (Client 2):**
```bash
./bin/vpn_client 127.0.0.1 8080
```

✅ **Success**: Both clients connect simultaneously

### Test 3: Data Transmission

The client automatically sends test data:
```
[INFO] Sending: Hello from VPN Client!
```

Server should receive and log the encrypted data:
```
[INFO] [Client 0] Received N bytes of encrypted data
```

### Test 4: Encryption Verification

Check that:
1. ✅ Handshake completes without encryption
2. ✅ Key exchange derives session key
3. ✅ Data is encrypted with AES-256-GCM
4. ✅ Server can decrypt data

---

## Part 5: Debugging

### Enable Debug Logging

Edit `src/common.h` and ensure `LOG_DEBUG` is enabled:

```c
#define LOG_DEBUG(fmt, ...) printf("[DEBUG] " fmt "\n", ##__VA_ARGS__)
```

Recompile:
```bash
cmake --build . --config Debug
```

### Common Issues and Solutions

#### "Port already in use"
```bash
# Linux/macOS - Find process using port 8080
lsof -i :8080
# Kill the process
kill -9 <PID>

# Windows - Find process using port 8080
netstat -ano | findstr :8080
# Kill the process
taskkill /PID <PID> /F
```

#### "OpenSSL not found"
```bash
# Linux
sudo apt-get install libssl-dev

# macOS
brew install openssl

# Windows - Set environment variable
set OPENSSL_DIR=C:\Program Files\OpenSSL-Win64
```

#### "Connection refused"
- Ensure server is running first
- Check firewall allows port 8080
- Verify correct hostname/IP address

#### "Handshake failed"
- Check network connectivity
- Verify both server and client versions match
- Check firewall/NAT rules

---

## Part 6: Performance Testing

### Benchmark Encryption Speed

The client sends 1 test packet and measures timing:

```c
// In production, batch multiple packets
unsigned char test_data[1000];
time_t start = time(NULL);

for (int i = 0; i < 1000; i++) {
    vpn_client_send_data(client, test_data, sizeof(test_data));
}

time_t elapsed = time(NULL) - start;
printf("Encrypted 1000 packets in %ld seconds\n", elapsed);
```

### Expected Performance

| Operation | Speed |
|-----------|-------|
| AES-256-GCM encryption | ~1 Gbps |
| Handshake | ~50ms |
| Key exchange | ~20ms |
| Per-packet encryption | ~0.1ms |

---

## Part 7: Production Deployment

### Security Checklist

- [ ] Use strong passwords for authentication
- [ ] Enable certificate-based authentication (future feature)
- [ ] Run server as non-root user (Linux)
- [ ] Enable firewall rules
- [ ] Monitor logs for attacks
- [ ] Use TLS for control channel (future)
- [ ] Implement rate limiting
- [ ] Regular security audits

### Deployment Steps

```bash
# 1. Build release binary
cmake --build . --config Release

# 2. Create user for VPN server
sudo useradd -r -s /bin/false vpn-server

# 3. Copy binary to /usr/local/bin
sudo cp bin/vpn_server /usr/local/bin/

# 4. Create systemd service (Linux)
sudo nano /etc/systemd/system/vpn-server.service
# [Unit]
# Description=VPN Server
# After=network.target
# [Service]
# Type=simple
# User=vpn-server
# ExecStart=/usr/local/bin/vpn_server 8080
# Restart=on-failure
# [Install]
# WantedBy=multi-user.target

# 5. Enable and start service
sudo systemctl daemon-reload
sudo systemctl enable vpn-server
sudo systemctl start vpn-server

# 6. Check status
sudo systemctl status vpn-server
```

---

## Part 8: Next Steps

After basic testing works:

1. **Extend Protocol**
   - Add session persistence
   - Implement keep-alive (ping/pong)
   - Add graceful disconnect

2. **Improve Security**
   - Certificate-based authentication
   - Perfect forward secrecy
   - Rate limiting per client

3. **Add Features**
   - TUN/TAP device integration
   - Route management
   - DNS tunneling
   - Traffic statistics

4. **Optimize Performance**
   - Use select()/epoll() for multiplexing
   - Add packet batching
   - Implement connection pooling

---

## Troubleshooting Checklist

- [ ] Compiler installed and in PATH
- [ ] OpenSSL development libraries installed
- [ ] CMake installed (version 3.10+)
- [ ] Repository cloned successfully
- [ ] Build completed without errors
- [ ] Server starts without crashing
- [ ] Client can connect to server
- [ ] Handshake completes successfully
- [ ] Key exchange derives matching keys
- [ ] Data transmission works

---

**Happy VPN Building!** 🚀
