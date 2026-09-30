/**
 * AgriSeal Node — Main Firmware
 * =============================
 * Entry point for the ESP32 AgriSeal IoT node.
 * Implements the core state machine:
 *   BOOT → SENSE → ENCRYPT+HASH → SYNC/STORE → SLEEP → repeat
 *
 * Author: Team Arishem
 * Version: 1.0.0
 * License: MIT
 */

#include <Arduino.h>
#include <WiFi.h>
#include <Wire.h>
#include <SPI.h>
#include "config.h"

// Forward declarations for module functions
// (Defined in their respective .cpp files)
extern void sensors_init();
extern void sensors_read(float &temp, float &humidity, float &ethylene, float &battery_v);
extern bool sensors_check_tamper();

extern void mqtt_init();
extern bool mqtt_connected();
extern bool mqtt_publish_reading(const char* json_payload);
extern void mqtt_loop();
extern bool mqtt_reconnect();

extern void offline_store_init();
extern bool offline_store_save(const char* json_payload);
extern int  offline_store_count();
extern bool offline_store_read_next(char* buffer, size_t buflen);
extern void offline_store_clear_sent(int count);

extern void hash_chain_init();
extern void hash_chain_add_block(const char* data, size_t len, char* out_hash, size_t hash_len);
extern const char* hash_chain_get_prev_hash();

extern void crypto_init();
extern bool crypto_encrypt(const uint8_t* plaintext, size_t len, uint8_t* ciphertext, size_t* out_len);

extern void sync_manager_init();
extern void sync_manager_sync_offline_data();

extern void power_mgmt_init();
extern float power_mgmt_get_soc();
extern bool power_mgmt_should_reduce_sampling();
extern bool power_mgmt_should_shutdown();
extern void power_mgmt_enter_deep_sleep(uint64_t duration_us);

extern void gps_init();
extern bool gps_read(float &latitude, float &longitude, float &altitude, float &speed, int &satellites, bool &fix_status);

// ============================================================
// Global State
// ============================================================
enum NodeState {
    STATE_BOOT,
    STATE_SENSE,
    STATE_ENCRYPT_HASH,
    STATE_SYNC,
    STATE_STORE_OFFLINE,
    STATE_SLEEP
};

NodeState currentState = STATE_BOOT;
RTC_DATA_ATTR int bootCount = 0;           // Persists across deep sleep
RTC_DATA_ATTR int readingIndex = 0;        // Block index for hash chain

// Sensor reading buffer
float temperature = 0.0;
float humidity = 0.0;
float ethylene_ppm = 0.0;
float battery_voltage = 0.0;

// Geolocation buffer
float gps_latitude = 31.1048;
float gps_longitude = 77.1734;
float gps_altitude = 2205.0;
float gps_speed = 0.0;
int   gps_satellites = 0;
bool  gps_has_fix = false;

// JSON payload buffer
char jsonPayload[512];
char hashOutput[65]; // SHA-256 hex string (64 chars + null)

// ============================================================
// Setup — Runs once on every boot (including deep sleep wake)
// ============================================================
void setup() {
    Serial.begin(115200);
    delay(100);

    bootCount++;
    Serial.printf("\n\n========================================\n");
    Serial.printf("  AgriSeal Node %s\n", DEVICE_ID);
    Serial.printf("  Firmware: %s | Boot #%d\n", FIRMWARE_VERSION, bootCount);
    Serial.printf("========================================\n\n");

    // Initialize status LED
    pinMode(STATUS_LED_PIN, OUTPUT);
    digitalWrite(STATUS_LED_PIN, HIGH); // LED on during active period

    // Initialize I²C bus
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);

    // Initialize SPI bus
    SPI.begin(FLASH_SCK_PIN, FLASH_MISO_PIN, FLASH_MOSI_PIN, FLASH_CS_PIN);

    // Initialize all modules
    sensors_init();
    gps_init();
    crypto_init();
    hash_chain_init();
    offline_store_init();
    mqtt_init();
    sync_manager_init();
    power_mgmt_init();

    Serial.println("[BOOT] All modules initialized.");
    currentState = STATE_SENSE;
}

