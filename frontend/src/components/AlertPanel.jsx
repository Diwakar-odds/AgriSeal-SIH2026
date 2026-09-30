import React from 'react';
import { AlertTriangle, ShieldAlert, CheckCircle } from 'lucide-react';

export default function AlertPanel({ alerts }) {
  if (!alerts || alerts.length === 0) {
    return (
      <div className="glass-panel" style={{ padding: '1.5rem', textAlign: 'center' }}>
        <CheckCircle size={32} style={{ color: 'var(--emerald-400)', margin: '0 auto 0.75rem auto' }} />
        <h4 style={{ marginBottom: '0.25rem' }}>No Active Breach Events</h4>
        <p style={{ fontSize: '0.82rem', color: 'var(--text-secondary)' }}>
          All monitored environmental factors are compliant with produce SLA thresholds.
        </p>
      </div>
    );
  }

  return (
    <div className="glass-panel" style={{ padding: '1.5rem' }}>
      <div style={{ display: 'flex', alignItems: 'center', gap: '0.5rem', marginBottom: '1.25rem' }}>
        <ShieldAlert size={20} style={{ color: 'var(--rose-500)' }} />
        <h3>SLA Breach & Tamper Logs</h3>
      </div>
      <div style={{ display: 'flex', flexDirection: 'column', gap: '0.85rem' }}>
        {alerts.map((alert, idx) => (
          <div
            key={idx}
            style={{
              padding: '0.85rem',
              borderRadius: '8px',
              background: 'rgba(244, 63, 94, 0.08)',
              border: '1px solid rgba(244, 63, 94, 0.25)',
              display: 'flex',
              gap: '0.75rem',
            }}
          >
            <AlertTriangle size={18} style={{ color: 'var(--rose-500)', flexShrink: 0, marginTop: '2px' }} />
            <div>
              <div style={{ fontWeight: 600, fontSize: '0.85rem', color: '#fff' }}>
                {alert.breachType || 'SLA Excursion'}
              </div>
              <div style={{ fontSize: '0.78rem', color: 'var(--text-secondary)', marginTop: '2px' }}>
                Shipment: <span className="mono">{alert.shipmentId}</span> | Value: {alert.recordedValue} (Limit: {alert.thresholdValue})
              </div>
              <div style={{ fontSize: '0.72rem', color: 'var(--text-muted)', marginTop: '4px' }}>
                {new Date(alert.timestamp).toLocaleString()} • Tx: <span className="mono">{alert.txId?.slice(0, 14)}...</span>
              </div>
            </div>
          </div>
        ))}
      </div>
    </div>
  );
}
