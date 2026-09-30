# 📄 Component Datasheets

## Quick Reference Links

| Component | Datasheet Link |
|---|---|
| ESP32-WROOM-32D | [Espressif Official](https://www.espressif.com/sites/default/files/documentation/esp32-wroom-32d_esp32-wroom-32u_datasheet_en.pdf) |
| DHT22 / AM2302 | [Aosong Datasheet](https://www.sparkfun.com/datasheets/Sensors/Temperature/DHT22.pdf) |
| MQ135 Gas Sensor | [Hanwei Electronics](https://www.olimex.com/Products/Components/Sensors/SNS-MQ135/resources/SNS-MQ135.pdf) |
| SIM7600E-H | [SIMCom Datasheet](https://simcom.ee/documents/SIM7600E-H/SIM7600E-H_Hardware_Design_V1.02.pdf) |
| W25Q128JVSIQ | [Winbond Datasheet](https://www.winbond.com/resource-files/w25q128jv%20revf%2003272018%20plus.pdf) |
| FM24C256 FRAM | [Cypress/Infineon](https://www.infineon.com/dgdl/Infineon-FM24C256-DataSheet-v08_00-EN.pdf) |
| ATECC608A | [Microchip Datasheet](https://ww1.microchip.com/downloads/en/DeviceDoc/ATECC608A-CryptoAuthentication-Device-Summary-Data-Sheet-DS40001977B.pdf) |
| TP4056 | [NanJing Top Power](https://dlnmh9ip6v2uc.cloudfront.net/datasheets/Prototyping/TP4056.pdf) |
| AMS1117-3.3 | [Advanced Monolithic Systems](http://www.advanced-monolithic.com/pdf/ds1117.pdf) |
| NCR18650B Battery | [Panasonic Datasheet](https://www.batteryspace.com/prod-specs/NCR18650B.pdf) |

## Key Specifications Summary

### ESP32-WROOM-32D
- **CPU:** Dual-core Xtensa LX6, up to 240 MHz
- **RAM:** 520 KB SRAM
- **Flash:** 4 MB (onboard)
- **WiFi:** 802.11 b/g/n, 2.4 GHz
- **Bluetooth:** v4.2 BR/EDR + BLE
- **GPIOs:** 34 programmable
- **ADC:** 12-bit, up to 18 channels
- **Crypto:** Hardware AES, SHA-2, RSA, ECC
- **Operating Temp:** -40°C to +85°C
- **Deep Sleep:** 10 μA (ULP co-processor active)

### ATECC608A Secure Element
- **Crypto Operations:** ECDSA (P-256), ECDH, AES-128
- **Key Storage:** 16 key slots (protected)
- **Random Number Generator:** FIPS 800-90A/B/C compliant
- **Interface:** I²C (up to 1 MHz)
- **Tamper Resistance:** Active metal shield, voltage glitch detection
- **Supply:** 2.0V – 5.5V
