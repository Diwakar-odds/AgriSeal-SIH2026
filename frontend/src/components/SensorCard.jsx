import React from 'react';

export default function SensorCard({ title, value, unit, icon: Icon, colorClass, statusText, subtext }) {
  return (
    <div className="glass-panel metric-card">
      <div className="metric-header">
        <span className="metric-title">{title}</span>
        <div className="metric-icon-wrap" style={{ background: `rgba(255, 255, 255, 0.05)` }}>
          {Icon && <Icon size={20} className={colorClass} />}
        </div>
      </div>
      <div className="metric-value-row">
        <span className={`metric-value ${colorClass}`}>{value}</span>
        {unit && <span className="metric-unit">{unit}</span>}
      </div>
      <div className="metric-subtext">
        {statusText && <span style={{ fontWeight: 600 }}>{statusText}</span>}
        {subtext && <span>• {subtext}</span>}
      </div>
    </div>
  );
}
