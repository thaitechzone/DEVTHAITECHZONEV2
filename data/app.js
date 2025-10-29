// Authentication
const auth = localStorage.getItem('auth');
if (!auth) {
    window.location.href = '/login.html';
}

const authHeaders = {
    'Authorization': 'Basic ' + auth
};

// Global variables
let updateInterval;
let logs = [];

// Initialize
document.addEventListener('DOMContentLoaded', function() {
    console.log('Dashboard loaded');
    startAutoUpdate();
    addLog('Dashboard loaded successfully');
});

// Auto update data every 2 seconds
function startAutoUpdate() {
    updateStatus();
    updateInterval = setInterval(updateStatus, 2000);
}

// Fetch and update status
async function updateStatus() {
    try {
        showUpdateIndicator();
        const response = await fetch('/api/status', { headers: authHeaders });
        
        if (!response.ok) {
            if (response.status === 401) {
                logout();
                return;
            }
            throw new Error('Failed to fetch status');
        }
        
        const data = await response.json();
        updateUI(data);
        updateConnectionStatus(true);
        hideUpdateIndicator();
    } catch (error) {
        console.error('Error fetching status:', error);
        updateConnectionStatus(false);
        hideUpdateIndicator();
    }
}

// Update UI with data
function updateUI(data) {
    // System info
    document.getElementById('systemMode').textContent = data.mode || 'AUTO';
    document.getElementById('systemMode').className = 'mode-switch ' + 
        (data.mode === 'AUTO' ? 'mode-auto' : 'mode-manual');
    document.getElementById('uptime').textContent = formatUptime(data.uptime || 0);
    document.getElementById('wifiRSSI').textContent = data.wifi_rssi || '--';
    document.getElementById('freeHeap').textContent = Math.round((data.free_heap || 0) / 1024);
    
    // Relays
    updateRelay(1, data.relay1);
    updateRelay(2, data.relay2);
    updateRelay(3, data.relay3);
    
    // Sensors
    document.getElementById('dhtTemp').textContent = data.dht_temperature ? data.dht_temperature.toFixed(1) : '--';
    document.getElementById('dhtHumid').textContent = data.dht_humidity ? data.dht_humidity.toFixed(1) : '--';
    document.getElementById('weatherTemp').textContent = data.weather_temp || '--';
    document.getElementById('weatherHumid').textContent = data.weather_humid || '--';
    document.getElementById('aqi').textContent = data.aqi || '--';
    document.getElementById('aqiQuality').textContent = data.aqi_quality || '--';
    
    // Inputs
    updateInput('sw1', data.switch1);
    updateInput('sw2', data.switch2);
    updateInput('sw3', data.switch3);
    updateInput('iso1', data.iso_input1);
    updateInput('iso2', data.iso_input2);
    
    // AUX
    updateAux('aux1', data.aux1);
    updateAux('aux2', data.aux2);
    updateAux('aux3', data.aux3);
    updateAux('aux4', data.aux4);
}

// Update relay display
function updateRelay(num, state) {
    const status = document.getElementById('relay' + num + 'Status');
    const btn = document.getElementById('relay' + num + 'Btn');
    
    if (state) {
        status.textContent = 'ON';
        status.className = 'badge bg-success';
        btn.className = 'btn btn-relay relay-on';
    } else {
        status.textContent = 'OFF';
        status.className = 'badge bg-secondary';
        btn.className = 'btn btn-relay relay-off';
    }
}

// Update input indicator
function updateInput(id, state) {
    const elem = document.getElementById(id);
    elem.className = 'input-indicator ' + (state ? 'input-active' : 'input-inactive');
}

// Update AUX display
function updateAux(id, state) {
    const elem = document.getElementById(id);
    elem.textContent = state ? 'ON' : 'OFF';
    elem.className = 'badge ' + (state ? 'bg-success' : 'bg-secondary');
}

