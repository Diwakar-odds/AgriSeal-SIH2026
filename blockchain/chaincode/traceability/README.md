# AgriSeal Farm-to-Fork Traceability Chaincode

This directory contains the Hyperledger Fabric Go Smart Contract for the AgriSeal supply chain platform.

## Architecture

The smart contract manages three key entities on the distributed ledger:

1. **Shipment Asset**: Root state tracking produce metadata, source farmer, logistics partner, current custodian, assigned AgriSeal IoT node, and overall compliance status (`CREATED`, `IN_TRANSIT`, `DELIVERED`, `BREACHED`).
2. **Sensor Digests (`DIGEST_<shipmentId>_<blockIndex>`)**: Periodic cryptographic hash anchors and summarized environmental conditions (average temperature, maximum temperature, humidity, ethylene ppm, and enclosure tamper status).
3. **Custodial Transfers (`TRANSFER_<shipmentId>_<txId>`)**: Verified provenance milestones recording timestamped custody handoffs across stakeholders.
4. **Breach Records (`BREACH_<shipmentId>_<txId>`)**: Immutable audit events generated automatically when sensor parameters violate cold-chain constraints or physical enclosure micro-switches trip.

## API Reference

| Method | Parameters | Description |
|---|---|---|
| `InitLedger` | None | Bootstraps sample shipment for network testing |
| `CreateShipment` | `id, produceType, qtyKg, origin, dest, custodian, deviceId` | Registers new shipment on-chain |
| `RecordSensorDigest` | `shipmentId, blockIndex, blockHash, avgTemp, maxTemp, minTemp, avgHum, avgEth, tamper, lat, lon, spd` | Commits cryptographic reading batch with GPS geo-coordinates |
| `TransferOwnership` | `shipmentId, newCustodian, location, notes` | Transfers custody from current holder to new stakeholder |
| `FlagBreach` | `shipmentId, breachType, recordedVal, thresholdVal, severity` | Records temperature excursion or tamper |
| `GetShipment` | `id` | Queries current shipment state |
| `GetShipmentHistory` | `shipmentId` | Returns complete immutable modification history for an asset |
| `VerifyCompliance` | `shipmentId` | Evaluates if cold-chain constraints were respected throughout transit |

## Local Unit Testing & Verification

Ensure Go 1.20+ is installed:

```bash
cd blockchain/chaincode/traceability
go mod tidy
go test -v ./...
```
