cmake_minimum_required(VERSION 3.10)
project(vpn_server C)

# Set C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Find required packages
find_package(OpenSSL REQUIRED)

# Platform-specific settings
if(WIN32)
    add_definitions(-D_WIN32_WINNT=0x0601)
    set(PLATFORM_LIBS ws2_32)
else()
    find_package(Threads REQUIRED)
    set(PLATFORM_LIBS Threads::Threads)
endif()

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${OPENSSL_INCLUDE_DIR}
)

# Server source files
set(SERVER_SOURCES
    src/main.c
    src/server.c
    src/socket_utils.c
    src/crypto.c
    src/protocol.c
    src/common.c
)

# Create server executable
add_executable(vpn_server ${SERVER_SOURCES})

# Link libraries
target_link_libraries(vpn_server
    OpenSSL::Crypto
    OpenSSL::SSL
    ${PLATFORM_LIBS}
)

# Compiler flags for different platforms
if(MSVC)
    target_compile_options(vpn_server PRIVATE /W4 /WX)
else()
    target_compile_options(vpn_server PRIVATE -Wall -Wextra -pedantic -Werror)
endif()

# Output directories
set_target_properties(vpn_server PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

print_status("VPN Server build configured")
print_status("  Source: ${SERVER_SOURCES}")
print_status("  OpenSSL: ${OPENSSL_LIBRARIES}")
