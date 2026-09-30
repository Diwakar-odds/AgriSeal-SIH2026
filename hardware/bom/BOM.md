# 📦 Bill of Materials — AgriSeal IoT Node v1.0

## Summary

| Metric | Value |
|---|---|
| **Total Components** | 17 line items |
| **Prototype Cost** | ₹2,580 per node |
| **At Scale (100+ units)** | ~₹1,800 per node |
| **PCB Dimensions** | 60mm × 40mm |
| **Assembly Time** | ~45 minutes (manual) |

---

## Detailed BOM

| # | Component | Part Number | Description | Package | Qty | Unit Cost (₹) | Total (₹) | Supplier |
|---|---|---|---|---|---|---|---|---|
| 1 | Microcontroller | ESP32-WROOM-32D | Dual-core 240MHz, 4MB Flash, WiFi+BLE | Module | 1 | 280 | 280 | Espressif / Robu.in |
| 2 | Temp/Humidity Sensor | DHT22 (AM2302) | -40~80°C (±0.5°C), 0-100%RH (±2%) | 4-pin SIP | 1 | 180 | 180 | Aosong / Amazon.in |
| 3 | Ethylene Gas Sensor | MQ135 Module | C₂H₄ detection, 10-1000 ppm, analog out | Breakout | 1 | 300 | 300 | Winsen / Robu.in |
| 4 | Cellular Module | SIM7600E-H | 4G LTE Cat-4, UART | Module | 1 | 650 | 650 | SIMCom / AliExpress |
| 5 | GPS/GNSS Module | NEO-6M / ATGM336H | Multi-constellation GNSS, 2.5m CEP, UART | Breakout | 1 | 220 | 220 | u-blox / Robu.in |
| 6 | SPI Flash | W25Q128JVSIQ | 128Mbit (16MB) SPI NOR Flash | SOIC-8 | 1 | 60 | 60 | Winbond / LCSC |
| 7 | FRAM | FM24C256-G | 256Kbit (32KB) I²C, 10¹⁴ R/W cycles | SOIC-8 | 1 | 80 | 80 | Cypress / Mouser |
| 8 | Secure Element | ATECC608A-MAHDA | HW crypto, ECDSA P-256, I²C | SOIC-8 | 1 | 120 | 120 | Microchip / DigiKey |
| 9 | Solar Panel | 6V 1W Poly | 110×60mm polycrystalline | Panel | 1 | 160 | 160 | Generic / Amazon.in |
| 10 | Battery Charger | TP4056 Module | Li-Ion charger, USB-C, DW01 protection | Breakout | 1 | 30 | 30 | Generic / Robu.in |
| 11 | Battery | NCR18650B | 3.7V 3400mAh Li-Ion cell | 18650 | 1 | 120 | 120 | Panasonic / Amazon.in |
| 12 | Voltage Regulator | AMS1117-3.3 | 3.3V LDO, 1A output | SOT-223 | 1 | 10 | 10 | AMS / LCSC |
| 13 | PCB | Custom 2-Layer | FR-4, 1.6mm, HASL, 60×40mm | PCB | 1 | 50 | 50 | JLCPCB / PCBWay |
| 14 | Enclosure | IP67 Box | Polycarbonate, 100×68×50mm, cable gland | Box | 1 | 250 | 250 | Polycase / Amazon.in |
| 15 | Conformal Coating | Humiseal 1B73 | Acrylic conformal coating (per board) | Liquid | 1 | 20 | 20 | Humiseal / DigiKey |
| 16 | Passive Components | Assorted | 10kΩ, 4.7kΩ resistors, 100nF, 10μF caps, LEDs | SMD 0805 | Lot | 30 | 30 | Generic / LCSC |
| 17 | Connectors & Wiring | JST-XH, Antennas | 2.54mm headers, active GPS patch, LTE pigtail | Through-hole | Lot | 50 | 50 | Generic / Robu.in |
| | | | | | | **TOTAL** | **₹2,580** | |

---

## Cost Optimization at Scale

| Volume | Cost/Node | Savings | Notes |
|---|---|---|---|
| 1 (Prototype) | ₹2,380 | — | Individual component purchase |
| 10 | ₹2,100 | 12% | Bulk PCB + assembly |
| 100 | ₹1,800 | 24% | Volume pricing + JLCPCB assembly |
| 1000 | ₹1,400 | 41% | Direct from IC distributors |
| 10000+ (Custom SoC) | ₹800 | 66% | ASIC integration reduces BOM |

---

## Alternative Components

| Primary | Alternative | Trade-off |
|---|---|---|
| ESP32-WROOM-32D | STM32L476RG | Lower power but no built-in WiFi/BLE |
| SIM7600E (4G) | BC66 (NB-IoT) | Lower cost/power but lower bandwidth |
| MQ135 (Ethylene) | MiCS-5524 | Better selectivity but higher cost |
| DHT22 | SHT31 | Higher accuracy (±0.2°C) but 2× cost |
| W25Q128 (Flash) | AT25SF128A | Pin-compatible alternative |
