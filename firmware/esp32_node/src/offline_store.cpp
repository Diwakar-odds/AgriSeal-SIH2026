/**
 * AgriSeal Node — Offline Storage Module
 * ========================================
 * Manages offline data storage in SPI Flash (W25Q128).
 * Implements a circular buffer for sensor readings when
 * network connectivity is unavailable.
 *
 * Storage Format:
 *   - 4-byte header: magic (0xAG), length (2 bytes), flags (1 byte)
 *   - N-byte payload: JSON string
 *   - 2-byte CRC16: integrity check
 */

#include <Arduino.h>
#include <SPI.h>
#include "config.h"

// ============================================================
// Storage Constants
// ============================================================
#define STORAGE_MAGIC       0xAA
#define RECORD_HEADER_SIZE  4
#define RECORD_CRC_SIZE     2
#define MAX_RECORD_SIZE     512
#define RECORDS_PER_SECTOR  (FLASH_SECTOR_SIZE / MAX_RECORD_SIZE)

// ============================================================
// State (persisted in FRAM for crash recovery)
// ============================================================
static uint32_t write_ptr = 0;     // Next write position (flash address)
static uint32_t read_ptr  = 0;     // Next read position (for sync)
static int      stored_count = 0;  // Number of unsynced readings

// ============================================================
// Internal helpers
// ============================================================

/**
 * Simple CRC16 for data integrity
 */
