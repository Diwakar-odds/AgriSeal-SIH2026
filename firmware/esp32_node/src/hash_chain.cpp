/**
 * AgriSeal Node — Hash Chain Module
 * ===================================
 * Implements a lightweight SHA-256 hash chain for tamper-evident
 * data recording. Each sensor reading is hashed with the previous
 * block's hash, creating an unbreakable chain of integrity.
 *
 * Chain Structure:
 *   Block_N.hash = SHA256(block_index || data || Block_(N-1).hash)
 */

#include <Arduino.h>
#include <mbedtls/sha256.h>
#include "config.h"

// ============================================================
// Hash Chain State
// ============================================================
static char prev_hash[65] = "0000000000000000000000000000000000000000000000000000000000000000"; // Genesis
static uint32_t chain_length = 0;

// RTC memory — persists across deep sleep
RTC_DATA_ATTR static char rtc_prev_hash[65] = {0};
RTC_DATA_ATTR static uint32_t rtc_chain_length = 0;

// ============================================================
// Internal — Convert bytes to hex string
// ============================================================
static void bytes_to_hex(const uint8_t* bytes, size_t len, char* hex_str) {
    for (size_t i = 0; i < len; i++) {
        sprintf(&hex_str[i * 2], "%02x", bytes[i]);
    }
    hex_str[len * 2] = '\0';
}

// ============================================================
// Public API
// ============================================================

/**
 * Initialize hash chain — restore from RTC memory if available
 */
void hash_chain_init() {
    Serial.println("[HASHCHAIN] Initializing...");

    // Restore chain state from RTC memory (survives deep sleep)
    if (rtc_chain_length > 0 && rtc_prev_hash[0] != '\0') {
        strncpy(prev_hash, rtc_prev_hash, sizeof(prev_hash));
        chain_length = rtc_chain_length;
        Serial.printf("[HASHCHAIN] Restored chain at block #%u\n", chain_length);
    } else {
        // Genesis block
        Serial.println("[HASHCHAIN] Starting new chain (genesis block).");
    }
}

/**
 * Add a new block to the hash chain
 *
 * Computes: hash = SHA256(chain_length || data || prev_hash)
 *
 * @param data      Input data to hash (typically JSON payload)
 * @param len       Length of input data
 * @param out_hash  Output buffer for hex hash string (must be >= 65 bytes)
 * @param hash_len  Size of output buffer
 */
void hash_chain_add_block(const char* data, size_t len, char* out_hash, size_t hash_len) {
    if (hash_len < 65) {
        Serial.println("[HASHCHAIN] ❌ Output buffer too small!");
        return;
    }

    chain_length++;

    // Prepare input: chain_length + data + prev_hash
    mbedtls_sha256_context ctx;
    mbedtls_sha256_init(&ctx);
    mbedtls_sha256_starts(&ctx, 0); // 0 = SHA-256 (not SHA-224)

    // Feed chain length
    mbedtls_sha256_update(&ctx, (const unsigned char*)&chain_length, sizeof(chain_length));

    // Feed data
    mbedtls_sha256_update(&ctx, (const unsigned char*)data, len);

    // Feed previous hash
    mbedtls_sha256_update(&ctx, (const unsigned char*)prev_hash, strlen(prev_hash));

    // Finalize
    uint8_t hash_bytes[32];
    mbedtls_sha256_finish(&ctx, hash_bytes);
    mbedtls_sha256_free(&ctx);

    // Convert to hex string
    bytes_to_hex(hash_bytes, 32, out_hash);

    // Update chain state
    strncpy(prev_hash, out_hash, sizeof(prev_hash));

    // Persist to RTC memory
    strncpy(rtc_prev_hash, prev_hash, sizeof(rtc_prev_hash));
    rtc_chain_length = chain_length;

    Serial.printf("[HASHCHAIN] Block #%u: %s\n", chain_length, out_hash);
}

/**
 * Get the previous block's hash (for inclusion in JSON payload)
 *
 * @return Pointer to 64-character hex hash string
 */
const char* hash_chain_get_prev_hash() {
    return prev_hash;
}
