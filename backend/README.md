# AgriSeal Ingestion & Gateway Backend

Node.js + Express backend service integrating IoT edge devices (ESP32), MQTT message broker, WebSocket telemetry broadcast, cryptographic hash-chain verification, and Hyperledger Fabric blockchain anchoring.

## Features

- **MQTT Ingestion & Stream Processing**: Subscribes to node telemetry topics (`agriseal/+/data`), decrypts and verifies packets.
- **Cryptographic Hash-Chain Verification**: Recomputes SHA-256 block hashes and checks sequence continuity to prevent sensor spoofing or record dropping.
- **Blockchain Gateway**: Interacts with Hyperledger Fabric smart contract (`traceability.go`) to commit sensor batch digests and custody transfers.
- **WebSocket Server**: Streams live temperature, humidity, ethylene, battery level, and breach alerts to the frontend dashboard.
- **Zero-Dependency Mock Mode**: Out-of-the-box in-memory simulation engine for demonstrations without requiring Docker or Fabric network setup.

## Setup & Running

```bash
cd backend
npm install
npm run dev
```

Server will start on `http://localhost:5000`.

## API Endpoints

- `GET /api/health` — Service health check
- `GET /api/sensors/readings?deviceId=AGRISEAL-NODE-0001` — Historical sensor readings and hash chain status
- `POST /api/sensors/ingest` — HTTP fallback telemetry ingestion
- `GET /api/shipments` — List active agricultural consignments
- `GET /api/shipments/:id` — Details, digests, and audit trail of a shipment
- `POST /api/shipments` — Register new shipment
- `POST /api/shipments/:id/transfer` — Record custodial handover
- `GET /api/alerts` — Fetch tamper alerts and temperature breach excursions
- `GET /api/devices` — Status, battery SoC, and connectivity of registered IoT nodes
