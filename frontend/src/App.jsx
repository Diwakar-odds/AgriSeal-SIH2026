import React, { useState, useEffect } from 'react';
import { io } from 'socket.io-client';
import {
  Activity,
  Layers,
  ShieldCheck,
  Radio,
  Boxes,
  Cpu,
  RefreshCw,
} from 'lucide-react';
import Dashboard from './pages/Dashboard';
import ShipmentTracker from './pages/ShipmentTracker';
import BlockchainLedger from './pages/BlockchainLedger';

export default function App() {
  const [activeTab, setActiveTab] = useState('dashboard');
  const [socket, setSocket] = useState(null);
  const [connected, setConnected] = useState(false);

  // Application Data States
  const [telemetryList, setTelemetryList] = useState([
    {
      idx: 18,
      dev: 'AGRISEAL-NODE-0001',
      t: 4.1,
      h: 88.5,
      e: 14.2,
      lat: 31.0542,
      lon: 77.1950,
      spd: 46.5,
      sats: 8,
      soc: 92,
      receivedAt: new Date().toISOString(),
      verified: true,
      hash: '3f8a42b109cde871409ab126c8104812a0f81746201849182740128491829104',
      prev: '9a7c31f408bdc652308fa015b70937019f706351097380716390173801918093',
    },
    {
      idx: 17,
      dev: 'AGRISEAL-NODE-0001',
      t: 4.0,
      h: 88.2,
      e: 13.9,
      lat: 31.0780,
      lon: 77.1820,
      spd: 44.0,
      sats: 8,
      soc: 92,
      receivedAt: new Date(Date.now() - 60000).toISOString(),
      verified: true,
      hash: '9a7c31f408bdc652308fa015b70937019f706351097380716390173801918093',
      prev: '5e4b20a307acb541207ea014a60826908e605240986270605280162701807082',
    },
    {
      idx: 16,
      dev: 'AGRISEAL-NODE-0001',
      t: 4.2,
      h: 87.9,
      e: 13.5,
      lat: 31.1048,
      lon: 77.1734,
      spd: 40.2,
      sats: 7,
      soc: 93,
      receivedAt: new Date(Date.now() - 120000).toISOString(),
      verified: true,
      hash: '5e4b20a307acb541207ea014a60826908e605240986270605280162701807082',
      prev: '1d3a19f2069ba430106d9013950715807d504130875160504170151601706071',
    },
  ]);

  const [shipments, setShipments] = useState([
    {
      id: 'SHIP-2026-0001',
      produceType: 'Shimla Royal Delicious Apples (Cold Stored)',
      quantityKg: 3200,
      origin: 'Shimla Cold Storage Cluster, HP',
      destination: 'Azadpur Mandi, New Delhi',
      currentCustodian: 'Himachal Fresh Transit Corp',
      deviceId: 'AGRISEAL-NODE-0001',
      status: 'IN_TRANSIT',
      totalDigests: 18,
      hasBreach: false,
    },
    {
      id: 'SHIP-2026-0002',
      produceType: 'Nashik Export Table Grapes (Thomson)',
      quantityKg: 5000,
      origin: 'Nashik Agro Terminal, MH',
      destination: 'JNPT Port Container Terminal, Mumbai',
      currentCustodian: 'Sahyadri Agro Transporters',
      deviceId: 'AGRISEAL-NODE-0002',
      status: 'IN_TRANSIT',
      totalDigests: 8,
      hasBreach: false,
    },
  ]);

  const [alerts, setAlerts] = useState([]);
  const [devices, setDevices] = useState([
    {
      id: 'AGRISEAL-NODE-0001',
      name: 'Shimla Transit Sensor 1',
      assignedShipment: 'SHIP-2026-0001',
      batterySoc: 92,
      status: 'ONLINE',
    },
  ]);

  // Connect to Backend WebSocket
  useEffect(() => {
    const s = io('http://localhost:5000', {
      transports: ['websocket', 'polling'],
      reconnectionAttempts: 5,
    });

    s.on('connect', () => {
      setConnected(true);
      console.log('Connected to AgriSeal backend gateway');
    });

    s.on('disconnect', () => {
      setConnected(false);
    });

    s.on('telemetry', (reading) => {
      setTelemetryList((prev) => [reading, ...prev.slice(0, 49)]);
    });

    setSocket(s);

    // Initial API Fetch
    fetchInitialData();

    return () => s.disconnect();
  }, []);

  const fetchInitialData = async () => {
    try {
      const shipRes = await fetch('/api/shipments');
      if (shipRes.ok) {
        const data = await shipRes.json();
        if (data.length > 0) setShipments(data);
      }

      const alertRes = await fetch('/api/alerts');
      if (alertRes.ok) {
        const data = await alertRes.json();
        setAlerts(data);
      }

      const devRes = await fetch('/api/devices');
      if (devRes.ok) {
        const data = await devRes.json();
        if (data.length > 0) setDevices(data);
      }
    } catch (e) {
      console.log('Using simulated offline cache:', e.message);
    }
  };

  const handleTransferCustody = async (shipmentId, newCustodian, location, notes) => {
    try {
      const res = await fetch(`/api/shipments/${shipmentId}/transfer`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ newCustodian, location, notes }),
      });
      if (res.ok) {
        fetchInitialData();
      }
    } catch (e) {
      console.error('Transfer failed', e);
    }
    // Optimistic UI update
    setShipments((prev) =>
      prev.map((s) => (s.id === shipmentId ? { ...s, currentCustodian: newCustodian } : s))
    );
  };

  return (
    <div style={{ minHeight: '100vh', display: 'flex', flexDirection: 'column' }}>
      {/* Top Header */}
      <header className="app-header">
        <div className="logo-group">
          <div className="logo-badge">
            <Radio size={22} color="#fff" />
          </div>
          <div>
            <div className="logo-text">AgriSeal</div>
            <div className="tagline">IoT Cold-Chain • Hyperledger Fabric</div>
          </div>
        </div>

        {/* Navigation Tabs */}
        <nav className="nav-tabs">
          <button
            className={`nav-tab-btn ${activeTab === 'dashboard' ? 'active' : ''}`}
            onClick={() => setActiveTab('dashboard')}
          >
            <Activity size={16} /> Telemetry Monitor
          </button>
          <button
            className={`nav-tab-btn ${activeTab === 'shipments' ? 'active' : ''}`}
            onClick={() => setActiveTab('shipments')}
          >
            <Boxes size={16} /> Consignments & Custody
          </button>
          <button
            className={`nav-tab-btn ${activeTab === 'blockchain' ? 'active' : ''}`}
            onClick={() => setActiveTab('blockchain')}
          >
            <Layers size={16} /> Blockchain Explorer
          </button>
        </nav>

        {/* Node Live Indicator */}
        <div style={{ display: 'flex', alignItems: 'center', gap: '1rem' }}>
          <div className="node-status-badge">
            <span className="pulse-dot" />
            <span>NODE TELEMETRY ACTIVE</span>
          </div>
        </div>
      </header>

      {/* Main Content Area */}
      <main className="container" style={{ flex: 1 }}>
        {activeTab === 'dashboard' && (
          <Dashboard
            socket={socket}
            telemetryList={telemetryList}
            alerts={alerts}
            devices={devices}
          />
        )}

        {activeTab === 'shipments' && (
          <ShipmentTracker
            shipments={shipments}
            onTransferCustody={handleTransferCustody}
          />
        )}

        {activeTab === 'blockchain' && (
          <BlockchainLedger
            telemetryList={telemetryList}
            shipments={shipments}
          />
        )}
      </main>

      {/* Footer */}
      <footer
        style={{
          borderTop: '1px solid var(--border-color)',
          padding: '1.25rem 2rem',
          textAlign: 'center',
          fontSize: '0.8rem',
          color: 'var(--text-muted)',
          background: 'rgba(10, 14, 23, 0.95)',
        }}
      >
        <span>
          Smart India Hackathon 2026 • Team Arishem (ID: 158445) • Problem Statement ID: 26232
        </span>
        <span style={{ margin: '0 0.75rem' }}>•</span>
        <span style={{ color: 'var(--emerald-400)' }}>
          Low-Cost IoT Blockchain Nodes for Farm-to-Fork Traceability
        </span>
      </footer>
    </div>
  );
}
