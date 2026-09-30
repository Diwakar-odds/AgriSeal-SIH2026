# 🏗️ Enclosure Design — AgriSeal Node

## Specifications

| Parameter | Value |
|---|---|
| **Material** | Polycarbonate (UV-stabilized) |
| **Dimensions** | 100mm × 68mm × 50mm (external) |
| **IP Rating** | IP67 (dustproof + waterproof at 1m/30min) |
| **Sealing** | Silicone gasket + stainless steel screws |
| **Cable Entry** | PG7 cable gland (×2) — sensor probe + solar cable |
| **Mounting** | DIN rail clip + 2× M4 mounting holes |
| **Color** | Light grey (RAL 7035) — reduces solar heat absorption |
| **Operating Temp** | -20°C to +60°C |

## Design Features

### Moisture Protection
- **Gasket seal:** Continuous silicone gasket in lid groove
- **Conformal coating:** Humiseal 1B73 acrylic coating on PCB
- **Cable glands:** IP68-rated PG7 compression glands
- **Drain hole:** 2mm weep hole with Gore-Tex vent (pressure equalization)

### Vibration Resistance
- **PCB mounting:** 4× M3 standoffs with nylon washers (vibration damping)
- **Battery holder:** Spring-loaded 18650 cradle with foam padding
- **Sensor headers:** Locking JST-XH connectors (anti-vibration)

### Thermal Management
- **Light color** reduces solar heat gain
- **Ventilation:** Gore-Tex membrane allows air exchange without moisture ingress
- **Heat sink:** No active cooling needed — ESP32 max dissipation <0.5W

### User-Facing Features
- **Status LED:** Visible through transparent window in lid
- **Reset button:** Recessed button accessible with paperclip
- **QR code label:** Device ID + setup URL printed on enclosure
- **Solar panel mount:** Top-surface mounting bracket with silicone adhesive
- **Tamper switch:** Micro switch detects lid removal → triggers alert

## Assembly Notes

1. Mount PCB on standoffs (4× M3 × 6mm)
2. Route sensor cable through bottom PG7 gland
3. Route solar cable through side PG7 gland
4. Seat battery in 18650 cradle
5. Apply conformal coating (if not pre-applied)
6. Place silicone gasket in lid groove
7. Secure lid with 4× stainless steel screws (torque: 0.5 Nm)
8. Tighten cable glands to IP68 spec
9. Affix QR code label
10. Perform visual inspection + power-on test
