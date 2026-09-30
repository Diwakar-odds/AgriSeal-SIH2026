/**
 * AgriSeal API Server & IoT Telemetry Gateway
 * ===========================================
 * Main Node.js application combining REST endpoints, WebSocket streaming,
 * MQTT ingestion, cryptographic verification, and Hyperledger Fabric gateway.
 *
 * Team Arishem (SIH 2026 - PS 26232)
 */

require('dotenv').config();
const http = require('http');
const express = require('express');
const cors = require('cors');
const helmet = require('helmet');
const morgan = require('morgan');
const { Server } = require('socket.io');

const fabricGateway = require('./services/fabricGateway');
const mqttService = require('./services/mqttBroker');

const sensorRoutes = require('./routes/sensorData');
const shipmentRoutes = require('./routes/shipments');
const alertRoutes = require('./routes/alerts');
const deviceRoutes = require('./routes/devices');

const app = express();
const server = http.createServer(app);

// CORS and Security Middleware
app.use(helmet({ contentSecurityPolicy: false }));
app.use(
  cors({
    origin: '*',
    methods: ['GET', 'POST', 'PUT', 'DELETE'],
  })
);
app.use(express.json());
app.use(morgan('dev'));

// Setup Socket.IO for real-time frontend dashboard streaming
const io = new Server(server, {
  cors: {
    origin: '*',
    methods: ['GET', 'POST'],
  },
});

io.on('connection', (socket) => {
  console.log(`[SOCKET] Client connected: ${socket.id}`);
  socket.on('disconnect', () => {
    console.log(`[SOCKET] Client disconnected: ${socket.id}`);
  });
});

// API Routes
app.use('/api/sensors', sensorRoutes);
app.use('/api/shipments', shipmentRoutes);
app.use('/api/alerts', alertRoutes);
app.use('/api/devices', deviceRoutes);

// Health check
app.get('/api/health', (req, res) => {
  res.json({
    status: 'ONLINE',
    project: 'AgriSeal (SIH 2026)',
    version: '1.0.0',
    timestamp: new Date().toISOString(),
  });
});

const PORT = process.env.PORT || 5000;

async function start() {
  try {
    // 1. Initialize Fabric Gateway
    await fabricGateway.connect();

    // 2. Initialize MQTT Broker & Telemetry processor
    mqttService.init(io);

    // 3. Start HTTP Server
    server.listen(PORT, () => {
      console.log(`====================================================`);
      console.log(`  🌾 AgriSeal Backend Server running on port ${PORT}`);
      console.log(`  🔗 REST API:   http://localhost:${PORT}/api/health`);
      console.log(`  📡 WebSocket:  ws://localhost:${PORT}`);
      console.log(`====================================================`);
    });
  } catch (error) {
    console.error('Fatal initialization error:', error);
    process.exit(1);
  }
}

start();
