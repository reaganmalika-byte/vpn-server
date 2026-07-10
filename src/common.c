#include "common.h"

/* Safe memory allocation */
void* safe_malloc(size_t size) {
    void* ptr = malloc(size);
    if (!ptr && size > 0) {
        LOG_ERROR("Memory allocation failed for %zu bytes", size);
        exit(EXIT_FAILURE);
    }
    return ptr;
}

/* Safe memory freeing */
void safe_free(void* ptr) {
    if (ptr) {
        free(ptr);
    }
}

/* Convert bytes to hex string */
int bytes_to_hex(const unsigned char* bytes, int len, char* hex_str) {
    if (!bytes || !hex_str) return VPN_ERROR;
    
    for (int i = 0; i < len; i++) {
        sprintf(hex_str + (i * 2), "%02x", bytes[i]);
    }
    hex_str[len * 2] = '\0';
    return VPN_SUCCESS;
}

/* Convert hex string to bytes */
int hex_to_bytes(const char* hex_str, unsigned char* bytes) {
    if (!hex_str || !bytes) return VPN_ERROR;
    
    int len = (int)strlen(hex_str);
    if (len % 2 != 0) return VPN_ERROR;
    
    for (int i = 0; i < len; i += 2) {
        sscanf(hex_str + i, "%2hhx", &bytes[i / 2]);
    }
    return VPN_SUCCESS;
}

/* Secure memory zeroing */
void secure_memzero(void* ptr, size_t len) {
    volatile unsigned char* p = (volatile unsigned char*)ptr;
    while (len--) {
        *p++ = 0;
    }
}
