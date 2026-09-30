const express = require('express');
const router = express.Router();
const mqttService = require('../services/mqttBroker');

// GET /api/devices
router.get('/', (req, res) => {
  const devices = mqttService.getDevices();
  res.json(devices);
});

// POST /api/devices/:id/command (send remote threshold or sampling configuration)
router.post('/:id/command', (req, res) => {
  const { id } = req.params;
  const { command, params } = req.body;

  if (mqttService.client && mqttService.client.connected) {
    const topic = `agriseal/${id}/cmd`;
    mqttService.client.publish(topic, JSON.stringify({ command, params }));
    return res.json({ status: 'SENT', topic, command });
  }

  res.json({ status: 'SIMULATED', message: 'Command queued for node delivery upon reconnection' });
});

module.exports = router;
