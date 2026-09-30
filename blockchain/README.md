# AgriSeal Blockchain Infrastructure

Enterprise-grade Distributed Ledger Infrastructure for Agricultural Cold-Chain Traceability built on **Hyperledger Fabric v2.5**.

## Overview

AgriSeal leverages a permissioned blockchain consortium comprising three core organization types:
1. **Farmer Organizations (FPOs / Clusters)**: Produce initial asset minting and packing registration.
2. **Logistics & 3PL Fleet Operators**: Record dynamic in-transit environmental digests anchored by IoT nodes.
3. **Retailers & Mandi Clusters**: Verify tamper-proof historical logs and execute transparent handover settlement.

```
                    +---------------------------+
                    |    Orderer (Raft Node)    |
                    |    orderer.agriseal.in    |
                    +-------------+-------------+
                                  |
              +-------------------+-------------------+
              |                   |                   |
      +-------v-------+   +-------v-------+   +-------v-------+
      |  Farmer Org   |   | Logistics Org |   | Retailer Org  |
      | Peer0 (Couch) |   | Peer0 (Couch) |   | Peer0 (Couch) |
      +---------------+   +---------------+   +---------------+
              \                   |                  /
               \                  |                 /
                +-----------------v----------------+
                |    Channel: `agriseal-channel`   |
                |  Chaincode: `traceability` (Go)  |
                +----------------------------------+
```

## Directory Structure

```
blockchain/
├── chaincode/
│   └── traceability/
│       ├── go.mod                # Go module dependencies
│       ├── traceability.go       # Core smart contract business logic
│       └── README.md             # Chaincode API documentation
├── network/
│   ├── docker-compose.yml        # Multi-peer + Orderer + CouchDB composition
│   ├── crypto-config.yaml        # Cryptographic identities & MSP topology
│   └── configtx.yaml             # Genesis block & channel profile definitions
└── README.md                     # This deployment guide
```

## Quick Start (Deploying Test Network)

### Prerequisites
- Docker Engine 24.0+ & Docker Compose v2.0+
- Hyperledger Fabric Binaries (`cryptogen`, `configtxgen`, `peer`) v2.5+
- Go 1.20+

### Step 1: Generate Crypto Material
```bash
cd blockchain/network
cryptogen generate --config=./crypto-config.yaml --output="crypto-config"
```

### Step 2: Generate Channel Artifacts
```bash
mkdir -p channel-artifacts
# Genesis block
configtxgen -profile TwoOrgsOrdererGenesis -channelID system-channel -outputBlock ./channel-artifacts/genesis.block
# Channel transaction
configtxgen -profile AgriSealChannel -outputCreateChannelTx ./channel-artifacts/agriseal-channel.tx -channelID agriseal-channel
```

### Step 3: Launch Containers
```bash
docker compose up -d
```

### Step 4: Package and Deploy Chaincode
```bash
# Package chaincode
peer lifecycle chaincode package traceability.tar.gz \
  --path ../chaincode/traceability/ \
  --lang golang \
  --label traceability_1.0

# Install, approve, and commit on channel agriseal-channel
peer lifecycle chaincode install traceability.tar.gz
```
