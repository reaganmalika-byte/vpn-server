# VPN Server Makefile (Alternative to CMake)

CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c11 -O2
LDFLAGS = -lssl -lcrypto

# Platform detection
uname_S := $(shell uname -s)
ifeq ($(uname_S),Linux)
    LDFLAGS += -lpthread
endif
ifeq ($(uname_S),Darwin)
    LDFLAGS += -lpthread
endif

# Source files
SOURCES = src/main.c src/server.c src/socket_utils.c src/crypto.c src/protocol.c src/common.c
OBJECTS = $(SOURCES:.c=.o)
TARGET = vpn_server

# Targets
all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "✓ Build complete: $(TARGET)"

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)
	@echo "✓ Clean complete"

run: $(TARGET)
	./$(TARGET) 8080

help:
	@echo "Available targets:"
	@echo "  make all   - Build the VPN server"
	@echo "  make clean - Remove build artifacts"
	@echo "  make run   - Build and run the server"
	@echo "  make help  - Show this help message"

.PHONY: all clean run help
