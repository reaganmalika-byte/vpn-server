# VPN Server - Custom C Implementation

A custom VPN server built in C with hybrid UDP/TCP protocol support. This project implements end-to-end encryption, key exchange, and multi-client support.

## Features

- **Hybrid Protocol**: UDP for speed, TCP for reliability
- **End-to-End Encryption**: AES-256 encryption
- **Multi-Client Support**: Handle multiple simultaneous connections
- **Socket Programming**: Raw socket implementation in C
- **Cross-Platform**: Windows, Linux, macOS support

## Project Structure

```
vpn-server/
├── src/
│   ├── main.c              # Server entry point
│   ├── server.c/h          # Server implementation
│   ├── crypto.c/h          # Encryption/decryption
│   ├── socket_utils.c/h    # Socket operations
│   ├── protocol.c/h        # VPN protocol implementation
│   └── client_handler.c/h  # Client connection handling
├── client/
│   └── client.c            # VPN client implementation
├── CMakeLists.txt          # Build configuration
├── Makefile                # Alternative build
├── docs/
│   ├── SETUP.md           # Windows setup guide
│   ├── ARCHITECTURE.md    # Design documentation
│   └── PROTOCOL.md        # Protocol specification
└── README.md
```

## Quick Start

### Prerequisites
- C Compiler (gcc, clang, or MSVC)
- CMake 3.10+
- OpenSSL development libraries

### Build

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

### Run Server

```bash
./vpn_server 8080
```

### Run Client

```bash
./vpn_client <server_ip> 8080
```

## Documentation

- [Windows Setup Guide](docs/SETUP.md) - Complete setup for Windows
- [Architecture](docs/ARCHITECTURE.md) - Design and components
- [Protocol Specification](docs/PROTOCOL.md) - Wire protocol details

## Learning Resources

This project covers:
- Socket programming (UDP/TCP)
- Network encryption
- Multi-threading
- Protocol design
- C networking best practices

## License

MIT License
