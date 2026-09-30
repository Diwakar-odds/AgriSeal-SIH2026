/**
 * AgriSeal Node Configuration
 * ===========================
 * Pin definitions, MQTT broker config, sensor thresholds,
 * and system parameters for the AgriSeal IoT Node.
 *
 * Edit this file before flashing firmware to your node.
 */

#ifndef CONFIG_H
#define CONFIG_H

// ============================================================
// Device Identity
// ============================================================
#define DEVICE_ID           "AGRISEAL-NODE-0001"
#define FIRMWARE_VERSION    "1.0.0"
#define HARDWARE_REVISION   "v1.0"

// ============================================================
// Pin Definitions — ESP32-WROOM-32D
// ============================================================

// Sensors
#define DHT22_PIN           4       // GPIO 4 — Temperature & Humidity
#define MQ135_ANALOG_PIN    34      // GPIO 34 (ADC1_CH6) — Ethylene gas
#define BATTERY_ADC_PIN     35      // GPIO 35 (ADC1_CH7) — Battery voltage divider
#define TAMPER_SWITCH_PIN   15      // GPIO 15 — Enclosure tamper detection

// SPI Flash (W25Q128)
#define FLASH_CS_PIN        5       // GPIO 5 — Chip Select
#define FLASH_SCK_PIN       18      // GPIO 18 — Clock
#define FLASH_MISO_PIN      19      // GPIO 19 — MISO
#define FLASH_MOSI_PIN      23      // GPIO 23 — MOSI

// I²C Bus (ATECC608A + FM24C256 FRAM)
#define I2C_SDA_PIN         21      // GPIO 21
#define I2C_SCL_PIN         22      // GPIO 22

// Cellular Module (SIM7600E) — UART2
#define SIM_TX_PIN          16      // GPIO 16 (TX2) → SIM7600 RXD
#define SIM_RX_PIN          17      // GPIO 17 (RX2) → SIM7600 TXD
#define SIM_PWRKEY_PIN      25      // GPIO 25 — SIM7600 power key
#define SIM_RST_PIN         26      // GPIO 26 — SIM7600 reset

// Status LED
#define STATUS_LED_PIN      2       // GPIO 2 — Onboard LED

// ============================================================
// MQTT Configuration
// ============================================================
#define MQTT_BROKER         "broker.agriseal.in"    // Replace with your broker
#define MQTT_PORT           8883                     // TLS port
#define MQTT_USER           "agriseal_node"
#define MQTT_PASSWORD       "change_this_password"

// MQTT Topics (device ID appended at runtime)
#define MQTT_TOPIC_DATA     "agriseal/%s/data"
#define MQTT_TOPIC_CMD      "agriseal/%s/cmd"
#define MQTT_TOPIC_OTA      "agriseal/%s/ota"
#define MQTT_TOPIC_ACK      "agriseal/%s/ack"

// ============================================================
// Sensor Thresholds (Default — can be overridden via MQTT)
// ============================================================

// Temperature thresholds (°C)
#define TEMP_WARNING_HIGH   8.0
#define TEMP_CRITICAL_HIGH  12.0
#define TEMP_WARNING_LOW    -1.0
#define TEMP_CRITICAL_LOW   -5.0

// Humidity thresholds (% RH)
#define HUMIDITY_WARNING     90.0
#define HUMIDITY_CRITICAL    95.0

// Ethylene thresholds (ppm)
#define ETHYLENE_WARNING     50.0
#define ETHYLENE_CRITICAL    100.0

// ============================================================
// Timing Configuration
// ============================================================
#define SENSOR_READ_INTERVAL_MS     60000   // 60 seconds
#define MQTT_RECONNECT_INTERVAL_MS  5000    // 5 seconds
#define DEEP_SLEEP_DURATION_US      55000000 // 55 seconds (+ ~5s active = 60s cycle)
#define SYNC_BATCH_SIZE             50       // Readings per sync batch

// ============================================================
// Power Management
// ============================================================
#define BATTERY_FULL_VOLTAGE    4.20    // Fully charged Li-Ion
#define BATTERY_LOW_VOLTAGE     3.50    // Low battery threshold
#define BATTERY_CRITICAL_VOLTAGE 3.30   // Critical — reduce sampling
#define BATTERY_SHUTDOWN_VOLTAGE 3.00   // Shutdown and save

// Voltage divider ratio (R1=100kΩ, R2=100kΩ → ratio = 2.0)
#define VDIVIDER_RATIO      2.0
#define ADC_REF_VOLTAGE     3.3
#define ADC_RESOLUTION      4096

// ============================================================
// Security
// ============================================================
#define AES_KEY_SLOT        0       // ATECC608A key slot for AES
#define HASH_CHAIN_ENABLED  true
#define ENCRYPTION_ENABLED  true

// ============================================================
// Storage
// ============================================================
#define FLASH_SECTOR_SIZE   4096    // 4KB sectors
#define MAX_OFFLINE_READINGS 16384  // Max readings stored in flash
#define FRAM_I2C_ADDRESS    0x50    // FM24C256 default address
#define ATECC_I2C_ADDRESS   0x60   // ATECC608A default address

#endif // CONFIG_H
