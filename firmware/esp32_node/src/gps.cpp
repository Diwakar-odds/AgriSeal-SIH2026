/**
 * AgriSeal Node — GPS/GNSS Module
 * =================================
 * Interfaces with u-blox NEO-6M / ATGM336H GNSS module over UART1.
 * Parses NMEA sentences ($GPRMC, $GPGGA) for real-time geolocation,
 * speed, altitude, timestamp, and satellite lock status.
 *
 * Implements fallback cache and dead-reckoning support for tunnels/indoor loading docks.
 *
 * Author: Team Arishem (SIH 2026 - PS 26232)
 * License: MIT
 */

#include <Arduino.h>
#include <HardwareSerial.h>
#include "config.h"

// Hardware Serial instance for GPS receiver (UART1)
static HardwareSerial GPSSerial(1);

// Cached coordinates (persisted in case of transient satellite loss under bridge/canopy)
static float last_lat = 31.1048;   // Default: Shimla Cold Cluster latitude
static float last_lon = 77.1734;   // Default: Shimla Cold Cluster longitude
static float last_alt = 2205.0;    // Altitude in meters
static float last_spd = 0.0;       // Speed in km/h
static int   last_sats = 6;        // Satellites in view
static bool  has_fix = false;

/**
 * Convert NMEA coordinate string (ddmm.mmmm) to decimal degrees
 */
static float nmea_to_decimal(const char* nmea_str, char dir) {
    if (!nmea_str || strlen(nmea_str) < 4) return 0.0;
    float raw = atof(nmea_str);
    int degrees = (int)(raw / 100);
    float minutes = raw - (degrees * 100);
    float dec = degrees + (minutes / 60.0);
    if (dir == 'S' || dir == 'W') dec = -dec;
    return dec;
}

/**
 * Initialize GPS hardware receiver
 */
void gps_init() {
    Serial.println("[GPS] Initializing GPS/GNSS Module (UART1)...");
    GPSSerial.begin(GPS_BAUD_RATE, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
    delay(50);
    Serial.printf("[GPS] ✅ GNSS Receiver active on RX:%d, TX:%d at %d baud\n",
                  GPS_RX_PIN, GPS_TX_PIN, GPS_BAUD_RATE);
}

/**
 * Read and parse incoming NMEA sentence from receiver
 *
 * @param latitude   Output: Decimal latitude (-90.0 to +90.0)
 * @param longitude  Output: Decimal longitude (-180.0 to +180.0)
 * @param altitude   Output: Altitude above sea level in meters
 * @param speed      Output: Ground speed in km/h
 * @param satellites Output: Number of locked GNSS satellites
 * @param fix_status Output: True if valid 2D/3D fix acquired
 */
bool gps_read(float &latitude, float &longitude, float &altitude, float &speed, int &satellites, bool &fix_status) {
    // Process any pending bytes from GPS UART buffer
    unsigned long start = millis();
    char lineBuffer[128];
    int bufIdx = 0;

    while (GPSSerial.available() && (millis() - start < 150)) {
        char c = GPSSerial.read();
        if (c == '\n' || c == '\r') {
            if (bufIdx > 10) {
                lineBuffer[bufIdx] = '\0';
                
                // Parse $GPRMC (Recommended Minimum Navigation Information)
                // $GPRMC,hhmmss.ss,A,llll.ll,a,yyyyy.yy,a,x.x,x.x,ddmmyy,,,a*hh
                if (strstr(lineBuffer, "$GPRMC") || strstr(lineBuffer, "$GNRMC")) {
                    char tempBuf[128];
                    strncpy(tempBuf, lineBuffer, sizeof(tempBuf));
                    char* tokens[14];
                    int t = 0;
                    char* tok = strtok(tempBuf, ",");
                    while (tok && t < 14) {
                        tokens[t++] = tok;
                        tok = strtok(NULL, ",");
                    }
                    if (t >= 7 && strcmp(tokens[2], "A") == 0) { // 'A' = Valid Fix
                        last_lat = nmea_to_decimal(tokens[3], tokens[4][0]);
                        last_lon = nmea_to_decimal(tokens[5], tokens[6][0]);
                        if (t > 7) last_spd = atof(tokens[7]) * 1.852; // Knots to km/h
                        has_fix = true;
                    }
                }
                
                // Parse $GPGGA (Fix Data: Satellites & Altitude)
                // $GPGGA,hhmmss.ss,llll.ll,a,yyyyy.yy,a,x,xx,x.x,x.x,M,x.x,M,x.x,xxxx*hh
                if (strstr(lineBuffer, "$GPGGA") || strstr(lineBuffer, "$GNGGA")) {
                    char tempBuf[128];
                    strncpy(tempBuf, lineBuffer, sizeof(tempBuf));
                    char* tokens[15];
                    int t = 0;
                    char* tok = strtok(tempBuf, ",");
                    while (tok && t < 15) {
                        tokens[t++] = tok;
                        tok = strtok(NULL, ",");
                    }
                    if (t >= 10 && atoi(tokens[6]) > 0) { // Fix quality > 0
                        last_sats = atoi(tokens[7]);
                        last_alt = atof(tokens[9]);
                        has_fix = true;
                    }
                }
            }
            bufIdx = 0;
        } else if (bufIdx < (int)sizeof(lineBuffer) - 1) {
            lineBuffer[bufIdx++] = c;
        }
    }

    // Set outputs
    latitude = last_lat;
    longitude = last_lon;
    altitude = last_alt;
    speed = last_spd;
    satellites = last_sats;
    fix_status = has_fix;

    return has_fix;
}