static uint16_t crc16(const uint8_t* data, size_t len) {
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (int j = 0; j < 8; j++) {
            if (crc & 1) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}

/**
 * Write a byte to SPI Flash at given address
 * (Simplified — in production, use SPIMemory library)
 */
static void flash_write(uint32_t addr, const uint8_t* data, size_t len) {
    // In production: use SPIMemory library for proper page writes
    // This is a simplified placeholder showing the concept
    digitalWrite(FLASH_CS_PIN, LOW);
    SPI.transfer(0x06); // Write Enable
    digitalWrite(FLASH_CS_PIN, HIGH);
    delayMicroseconds(10);

    digitalWrite(FLASH_CS_PIN, LOW);
    SPI.transfer(0x02); // Page Program
    SPI.transfer((addr >> 16) & 0xFF);
    SPI.transfer((addr >> 8) & 0xFF);
    SPI.transfer(addr & 0xFF);
    for (size_t i = 0; i < len; i++) {
        SPI.transfer(data[i]);
    }
    digitalWrite(FLASH_CS_PIN, HIGH);
    delay(5); // Wait for write completion
}

/**
 * Read bytes from SPI Flash
 */
static void flash_read(uint32_t addr, uint8_t* data, size_t len) {
    digitalWrite(FLASH_CS_PIN, LOW);
    SPI.transfer(0x03); // Read Data
    SPI.transfer((addr >> 16) & 0xFF);
    SPI.transfer((addr >> 8) & 0xFF);
    SPI.transfer(addr & 0xFF);
    for (size_t i = 0; i < len; i++) {
        data[i] = SPI.transfer(0x00);
    }
    digitalWrite(FLASH_CS_PIN, HIGH);
}

// ============================================================
// Public API
// ============================================================

/**
 * Initialize offline storage — recover pointers from FRAM
 */
void offline_store_init() {
    Serial.println("[STORAGE] Initializing Flash storage...");

    pinMode(FLASH_CS_PIN, OUTPUT);
    digitalWrite(FLASH_CS_PIN, HIGH);

    // TODO: Read write_ptr, read_ptr, stored_count from FRAM
    // For now, start fresh each boot
    write_ptr = 0;
    read_ptr = 0;
    stored_count = 0;

    Serial.printf("[STORAGE] ✅ Ready. Capacity: %d readings\n", MAX_OFFLINE_READINGS);
}

/**
 * Save a JSON payload to flash storage
 *
 * @param json_payload  Null-terminated JSON string
 * @return true if saved successfully
 */
bool offline_store_save(const char* json_payload) {
    size_t payload_len = strlen(json_payload);

    if (payload_len > MAX_RECORD_SIZE - RECORD_HEADER_SIZE - RECORD_CRC_SIZE) {
        Serial.println("[STORAGE] ❌ Payload too large!");
        return false;
    }

    if (stored_count >= MAX_OFFLINE_READINGS) {
        Serial.println("[STORAGE] ⚠️ Storage full — overwriting oldest.");
        // Circular buffer: oldest data gets overwritten
    }

    // Build record: [header][payload][crc]
    uint8_t record[MAX_RECORD_SIZE] = {0};
    record[0] = 0xAA;                          // Magic byte
    record[1] = (payload_len >> 8) & 0xFF;     // Length high
    record[2] = payload_len & 0xFF;            // Length low
    record[3] = 0x01;                          // Flags: 0x01 = valid, unsynced

    memcpy(&record[RECORD_HEADER_SIZE], json_payload, payload_len);

    uint16_t crc = crc16((uint8_t*)json_payload, payload_len);
    record[RECORD_HEADER_SIZE + payload_len]     = (crc >> 8) & 0xFF;
    record[RECORD_HEADER_SIZE + payload_len + 1] = crc & 0xFF;

    // Write to flash
    flash_write(write_ptr, record, RECORD_HEADER_SIZE + payload_len + RECORD_CRC_SIZE);

    // Advance write pointer (circular)
    write_ptr += MAX_RECORD_SIZE;
    if (write_ptr >= (uint32_t)(MAX_OFFLINE_READINGS * MAX_RECORD_SIZE)) {
        write_ptr = 0; // Wrap around
    }

    stored_count++;

    Serial.printf("[STORAGE] Saved record at 0x%08X (%d bytes)\n",
                  write_ptr - MAX_RECORD_SIZE, payload_len);

    return true;
}

/**
 * Get count of unsynced readings in storage
 */
int offline_store_count() {
    return stored_count;
}

/**
 * Read next unsynced record from storage
 *
 * @param buffer  Output buffer for JSON string
 * @param buflen  Size of output buffer
 * @return true if a record was read
 */
bool offline_store_read_next(char* buffer, size_t buflen) {
    if (stored_count <= 0) return false;

    uint8_t record[MAX_RECORD_SIZE];
    flash_read(read_ptr, record, MAX_RECORD_SIZE);

    // Validate magic byte
    if (record[0] != 0xAA) {
        Serial.println("[STORAGE] ⚠️ Invalid record — skipping.");
        read_ptr += MAX_RECORD_SIZE;
        stored_count--;
        return false;
    }

    // Extract payload length
    uint16_t payload_len = (record[1] << 8) | record[2];
    if (payload_len >= buflen) {
        Serial.println("[STORAGE] ⚠️ Buffer too small.");
        return false;
    }

    // Verify CRC
    uint16_t stored_crc = (record[RECORD_HEADER_SIZE + payload_len] << 8) |
                           record[RECORD_HEADER_SIZE + payload_len + 1];
    uint16_t calc_crc = crc16(&record[RECORD_HEADER_SIZE], payload_len);

    if (stored_crc != calc_crc) {
        Serial.println("[STORAGE] ⚠️ CRC mismatch — data corrupted!");
        read_ptr += MAX_RECORD_SIZE;
        stored_count--;
        return false;
    }

    // Copy payload to output
    memcpy(buffer, &record[RECORD_HEADER_SIZE], payload_len);
    buffer[payload_len] = '\0';

    return true;
}

/**
 * Mark N records as synced (advance read pointer)
 */
void offline_store_clear_sent(int count) {
    for (int i = 0; i < count && stored_count > 0; i++) {
        read_ptr += MAX_RECORD_SIZE;
        if (read_ptr >= (uint32_t)(MAX_OFFLINE_READINGS * MAX_RECORD_SIZE)) {
            read_ptr = 0;
        }
        stored_count--;
    }
    Serial.printf("[STORAGE] Cleared %d records. Remaining: %d\n", count, stored_count);
}
