import React, { useState } from 'react';
import { Package, Truck, MapPin, ArrowRight, UserCheck, ShieldCheck, AlertCircle, PlusCircle } from 'lucide-react';

export default function ShipmentTracker({ shipments, onTransferCustody, onCreateShipment }) {
  const [selectedId, setSelectedId] = useState(shipments[0]?.id || 'SHIP-2026-0001');
  const [showHandoffModal, setShowHandoffModal] = useState(false);
  const [newCustodian, setNewCustodian] = useState('');
  const [location, setLocation] = useState('');
  const [notes, setNotes] = useState('');

  const currentShipment = shipments.find((s) => s.id === selectedId) || shipments[0];

  const handleHandoff = (e) => {
    e.preventDefault();
    if (!newCustodian || !location) return;
    onTransferCustody(selectedId, newCustodian, location, notes);
    setShowHandoffModal(false);
    setNewCustodian('');
    setLocation('');
    setNotes('');
  };

  return (
    <div>
      <div style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', marginBottom: '1.75rem' }}>
        <div>
          <h2>Consignment Lifecycle & Custody Tracking</h2>
          <p style={{ color: 'var(--text-secondary)', fontSize: '0.9rem', marginTop: '0.25rem' }}>
            Auditable custody handovers and cold chain compliance certificates anchored on Hyperledger Fabric
          </p>
        </div>
      </div>

      <div style={{ display: 'grid', gridTemplateColumns: '1fr 2fr', gap: '1.5rem' }}>
        {/* Left: Consignment List */}
        <div className="glass-panel" style={{ padding: '1.25rem' }}>
          <h3 style={{ fontSize: '1rem', marginBottom: '1rem', color: 'var(--text-secondary)' }}>Active Consignments</h3>
          <div style={{ display: 'flex', flexDirection: 'column', gap: '0.75rem' }}>
            {shipments.map((shipment) => (
              <div
                key={shipment.id}
                onClick={() => setSelectedId(shipment.id)}
                style={{
                  padding: '1rem',
                  borderRadius: '10px',
                  cursor: 'pointer',
                  border: `1px solid ${selectedId === shipment.id ? 'var(--emerald-500)' : 'var(--border-color)'}`,
                  background: selectedId === shipment.id ? 'rgba(16, 185, 129, 0.08)' : 'rgba(255, 255, 255, 0.02)',
                  transition: 'all 0.2s ease',
                }}
              >
                <div style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', marginBottom: '0.4rem' }}>
                  <span className="mono" style={{ fontWeight: 700, color: 'var(--emerald-400)', fontSize: '0.85rem' }}>
                    {shipment.id}
                  </span>
                  <span
                    style={{
                      fontSize: '0.7rem',
                      fontWeight: 700,
                      padding: '0.15rem 0.5rem',
                      borderRadius: '4px',
                      background: shipment.status === 'BREACHED' ? 'rgba(244, 63, 94, 0.2)' : 'rgba(16, 185, 129, 0.2)',
                      color: shipment.status === 'BREACHED' ? 'var(--rose-500)' : 'var(--emerald-400)',
                    }}
                  >
                    {shipment.status}
                  </span>
                </div>
                <div style={{ fontWeight: 600, fontSize: '0.92rem', marginBottom: '0.25rem' }}>
                  {shipment.produceType}
                </div>
                <div style={{ fontSize: '0.78rem', color: 'var(--text-muted)' }}>
                  {shipment.quantityKg} kg • Custodian: {shipment.currentCustodian}
                </div>
              </div>
            ))}
          </div>
        </div>

        {/* Right: Selected Consignment Details & Provenance Trail */}
        {currentShipment && (
          <div className="glass-panel" style={{ padding: '1.75rem' }}>
            <div style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'flex-start', marginBottom: '1.5rem' }}>
              <div>
                <span className="mono" style={{ color: 'var(--emerald-400)', fontSize: '0.9rem', fontWeight: 700 }}>
                  {currentShipment.id}
                </span>
                <h3 style={{ fontSize: '1.4rem', marginTop: '0.25rem' }}>{currentShipment.produceType}</h3>
                <div style={{ display: 'flex', gap: '1rem', marginTop: '0.5rem', fontSize: '0.82rem', color: 'var(--text-secondary)' }}>
                  <span>Weight: <strong style={{ color: '#fff' }}>{currentShipment.quantityKg} kg</strong></span>
                  <span>Node: <strong className="mono" style={{ color: 'var(--cyan-500)' }}>{currentShipment.deviceId}</strong></span>
                  <span>Digests: <strong style={{ color: '#fff' }}>{currentShipment.totalDigests || 18} Anchored</strong></span>
                </div>
              </div>

              <button className="btn-primary" onClick={() => setShowHandoffModal(true)}>
                <UserCheck size={16} /> Record Custody Handoff
              </button>
            </div>

            {/* Route Summary */}
            <div
              style={{
                display: 'grid',
                gridTemplateColumns: '1fr auto 1fr',
                alignItems: 'center',
                gap: '1rem',
                padding: '1.25rem',
                borderRadius: '12px',
                background: 'rgba(255, 255, 255, 0.03)',
                border: '1px solid var(--border-color)',
                marginBottom: '2rem',
              }}
            >
              <div>
                <div style={{ fontSize: '0.75rem', textTransform: 'uppercase', color: 'var(--text-muted)', marginBottom: '0.25rem' }}>
                  Origin Mandi / Cluster
                </div>
                <div style={{ fontWeight: 600, fontSize: '0.95rem' }}>{currentShipment.origin}</div>
              </div>
              <div style={{ color: 'var(--emerald-400)', display: 'flex', flexDirection: 'column', alignItems: 'center' }}>
                <ArrowRight size={22} />
                <span style={{ fontSize: '0.7rem', color: 'var(--text-muted)' }}>Cold Transit</span>
              </div>
              <div>
                <div style={{ fontSize: '0.75rem', textTransform: 'uppercase', color: 'var(--text-muted)', marginBottom: '0.25rem' }}>
                  Destination Terminal
                </div>
                <div style={{ fontWeight: 600, fontSize: '0.95rem' }}>{currentShipment.destination}</div>
              </div>
            </div>

            {/* GPS Live Geolocation Banner */}
            <div
              style={{
                padding: '1rem 1.25rem',
                borderRadius: '10px',
                background: 'rgba(16, 185, 129, 0.06)',
                border: '1px solid rgba(16, 185, 129, 0.25)',
                marginBottom: '1.75rem',
                display: 'flex',
                alignItems: 'center',
                justifyContent: 'space-between',
              }}
            >
              <div style={{ display: 'flex', alignItems: 'center', gap: '0.85rem' }}>
                <div
                  style={{
                    width: '36px',
                    height: '36px',
                    borderRadius: '8px',
                    background: 'rgba(16, 185, 129, 0.15)',
                    display: 'flex',
                    alignItems: 'center',
                    justifyContent: 'center',
                  }}
                >
                  <MapPin size={20} style={{ color: 'var(--emerald-400)' }} />
                </div>
                <div>
                  <div style={{ fontWeight: 600, fontSize: '0.88rem', color: '#fff' }}>
                    Live GNSS Position: <span className="mono" style={{ color: 'var(--emerald-400)' }}>31.0542°N, 77.1950°E</span>
                  </div>
                  <div style={{ fontSize: '0.78rem', color: 'var(--text-secondary)' }}>
                    Solan Expressway Bypass • Velocity: 46.5 km/h • 8 GNSS Satellites Locked
                  </div>
                </div>
              </div>
              <div style={{ textAlign: 'right' }}>
                <span
                  style={{
                    fontSize: '0.72rem',
                    fontWeight: 700,
                    padding: '0.2rem 0.55rem',
                    borderRadius: '9999px',
                    background: 'rgba(6, 182, 212, 0.15)',
                    color: 'var(--cyan-500)',
                    border: '1px solid rgba(6, 182, 212, 0.3)',
                  }}
                >
                  CORRIDOR GEO-FENCE: SECURE
                </span>
                <div style={{ fontSize: '0.72rem', color: 'var(--text-muted)', marginTop: '3px' }}>
                  Deviation: 0.0 m (Within 50m tolerance)
                </div>
              </div>
            </div>

            {/* Custody Provenance Timeline */}
            <h4 style={{ marginBottom: '1.25rem', color: 'var(--text-secondary)' }}>Immutable Handover Trail</h4>

            <div style={{ paddingLeft: '0.5rem' }}>
              <div className="timeline-item">
                <div className="timeline-icon">
                  <Package size={16} style={{ color: 'var(--emerald-400)' }} />
                </div>
                <div className="timeline-content">
                  <div style={{ fontWeight: 600, fontSize: '0.92rem' }}>Harvest & Consignment Sealing</div>
                  <div style={{ fontSize: '0.8rem', color: 'var(--text-secondary)', marginTop: '2px' }}>
                    Sealed by: Shimla Farmer Producer Co-operative (FPO) • IoT Device Armed & Anchored
                  </div>
                  <div style={{ fontSize: '0.72rem', color: 'var(--text-muted)', marginTop: '4px' }}>
                    Block Hash: <span className="mono">0x4a8b...19e0</span> • Status: COMPLIANT
                  </div>
                </div>
              </div>

              <div className="timeline-item">
                <div className="timeline-icon">
                  <Truck size={16} style={{ color: 'var(--cyan-500)' }} />
                </div>
                <div className="timeline-content">
                  <div style={{ fontWeight: 600, fontSize: '0.92rem' }}>Reefer Cold Logistics Loading</div>
                  <div style={{ fontSize: '0.8rem', color: 'var(--text-secondary)', marginTop: '2px' }}>
                    Transferred to: {currentShipment.currentCustodian} • Reefer Vehicle #HP-01-A-4892
                  </div>
                  <div style={{ fontSize: '0.72rem', color: 'var(--text-muted)', marginTop: '4px' }}>
                    Pre-cooling Verified: 2.8°C • Location: Parwanoo Agro Toll Gate
                  </div>
                </div>
              </div>

              <div className="timeline-item">
                <div className="timeline-icon" style={{ borderColor: 'var(--emerald-500)' }}>
                  <ShieldCheck size={16} style={{ color: 'var(--emerald-400)' }} />
                </div>
                <div className="timeline-content">
                  <div style={{ fontWeight: 600, fontSize: '0.92rem' }}>Current In-Transit Supervision</div>
                  <div style={{ fontSize: '0.8rem', color: 'var(--text-secondary)', marginTop: '2px' }}>
                    Active continuous verification • 18 verified sensor digests anchored on ledger
                  </div>
                </div>
              </div>
            </div>
          </div>
        )}
      </div>

      {/* Custody Handoff Modal */}
      {showHandoffModal && (
        <div
          style={{
            position: 'fixed',
            inset: 0,
            background: 'rgba(0, 0, 0, 0.75)',
            display: 'flex',
            alignItems: 'center',
            justifyContent: 'center',
            zIndex: 100,
          }}
        >
          <div className="glass-panel" style={{ width: '100%', maxWidth: '480px', padding: '2rem' }}>
            <h3 style={{ marginBottom: '0.5rem' }}>Record Custodial Handover</h3>
            <p style={{ fontSize: '0.82rem', color: 'var(--text-secondary)', marginBottom: '1.25rem' }}>
              Submits an endorsement transaction to the Hyperledger Fabric channel.
            </p>

            <form onSubmit={handleHandoff}>
              <div style={{ marginBottom: '1rem' }}>
                <label style={{ display: 'block', fontSize: '0.8rem', color: 'var(--text-secondary)', marginBottom: '0.35rem' }}>
                  New Custodian (Entity / Representative)
                </label>
                <input
                  type="text"
                  required
                  placeholder="e.g. Azadpur Mandi Cold Storage Ltd."
                  value={newCustodian}
                  onChange={(e) => setNewCustodian(e.target.value)}
                  style={{
                    width: '100%',
                    padding: '0.65rem',
                    borderRadius: '8px',
                    background: 'rgba(255, 255, 255, 0.05)',
                    border: '1px solid var(--border-color)',
                    color: '#fff',
                  }}
                />
              </div>

              <div style={{ marginBottom: '1rem' }}>
                <label style={{ display: 'block', fontSize: '0.8rem', color: 'var(--text-secondary)', marginBottom: '0.35rem' }}>
                  Checkpoint Location / Geo-Coordinates
                </label>
                <input
                  type="text"
                  required
                  placeholder="e.g. Kundli Border Agro Checkpost"
                  value={location}
                  onChange={(e) => setLocation(e.target.value)}
                  style={{
                    width: '100%',
                    padding: '0.65rem',
                    borderRadius: '8px',
                    background: 'rgba(255, 255, 255, 0.05)',
                    border: '1px solid var(--border-color)',
                    color: '#fff',
                  }}
                />
              </div>

              <div style={{ marginBottom: '1.5rem' }}>
                <label style={{ display: 'block', fontSize: '0.8rem', color: 'var(--text-secondary)', marginBottom: '0.35rem' }}>
                  Verification Notes / Quality Inspection
                </label>
                <input
                  type="text"
                  placeholder="e.g. Visual inspection passed, seal intact"
                  value={notes}
                  onChange={(e) => setNotes(e.target.value)}
                  style={{
                    width: '100%',
                    padding: '0.65rem',
                    borderRadius: '8px',
                    background: 'rgba(255, 255, 255, 0.05)',
                    border: '1px solid var(--border-color)',
                    color: '#fff',
                  }}
                />
              </div>

              <div style={{ display: 'flex', gap: '0.75rem', justifyContent: 'flex-end' }}>
                <button
                  type="button"
                  onClick={() => setShowHandoffModal(false)}
                  style={{
                    background: 'transparent',
                    border: '1px solid var(--border-color)',
                    color: 'var(--text-secondary)',
                    padding: '0.6rem 1rem',
                    borderRadius: '8px',
                    cursor: 'pointer',
                  }}
                >
                  Cancel
                </button>
                <button type="submit" className="btn-primary">
                  Commit to Fabric Ledger
                </button>
              </div>
            </form>
          </div>
        </div>
      )}
    </div>
  );
}
