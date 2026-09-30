const express = require('express');
const router = express.Router();
const fabricGateway = require('../services/fabricGateway');

// GET /api/alerts
router.get('/', async (req, res) => {
  try {
    const { shipmentId } = req.query;
    const breaches = await fabricGateway.getBreaches(shipmentId);
    res.json(breaches);
  } catch (err) {
    res.status(500).json({ error: err.message });
  }
});

module.exports = router;
