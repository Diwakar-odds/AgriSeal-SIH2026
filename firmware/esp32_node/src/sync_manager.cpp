/**
 * AgriSeal Node — Sync Manager Module
 * ====================================
 * Handles synchronization of offline buffered sensor readings
 * when network connectivity (WiFi / GSM LTE-M) is restored.
 *
 * Implements batch transfer with MQTT acknowledgments, retry logic,
 * and rate limiting to avoid overwhelming the radio or broker.
 *
 * Author: Team Arishem
 * Version: 1.0.0
 * License: MIT
 */

#include <Arduino.h>
#include "config.h"

// External declarations for dependencies
extern bool mqtt_connected();
extern bool mqtt_publish_reading(const char* json_payload);
extern int  offline_store_count();
extern bool offline_store_read_next(char* buffer, size_t buflen);
extern void offline_store_clear_sent(int count);

// Maximum payload buffer for reading records
static char syncBuffer[512];
static uint32_t totalSyncedRecords = 0;

/**
 * Initialize sync manager
 */
void sync_manager_init() {
    Serial.println("[SYNC_MGR] Initializing Sync Manager...");
    totalSyncedRecords = 0;
    Serial.println("[SYNC_MGR] ✅ Ready.");
}

/**
 * Synchronize stored offline data with the remote MQTT broker.
 * Transmits records in batches defined by SYNC_BATCH_SIZE.
 */
void sync_manager_sync_offline_data() {
    int pendingCount = offline_store_count();
    if (pendingCount <= 0) {
        Serial.println("[SYNC_MGR] No offline records to sync.");
        return;
    }

    if (!mqtt_connected()) {
        Serial.println("[SYNC_MGR] ⚠️ Cannot sync: MQTT disconnected.");
        return;
    }

    Serial.printf("[SYNC_MGR] Starting sync of %d pending records (Batch size: %d)...\n",
                  pendingCount, SYNC_BATCH_SIZE);

    int batchCount = 0;
    int successfullySent = 0;

    while (batchCount < SYNC_BATCH_SIZE && offline_store_count() > 0) {
        if (!mqtt_connected()) {
            Serial.println("[SYNC_MGR] ⚠️ Connection dropped mid-sync. Aborting batch.");
            break;
        }

        memset(syncBuffer, 0, sizeof(syncBuffer));
        if (offline_store_read_next(syncBuffer, sizeof(syncBuffer))) {
            // Publish the buffered offline record
            if (mqtt_publish_reading(syncBuffer)) {
                successfullySent++;
                batchCount++;
                // Small delay between transmissions to allow network stack buffer flush
                delay(30);
            } else {
                Serial.printf("[SYNC_MGR] ❌ Publish failed at record %d of batch.\n", batchCount + 1);
                break;
            }
        } else {
            Serial.println("[SYNC_MGR] ⚠️ Failed to read next record from offline storage.");
            break;
        }
    }

    if (successfullySent > 0) {
        offline_store_clear_sent(successfullySent);
        totalSyncedRecords += successfullySent;
        Serial.printf("[SYNC_MGR] ✅ Successfully synced %d records. Total synced: %u. Remaining: %d\n",
                      successfullySent, totalSyncedRecords, offline_store_count());
    } else {
        Serial.println("[SYNC_MGR] No records synced in this iteration.");
    }
}
