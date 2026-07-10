#ifndef VPN_CRYPTO_H
#define VPN_CRYPTO_H

#include "common.h"

/* Key generation and derivation */
int generate_random_bytes(unsigned char* buffer, int len);
int derive_key_from_password(const char* password, int password_len,
                             const unsigned char* salt, int salt_len,
                             unsigned char* key, int key_len);

/* AES-256-GCM encryption/decryption */
int encrypt_aes256_gcm(const unsigned char* plaintext, int plaintext_len,
                       const unsigned char* key, int key_len,
                       const unsigned char* iv, int iv_len,
                       const unsigned char* aad, int aad_len,
                       unsigned char* ciphertext,
                       unsigned char* tag);

int decrypt_aes256_gcm(const unsigned char* ciphertext, int ciphertext_len,
                       const unsigned char* key, int key_len,
                       const unsigned char* iv, int iv_len,
                       const unsigned char* aad, int aad_len,
                       const unsigned char* tag,
                       unsigned char* plaintext);

/* Hashing */
void sha256_hash(const unsigned char* data, int len, unsigned char* hash);
int hmac_sha256(const unsigned char* key, int key_len,
                const unsigned char* data, int data_len,
                unsigned char* hmac);

/* Utilities */
void secure_memzero(void* ptr, size_t len);

#endif /* VPN_CRYPTO_H */
