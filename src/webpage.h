#ifndef WEBPAGE_H
#define WEBPAGE_H

const char* index_html = R"=====(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Smart Relay Control</title>
    <link href="https://fonts.googleapis.com/css2?family=Inter:wght@400;500;600;700&display=swap" rel="stylesheet">
    <style>
        :root {
            --bg-color: #0f172a;
            --glass-bg: rgba(30, 41, 59, 0.7);
            --glass-border: rgba(255, 255, 255, 0.1);
            --text-main: #f8fafc;
            --text-muted: #94a3b8;
            --accent: #3b82f6;
            --accent-hover: #2563eb;
            --success: #10b981;
            --danger: #ef4444;
        }

        * {
            box-sizing: border-box;
            margin: 0;
            padding: 0;
            font-family: 'Inter', sans-serif;
        }

        body {
            background-color: var(--bg-color);
            background-image: 
                radial-gradient(at 0% 0%, rgba(59, 130, 246, 0.15) 0px, transparent 50%),
                radial-gradient(at 100% 100%, rgba(16, 185, 129, 0.15) 0px, transparent 50%);
            background-attachment: fixed;
            color: var(--text-main);
            min-height: 100vh;
            display: flex;
            justify-content: center;
            align-items: center;
            padding: 20px;
        }

        .container {
            width: 100%;
            max-width: 500px;
            display: flex;
            flex-direction: column;
            gap: 20px;
        }

        .card {
            background: var(--glass-bg);
            backdrop-filter: blur(12px);
            -webkit-backdrop-filter: blur(12px);
            border: 1px solid var(--glass-border);
            border-radius: 16px;
            padding: 24px;
            box-shadow: 0 4px 30px rgba(0, 0, 0, 0.1);
        }

        .header {
            text-align: center;
            margin-bottom: 10px;
        }

        .header h1 {
            font-size: 1.5rem;
            font-weight: 700;
            margin-bottom: 8px;
        }

        .status-bar {
            display: flex;
            justify-content: space-between;
            font-size: 0.875rem;
            color: var(--text-muted);
            margin-bottom: 20px;
            padding-bottom: 15px;
            border-bottom: 1px solid var(--glass-border);
        }

        .relay-group {
            display: flex;
            flex-direction: column;
            gap: 15px;
            margin-bottom: 20px;
        }

        .relay-card {
            background: rgba(15, 23, 42, 0.5);
            border: 1px solid var(--glass-border);
            border-radius: 12px;
            padding: 16px;
        }

        .relay-header {
            display: flex;
            justify-content: space-between;
            align-items: center;
            margin-bottom: 15px;
        }

        .relay-title {
            font-weight: 600;
            font-size: 1.1rem;
        }

        /* Toggle Switch */
        .switch {
            position: relative;
            display: inline-block;
            width: 50px;
            height: 28px;
        }

        .switch input {
            opacity: 0;
            width: 0;
            height: 0;
        }

        .slider {
            position: absolute;
            cursor: pointer;
            top: 0; left: 0; right: 0; bottom: 0;
            background-color: rgba(255,255,255,0.2);
            transition: .4s;
            border-radius: 34px;
        }

        .slider:before {
            position: absolute;
            content: "";
            height: 20px;
            width: 20px;
            left: 4px;
            bottom: 4px;
            background-color: white;
            transition: .4s;
            border-radius: 50%;
            box-shadow: 0 2px 4px rgba(0,0,0,0.2);
        }

        input:checked + .slider {
            background-color: var(--success);
        }

        input:checked + .slider:before {
            transform: translateX(22px);
        }

        .schedule-form {
            display: flex;
            flex-direction: column;
            gap: 10px;
            font-size: 0.9rem;
        }

        .schedule-row {
            display: flex;
            align-items: center;
            justify-content: space-between;
            gap: 10px;
        }

        input[type="time"], input[type="text"], input[type="password"] {
            background: rgba(0,0,0,0.2);
            border: 1px solid var(--glass-border);
            color: var(--text-main);
            padding: 8px 12px;
            border-radius: 8px;
            outline: none;
            flex: 1;
            font-family: 'Inter', sans-serif;
            transition: border-color 0.3s;
        }
        
        input[type="time"]:focus, input[type="text"]:focus, input[type="password"]:focus {
            border-color: var(--accent);
        }

        /* Specifically for time inputs icon color on webkit */
        ::-webkit-calendar-picker-indicator {
            filter: invert(1);
            opacity: 0.7;
            cursor: pointer;
        }

        button {
            background: var(--accent);
            color: white;
            border: none;
            padding: 10px 16px;
            border-radius: 8px;
            cursor: pointer;
            font-weight: 600;
            font-size: 0.9rem;
            transition: background 0.3s, transform 0.1s;
        }

        button:hover {
            background: var(--accent-hover);
        }
        
        button:active {
            transform: scale(0.98);
        }

        .wifi-section h3 {
            font-size: 1.1rem;
            margin-bottom: 15px;
        }

        .wifi-form {
            display: flex;
            flex-direction: column;
            gap: 12px;
        }
        
        .toast {
            position: fixed;
            bottom: 20px;
            left: 50%;
            transform: translateX(-50%) translateY(100px);
            background: var(--success);
            color: white;
            padding: 12px 24px;
            border-radius: 30px;
            font-weight: 500;
            box-shadow: 0 4px 12px rgba(0,0,0,0.2);
            transition: transform 0.3s cubic-bezier(0.68, -0.55, 0.265, 1.55);
            z-index: 1000;
            opacity: 0;
        }
        
        .toast.show {
            transform: translateX(-50%) translateY(0);
            opacity: 1;
        }
        
        .loader {
            display: inline-block;
            width: 16px;
            height: 16px;
            border: 2px solid rgba(255,255,255,0.3);
            border-radius: 50%;
            border-top-color: white;
            animation: spin 1s ease-in-out infinite;
            margin-left: 8px;
            vertical-align: middle;
            display: none;
        }
        
        @keyframes spin {
            to { transform: rotate(360deg); }
        }
    </style>
