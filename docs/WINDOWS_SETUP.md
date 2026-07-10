# Windows VPN Setup - Complete Step-by-Step Guide

## 🪟 System Requirements

- **Windows 10/11** (64-bit recommended)
- **4GB RAM** minimum (8GB recommended)
- **2GB free disk space**
- **Administrator access** (for some installations)

---

## Part 1: Install Required Tools

### Step 1.1: Install a C Compiler

You have three options. Choose **ONE**:

#### Option A: Microsoft Visual Studio Community (Recommended) ⭐

**Most compatible with Windows**

1. Download from: https://visualstudio.microsoft.com/downloads/
2. Click **"Community"** → **"Free Download"**
3. Run the installer
4. Select **"Desktop development with C++"**
5. Click **"Install"** (will take 5-10 minutes)
6. After installation, open **"Developer Command Prompt for VS 2022"**

**Verify installation:**
```powershell
cl.exe
```

Should show:
```
Microsoft (R) C/C++ Optimizing Compiler Version...
```

---

#### Option B: MinGW (GNU Compiler Collection)

1. Download from: https://sourceforge.net/projects/mingw-w64/files/
2. Download: `mingw-w64-install.exe`
3. Run installer
4. Settings:
   - Architecture: **x86_64**
   - Threads: **posix**
   - Exception: **seh**
5. Click **"Install"**
6. Installation folder: `C:\mingw-w64\`
7. Wait for completion (5-10 minutes)

**Add to PATH:**
1. Press `Win + R`
2. Type: `sysdm.cpl`
3. Go to **"Advanced"** tab
4. Click **"Environment Variables"**
5. Under "System variables", find **"Path"**
6. Click **"Edit"**
7. Click **"New"**
8. Add: `C:\mingw-w64\x86_64-w64-mingw32\bin`
9. Click **"OK"** three times
10. Restart your computer

**Verify installation:**
```powershell
gcc --version
```

Should show:
```
gcc.exe (x86_64-w64-mingw32) 12.x.x
```

---

#### Option C: Windows Subsystem for Linux (WSL2)

For Linux development environment on Windows:

1. Open **PowerShell as Administrator**
2. Run:
```powershell
wsl --install
```
3. Restart your computer
4. Open **Ubuntu** from Start menu
5. Create username and password
6. Run:
```bash
sudo apt update
sudo apt install build-essential cmake libssl-dev
```

Then follow Linux instructions in `docs/BUILD_AND_TEST.md`

---

### Step 1.2: Install CMake

1. Download from: https://cmake.org/download/
2. Click **"Windows x64 Installer"**
3. Run the installer
4. **IMPORTANT**: Check **"Add CMake to the system PATH for all users"**
5. Click **"Install"**

**Verify installation:**
```powershell
cmake --version
```

Should show:
```
cmake version 3.25.x
```

---

### Step 1.3: Install OpenSSL

**Option A: Pre-built Binaries (Easiest)** ⭐

1. Go to: https://slproweb.com/products/Win32OpenSSL.html
2. Download **"Win64 OpenSSL v3.x.x"** (not "Light")
3. Run the installer (`.msi` file)
4. Click **"Next"**
5. Accept license → **"Next"**
6. Installation location: Keep default `C:\Program Files\OpenSSL-Win64`
7. OpenSSL binaries: Keep default
8. Click **"Install"**
9. **Uncheck "Launch OpenSSL"** at the end
10. Click **"Finish"**

**Verify installation:**
```powershell
"C:\Program Files\OpenSSL-Win64\bin\openssl.exe" version
```

Should show:
```
OpenSSL 3.x.x ...
```

---

**Option B: Using VCPKG (Advanced)**

```powershell
# Clone vcpkg
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg

# Run bootstrap
.\bootstrap-vcpkg.bat

# Install OpenSSL
.\vcpkg install openssl:x64-windows

