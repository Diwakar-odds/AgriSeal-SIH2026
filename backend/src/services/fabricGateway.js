/**
 * Hyperledger Fabric Gateway Service
 * ==================================
 * Connects the Express backend to the Hyperledger Fabric network
 * using the official Fabric Network SDK. Includes a high-fidelity
 * mock simulation fallback for development and test environments.
 */

const { Gateway, Wallets } = require('fabric-network');
const path = require('path');
const fs = require('fs');

const USE_MOCK = process.env.USE_MOCK_FABRIC !== 'false';

// High-fidelity in-memory mock ledger for zero-dependency demos
const mockLedger = {
  shipments: new Map([
    [
      'SHIP-2026-0001',
      {
        id: 'SHIP-2026-0001',
        produceType: 'Shimla Royal Delicious Apples (Cold Stored)',
        quantityKg: 3200,
        origin: 'Shimla Mandi Cluster, HP',
        destination: 'Delhi NCR Cold Logistics Hub',
        currentCustodian: 'Himachal Fresh Transit Corp',
        deviceId: 'AGRISEAL-NODE-0001',
        status: 'IN_TRANSIT',
        createdAt: new Date(Date.now() - 3600000 * 18).toISOString(),
        updatedAt: new Date().toISOString(),
        totalDigests: 18,
        hasBreach: false,
      },
    ],
    [
      'SHIP-2026-0002',
      {
        id: 'SHIP-2026-0002',
        produceType: 'Nashik Export Table Grapes (Thomson)',
        quantityKg: 5000,
        origin: 'Nashik Agro Terminal, MH',
        destination: 'JNPT Port Container Terminal, Mumbai',
        currentCustodian: 'Sahyadri Agro Transporters',
        deviceId: 'AGRISEAL-NODE-0002',
        status: 'IN_TRANSIT',
        createdAt: new Date(Date.now() - 3600000 * 8).toISOString(),
        updatedAt: new Date().toISOString(),
        totalDigests: 8,
        hasBreach: false,
      },
    ],
  ]),
  digests: [],
  transfers: [],
  breaches: [],
  history: new Map(),
};

// Initialize history entries for sample shipments
mockLedger.history.set('SHIP-2026-0001', [
  {
    txId: 'tx-genesis-981247a8',
    timestamp: new Date(Date.now() - 3600000 * 18).toISOString(),
    isDelete: false,
    record: { ...mockLedger.shipments.get('SHIP-2026-0001') },
  },
]);

class FabricGatewayService {
  constructor() {
    this.gateway = null;
    this.network = null;
    this.contract = null;
  }

  async connect() {
    if (USE_MOCK) {
      console.log('[FABRIC] ℹ️ Running in Mock Ledger Mode (Zero-Docker required)');
      return;
    }

    try {
      const ccpPath = path.resolve(process.env.FABRIC_CONNECTION_PROFILE || './identity/connection-org1.json');
      const ccp = JSON.parse(fs.readFileSync(ccpPath, 'utf8'));

      const walletPath = path.resolve(process.env.FABRIC_WALLET_PATH || './identity/wallet');
      const wallet = await Wallets.newFileSystemWallet(walletPath);

      const identity = await wallet.get(process.env.FABRIC_USER || 'admin');
      if (!identity) {
        console.warn(`[FABRIC] ⚠️ User identity not found in wallet. Falling back to Mock.`);
        return;
      }

      this.gateway = new Gateway();
      await this.gateway.connect(ccp, {
        wallet,
        identity: process.env.FABRIC_USER || 'admin',
        discovery: { enabled: true, asLocalhost: true },
      });

      this.network = await this.gateway.getNetwork(process.env.FABRIC_CHANNEL || 'agriseal-channel');
      this.contract = this.network.getContract(process.env.FABRIC_CHAINCODE || 'traceability');
      console.log('[FABRIC] ✅ Connected to Hyperledger Fabric Network successfully');
    } catch (err) {
      console.warn('[FABRIC] ⚠️ Failed to connect to Fabric peer, using mock fallback:', err.message);
    }
  }

  async createShipment(data) {
    if (this.contract) {
      const res = await this.contract.submitTransaction(
        'CreateShipment',
        data.id,
        data.produceType,
        data.quantityKg.toString(),
        data.origin,
        data.destination,
        data.currentCustodian,
        data.deviceId
      );
      return JSON.parse(res.toString());
    }

    // Mock handler
    const now = new Date().toISOString();
    const shipment = {
      ...data,
      status: 'CREATED',
      createdAt: now,
      updatedAt: now,
      totalDigests: 0,
      hasBreach: false,
    };
    mockLedger.shipments.set(data.id, shipment);

    const history = mockLedger.history.get(data.id) || [];
    history.push({
      txId: `tx-${Math.random().toString(16).slice(2, 10)}`,
      timestamp: now,
      isDelete: false,
      record: shipment,
    });
    mockLedger.history.set(data.id, history);

    return shipment;
  }