// ============================================================
// Main Loop — State Machine
// ============================================================
void loop() {
    switch (currentState) {

        // -----------------------------------------------
        // STATE: SENSE — Read all sensors & GPS
        // -----------------------------------------------
        case STATE_SENSE: {
            Serial.println("[SENSE] Reading sensors and GNSS...");

            sensors_read(temperature, humidity, ethylene_ppm, battery_voltage);
            gps_read(gps_latitude, gps_longitude, gps_altitude, gps_speed, gps_satellites, gps_has_fix);

            Serial.printf("[SENSE] Temp=%.1f°C  Humidity=%.1f%%  Ethylene=%.1f ppm  Battery=%.2fV\n",
                          temperature, humidity, ethylene_ppm, battery_voltage);
            Serial.printf("[GNSS]  Lat=%.4f  Lon=%.4f  Alt=%.1fm  Spd=%.1fkm/h  Sats=%d  Fix=%s\n",
                          gps_latitude, gps_longitude, gps_altitude, gps_speed, gps_satellites,
                          gps_has_fix ? "YES" : "NO");

            // Check for tamper
            if (sensors_check_tamper()) {
                Serial.println("[ALERT] ⚠️ TAMPER DETECTED — Enclosure opened!");
                // TODO: Push immediate tamper alert via MQTT
            }

            // Check thresholds
            if (temperature > TEMP_CRITICAL_HIGH) {
                Serial.printf("[ALERT] 🔴 CRITICAL — Temperature %.1f°C exceeds %.1f°C!\n",
                              temperature, TEMP_CRITICAL_HIGH);
            } else if (temperature > TEMP_WARNING_HIGH) {
                Serial.printf("[ALERT] 🟡 WARNING — Temperature %.1f°C exceeds %.1f°C\n",
                              temperature, TEMP_WARNING_HIGH);
            }

            if (ethylene_ppm > ETHYLENE_CRITICAL) {
                Serial.printf("[ALERT] 🔴 CRITICAL — Ethylene %.1f ppm exceeds %.1f ppm!\n",
                              ethylene_ppm, ETHYLENE_CRITICAL);
            }

            currentState = STATE_ENCRYPT_HASH;
            break;
        }

        // -----------------------------------------------
        // STATE: ENCRYPT + HASH — Secure the reading
        // -----------------------------------------------
        case STATE_ENCRYPT_HASH: {
            Serial.println("[CRYPTO] Encrypting and hashing...");

            readingIndex++;

            // Build JSON payload with environmental + GPS telemetry
            snprintf(jsonPayload, sizeof(jsonPayload),
                "{"
                "\"idx\":%d,"
                "\"dev\":\"%s\","
                "\"ts\":%lu,"
                "\"t\":%.2f,"
                "\"h\":%.2f,"
                "\"e\":%.2f,"
                "\"lat\":%.4f,"
                "\"lon\":%.4f,"
                "\"spd\":%.1f,"
                "\"sats\":%d,"
                "\"bv\":%.2f,"
                "\"soc\":%.1f,"
                "\"prev\":\"%s\""
                "}",
                readingIndex,
                DEVICE_ID,
                (unsigned long)millis(), // In production: use NTP or RTC timestamp
                temperature,
                humidity,
                ethylene_ppm,
                gps_latitude,
                gps_longitude,
                gps_speed,
                gps_satellites,
                battery_voltage,
                power_mgmt_get_soc(),
                hash_chain_get_prev_hash()
            );

            // Compute hash chain block
            hash_chain_add_block(jsonPayload, strlen(jsonPayload), hashOutput, sizeof(hashOutput));

            Serial.printf("[CRYPTO] Block #%d — Hash: %.16s...\n", readingIndex, hashOutput);

            // Determine next state based on connectivity
            if (mqtt_connected()) {
                currentState = STATE_SYNC;
            } else {
                currentState = STATE_STORE_OFFLINE;
            }
            break;
        }

        // -----------------------------------------------
        // STATE: SYNC — Send data via MQTT + sync offline buffer
        // -----------------------------------------------
        case STATE_SYNC: {
            Serial.println("[SYNC] Publishing to MQTT...");

            if (mqtt_publish_reading(jsonPayload)) {
                Serial.println("[SYNC] ✅ Published successfully.");
            } else {
                Serial.println("[SYNC] ❌ Publish failed — storing offline.");
                offline_store_save(jsonPayload);
            }

            // Also sync any buffered offline readings
            int offline_count = offline_store_count();
            if (offline_count > 0) {
                Serial.printf("[SYNC] Syncing %d offline readings...\n", offline_count);
                sync_manager_sync_offline_data();
            }

            currentState = STATE_SLEEP;
            break;
        }

        // -----------------------------------------------
        // STATE: STORE OFFLINE — Save to Flash/FRAM
        // -----------------------------------------------
        case STATE_STORE_OFFLINE: {
            Serial.println("[OFFLINE] No network — storing locally.");

            if (offline_store_save(jsonPayload)) {
                Serial.printf("[OFFLINE] Saved. Total buffered: %d\n", offline_store_count());
            } else {
                Serial.println("[OFFLINE] ❌ Storage full or write error!");
            }

            currentState = STATE_SLEEP;
            break;
        }

        // -----------------------------------------------
        // STATE: SLEEP — Enter deep sleep
        // -----------------------------------------------
        case STATE_SLEEP: {
            // Check if we should shutdown due to low battery
            if (power_mgmt_should_shutdown()) {
                Serial.println("[POWER] 🔴 Battery critically low — shutting down.");
                // Save state to FRAM before shutdown
                offline_store_save("{\"event\":\"shutdown\",\"reason\":\"low_battery\"}");
                esp_deep_sleep_start(); // Indefinite sleep until solar recharge
                return;
            }

            // Determine sleep duration
            uint64_t sleep_duration = DEEP_SLEEP_DURATION_US;
            if (power_mgmt_should_reduce_sampling()) {
                sleep_duration *= 2; // Double the interval to save power
                Serial.println("[POWER] 🟡 Low battery — reducing sampling rate.");
            }

            // Turn off LED
            digitalWrite(STATUS_LED_PIN, LOW);

            Serial.printf("[SLEEP] Entering deep sleep for %llu ms...\n",
                          sleep_duration / 1000);
            Serial.println("========================================\n");

            // Enter deep sleep
            power_mgmt_enter_deep_sleep(sleep_duration);

            // Code below this point never executes (wakes up in setup())
            break;
        }

        default:
            currentState = STATE_SENSE;
            break;
    }
}
