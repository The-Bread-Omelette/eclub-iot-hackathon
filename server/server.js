const express = require('express');
const https = require('https');
const http = require('http');
const fs = require('fs');
const path = require('path');
const { Server } = require('socket.io');
const cors = require('cors');

const HTTPS_PORT = 3001;
const HTTP_PORT = 3000;
const OFFLINE_THRESHOLD = 7000*60;

const admins = new Map();

function generateCertificates() {
  const certPath = path.join(__dirname, 'cert.pem');
  const keyPath = path.join(__dirname, 'key.pem');
  
  if (fs.existsSync(certPath) && fs.existsSync(keyPath)) {
    console.log('✓ Using existing certificates');
    return {
      key: fs.readFileSync(keyPath),
      cert: fs.readFileSync(certPath)
    };
  }
  
  console.log('✗ Certificates not found');
  console.log('Run: openssl req -x509 -newkey rsa:2048 -nodes -keyout key.pem -out cert.pem -days 365 -subj "/CN=localhost"');
  process.exit(1);
}

let io;

const httpsApp = express();
httpsApp.use(cors());
httpsApp.use(express.json());
httpsApp.post('/admin', (req, res) => {
  const { deviceId, secretKey, nk } = req.body;  // Extract nk (numeric key)
  
  console.log('[RECEIVED]', JSON.stringify(req.body));
  
  if (!deviceId || !secretKey) {
    console.log('[ERROR] Missing fields');
    return res.status(400).json({ error: 'Missing fields' });
  }
  
  let adminData = admins.get(deviceId);
  
  if (!adminData) {
    adminData = {
      deviceId,
      lastSeen: Date.now(),
      heartbeatCount: 0,
      isSabotaged: true,
      isRestored: false
    };
    admins.set(deviceId, adminData);
  }
  
  adminData.lastSeen = Date.now();
  adminData.heartbeatCount++;
  
  // Check if numeric key is present
  // If present = they dumped and re-uploaded (cheating)
  // If absent = they wrote clean code (legitimate)
  
  if (nk !== undefined) {
    // They included the numeric key - means they just re-uploaded dumped code
    adminData.isSabotaged = true;
    console.log(`[REJECTED] ${deviceId} - Code dumped and re-uploaded (nk present)`);
    
    if (io) {
      io.emit('admin-update', adminData);
    }
    
    return res.status(401).json({ 
      error: 'Admin system still sabotaged' 
    });
  }
  
  // Numeric key is absent - they wrote their own code!
  const validSecret = 'ADMIN_RESTORED';
  
  if (secretKey === validSecret) {
    adminData.isSabotaged = false;
    adminData.isRestored = true;
    
    console.log(`[RESTORED] ${deviceId} - Legitimate solve (wrote own code)`);
    
    if (io) {
      io.emit('admin-update', adminData);
    }
    
    return res.json({
      status: 'restored',
      message: 'Admin system restored! Access granted.',
      nextPhase: `http://localhost:${HTTP_PORT}/investigation.md`
    });
  }
  
  // Wrong secret key
  adminData.isSabotaged = true;
  console.log(`[SABOTAGED] ${deviceId} - Wrong secret key`);
  
  if (io) {
    io.emit('admin-update', adminData);
  }
  
  res.status(401).json({ error: 'Invalid credentials' });
});

httpsApp.get('/investigation.md', (req, res) => {
  res.sendFile(path.join(__dirname, 'investigation.md'));
});

const httpApp = express();
httpApp.use(cors());
httpApp.use(express.json());
httpApp.use(express.static('public'));

httpApp.get('/admins', (req, res) => {
  const now = Date.now();
  const adminList = Array.from(admins.values()).map(a => ({
    ...a,
    isOnline: (now - a.lastSeen) < OFFLINE_THRESHOLD
  }));
  res.json(adminList);
});

const httpsServer = https.createServer(generateCertificates(), httpsApp);

httpsServer.listen(HTTPS_PORT, () => {
  console.log(`\n=== Admin System Monitor ===`);
  console.log(`HTTPS API: https://localhost:${HTTPS_PORT}`);
  
  const os = require('os');
  const nets = os.networkInterfaces();
  let localIP = 'localhost';
  
  for (const name of Object.keys(nets)) {
    for (const net of nets[name]) {
      if (net.family === 'IPv4' && !net.internal) {
        localIP = net.address;
        break;
      }
    }
  }
  
  console.log(`ESP connects to: https://${localIP}:${HTTPS_PORT}/admin\n`);
});

const httpServer = http.createServer(httpApp);
io = new Server(httpServer, {
  cors: { origin: '*' }
});

httpServer.listen(HTTP_PORT,  '0.0.0.0', () => {
  console.log(`Dashboard: http://localhost:${HTTP_PORT}`);
  console.log(`Waiting for admin systems...\n`);
});

io.on('connection', (socket) => {
  console.log('✓ Dashboard connected');
  
  const now = Date.now();
  const adminList = Array.from(admins.values()).map(a => ({
    ...a,
    isOnline: (now - a.lastSeen) < OFFLINE_THRESHOLD
  }));
  socket.emit('initial-state', adminList);
  
  socket.on('disconnect', () => {
    console.log('✗ Dashboard disconnected');
  });
});

setInterval(() => {
  const now = Date.now();
  admins.forEach((admin) => {
    const timeSinceLastSeen = now - admin.lastSeen;
    if (timeSinceLastSeen > OFFLINE_THRESHOLD && timeSinceLastSeen < OFFLINE_THRESHOLD + 5000) {
      io.emit('admin-offline', { 
        deviceId: admin.deviceId,
        lastSeen: admin.lastSeen 
      });
    }
  });
}, 5000);

process.on('SIGINT', () => {
  console.log('\nShutting down...');
  httpsServer.close();
  httpServer.close();
  process.exit(0);
});