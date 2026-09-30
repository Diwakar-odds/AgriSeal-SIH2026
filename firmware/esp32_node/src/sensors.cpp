/**
 * AgriSeal Node — Sensor Module
 * ==============================
 * Reads DHT22 (temperature/humidity), MQ135 (ethylene),
 * battery voltage, and tamper switch.
 *
 * Includes calibration offset support and moving average filter.
 */

#include <Arduino.h>
#include <DHT.h>
#include "config.h"

// ============================================================
// DHT22 Sensor Instance
// ============================================================
DHT dht(DHT22_PIN, DHT22);

// ============================================================
// Calibration offsets (stored in FRAM in production)
// ============================================================
static float temp_cal_offset  = 0.0;   // °C offset from calibration
static float hum_cal_offset   = 0.0;   // %RH offset from calibration

// MQ135 calibration constants (from datasheet curve fitting)
static const float MQ135_A    = 116.602;  // Gas-specific constant a
static const float MQ135_B    = -2.769;   // Gas-specific constant b
static float mq135_r0         = 76.63;    // Sensor resistance in clean air (calibrated)

// ============================================================
// Moving average filter (5 samples)
// ============================================================
#define MA_WINDOW 5
static float temp_buffer[MA_WINDOW]     = {0};
static float hum_buffer[MA_WINDOW]      = {0};
static float ethylene_buffer[MA_WINDOW] = {0};
static int   ma_index = 0;
static bool  ma_filled = false;

static float moving_average(float* buffer) {
    float sum = 0;
    int count = ma_filled ? MA_WINDOW : (ma_index + 1);
    for (int i = 0; i < count; i++) {
        sum += buffer[i];
    }
    return sum / count;
}

// ============================================================
// Public API
// ============================================================

/**
 * Initialize all sensor hardware
 */
void sensors_init() {
    Serial.println("[SENSORS] Initializing...");

    // DHT22
    dht.begin();

    // MQ135 analog input
    analogReadResolution(12);
    analogSetAttenuation(ADC_11db); // Full range 0-3.3V
    pinMode(MQ135_ANALOG_PIN, INPUT);

    // Battery ADC
    pinMode(BATTERY_ADC_PIN, INPUT);

    // Tamper switch (active LOW with internal pull-up)
    pinMode(TAMPER_SWITCH_PIN, INPUT_PULLUP);

    Serial.println("[SENSORS] ✅ All sensors initialized.");
}

/**
 * Read all sensors and apply calibration + filtering
 *
 * @param temp       Output: calibrated temperature in °C
 * @param humidity   Output: calibrated humidity in %RH
 * @param ethylene   Output: ethylene concentration in ppm
 * @param battery_v  Output: battery voltage in V
 */
void sensors_read(float &temp, float &humidity, float &ethylene, float &battery_v) {

    // --- Temperature & Humidity (DHT22) ---
    float raw_temp = dht.readTemperature();
    float raw_hum  = dht.readHumidity();

    // Check for read errors
    if (isnan(raw_temp) || isnan(raw_hum)) {
        Serial.println("[SENSORS] ⚠️ DHT22 read error — using last values.");
        raw_temp = temp_buffer[ma_index > 0 ? ma_index - 1 : MA_WINDOW - 1];
        raw_hum  = hum_buffer[ma_index > 0 ? ma_index - 1 : MA_WINDOW - 1];
    }

    // Apply calibration offsets
    raw_temp += temp_cal_offset;
    raw_hum  += hum_cal_offset;

    // Clamp humidity
    if (raw_hum > 100.0) raw_hum = 100.0;
    if (raw_hum < 0.0)   raw_hum = 0.0;

    // --- Ethylene (MQ135) ---
    int adc_raw = analogRead(MQ135_ANALOG_PIN);
    float sensor_voltage = (adc_raw / (float)ADC_RESOLUTION) * ADC_REF_VOLTAGE;

    // Compute Rs (sensor resistance)
    float rs = ((ADC_REF_VOLTAGE * 10.0) / sensor_voltage) - 10.0; // RL = 10kΩ
    float ratio = rs / mq135_r0;

    // Convert to ppm using power curve: ppm = a * (Rs/R0)^b
    float raw_ethylene = MQ135_A * pow(ratio, MQ135_B);
    if (raw_ethylene < 0) raw_ethylene = 0;
    if (raw_ethylene > 1000) raw_ethylene = 1000;

    // --- Moving Average Filter ---
    temp_buffer[ma_index]     = raw_temp;
    hum_buffer[ma_index]      = raw_hum;
    ethylene_buffer[ma_index] = raw_ethylene;

    ma_index++;
    if (ma_index >= MA_WINDOW) {
        ma_index = 0;
        ma_filled = true;
    }

    temp     = moving_average(temp_buffer);
    humidity = moving_average(hum_buffer);
    ethylene = moving_average(ethylene_buffer);

    // --- Battery Voltage ---
    int batt_adc = analogRead(BATTERY_ADC_PIN);
    battery_v = (batt_adc / (float)ADC_RESOLUTION) * ADC_REF_VOLTAGE * VDIVIDER_RATIO;
}

/**
 * Check if tamper switch has been triggered (enclosure opened)
 *
 * @return true if tamper detected (switch open = HIGH due to pull-up)
 */
bool sensors_check_tamper() {
    return digitalRead(TAMPER_SWITCH_PIN) == HIGH;
}
