/**
 * AgriSeal Node — Power Management Module
 * ========================================
 * Monitors battery state-of-charge (SoC), manages power modes,
 * controls dynamic sampling intervals based on available solar/battery
 * reserves, and handles deep sleep entry with wakeup sources.
 *
 * Author: Team Arishem
 * Version: 1.0.0
 * License: MIT
 */

#include <Arduino.h>
#include <esp_sleep.h>
#include "config.h"

// Latest calculated State of Charge (0 - 100%)
static float current_soc = 100.0;
static float current_voltage = BATTERY_FULL_VOLTAGE;

/**
 * Measure raw battery voltage using internal ADC & calibrated divider ratio
 */
static float read_raw_battery_voltage() {
    int raw_adc = analogRead(BATTERY_ADC_PIN);
    float pin_voltage = (raw_adc / (float)ADC_RESOLUTION) * ADC_REF_VOLTAGE;
    return pin_voltage * VDIVIDER_RATIO;
}

/**
 * Convert Li-Ion cell voltage to approximate State of Charge (%)
 * Uses an empirical 5-point piecewise linear curve for 18650 Li-ion cells:
 *   >= 4.20V -> 100%
 *      4.00V -> 80%
 *      3.80V -> 55%
 *      3.60V -> 25%
 *      3.40V -> 10%
 *   <= 3.00V -> 0%
 */
static float voltage_to_soc(float voltage) {
    if (voltage >= BATTERY_FULL_VOLTAGE) return 100.0;
    if (voltage <= BATTERY_SHUTDOWN_VOLTAGE) return 0.0;

    if (voltage > 4.00) {
        return 80.0 + (voltage - 4.00) * (20.0 / (4.20 - 4.00));
    } else if (voltage > 3.80) {
        return 55.0 + (voltage - 3.80) * (25.0 / (4.00 - 3.80));
    } else if (voltage > 3.60) {
        return 25.0 + (voltage - 3.60) * (30.0 / (3.80 - 3.60));
    } else if (voltage > 3.40) {
        return 10.0 + (voltage - 3.40) * (15.0 / (3.60 - 3.40));
    } else {
        return (voltage - BATTERY_SHUTDOWN_VOLTAGE) * (10.0 / (3.40 - BATTERY_SHUTDOWN_VOLTAGE));
    }
}

/**
 * Initialize power management unit
 */
void power_mgmt_init() {
    Serial.println("[POWER] Initializing Power Management Unit...");
    
    // Sample initial voltage
    current_voltage = read_raw_battery_voltage();
    current_soc = voltage_to_soc(current_voltage);

    Serial.printf("[POWER] Initial Battery: %.2fV (SoC: %.1f%%)\n", current_voltage, current_soc);
    Serial.println("[POWER] ✅ PMU Ready.");
}

/**
 * Get current State of Charge (%)
 */
float power_mgmt_get_soc() {
    current_voltage = read_raw_battery_voltage();
    current_soc = voltage_to_soc(current_voltage);
    return current_soc;
}

/**
 * Determine if sampling interval should be reduced to conserve energy
 */
bool power_mgmt_should_reduce_sampling() {
    current_voltage = read_raw_battery_voltage();
    return (current_voltage < BATTERY_LOW_VOLTAGE);
}

/**
 * Determine if battery has hit critical cutoff threshold requiring system shutdown
 */
bool power_mgmt_should_shutdown() {
    current_voltage = read_raw_battery_voltage();
    return (current_voltage <= BATTERY_SHUTDOWN_VOLTAGE);
}

/**
 * Configure wakeup sources and enter ESP32 deep sleep
 *
 * Wakeup sources:
 * 1. Timer (duration_us)
 * 2. External interrupt on TAMPER_SWITCH_PIN (wake immediately if box is opened)
 */
void power_mgmt_enter_deep_sleep(uint64_t duration_us) {
    // Enable timer wakeup
    esp_sleep_enable_timer_wakeup(duration_us);

    // Enable external wakeup on tamper switch (active HIGH)
    // GPIO 15 is RTC GPIO 13 on ESP32
    esp_sleep_enable_ext0_wakeup((gpio_num_t)TAMPER_SWITCH_PIN, 1);

    // Isolate GPIO pins to minimize leakage current during sleep
    gpio_hold_en((gpio_num_t)FLASH_CS_PIN);

    // Flush serial buffer before sleep
    Serial.flush();

    // Enter deep sleep
    esp_deep_sleep_start();
}