# Integrate with Visual Studio
.\vcpkg integrate install
```

---

### Step 1.4: Install Git (Optional but Recommended)

1. Download from: https://git-scm.com/download/win
2. Run installer
3. Use default settings
4. Click **"Install"**

**Verify installation:**
```powershell
git --version
```

---

## Part 2: Get the VPN Server Code

### Step 2.1: Clone the Repository

**Option A: Using Git (Recommended)**

```powershell
cd C:\Users\YourUsername\Desktop
git clone https://github.com/reaganmalika-byte/vpn-server.git
cd vpn-server
```

**Option B: Download as ZIP

1. Go to: https://github.com/reaganmalika-byte/vpn-server
2. Click **"Code"** → **"Download ZIP"**
3. Extract to: `C:\Users\YourUsername\Desktop\vpn-server`
4. Open PowerShell and navigate:
```powershell
cd C:\Users\YourUsername\Desktop\vpn-server
```

---

## Part 3: Build the VPN Server

### Step 3.1: Configure with CMake

**Using Visual Studio:**

1. Open **"Developer Command Prompt for VS 2022"**
2. Navigate to VPN directory:
```powershell
cd C:\Users\YourUsername\Desktop\vpn-server
```

3. Create build directory:
```powershell
mkdir build
cd build
```

4. Configure:
```powershell
cmake -G "Visual Studio 17 2022" -A x64 ..
```

**Expected output:**
```
-- The C compiler identification is MSVC...
-- OpenSSL found
-- Configuring done
-- Generating done
-- Build files have been written to: C:.../build
```

---

**Using MinGW:**

```powershell
cmake -G "MinGW Makefiles" ..
```

---

### Step 3.2: Build the Project

**Using Visual Studio (Recommended):**

```powershell
cmake --build . --config Release
```

This will take 2-5 minutes. You should see:
```
Building 100% complete
vpn_server.exe - 0 error(s), 0 warning(s)
```

**Using MinGW:**

```powershell
make
```

---

### Step 3.3: Verify Build Success

Check for executables:

```powershell
dir bin\Release
```

You should see:
```
vpn_server.exe
vpn_client.exe
```

---

## Part 4: Run the VPN Server

### Step 4.1: Start the Server

```powershell
.\bin\Release\vpn_server.exe 8080
```

**Expected output:**
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

**✅ Server is running!** Don't close this window.

---

### Step 4.2: Run the Client (in NEW Terminal)

1. **Open a NEW PowerShell window** (don't close the server one)
2. Navigate to same directory:
```powershell
cd C:\Users\YourUsername\Desktop\vpn-server\build
```

3. Run client:
```powershell
.\bin\Release\vpn_client.exe 127.0.0.1 8080
```

**Expected output:**
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
[INFO] Key exchange complete - Tunnel encrypted

=== VPN Tunnel Established ===
Connection to 127.0.0.1:8080 successful
Tunnel is encrypted

[INFO] Sending: Hello from VPN Client!
[INFO] Disconnecting from server
[INFO] Client closed
```

**✅ Connection successful!**

---

## Part 5: Firewall Configuration

If Windows Firewall blocks the VPN:

### Step 5.1: Allow VPN Through Firewall

1. Press `Win + R`
2. Type: `wf.msc`
3. Click **"Inbound Rules"** (left panel)
4. Click **"New Rule"** (right panel)
5. Select **"Program"** → **"Next"**
6. Select **"This program path:"**
7. Click **"Browse"**
8. Navigate to: `C:\Users\YourUsername\Desktop\vpn-server\build\bin\Release\vpn_server.exe`
9. Click **"Open"** → **"Next"**
10. Select **"Allow the connection"** → **"Next"**
11. Check all: Domain, Private, Public → **"Next"**
12. Name: `VPN Server`
13. Click **"Finish"**

**Repeat for vpn_client.exe**

---

## Part 6: Using the VPN

### Create Batch Files for Easy Startup

**Create `start_server.bat`:**

1. Open Notepad
2. Paste:
```batch
@echo off
cd C:\Users\YourUsername\Desktop\vpn-server\build
echo Starting VPN Server on port 8080...
.\bin\Release\vpn_server.exe 8080
pause
```

3. Save as: `C:\Users\YourUsername\Desktop\start_server.bat`

**Create `start_client.bat`:**

1. Open Notepad
2. Paste:
```batch
@echo off
cd C:\Users\YourUsername\Desktop\vpn-server\build
echo Connecting to VPN Server...
.\bin\Release\vpn_client.exe 127.0.0.1 8080
pause
```

3. Save as: `C:\Users\YourUsername\Desktop\start_client.bat`

**Now you can just double-click these files!**

---

## Part 7: Connect to Remote Server

If your VPN server is on another computer:

1. Find server's IP address:
   - On server computer, open PowerShell:
   ```powershell
   ipconfig
   ```
   - Look for "IPv4 Address" (e.g., `192.168.1.100`)

2. Modify `start_client.bat`:
   ```batch
   .\bin\Release\vpn_client.exe 192.168.1.100 8080
   ```

3. Make sure firewall allows port 8080 on server

---

## Part 8: Troubleshooting

### Issue: "'cl.exe' not found"

**Solution**: Use Developer Command Prompt for VS, not regular PowerShell

1. Press `Win + R`
2. Type: `Developer Command Prompt for VS 2022`
3. Press Enter

---

### Issue: "OpenSSL not found"

**Solution**: Set environment variable

1. Press `Win + R`
2. Type: `sysdm.cpl`
3. Click **"Environment Variables"**
4. Click **"New"** (System variables)
5. Variable name: `OPENSSL_DIR`
6. Variable value: `C:\Program Files\OpenSSL-Win64`
7. Click **"OK"** three times
8. Restart PowerShell

---

### Issue: "Port 8080 already in use"

```powershell
# Find what's using port 8080
netstat -ano | findstr :8080

# Kill the process (replace XXXX with PID)
taskkill /PID XXXX /F

# Or use a different port
.\bin\Release\vpn_server.exe 9090
```

---

### Issue: "Connection refused"

1. Make sure server is running in first terminal
2. Check firewall is allowing the port
3. Use correct IP address (127.0.0.1 for localhost)
4. Check port number matches (8080 in both commands)

---

### Issue: Build fails with "CMake not found"

**Solution**:

1. Restart your computer (to refresh PATH)
2. Open NEW PowerShell after restart
3. Run `cmake --version` to verify

---

## Part 9: Performance Tips

### Optimize Build

For faster compilation (Release mode):

```powershell
cmake --build . --config Release -j 4
```

### Monitor Resource Usage

While VPN is running:

1. Open Task Manager: `Ctrl + Shift + Esc`
2. Look for `vpn_server.exe` and `vpn_client.exe`
3. Check CPU and Memory usage

---

## Part 10: Advanced: Debug Mode

For development and debugging:

```powershell
# Configure for Debug
cmake -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Debug ..

# Build Debug version
cmake --build . --config Debug

# Run with Debug output
.\bin\Debug\vpn_server.exe 8080
```

**Debug mode shows:**
- All packet details
- Encryption/decryption steps
- Connection lifecycle
- Error messages

---

## 🎉 Success!

You now have:

✅ C compiler installed  
✅ CMake build system  
✅ OpenSSL cryptography  
✅ VPN server compiled  
✅ VPN running on Windows  
✅ Client connecting successfully  

---

## 📚 Next Steps

1. **Understand the code**: Read `docs/PROTOCOL.md`
2. **Learn networking**: Read `docs/SOCKET_PROGRAMMING.md`
3. **Modify the code**: Change port, encryption, protocol
4. **Add features**: Implement keep-alive, persistence, etc.

---

## 💡 Pro Tips

1. **Keep terminal windows organized**:
   - Server in one window
   - Client in another
   - Use `title` command to label them

   ```powershell
   title VPN Server
   title VPN Client
   ```

2. **Enable detailed logging**:
   - Edit `src/common.h`
   - Uncomment `LOG_DEBUG` lines
   - Rebuild to see detailed output

3. **Test with multiple clients**:
   - Open multiple client terminals
   - Each can connect independently
   - Server handles up to 100 clients

4. **Use different ports**:
   ```powershell
   # Server on 8080
   .\bin\Release\vpn_server.exe 8080
   
   # Server on 9090
   .\bin\Release\vpn_server.exe 9090
   
   # Connect to different ports
   .\bin\Release\vpn_client.exe 127.0.0.1 8080
   .\bin\Release\vpn_client.exe 127.0.0.1 9090
   ```

---

## 🆘 Still Having Issues?

1. Check `docs/TROUBLESHOOTING.md` for more solutions
2. Enable Debug mode to see detailed logs
3. Verify each tool: `cl.exe`, `cmake`, `openssl`
4. Check firewall and antivirus settings

---

**Congratulations on setting up your Windows VPN!** 🎉

You're now ready to:
- Understand VPN networking
- Learn cryptography
- Develop system software
- Customize the protocol

**Happy VPN development!** 🚀
