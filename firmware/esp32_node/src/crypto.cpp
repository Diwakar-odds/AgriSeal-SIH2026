/**
 * AgriSeal Node — Cryptography Module
 * =====================================
 * AES-128-CBC encryption using ESP32's hardware crypto engine.
 * In production, keys are stored in the ATECC608A secure element.
 */

#include <Arduino.h>
#include <mbedtls/aes.h>
#include "config.h"

// ============================================================
// AES Key (In production: read from ATECC608A)
// ============================================================
// WARNING: This is a placeholder key for development only!
// In production, the key must be stored in the ATECC608A secure element
// and never appear in source code.
static const uint8_t AES_KEY[16] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F
};

// Initialization Vector (should be unique per encryption — use counter or random)
static uint8_t aes_iv[16] = {
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F
};

static mbedtls_aes_context aes_ctx;
static bool _crypto_initialized = false;

// ============================================================
// Public API
// ============================================================

/**
 * Initialize the AES crypto engine
 */
void crypto_init() {
    Serial.println("[CRYPTO] Initializing AES-128...");

    mbedtls_aes_init(&aes_ctx);

    // Set encryption key
    int ret = mbedtls_aes_setkey_enc(&aes_ctx, AES_KEY, 128);
    if (ret != 0) {
        Serial.printf("[CRYPTO] ❌ AES key setup failed: %d\n", ret);
        return;
    }

    _crypto_initialized = true;
    Serial.println("[CRYPTO] ✅ AES-128-CBC ready (hardware accelerated).");

    // TODO: Initialize ATECC608A and read key from secure element
    // Wire.beginTransmission(ATECC_I2C_ADDRESS);
    // ...
}

/**
 * Encrypt data using AES-128-CBC
 *
 * @param plaintext   Input data to encrypt
 * @param len         Length of input data
 * @param ciphertext  Output buffer for encrypted data (must be >= len + 16 for padding)
 * @param out_len     Output: actual length of encrypted data
 * @return true if encryption successful
 */
bool crypto_encrypt(const uint8_t* plaintext, size_t len, uint8_t* ciphertext, size_t* out_len) {
    if (!_crypto_initialized) {
        Serial.println("[CRYPTO] ❌ Not initialized!");
        return false;
    }

    // PKCS7 padding
    size_t padded_len = ((len / 16) + 1) * 16;
    uint8_t padded[512];

    if (padded_len > sizeof(padded)) {
        Serial.println("[CRYPTO] ❌ Input too large!");
        return false;
    }

    memcpy(padded, plaintext, len);
    uint8_t pad_value = padded_len - len;
    for (size_t i = len; i < padded_len; i++) {
        padded[i] = pad_value;
    }

    // Reset IV for each encryption (in production: use unique IV)
    uint8_t iv_copy[16];
    memcpy(iv_copy, aes_iv, 16);

    // Encrypt
    int ret = mbedtls_aes_crypt_cbc(&aes_ctx, MBEDTLS_AES_ENCRYPT,
                                     padded_len, iv_copy,
                                     padded, ciphertext);

    if (ret != 0) {
        Serial.printf("[CRYPTO] ❌ Encryption failed: %d\n", ret);
        return false;
    }

    *out_len = padded_len;

    Serial.printf("[CRYPTO] Encrypted %d → %d bytes\n", len, padded_len);
    return true;
}
