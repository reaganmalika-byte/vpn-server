# Windows Quick Reference Card

## 📋 Installation Checklist

### Tools to Install

- [ ] Visual Studio Community OR MinGW OR WSL2
- [ ] CMake (3.10+)
- [ ] OpenSSL (Win64)
- [ ] Git (optional)

### Verification Commands

```powershell
# Visual Studio
cl.exe

# MinGW
gcc --version

# CMake
cmake --version

# OpenSSL
"C:\Program Files\OpenSSL-Win64\bin\openssl.exe" version
```

---

## 🔨 Build Commands

### Visual Studio

```powershell
cd vpn-server
mkdir build
cd build
cmake -G "Visual Studio 17 2022" -A x64 ..
cmake --build . --config Release
```

### MinGW

```powershell
cd vpn-server
mkdir build
cd build
cmake -G "MinGW Makefiles" ..
make
```

### WSL2

```bash
cd vpn-server
mkdir build
cd build
cmake ..
make
```

---

## 🚀 Run Commands

### Start Server

**Visual Studio:**
```powershell
.\bin\Release\vpn_server.exe 8080
```

**MinGW:**
```powershell
.\bin\vpn_server.exe 8080
```

### Start Client (NEW Terminal)

**Visual Studio:**
```powershell
.\bin\Release\vpn_client.exe 127.0.0.1 8080
```

**MinGW:**
```powershell
.\bin\vpn_client.exe 127.0.0.1 8080
```

---

## 🔧 Common Issues

| Issue | Solution |
|-------|----------|
| `cl.exe` not found | Use Developer Command Prompt |
| OpenSSL not found | Set `OPENSSL_DIR` environment variable |
| Port in use | Use `netstat -ano \| findstr :8080` to find PID, then `taskkill /PID XXX /F` |
| CMake not found | Restart PowerShell after installation |
| Firewall blocks | Add exceptions in Windows Firewall |

---

## 📁 File Locations

```
C:\Program Files\OpenSSL-Win64\          <- OpenSSL installation
C:\Program Files\CMake\                   <- CMake installation
C:\mingw-w64\                             <- MinGW installation (if used)
C:\Users\YourUsername\Desktop\vpn-server <- VPN source code
C:\Users\YourUsername\Desktop\vpn-server\build <- Build directory
```

---

## 🌐 Remote Connection

```powershell
# On server machine, find IP:
ipconfig

# Look for IPv4 Address (e.g., 192.168.1.100)
# Then on client machine:
.\bin\Release\vpn_client.exe 192.168.1.100 8080
```

---

## 📚 Documentation Files

- `WINDOWS_SETUP.md` - This complete guide
- `BUILD_AND_TEST.md` - Testing and debugging
- `PROTOCOL.md` - VPN protocol details
- `TROUBLESHOOTING.md` - Common problems
- `SOCKET_PROGRAMMING.md` - Network concepts

---

## 💾 Create Batch Files

**start_server.bat:**
```batch
@echo off
cd C:\Users\YourUsername\Desktop\vpn-server\build
echo Starting VPN Server on port 8080...
.\bin\Release\vpn_server.exe 8080
pause
```

**start_client.bat:**
```batch
@echo off
cd C:\Users\YourUsername\Desktop\vpn-server\build
echo Connecting to VPN Server...
.\bin\Release\vpn_client.exe 127.0.0.1 8080
pause
```

---

## 🎯 Next Steps After Installation

1. ✅ Build successfully
2. ✅ Start server
3. ✅ Connect client
4. Read `BUILD_AND_TEST.md` for testing
5. Read `PROTOCOL.md` to understand the protocol
6. Modify code and rebuild

---

**You're all set!** 🚀