  async recordSensorDigest(digestData) {
    if (this.contract) {
      const res = await this.contract.submitTransaction(
        'RecordSensorDigest',
        digestData.shipmentId,
        digestData.blockIndex.toString(),
        digestData.blockHash,
        digestData.avgTemp.toString(),
        digestData.maxTemp.toString(),
        digestData.minTemp.toString(),
        digestData.avgHum.toString(),
        digestData.avgEth.toString(),
        digestData.tamper.toString(),
        (digestData.latitude || 31.1048).toString(),
        (digestData.longitude || 77.1734).toString(),
        (digestData.speed || 0.0).toString()
      );
      return JSON.parse(res.toString());
    }

    // Mock handler
    const shipment = mockLedger.shipments.get(digestData.shipmentId);
    if (!shipment) throw new Error(`Shipment ${digestData.shipmentId} not found`);

    const now = new Date().toISOString();
    const txId = `tx-digest-${Math.random().toString(16).slice(2, 10)}`;

    const digestRecord = {
      ...digestData,
      timestamp: now,
      txId,
    };
    mockLedger.digests.push(digestRecord);

    shipment.totalDigests++;
    shipment.updatedAt = now;
    if (shipment.status === 'CREATED') shipment.status = 'IN_TRANSIT';

    if (digestData.tamper) {
      shipment.hasBreach = true;
      shipment.status = 'BREACHED';
      mockLedger.breaches.push({
        shipmentId: digestData.shipmentId,
        breachType: 'TAMPER_SWITCH',
        recordedValue: 1.0,
        thresholdValue: 0.0,
        severity: 'CRITICAL',
        timestamp: now,
        txId,
      });
    } else if (digestData.maxTemp > 12.0) {
      shipment.hasBreach = true;
      shipment.status = 'BREACHED';
      mockLedger.breaches.push({
        shipmentId: digestData.shipmentId,
        breachType: 'TEMP_EXCURSION_HIGH',
        recordedValue: digestData.maxTemp,
        thresholdValue: 12.0,
        severity: 'CRITICAL',
        timestamp: now,
        txId,
      });
    }

    return digestRecord;
  }

  async transferOwnership(shipmentId, newCustodian, location, notes) {
    if (this.contract) {
      const res = await this.contract.submitTransaction(
        'TransferOwnership',
        shipmentId,
        newCustodian,
        location,
        notes
      );
      return JSON.parse(res.toString());
    }

    const shipment = mockLedger.shipments.get(shipmentId);
    if (!shipment) throw new Error(`Shipment ${shipmentId} not found`);

    const now = new Date().toISOString();
    const txId = `tx-transfer-${Math.random().toString(16).slice(2, 10)}`;
    const transfer = {
      shipmentId,
      fromParty: shipment.currentCustodian,
      toParty: newCustodian,
      location,
      notes,
      timestamp: now,
      txId,
    };
    mockLedger.transfers.push(transfer);

    shipment.currentCustodian = newCustodian;
    shipment.updatedAt = now;
    if (notes && notes.toLowerCase().includes('delivered')) {
      shipment.status = 'DELIVERED';
    }

    return transfer;
  }

  async getShipment(id) {
    if (this.contract) {
      const res = await this.contract.evaluateTransaction('GetShipment', id);
      return JSON.parse(res.toString());
    }
    return mockLedger.shipments.get(id) || null;
  }

  async getAllShipments() {
    return Array.from(mockLedger.shipments.values());
  }

  async getShipmentHistory(id) {
    if (this.contract) {
      const res = await this.contract.evaluateTransaction('GetShipmentHistory', id);
      return JSON.parse(res.toString());
    }
    return mockLedger.history.get(id) || [];
  }

  async getShipmentDigests(shipmentId) {
    return mockLedger.digests.filter((d) => d.shipmentId === shipmentId);
  }

  async getBreaches(shipmentId) {
    if (shipmentId) {
      return mockLedger.breaches.filter((b) => b.shipmentId === shipmentId);
    }
    return mockLedger.breaches;
  }
}

const fabricService = new FabricGatewayService();
module.exports = fabricService;
