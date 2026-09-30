const express = require('express');
const router = express.Router();
const fabricGateway = require('../services/fabricGateway');

// GET /api/shipments
router.get('/', async (req, res) => {
  try {
    const list = await fabricGateway.getAllShipments();
    res.json(list);
  } catch (err) {
    res.status(500).json({ error: err.message });
  }
});

// GET /api/shipments/:id
router.get('/:id', async (req, res) => {
  try {
    const shipment = await fabricGateway.getShipment(req.params.id);
    if (!shipment) return res.status(404).json({ error: 'Shipment not found' });

    const digests = await fabricGateway.getShipmentDigests(req.params.id);
    const history = await fabricGateway.getShipmentHistory(req.params.id);

    res.json({
      shipment,
      digests,
      history,
    });
  } catch (err) {
    res.status(500).json({ error: err.message });
  }
});

// POST /api/shipments
router.post('/', async (req, res) => {
  try {
    const { id, produceType, quantityKg, origin, destination, currentCustodian, deviceId } = req.body;
    if (!id || !produceType || !quantityKg || !origin || !destination) {
      return res.status(400).json({ error: 'Missing mandatory shipment attributes' });
    }

    const created = await fabricGateway.createShipment({
      id,
      produceType,
      quantityKg: parseFloat(quantityKg),
      origin,
      destination,
      currentCustodian: currentCustodian || origin,
      deviceId: deviceId || 'AGRISEAL-NODE-0001',
    });

    res.status(201).json(created);
  } catch (err) {
    res.status(400).json({ error: err.message });
  }
});

// POST /api/shipments/:id/transfer
router.post('/:id/transfer', async (req, res) => {
  try {
    const { newCustodian, location, notes } = req.body;
    if (!newCustodian) {
      return res.status(400).json({ error: 'newCustodian is required' });
    }

    const transfer = await fabricGateway.transferOwnership(
      req.params.id,
      newCustodian,
      location || 'En-route Checkpoint',
      notes || 'Standard Custody Handoff'
    );

    res.json(transfer);
  } catch (err) {
    res.status(400).json({ error: err.message });
  }
});

module.exports = router;