// Toggle relay
async function toggleRelay(num) {
    try {
        const response = await fetch('/api/relay/' + num + '/toggle', {
            method: 'POST',
            headers: authHeaders
        });
        
        if (response.ok) {
            const data = await response.json();
            addLog('Relay ' + num + ' toggled to ' + (data.state ? 'ON' : 'OFF'));
            updateStatus();
        } else {
            addLog('Failed to toggle Relay ' + num, 'error');
        }
    } catch (error) {
        console.error('Error toggling relay:', error);
        addLog('Error toggling Relay ' + num, 'error');
    }
}

// Toggle mode
async function toggleMode() {
    try {
        const response = await fetch('/api/mode/toggle', {
            method: 'POST',
            headers: authHeaders
        });
        
        if (response.ok) {
            const data = await response.json();
            addLog('Mode changed to ' + data.mode);
            updateStatus();
        } else {
            addLog('Failed to toggle mode', 'error');
        }
    } catch (error) {
        console.error('Error toggling mode:', error);
        addLog('Error toggling mode', 'error');
    }
}

// Start timer
async function startTimer() {
    const relay = document.getElementById('timerRelay').value;
    const duration = document.getElementById('timerDuration').value;
    
    if (!duration || duration < 1) {
        alert('Please enter valid duration');
        return;
    }
    
    try {
        const response = await fetch('/api/timer', {
            method: 'POST',
            headers: {
                ...authHeaders,
                'Content-Type': 'application/json'
            },
            body: JSON.stringify({
                relay: parseInt(relay),
                duration: parseInt(duration)
            })
        });
        
        if (response.ok) {
            addLog('Timer started: Relay ' + relay + ' for ' + duration + 's');
            document.getElementById('timerStatus').style.display = 'block';
            setTimeout(() => {
                document.getElementById('timerStatus').style.display = 'none';
                addLog('Timer completed');
            }, duration * 1000);
        } else {
            addLog('Failed to start timer', 'error');
        }
    } catch (error) {
        console.error('Error starting timer:', error);
        addLog('Error starting timer', 'error');
    }
}

// Add log entry
function addLog(message, type = 'info') {
    const now = new Date();
    const time = now.toLocaleTimeString('th-TH');
    const logEntry = document.createElement('div');
    logEntry.className = 'log-entry';
    logEntry.innerHTML = '<strong>' + time + '</strong> - ' + message;
    
    const container = document.getElementById('logContainer');
    container.insertBefore(logEntry, container.firstChild);
    
    // Keep only last 50 logs
    while (container.children.length > 50) {
        container.removeChild(container.lastChild);
    }
    
    logs.push({ time: time, message: message, type: type });
}

// Clear logs
function clearLogs() {
    if (confirm('Clear all logs?')) {
        document.getElementById('logContainer').innerHTML = '';
        logs = [];
        addLog('Logs cleared');
    }
}

// Format uptime
function formatUptime(seconds) {
    const hours = Math.floor(seconds / 3600);
    const minutes = Math.floor((seconds % 3600) / 60);
    const secs = seconds % 60;
    
    if (hours > 0) {
        return hours + 'h ' + minutes + 'm ' + secs + 's';
    } else if (minutes > 0) {
        return minutes + 'm ' + secs + 's';
    } else {
        return secs + 's';
    }
}

// Update connection status
function updateConnectionStatus(connected) {
    const elem = document.getElementById('connectionStatus');
    if (connected) {
        elem.innerHTML = '<i class="fas fa-wifi"></i> Connected';
        elem.className = 'status-badge status-online me-3';
    } else {
        elem.innerHTML = '<i class="fas fa-wifi"></i> Disconnected';
        elem.className = 'status-badge status-offline me-3';
    }
}

// Show/hide update indicator
function showUpdateIndicator() {
    document.getElementById('updateIndicator').style.display = 'block';
}

function hideUpdateIndicator() {
    setTimeout(() => {
        document.getElementById('updateIndicator').style.display = 'none';
    }, 300);
}

// Logout
function logout() {
    localStorage.removeItem('auth');
    window.location.href = '/login.html';
}

// Handle window visibility change
document.addEventListener('visibilitychange', function() {
    if (document.hidden) {
        clearInterval(updateInterval);
    } else {
        startAutoUpdate();
    }
});
