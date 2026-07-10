# Troubleshooting Guide

## Common Compilation Errors

### 1. "stdio.h: No such file or directory"

**Problem**: C compiler not found or not in PATH

**Solution**:
```bash
# Windows - Use Developer Command Prompt for VS
# Or add to PATH manually:
set PATH=C:\Program Files (x86)\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.XX.XXXXX\bin\Hostx64\x64;%PATH%

# Linux
sudo apt install build-essential

# macOS
brew install gcc
```

### 2. "openssl/aes.h: No such file"

**Problem**: OpenSSL development headers not installed

**Solution**:
```bash
# Windows - Reinstall OpenSSL from https://slproweb.com/products/Win32OpenSSL.html
# Or use vcpkg

# Linux
sudo apt install libssl-dev

# macOS
brew install openssl
```

### 3. "error: undefined reference to `SSL_*'"

**Problem**: OpenSSL libraries not linked

**Solution**:
```bash
# In CMakeLists.txt, ensure:
target_link_libraries(vpn_server OpenSSL::Crypto OpenSSL::SSL)

# Or in Makefile:
LDFLAGS = -lssl -lcrypto

# Then rebuild:
make clean
make
```

---

## Runtime Errors

### 1. "Failed to create UDP socket"

**Problem**: Port already in use or permission denied

**Solution**:
```bash
# Check what's using the port
lsof -i :8080  # Linux/macOS
netstat -ano | findstr :8080  # Windows

# Kill the process
kill -9 <PID>  # Linux/macOS
taskkill /PID <PID> /F  # Windows

# Or use a different port
./vpn_server 9090
```

### 2. "Failed to bind TCP socket to port 8080"

**Problem**: Permission denied (usually port < 1024 on Linux)

**Solution**:
```bash
# Use port > 1024
./vpn_server 8080  # Use 8080 instead of 80

# Or run with sudo (not recommended for security)
sudo ./vpn_server 80
```

### 3. "Connection refused"

**Problem**: Server not running or wrong host/port

**Solution**:
```bash
# Terminal 1: Start server
./vpn_server 8080

# Terminal 2: Connect with correct parameters
./vpn_client 127.0.0.1 8080  # Use localhost for testing
```

### 4. "Handshake failed"

**Problem**: Protocol mismatch or network issue

**Solution**:
```bash
# 1. Ensure both are same version
git log --oneline | head -1

# 2. Rebuild both
make clean && make

# 3. Check firewall
# Windows Firewall:
# Settings → Firewall → Allow app through firewall
# Linux firewall:
sudo ufw allow 8080/tcp
sudo ufw allow 8080/udp

# 4. Enable debug logging
# Edit src/common.h to enable LOG_DEBUG
# Rebuild and check output
```

### 5. "Failed to initialize AES-256-GCM encryption"

**Problem**: OpenSSL EVP functions failing

**Solution**:
```bash
# Check OpenSSL version
openssl version

# If using OpenSSL 1.0.2, upgrade to 1.1.1+
# Windows: Use installer from https://slproweb.com/
# Linux: sudo apt install libssl1.1
# macOS: brew upgrade openssl
```

---

## Network Debugging

### Monitor Network Traffic

```bash
# Linux/macOS - Monitor port 8080
sudo tcpdump -i lo -n port 8080
sudo tcpdump -i any -n port 8080

# Windows - Use Wireshark
# Download from https://www.wireshark.org/
# Filter: tcp.port == 8080 || udp.port == 8080
```

### Check Port Status

```bash
# Linux/macOS
netstat -an | grep 8080
ss -an | grep 8080
lsof -i :8080

# Windows
netstat -ano | findstr :8080
Get-NetTCPConnection -LocalPort 8080  # PowerShell
```

### Test Connectivity

```bash
# Check if server is responding
telnet 127.0.0.1 8080  # TCP

# For UDP, use nc (netcat)
nc -u -l 8080  # Listen
nc -u 127.0.0.1 8080  # Connect
```

---

## Memory and Debugging

### Memory Leaks

```bash
# Linux - Using valgrind
sudo apt install valgrind
valgrind --leak-check=full ./vpn_server 8080

# macOS - Using instruments
instruments -t "Leaks" ./vpn_server

# Windows - Using Visual Studio
# Debug → Windows → Memory Diagnostic
```

### GDB Debugging

```bash
# Compile with debug symbols
CFLAGS="-g" make

# Run under debugger
gdb ./vpn_server

# Common GDB commands
(gdb) break main           # Set breakpoint
(gdb) run 8080             # Run with args
(gdb) next                 # Next line
(gdb) step                 # Step into function
(gdb) continue             # Continue execution
(gdb) print variable       # Print variable
(gdb) backtrace            # Show stack trace
(gdb) quit                 # Exit
```

---

## Platform-Specific Issues

### Windows

**Issue**: "'pthread.h' file not found"

**Solution**: Use Windows threading API or install pthreads-w32
```powershell
vcpkg install pthreads:x64-windows
```

**Issue**: "cl.exe not found"

**Solution**: Run from Developer Command Prompt
```powershell
# Or set up environment
"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
```

### Linux

**Issue**: "libssl.so.1.1: cannot open shared object file"

**Solution**: Install OpenSSL 1.1
```bash
sudo apt install libssl1.1
```

### macOS

**Issue**: "ld: library not found for -lssl"

**Solution**: Link OpenSSL from Homebrew
```bash
export LDFLAGS="-L/usr/local/opt/openssl/lib"
export CPPFLAGS="-I/usr/local/opt/openssl/include"
make
```

---

## Performance Issues

### Server is Slow

1. **Check CPU usage**
   ```bash
   top    # Linux/macOS
   Task Manager  # Windows
   ```

2. **Check memory usage**
   ```bash
   free -h       # Linux
   vm_stat       # macOS
   tasklist /v   # Windows
   ```

3. **Profile with flame graphs**
   ```bash
   # Linux
   perf record -g ./vpn_server 8080
   perf report
   ```

### High Latency

1. Check network delay
   ```bash
   ping 127.0.0.1
   mtr 127.0.0.1
   ```

2. Check for packet loss
   ```bash
   ping -i 0.1 -c 100 127.0.0.1
   ```

---

## Getting Help

1. **Enable all debug logs**
   - Edit `src/common.h`
   - Enable `LOG_DEBUG`
   - Rebuild and run

2. **Capture complete output**
   ```bash
   ./vpn_server 8080 2>&1 | tee server.log
   ```

3. **Check GitHub Issues**
   - https://github.com/reaganmalika-byte/vpn-server/issues

4. **Provide information when asking for help**
   - OS and version
   - Compiler and version
   - OpenSSL version
   - Error message (full output)
   - Steps to reproduce

---

**Still stuck? Don't give up!** 💪
