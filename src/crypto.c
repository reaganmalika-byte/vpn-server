#include "crypto.h"
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/sha.h>
#include <openssl/hmac.h>

/* Generate random bytes */
int generate_random_bytes(unsigned char* buffer, int len) {
    if (!buffer || len <= 0) return VPN_ERROR;
    
    if (RAND_bytes(buffer, len) != 1) {
        LOG_ERROR("Failed to generate random bytes");
        return VPN_ERROR_CRYPTO;
    }
    
    return VPN_SUCCESS;
}

/* Derive key from password using PBKDF2 */
int derive_key_from_password(const char* password, int password_len,
                             const unsigned char* salt, int salt_len,
                             unsigned char* key, int key_len) {
    if (!password || !salt || !key) return VPN_ERROR;
    
    if (PKCS5_PBKDF2_HMAC(password, password_len, salt, salt_len,
                          10000, EVP_sha256(), key_len, key) != 1) {
        LOG_ERROR("Failed to derive key from password");
        return VPN_ERROR_CRYPTO;
    }
    
    return VPN_SUCCESS;
}

/* Encrypt using AES-256-GCM */
int encrypt_aes256_gcm(const unsigned char* plaintext, int plaintext_len,
                       const unsigned char* key, int key_len,
                       const unsigned char* iv, int iv_len,
                       const unsigned char* aad, int aad_len,
                       unsigned char* ciphertext,
                       unsigned char* tag) {
    EVP_CIPHER_CTX* ctx;
    int len = 0;
    int ciphertext_len = 0;
    
    if (!plaintext || !key || !iv || !ciphertext || !tag) {
        return VPN_ERROR_CRYPTO;
    }
    
    if (plaintext_len <= 0) return VPN_ERROR_CRYPTO;
    
    /* Create and initialize context */
    ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        LOG_ERROR("Failed to create cipher context");
        return VPN_ERROR_CRYPTO;
    }
    
    /* Initialize encryption */
    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, key, iv) != 1) {
        LOG_ERROR("Failed to initialize AES-256-GCM encryption");
        EVP_CIPHER_CTX_free(ctx);
        return VPN_ERROR_CRYPTO;
    }
    
    /* Add AAD if provided */
    if (aad && aad_len > 0) {
        if (EVP_EncryptUpdate(ctx, NULL, &len, aad, aad_len) != 1) {
            LOG_ERROR("Failed to add AAD");
            EVP_CIPHER_CTX_free(ctx);
            return VPN_ERROR_CRYPTO;
        }
    }
    
    /* Encrypt plaintext */
    if (EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len) != 1) {
        LOG_ERROR("Failed to encrypt data");
        EVP_CIPHER_CTX_free(ctx);
        return VPN_ERROR_CRYPTO;
    }
    ciphertext_len = len;
    
    /* Finalize encryption */
    if (EVP_EncryptFinal_ex(ctx, ciphertext + len, &len) != 1) {
        LOG_ERROR("Failed to finalize encryption");
        EVP_CIPHER_CTX_free(ctx);
        return VPN_ERROR_CRYPTO;
    }
    ciphertext_len += len;
    
    /* Get authentication tag */
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, MAC_TAG_SIZE, tag) != 1) {
        LOG_ERROR("Failed to get GCM tag");
        EVP_CIPHER_CTX_free(ctx);
        return VPN_ERROR_CRYPTO;
    }
    
    EVP_CIPHER_CTX_free(ctx);
    return ciphertext_len;
}

/* Decrypt using AES-256-GCM */
int decrypt_aes256_gcm(const unsigned char* ciphertext, int ciphertext_len,
                       const unsigned char* key, int key_len,
                       const unsigned char* iv, int iv_len,
                       const unsigned char* aad, int aad_len,
                       const unsigned char* tag,
                       unsigned char* plaintext) {
    EVP_CIPHER_CTX* ctx;
    int len = 0;
    int plaintext_len = 0;
    
    if (!ciphertext || !key || !iv || !tag || !plaintext) {
        return VPN_ERROR_CRYPTO;
    }
    
    if (ciphertext_len <= 0) return VPN_ERROR_CRYPTO;
    
    /* Create and initialize context */
    ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        LOG_ERROR("Failed to create cipher context");
        return VPN_ERROR_CRYPTO;
    }
    
    /* Initialize decryption */
    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, key, iv) != 1) {
        LOG_ERROR("Failed to initialize AES-256-GCM decryption");
        EVP_CIPHER_CTX_free(ctx);
        return VPN_ERROR_CRYPTO;
    }
    
    /* Add AAD if provided */
    if (aad && aad_len > 0) {
        if (EVP_DecryptUpdate(ctx, NULL, &len, aad, aad_len) != 1) {
            LOG_ERROR("Failed to add AAD");
            EVP_CIPHER_CTX_free(ctx);
            return VPN_ERROR_CRYPTO;
        }
    }
    
    /* Set authentication tag */
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG,
                            MAC_TAG_SIZE, (void*)tag) != 1) {
        LOG_ERROR("Failed to set GCM tag");
        EVP_CIPHER_CTX_free(ctx);
        return VPN_ERROR_CRYPTO;
    }
    
    /* Decrypt ciphertext */
    if (EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len) != 1) {
        LOG_ERROR("Failed to decrypt data");
        EVP_CIPHER_CTX_free(ctx);
        return VPN_ERROR_CRYPTO;
    }
    plaintext_len = len;
    
    /* Finalize decryption and verify tag */
    if (EVP_DecryptFinal_ex(ctx, plaintext + len, &len) != 1) {
        LOG_ERROR("Decryption failed or authentication tag invalid");
        EVP_CIPHER_CTX_free(ctx);
        return VPN_ERROR_CRYPTO;
    }
    plaintext_len += len;
    
    EVP_CIPHER_CTX_free(ctx);
    return plaintext_len;
}

/* SHA-256 hashing */
void sha256_hash(const unsigned char* data, int len, unsigned char* hash) {
    EVP_MD_CTX* mdctx;
    unsigned int hash_len = 0;
    
    if (!data || !hash) return;
    
    mdctx = EVP_MD_CTX_new();
    if (!mdctx) {
        LOG_ERROR("Failed to create digest context");
        return;
    }
    
    EVP_DigestInit_ex(mdctx, EVP_sha256(), NULL);
    EVP_DigestUpdate(mdctx, data, len);
    EVP_DigestFinal_ex(mdctx, hash, &hash_len);
    EVP_MD_CTX_free(mdctx);
}

/* HMAC-SHA256 */
int hmac_sha256(const unsigned char* key, int key_len,
                const unsigned char* data, int data_len,
                unsigned char* hmac) {
    unsigned int hmac_len = 0;
    
    if (!key || !data || !hmac) return VPN_ERROR_CRYPTO;
    
    if (HMAC(EVP_sha256(), key, key_len, data, data_len,
             hmac, &hmac_len) == NULL) {
        LOG_ERROR("Failed to compute HMAC");
        return VPN_ERROR_CRYPTO;
    }
    
    return hmac_len;
}

/* Secure memory zeroing */
void secure_memzero(void* ptr, size_t len) {
    OPENSSL_cleanse(ptr, len);
}