</head>
<body>

    <div class="container">
        <div class="card">
            <div class="header">
                <h1>Smart Relay Hub</h1>
            </div>
            
            <div class="status-bar">
                <span id="current-time">Loading time...</span>
                <span id="ntp-status">NTP: Syncing...</span>
            </div>

            <div class="relay-group">
                <!-- Relay 1 -->
                <div class="relay-card">
                    <div class="relay-header">
                        <span class="relay-title">Channel 1</span>
                        <label class="switch">
                            <input type="checkbox" id="r1-state" onchange="toggleRelay(1, this.checked)">
                            <span class="slider"></span>
                        </label>
                    </div>
                    <div class="schedule-form">
                        <div class="schedule-row">
                            <label>Schedule:</label>
                            <label class="switch" style="transform: scale(0.8); margin-right: auto;">
                                <input type="checkbox" id="r1-en">
                                <span class="slider"></span>
                            </label>
                        </div>
                        <div class="schedule-row">
                            <span>ON:</span>
                            <input type="time" id="r1-on">
                            <span>OFF:</span>
                            <input type="time" id="r1-off">
                        </div>
                        <button onclick="saveSchedule(1)">Save Schedule <span class="loader" id="l-s1"></span></button>
                    </div>
                </div>

                <!-- Relay 2 -->
                <div class="relay-card">
                    <div class="relay-header">
                        <span class="relay-title">Channel 2</span>
                        <label class="switch">
                            <input type="checkbox" id="r2-state" onchange="toggleRelay(2, this.checked)">
                            <span class="slider"></span>
                        </label>
                    </div>
                    <div class="schedule-form">
                        <div class="schedule-row">
                            <label>Schedule:</label>
                            <label class="switch" style="transform: scale(0.8); margin-right: auto;">
                                <input type="checkbox" id="r2-en">
                                <span class="slider"></span>
                            </label>
                        </div>
                        <div class="schedule-row">
                            <span>ON:</span>
                            <input type="time" id="r2-on">
                            <span>OFF:</span>
                            <input type="time" id="r2-off">
                        </div>
                        <button onclick="saveSchedule(2)">Save Schedule <span class="loader" id="l-s2"></span></button>
                    </div>
                </div>
            </div>

            <div class="wifi-section">
                <h3>Station WiFi (STA)</h3>
                <div class="wifi-form">
                    <input type="text" id="wifi-ssid" placeholder="Network Name (SSID)">
                    <input type="password" id="wifi-pass" placeholder="Password (leave blank if unchanged)">
                    <button onclick="saveWiFi()">Connect & Save <span class="loader" id="l-wifi"></span></button>
                </div>
            </div>
        </div>
    </div>

    <div class="toast" id="toast">Settings Saved!</div>

    <script>
        function showToast(msg) {
            const toast = document.getElementById('toast');
            toast.textContent = msg;
            toast.classList.add('show');
            setTimeout(() => toast.classList.remove('show'), 3000);
        }

        async function fetchStatus() {
            try {
                const res = await fetch('/api/status');
                const data = await res.json();
                
                document.getElementById('current-time').textContent = data.time;
                document.getElementById('ntp-status').textContent = data.ntp_synced ? 'NTP: Synced' : 'NTP: Local';
                
                document.getElementById('r1-state').checked = data.relay1.state;
                document.getElementById('r1-en').checked = data.relay1.sched_en;
                document.getElementById('r1-on').value = data.relay1.on_time;
                document.getElementById('r1-off').value = data.relay1.off_time;

                document.getElementById('r2-state').checked = data.relay2.state;
                document.getElementById('r2-en').checked = data.relay2.sched_en;
                document.getElementById('r2-on').value = data.relay2.on_time;
                document.getElementById('r2-off').value = data.relay2.off_time;

                document.getElementById('wifi-ssid').value = data.wifi_ssid;
            } catch (err) {
                console.error("Failed to fetch status");
            }
        }

        async function toggleRelay(num, state) {
            try {
                await fetch('/api/relay', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify({ relay: num, state: state })
                });
            } catch (err) {
                console.error("Toggle failed", err);
            }
        }

        async function saveSchedule(num) {
            const loader = document.getElementById(`l-s${num}`);
            loader.style.display = 'inline-block';
            
            const payload = {
                relay: num,
                en: document.getElementById(`r${num}-en`).checked,
                on: document.getElementById(`r${num}-on`).value,
                off: document.getElementById(`r${num}-off`).value
            };

            try {
                const res = await fetch('/api/schedule', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify(payload)
                });
                if (res.ok) showToast(`Channel ${num} Schedule Saved!`);
            } catch (err) {
                console.error("Save schedule failed", err);
            }
            loader.style.display = 'none';
        }

        async function saveWiFi() {
            const loader = document.getElementById('l-wifi');
            loader.style.display = 'inline-block';
            
            const payload = {
                ssid: document.getElementById('wifi-ssid').value,
                pass: document.getElementById('wifi-pass').value
            };

            try {
                const res = await fetch('/api/wifi', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify(payload)
                });
                if (res.ok) showToast(`WiFi config saved. Rebooting...`);
            } catch (err) {
                console.error("Save WiFi failed", err);
            }
            loader.style.display = 'none';
        }

        // Initialize and poll
        fetchStatus();
        setInterval(fetchStatus, 5000); // Poll every 5 seconds to keep time updated
    </script>
</body>
</html>
)=====";

#endif
