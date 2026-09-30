const express = require('express');
const router = express.Router();
const mqttService = require('../services/mqttBroker');
const { getDeviceState } = require('../services/hashVerifier');

// GET /api/sensors/readings?deviceId=...&limit=50
router.get('/readings', (req, res) => {
  const { deviceId, limit } = req.query;
  if (!deviceId) {
    return res.status(400).json({ error: 'deviceId query parameter is required' });
  }

  const readings = mqttService.getRecentReadings(deviceId, limit ? parseInt(limit) : 50);
  const hashState = getDeviceState(deviceId);

  res.json({
    deviceId,
    count: readings.length,
    hashVerificationState: hashState,
    data: readings,
  });
});

// POST /api/sensors/ingest (HTTP fallback for devices without direct MQTT)
router.post('/ingest', (req, res) => {
  const payload = req.body;
  if (!payload.dev || payload.idx === undefined) {
    return res.status(400).json({ error: 'Invalid payload format' });
  }

  mqttService.handleMessage(`agriseal/${payload.dev}/data`, JSON.stringify(payload));
  res.status(200).json({ status: 'ACCEPTED', timestamp: new Date().toISOString() });
});

module.exports = router;
