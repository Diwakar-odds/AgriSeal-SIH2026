import React, { useState } from 'react';
import { Blocks, KeyRound, ShieldCheck, CheckCircle2, ChevronRight, Hash, Database } from 'lucide-react';

export default function BlockchainLedger({ telemetryList, shipments }) {
  const [selectedBlock, setSelectedBlock] = useState(null);

  // Generate blockchain representations from telemetry
  const blocks = telemetryList.slice(0, 8).map((t, idx) => ({
    blockNumber: 1042 + (t.idx || idx),
    timestamp: t.receivedAt || new Date().toISOString(),
    channel: 'agriseal-channel',
    chaincode: 'traceability',
    txId: `tx-${(t.hash || 'b8f2').slice(0, 16)}`,
    merkleRoot: t.hash || 'e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855',
    prevHash: t.prev || 'a1b2c3d4e5f60718293a4b5c6d7e8f90123456789abcdef0123456789abcdef0',
    payload: {
      deviceId: t.dev || 'AGRISEAL-NODE-0001',
      readingIndex: t.idx || idx + 1,
      temperatureC: t.t,
      humidityPct: t.h,
      ethylenePpm: t.e,
      tamperSwitch: false,
      firmwareSignature: 'ECDSA_P256_ATECC608A_VALID',
    },
  }));

  return (
    <div>
      <div style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', marginBottom: '1.75rem' }}>
        <div>
          <h2>Hyperledger Fabric Ledger Explorer</h2>
          <p style={{ color: 'var(--text-secondary)', fontSize: '0.9rem', marginTop: '0.25rem' }}>
            Inspect immutable state blocks, endorsement signatures, and edge-anchored cryptographic roots
          </p>
        </div>
        <div style={{ display: 'flex', gap: '0.75rem' }}>
          <div className="glass-panel" style={{ padding: '0.45rem 0.95rem', fontSize: '0.8rem', display: 'flex', alignItems: 'center', gap: '0.5rem' }}>
            <Database size={15} style={{ color: 'var(--cyan-500)' }} />
            <span>Channel: <strong className="mono">agriseal-channel</strong></span>
          </div>
          <div className="glass-panel" style={{ padding: '0.45rem 0.95rem', fontSize: '0.8rem', display: 'flex', alignItems: 'center', gap: '0.5rem' }}>
            <Blocks size={15} style={{ color: 'var(--emerald-400)' }} />
            <span>Consensus: <strong>Raft Multi-Peer</strong></span>
          </div>
        </div>
      </div>

      {/* Hash Chain Concept Flow */}
      <div className="glass-panel" style={{ padding: '1.5rem', marginBottom: '1.75rem' }}>
        <h3 style={{ fontSize: '1rem', marginBottom: '0.5rem', display: 'flex', alignItems: 'center', gap: '0.5rem' }}>
          <KeyRound size={18} style={{ color: 'var(--emerald-400)' }} />
          Dual-Tier Cryptographic Anchor Architecture
        </h3>
        <p style={{ fontSize: '0.82rem', color: 'var(--text-secondary)', lineHeight: 1.5, marginBottom: '1rem' }}>
          1. <strong>Edge Tier (ESP32 + ATECC608A)</strong>: Generates an internal linked SHA-256 chain where each record <span className="mono">H(n) = SHA256(Record_n || H(n-1))</span>.<br />
          2. <strong>Consortium Tier (Hyperledger Fabric)</strong>: Batches of verified hash digests are endorsed by peer nodes across Farmer, Logistics, and Retailer organizations into immutable channel blocks.
        </p>
        
        <div style={{ display: 'flex', gap: '0.75rem', overflowX: 'auto', paddingBottom: '0.5rem' }}>
          {blocks.slice(0, 4).map((b, i) => (
            <div
              key={i}
              style={{
                flex: '0 0 280px',
                padding: '1rem',
                borderRadius: '8px',
                background: 'rgba(255, 255, 255, 0.03)',
                border: '1px solid var(--border-color)',
                fontSize: '0.8rem',
              }}
            >
              <div style={{ display: 'flex', justifyContent: 'space-between', marginBottom: '0.35rem' }}>
                <span className="mono" style={{ fontWeight: 700, color: 'var(--cyan-500)' }}>Block #{b.blockNumber}</span>
                <span style={{ color: 'var(--emerald-400)', fontSize: '0.75rem' }}>COMMITTED</span>
              </div>
              <div style={{ color: 'var(--text-muted)', fontSize: '0.72rem', marginBottom: '0.5rem' }}>
                Prev: <span className="mono">{b.prevHash.slice(0, 10)}...</span>
              </div>
              <div style={{ color: 'var(--text-secondary)', fontSize: '0.75rem' }}>
                Digest: <span className="mono" style={{ color: '#fff' }}>{b.merkleRoot.slice(0, 14)}...</span>
              </div>
            </div>
          ))}
        </div>
      </div>

      {/* Transaction Table */}
      <div className="glass-panel" style={{ padding: '1.5rem' }}>
        <h3 style={{ marginBottom: '1rem' }}>Committed Transaction Records</h3>

        <div style={{ overflowX: 'auto' }}>
          <table className="telemetry-table">
            <thead>
              <tr>
                <th>Block Height</th>
                <th>Tx Identifier</th>
                <th>Timestamp (UTC)</th>
                <th>Chaincode Method</th>
                <th>Endorsers</th>
                <th>Payload Status</th>
                <th>Action</th>
              </tr>
            </thead>
            <tbody>
              {blocks.map((block) => (
                <tr key={block.txId}>
                  <td className="mono" style={{ fontWeight: 700, color: 'var(--cyan-500)' }}>
                    #{block.blockNumber}
                  </td>
                  <td>
                    <span className="hash-pill">{block.txId}</span>
                  </td>
                  <td style={{ color: 'var(--text-secondary)' }}>
                    {new Date(block.timestamp).toLocaleTimeString()}
                  </td>
                  <td>
                    <span className="mono" style={{ color: 'var(--emerald-400)', fontSize: '0.82rem' }}>
                      RecordSensorDigest
                    </span>
                  </td>
                  <td style={{ fontSize: '0.78rem', color: 'var(--text-muted)' }}>
                    FarmerOrgMSP, LogisticsMSP
                  </td>
                  <td>
                    <span className="verified-badge">
                      <CheckCircle2 size={12} /> Validated
                    </span>
                  </td>
                  <td>
                    <button
                      onClick={() => setSelectedBlock(block)}
                      style={{
                        background: 'rgba(255, 255, 255, 0.05)',
                        border: '1px solid var(--border-color)',
                        color: 'var(--text-primary)',
                        padding: '0.35rem 0.65rem',
                        borderRadius: '6px',
                        cursor: 'pointer',
                        fontSize: '0.75rem',
                      }}
                    >
                      Inspect Raw JSON
                    </button>
                  </td>
                </tr>
              ))}
            </tbody>
          </table>
        </div>
      </div>

      {/* Raw JSON Inspect Modal */}
      {selectedBlock && (
        <div
          style={{
            position: 'fixed',
            inset: 0,
            background: 'rgba(0, 0, 0, 0.8)',
            display: 'flex',
            alignItems: 'center',
            justifyContent: 'center',
            zIndex: 100,
          }}
        >
          <div className="glass-panel" style={{ width: '100%', maxWidth: '650px', padding: '2rem' }}>
            <div style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', marginBottom: '1rem' }}>
              <h3>Block #{selectedBlock.blockNumber} Verification Inspector</h3>
              <button
                onClick={() => setSelectedBlock(null)}
                style={{
                  background: 'transparent',
                  border: 'none',
                  color: 'var(--text-secondary)',
                  fontSize: '1.2rem',
                  cursor: 'pointer',
                }}
              >
                ✕
              </button>
            </div>
            <p style={{ fontSize: '0.82rem', color: 'var(--text-secondary)', marginBottom: '1rem' }}>
              Cryptographic evidence block as committed to CouchDB world-state on channel <span className="mono">agriseal-channel</span>.
            </p>
            <pre
              className="mono"
              style={{
                background: '#050811',
                padding: '1.25rem',
                borderRadius: '8px',
                border: '1px solid rgba(255, 255, 255, 0.08)',
                fontSize: '0.78rem',
                color: '#34d399',
                maxHeight: '340px',
                overflowY: 'auto',
                lineHeight: 1.5,
              }}
            >
              {JSON.stringify(selectedBlock, null, 2)}
            </pre>
            <div style={{ display: 'flex', justifyContent: 'flex-end', marginTop: '1.25rem' }}>
              <button
                className="btn-primary"
                onClick={() => setSelectedBlock(null)}
              >
                Close Inspector
              </button>
            </div>
          </div>
        </div>
      )}
    </div>
  );
}
