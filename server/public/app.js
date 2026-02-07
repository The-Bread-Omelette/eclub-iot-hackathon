const socket = io('http://localhost:3000');
const OFFLINE_THRESHOLD = 7000*60;

const adminGrid = document.getElementById('adminGrid');
const connectionStatus = document.getElementById('connectionStatus');
const connectionDot = document.getElementById('connectionDot');
const deviceCount = document.getElementById('deviceCount');

const audioContext = new (window.AudioContext || window.webkitAudioContext)();
let admins = new Map();
let previousOfflineDevices = new Set();

function playBeep() {
    const oscillator = audioContext.createOscillator();
    const gainNode = audioContext.createGain();
    
    oscillator.connect(gainNode);
    gainNode.connect(audioContext.destination);
    
    oscillator.frequency.value = 800;
    oscillator.type = 'sine';
    
    gainNode.gain.setValueAtTime(0.3, audioContext.currentTime);
    gainNode.gain.exponentialRampToValueAtTime(0.01, audioContext.currentTime + 0.5);
    
    oscillator.start(audioContext.currentTime);
    oscillator.stop(audioContext.currentTime + 0.5);
}
socket.on('connect', () => {
    console.log('✓ Connected');
    connectionStatus.textContent = 'LIVE';
    connectionDot.classList.add('connected');
});

socket.on('disconnect', () => {
    console.log('✗ Disconnected');
    connectionStatus.textContent = 'DISCONNECTED';
    connectionDot.classList.remove('connected');
});

socket.on('initial-state', (adminList) => {
    adminList.forEach(admin => {
        admins.set(admin.deviceId, admin);
    });
    renderAdmins();
});

socket.on('admin-update', (admin) => {
    admins.set(admin.deviceId, admin);
    renderAdmins();
});

socket.on('admin-offline', ({ deviceId }) => {
    const admin = admins.get(deviceId);
    if (admin) {
        admin.isOnline = false;
        renderAdmins();
    }
});


function renderAdmins() {
    adminGrid.innerHTML = '';
    
    const adminArray = Array.from(admins.values()).sort((a, b) => 
        a.deviceId.localeCompare(b.deviceId)
    );
    
    deviceCount.textContent = adminArray.length;
    
    const currentOfflineDevices = new Set();
    
    adminArray.forEach(admin => {
        const now = Date.now();
        const isOnline = (now - admin.lastSeen) < OFFLINE_THRESHOLD;
        
        if (!isOnline) {
            currentOfflineDevices.add(admin.deviceId);
            
            // Check if this device just went offline
            if (!previousOfflineDevices.has(admin.deviceId)) {
                playBeep();
                console.log(`🔊 BEEP: ${admin.deviceId} went offline`);
            }
        }
        
        const card = createAdminCard(admin);
        adminGrid.appendChild(card);
    });
    
    previousOfflineDevices = currentOfflineDevices;
}

function createAdminCard(admin) {
    const card = document.createElement('div');
    
    const now = Date.now();
    const isOnline = (now - admin.lastSeen) < OFFLINE_THRESHOLD;
    
    let state, icon, stateText;
    
    if (!isOnline) {
        state = 'offline';
        icon = '⚠️';
        stateText = 'OFFLINE';
    } else if (admin.isRestored) {
        state = 'restored';
        icon = '✅';
        stateText = 'RESTORED';
    } else if (admin.isSabotaged) {
        state = 'sabotaged';
        icon = '❌';
        stateText = 'SABOTAGED';
    } else {
        state = 'unknown';
        icon = '❓';
        stateText = 'UNKNOWN';
    }
    
    card.className = `admin-card ${state}`;
    card.innerHTML = `
        <div class="admin-header">
            <span class="group-label">${admin.deviceId}</span>
        </div>
        <div class="admin-icon">${icon}</div>
        <div class="admin-state ${state}">${stateText}</div>
        <div class="admin-stats">
            <div class="stat">
                <span class="stat-label">Heartbeats:</span>
                <span class="stat-value">${admin.heartbeatCount || 0}</span>
            </div>
            <div class="stat">
                <span class="stat-label">Last seen:</span>
                <span class="stat-value">${formatTimestamp(admin.lastSeen)}</span>
            </div>
        </div>
    `;
    
    return card;
}

function formatTimestamp(timestamp) {
    const seconds = Math.floor((Date.now() - timestamp) / 1000);
    
    if (seconds < 10) return `${seconds}s ago`;
    if (seconds < 60) return `${seconds}s ago`;
    if (seconds < 3600) return `${Math.floor(seconds / 60)}m ago`;
    return `${Math.floor(seconds / 3600)}h ago`;
}

setInterval(() => {
    renderAdmins();
}, 1000);