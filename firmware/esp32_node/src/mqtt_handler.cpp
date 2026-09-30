/**
 * AgriSeal Node — MQTT Handler
 * =============================
 * Manages MQTT connection to cloud broker via WiFi or cellular.
 * Handles publish, subscribe, and reconnection logic.
 */

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "config.h"

// ============================================================
// MQTT Client
// ============================================================
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

// Topic buffers
static char topicData[64];
static char topicCmd[64];
static char topicAck[64];

// Connection state
static bool _mqtt_initialized = false;
static unsigned long lastReconnectAttempt = 0;

// ============================================================
// MQTT Callback — Handle incoming commands
// ============================================================
static void mqtt_callback(char* topic, byte* payload, unsigned int length) {
    Serial.printf("[MQTT] Message on topic: %s\n", topic);

    // Parse command (JSON expected)
    char msg[256];
    size_t copyLen = (length < sizeof(msg) - 1) ? length : sizeof(msg) - 1;
    memcpy(msg, payload, copyLen);
    msg[copyLen] = '\0';

    Serial.printf("[MQTT] Payload: %s\n", msg);

    // TODO: Parse JSON and handle commands:
    //   - "set_interval": Change sampling interval
    //   - "set_thresholds": Update temperature/humidity thresholds
    //   - "reboot": Restart the node
    //   - "ota_update": Trigger firmware update
}

// ============================================================
// Public API
// ============================================================

/**
 * Initialize MQTT client and build topic strings
 */
void mqtt_init() {
    Serial.println("[MQTT] Initializing...");

    // Build device-specific topics
    snprintf(topicData, sizeof(topicData), MQTT_TOPIC_DATA, DEVICE_ID);
    snprintf(topicCmd,  sizeof(topicCmd),  MQTT_TOPIC_CMD,  DEVICE_ID);
    snprintf(topicAck,  sizeof(topicAck),  MQTT_TOPIC_ACK,  DEVICE_ID);

    // Configure MQTT client
    mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
    mqttClient.setCallback(mqtt_callback);
    mqttClient.setBufferSize(512); // Increase for larger payloads

    _mqtt_initialized = true;
    Serial.printf("[MQTT] Broker: %s:%d\n", MQTT_BROKER, MQTT_PORT);
    Serial.printf("[MQTT] Data topic: %s\n", topicData);
}

/**
 * Check if MQTT is currently connected
 */
bool mqtt_connected() {
    if (!_mqtt_initialized) return false;

    // Also check WiFi
    if (WiFi.status() != WL_CONNECTED) return false;

    return mqttClient.connected();
}

/**
 * Attempt to reconnect to MQTT broker
 *
 * @return true if connection established
 */
bool mqtt_reconnect() {
    if (!_mqtt_initialized) return false;

    unsigned long now = millis();
    if (now - lastReconnectAttempt < MQTT_RECONNECT_INTERVAL_MS) {
        return false; // Rate limit reconnect attempts
    }
    lastReconnectAttempt = now;

    Serial.println("[MQTT] Attempting connection...");

    if (mqttClient.connect(DEVICE_ID, MQTT_USER, MQTT_PASSWORD)) {
        Serial.println("[MQTT] ✅ Connected!");

        // Subscribe to command topic
        mqttClient.subscribe(topicCmd);
        Serial.printf("[MQTT] Subscribed to: %s\n", topicCmd);

        return true;
    } else {
        Serial.printf("[MQTT] ❌ Failed, rc=%d\n", mqttClient.state());
        return false;
    }
}

/**
 * Publish a sensor reading to the data topic
 *
 * @param json_payload  JSON string to publish
 * @return true if published successfully
 */
bool mqtt_publish_reading(const char* json_payload) {
    if (!mqtt_connected()) {
        if (!mqtt_reconnect()) {
            return false;
        }
    }

    bool result = mqttClient.publish(topicData, json_payload, false); // QoS 0

    if (result) {
        Serial.printf("[MQTT] Published %d bytes to %s\n",
                      strlen(json_payload), topicData);
    } else {
        Serial.println("[MQTT] ❌ Publish failed.");
    }

    return result;
}

/**
 * Process MQTT client loop (must be called regularly)
 */
void mqtt_loop() {
    if (mqtt_connected()) {
        mqttClient.loop();
    }
}
