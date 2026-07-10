# Windows Setup Guide - VPN Server Development

This guide will help you set up C development environment on Windows for building the VPN server.

## Step 1: Install a C Compiler

### Option A: MSVC (Microsoft Visual C++) - Recommended for Windows

1. **Download Visual Studio Community**
   - Go to: https://visualstudio.microsoft.com/downloads/
   - Download "Visual Studio Community 2022"

2. **Install with C++ Workload**
   - Run the installer
   - Select "Desktop development with C++"
   - Complete installation

3. **Verify Installation**
   ```bash
   cl.exe /?
   ```

### Option B: MinGW-w64 (GCC for Windows)

1. **Download MinGW-w64**
   - Go to: https://www.mingw-w64.org/
   - Download latest version

2. **Install and Add to PATH**
   - Extract to `C:\MinGW`
   - Add `C:\MinGW\bin` to System Environment Variables

3. **Verify Installation**
   ```bash
   gcc --version
   ```

### Option C: Windows Subsystem for Linux (WSL2) - Best for Learning

1. **Enable WSL2**
   ```powershell
   wsl --install
   ```

2. **Install Ubuntu on WSL2**
   - Windows Store → Search "Ubuntu 22.04"
   - Install and launch

3. **Install Build Tools in WSL**
   ```bash
   sudo apt update
   sudo apt install build-essential cmake libssl-dev
   ```

---

## Step 2: Install CMake

### Using Installer (Easiest)

1. Go to: https://cmake.org/download/
2. Download "Windows x64 Installer"
3. Run installer, check "Add CMake to PATH"
4. Verify: `cmake --version`

### Using Chocolatey (if installed)

```powershell
choco install cmake
```

---

## Step 3: Install OpenSSL

### Option A: Pre-built Binaries

1. Download from: https://slproweb.com/products/Win32OpenSSL.html
   - Choose "Win64 OpenSSL v3.x"
   - Download the installer (not "Light")

2. Run installer with defaults
   - Default location: `C:\Program Files\OpenSSL-Win64`

3. Add to Environment Variables:
   ```
   OPENSSL_DIR = C:\Program Files\OpenSSL-Win64
   ```

### Option B: Chocolatey

```powershell
choco install openssl
```

### Option C: vcpkg (Microsoft's Package Manager)

```powershell
# Install vcpkg
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat

# Install OpenSSL
.\vcpkg install openssl:x64-windows
```

---

## Step 4: Verify Everything

Open Command Prompt and run:

```bash
# Test C compiler
cl.exe                    # MSVC
# or
gcc --version            # MinGW

# Test CMake
cmake --version

# Test OpenSSL
openssl version
```

All three should return version information.

---

## Step 5: Clone and Build the Project

```bash
# Clone the repository
git clone https://github.com/reaganmalika-byte/vpn-server.git
cd vpn-server

# Create build directory
mkdir build
cd build

# Configure with CMake
cmake ..

# Build the project
cmake --build . --config Release
```

---

## Troubleshooting

### "cl.exe not found"
- **Solution**: Run from "Developer Command Prompt for VS 2022" instead of regular CMD
- Or add Visual Studio to PATH

### "cmake: command not found"
- **Solution**: Add CMake to System PATH
  - Windows Settings → Environment Variables → Add `C:\Program Files\CMake\bin`

### "OpenSSL not found"
- **Solution**: Set environment variable
  ```powershell
  set OPENSSL_DIR=C:\Program Files\OpenSSL-Win64
  ```

### Link errors with OpenSSL
- **Solution**: Ensure 64-bit (x64) libraries match your compiler (x64)
- Reinstall OpenSSL if mismatched

---

## Recommended IDE/Editor

### Option 1: Visual Studio Community (Free)
- Full C/C++ IDE
- CMake support built-in
- Best for Windows development

### Option 2: Visual Studio Code + Extensions
```
1. Install VS Code: https://code.visualstudio.com/
2. Install Extensions:
   - C/C++ by Microsoft
   - CMake Tools
   - CMake by twxs
```

### Option 3: CLion (Paid, Trial Available)
- Cross-platform C/C++ IDE
- Excellent CMake integration

---

## Next Steps

Once setup is complete:

1. Read [ARCHITECTURE.md](ARCHITECTURE.md) - Understand the design
2. Read [PROTOCOL.md](PROTOCOL.md) - Learn the protocol
3. Review socket programming examples in `src/socket_utils.c`
4. Build and run the basic server

You're ready to start coding! 🚀
