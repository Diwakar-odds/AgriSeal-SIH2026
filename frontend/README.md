# AgriSeal Frontend Dashboard

A modern, responsive React + Vite web dashboard for real-time cold-chain monitoring, consignment lifecycle tracking, and Hyperledger Fabric ledger inspection.

## Features

- **Live Telemetry Monitoring**: Real-time streaming of cargo temperature (°C), relative humidity (% RH), ethylene gas (ppm), and IoT node battery state-of-charge (SoC).
- **Edge Hash Chain Audit**: Sequential block table showing continuous SHA-256 digests validated at the edge.
- **Consignment Tracking & Custody Handover**: Interactive interface to inspect agricultural shipments, provenance timeline, and record custodial handovers directly to the blockchain.
- **Hyperledger Fabric Block Explorer**: Detailed ledger view showing block heights, transaction identifiers, chaincode invocations, and raw cryptographic JSON payloads.
- **Glassmorphic Industrial Aesthetic**: Curated dark design system with glowing status badges, pulse animations, and micro-interactions.

## Setup & Running

```bash
cd frontend
npm install
npm run dev
```

The application will launch on `http://localhost:5173`.
