<!DOCTYPE html>
<html>
<head>
    <title>FALCONE Tracker</title>
    <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no" />
    <link rel="stylesheet" href="https://unpkg.com/leaflet@1.7.1/dist/leaflet.css" />
    <style>
        * {
            box-sizing: border-box;
            touch-action: manipulation;
        }
        body {
            margin: 0;
            padding: 10px;
            font-family: Arial, sans-serif;
            background: #f0f0f0;
            display: flex;
            flex-direction: column;
            align-items: center;
            min-height: 100vh;
        }
        #container {
            width: 100%;
            max-width: 800px;
            border: 2px solid #333;
            border-radius: 8px;
            overflow: hidden;
            box-shadow: 0 0 15px rgba(0,0,0,0.1);
            display: flex;
            flex-direction: column;
        }
        #map-container {
            position: relative;
            width: 100%;
            height: 60vh;
            min-height: 300px;
        }
        #map {
            width: 100%;
            height: 100%;
        }
        .falcone1 { color: red; }
        .falcone2 { color: blue; }
        .self { color: green; }

        #control-panel {
            display: flex;
            flex-wrap: wrap;
            justify-content: center;
            gap: 8px;
            padding: 12px;
            background: #f5f5f5;
            border-top: 1px solid #ddd;
        }

        #toggle-map {
            flex: 1 1 100%; /* Make Satellite View button take the full row */
            text-align: center;
        }

        .control-btn {
            padding: 10px 12px;
            background: white;
            border: 1px solid #ccc;
            border-radius: 4px;
            cursor: pointer;
            transition: all 0.3s ease;
            flex: 1 1 30%; /* Default button size */
            text-align: center;
            font-size: 14px;
            min-width: 0;
        }

        /* Hover effect for all buttons */
        .control-btn:hover {
            background: #e9e9e9;
            box-shadow: 0 4px 8px rgba(0, 0, 0, 0.2); /* Add shadow on hover */
        }

        /* Active state for all buttons */
        .control-btn:active {
            background: #dcdcdc; /* Slightly darker background when clicked */
            transform: scale(0.98); /* Slightly shrink the button on click */
        }

        /* Specific styles for the Start Alignment button */
        button[onclick="startAlignment()"] {
            flex: 1 1 100%; /* Full width */
            text-align: center;
            background: #4CAF50; /* Green background */
            color: white; /* White text */
            border: none; /* Remove border */
            border-radius: 4px; /* Rounded corners */
            padding: 12px 16px; /* Padding for better spacing */
            font-size: 16px; /* Larger font size */
            font-weight: bold; /* Bold text */
            cursor: pointer; /* Pointer cursor on hover */
            transition: all 0.3s ease; /* Smooth transition for hover effects */
        }

        button[onclick="startAlignment()"]:hover {
            background: #45a049; /* Darker green on hover */
            box-shadow: 0 4px 8px rgba(0, 0, 0, 0.2); /* Add shadow on hover */
        }

        button[onclick="startAlignment()"]:active {
            background: #3e8e41; /* Even darker green when clicked */
            transform: scale(0.98); /* Slightly shrink the button on click */
        }

        #center-self { border-left: 4px solid green; }
        #center-falcone1 { border-left: 4px solid red; }
        #center-falcone2 { border-left: 4px solid blue; }

        #connection-status {
            position: absolute;
            top: 10px;
            right: 10px;
            background: white;
            padding: 5px 10px;
            border-radius: 4px;
            z-index: 1000;
            box-shadow: 0 0 5px rgba(0,0,0,0.2);
            border: 1px solid #eee;
            font-size: 14px;
        }
        .connected { color: green; }
        .disconnected { color: red; }

        #title-bar {
            background: #333;
            color: white;
            padding: 12px;
            text-align: center;
            font-weight: bold;
            font-size: 18px;
        }

        #distance-panel {
            background: white;
            padding: 10px;
            border-bottom: 1px solid #ddd;
            display: flex;
            justify-content: space-around;
            flex-wrap: wrap;
            gap: 10px;
        }
        .distance-item {
            text-align: center;
            min-width: 120px;
        }
        .distance-value {
            font-weight: bold;
            font-size: 16px;
        }
        .distance-label {
            font-size: 12px;
            color: #666;
        }

        @media (max-width: 600px) {
            body { padding: 5px; }
            #container { max-width: 100%; }
            #map-container { height: 50vh; }
            .control-btn {
                padding: 8px 6px;
                font-size: 13px;
            }
            #title-bar {
                padding: 10px;
                font-size: 16px;
            }
            #distance-panel {
                flex-direction: column;
                gap: 5px;
            }
            .distance-item {
                min-width: 100%;
                display: flex;
                justify-content: space-between;
            }
        }

        #alignment-screen {
            display: none;
            position: absolute;
            top: 0;
            left: 0;
            width: 100%;
            height: 100%;
            background: #f0f0f0;
            z-index: 1000;
            padding: 20px;
            text-align: center;
        }

        #alignment-screen h2 {
            margin-bottom: 20px;
        }

        #alignment-screen .control-btn {
            margin: 10px;
            padding: 12px 20px;
            font-size: 16px;
            background: #4CAF50;
            color: white;
            border: none;
            border-radius: 4px;
            cursor: pointer;
            transition: all 0.3s ease;
        }

        #alignment-screen .control-btn:hover {
            background: #45a049;
        }

        #alignment-screen .control-btn:active {
            background: #3e8e41;
            transform: scale(0.98);
        }

        #alignment-screen .control-btn:last-child {
            background: #f44336; /* Red for Exit button */
        }

        #alignment-screen .control-btn:last-child:hover {
            background: #d32f2f;
        }

        .cone-controls {
            margin-bottom: 20px;
            padding: 10px;
            border: 1px solid #ccc;
            border-radius: 8px;
            background: #fff;
            text-align: center;
        }

        .cone-controls h3 {
            margin-bottom: 10px;
            font-size: 18px;
            color: #333;
        }

        .controls button {
            margin: 5px;
            padding: 10px 15px;
            font-size: 14px;
            background: #4CAF50;
            color: white;
            border: none;
            border-radius: 4px;
            cursor: pointer;
            transition: all 0.3s ease;
        }

        .controls button:hover {
            background: #45a049;
        }

        .controls button:active {
            background: #3e8e41;
            transform: scale(0.98);
        }

        .control-panel {
        margin-top: 10px;
        padding: 10px;
        border: 1px solid #ccc;
        border-radius: 8px;
        background: #f9f9f9;
        text-align: center;
    }

        .control-panel button {
            margin: 5px;
            padding: 10px 15px;
            font-size: 14px;
            background: #4CAF50;
            color: white;
            border: none;
            border-radius: 4px;
            cursor: pointer;
            transition: all 0.3s ease;
        }

        .control-panel button:hover {
            background: #45a049;
        }

        .control-panel button:active {
            background: #3e8e41;
            transform: scale(0.98);
        }

        .control-panel .align-btn {
            background: #2196F3;
        }

        .control-panel .align-btn:hover {
            background: #1976D2;
        }

        .control-panel .stop-btn {
            background: #f44336;
        }

        .control-panel .stop-btn:hover {
            background: #d32f2f;
        }

        .speed-control {
            margin-top: 10px;
            display: flex;
            align-items: center;
            justify-content: center;
        }

        .speed-control span {
            margin: 0 5px;
            font-size: 14px;
        }

        .speed-control input[type="range"] {
            margin: 0 10px;
        }
    </style>
</head>
<body>
    <div id="container">
        <div id="title-bar">FALCONE TRACKING SYSTEM</div>
        <div id="map-container">
            <div id="map"></div>
            <div id="connection-status" class="disconnected">Disconnected</div>
        </div>
        <div id="control-panel">
            <button id="center-self" class="control-btn" onclick="centerMap('Self')">Falcone1</button>
            <button id="center-falcone1" class="control-btn" onclick="centerMap('Falcone1')">Falcone2</button>
            <button id="center-falcone2" class="control-btn" onclick="centerMap('Falcone2')">Falcone3</button>
            <button id="toggle-map" class="control-btn">Satellite View</button>
            <button onclick="startAlignment()" class="control-btn" style="flex: 1 1 100%;">Start Alignment</button>
        </div>
        <div id="log-container" style="margin-top: 10px; max-height: 200px; overflow-y: auto; border: 1px solid #ccc; padding: 10px; background: #f9f9f9; font-size: 14px;">
            <strong>Status Log:</strong>
            <ul id="log-list" style="list-style: none; padding: 0; margin: 0;"></ul>
        </div>
    </div>
    <div id="alignment-screen">
        <h2>Manual Control</h2>
        <p>Use the controls below to manually control each cone.</p>

        <div class="cone-controls">
            <h3>Falcone1</h3>
            <div class="control-panel">
                <!-- Movement Controls -->
                <button onclick="sendCommand('Falcone1', 'MOVE_FORWARD')">Forward</button>
                <button onclick="sendCommand('Falcone1', 'MOVE_BACKWARD')">Backward</button>
                <button onclick="sendCommand('Falcone1', 'ROTATE_LEFT')">Rotate Left</button>
                <button onclick="sendCommand('Falcone1', 'ROTATE_RIGHT')">Rotate Right</button>
        
                <!-- Emergency Stop -->
                <button onclick="sendCommand('Falcone1', 'STOP')" class="stop-btn">Emergency Stop</button>
            </div>
        </div>
        <div class="cone-controls">
            <h3>Falcone2</h3>
            <div class="control-panel">
                <!-- Movement Controls -->
                <button onclick="sendCommand('Falcone2', 'MOVE_FORWARD')">Forward</button>
                <button onclick="sendCommand('Falcone2', 'MOVE_BACKWARD')">Backward</button>
                <button onclick="sendCommand('Falcone2', 'ROTATE_LEFT')">Rotate Left</button>
                <button onclick="sendCommand('Falcone2', 'ROTATE_RIGHT')">Rotate Right</button>
        
                <!-- Emergency Stop -->
                <button onclick="sendCommand('Falcone2', 'STOP')" class="stop-btn">Emergency Stop</button>
            </div>
        </div>
    
        <!-- Controls for Self -->
        <div class="cone-controls">
            <h3>Falcone3</h3>
            <div class="control-panel">
                <!-- Movement Controls -->
                <button onclick="sendCommand('Self', 'MOVE_FORWARD')">Forward</button>
                <button onclick="sendCommand('Self', 'MOVE_BACKWARD')">Backward</button>
                <button onclick="sendCommand('Self', 'ROTATE_LEFT')">Rotate Left</button>
                <button onclick="sendCommand('Self', 'ROTATE_RIGHT')">Rotate Right</button>
        
                <!-- Emergency Stop -->
                <button onclick="sendCommand('Self', 'STOP')" class="stop-btn">Emergency Stop</button>
            </div>
        </div>
    
        <!-- Exit Button -->
        <button onclick="exitAlignment()" class="control-btn" style="background: #f44336; color: white;">Exit Alignment</button>
    </div>
    <script src="https://unpkg.com/leaflet@1.7.1/dist/leaflet.js"></script>
    <script>
        // Use the hostname of the device serving the HTML file
        const selfIP = window.location.hostname;

        // Define the IP addresses of the other devices
        const falcone1IP = "192.168.254.106"; // Replace with the actual IP of Falcone1
        const falcone2IP = "192.168.254.107"; // Replace with the actual IP of Falcone2

        // Create WebSocket connections for each device
        const wsSelf = new WebSocket(`ws://${selfIP}:81/ws`);
        const wsFalcone1 = new WebSocket(`ws://${falcone1IP}:81/ws`);
        const wsFalcone2 = new WebSocket(`ws://${falcone2IP}:81/ws`);

        // Initialize map
        const map = L.map('map', {
            zoomControl: true,
            tap: false,
            dragging: true
        }).setView([0, 0], 2);

        // Define map layers
        const streets = L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
            attribution: '&copy; OpenStreetMap contributors',
            maxZoom: 19
        }).addTo(map);

        const satellite = L.tileLayer('https://{s}.google.com/vt/lyrs=s&x={x}&y={y}&z={z}', {
            subdomains: ['mt0', 'mt1', 'mt2', 'mt3'],
            maxZoom: 20
        });

        let currentLayer = 'streets';
        document.getElementById('toggle-map').addEventListener('click', function () {
            if (currentLayer === 'streets') {
                map.removeLayer(streets);
                map.addLayer(satellite);
                this.textContent = "Street View";
                currentLayer = 'satellite';
            } else {
                map.removeLayer(satellite);
                map.addLayer(streets);
                this.textContent = "Satellite View";
                currentLayer = 'streets';
            }
        });

        // Initialize markers and positions
        const positions = {
            "Self": null,
            "Falcone1": null,
            "Falcone2": null
        };

        const markers = {
            "Self": L.marker([0, 0], {
                icon: createIcon('self'),
                title: "Self"
            }).addTo(map).bindPopup("Waiting for data..."),

            "Falcone1": L.marker([0, 0], {
                icon: createIcon('falcone1'),
                title: "Falcone1"
            }).addTo(map).bindPopup("Waiting for data..."),

            "Falcone2": L.marker([0, 0], {
                icon: createIcon('falcone2'),
                title: "Falcone2"
            }).addTo(map).bindPopup("Waiting for data..."),
        };

        function sendCommand(deviceID, command) {
            let ws;
            if (deviceID === "Self") {
                ws = wsSelf;
            } else if (deviceID === "Falcone1") {
                ws = wsFalcone1;
            } else if (deviceID === "Falcone2") {
                ws = wsFalcone2;
            } else {
                console.error("Invalid device ID:", deviceID);
                return;
            }

            if (ws.readyState === WebSocket.OPEN) {
                ws.send(command);
                console.log(`Command sent to ${deviceID}: ${command}`);
                showNotification(`Command sent to ${deviceID}: ${command}`);
            } else {
                console.error(`WebSocket for ${deviceID} is not open`);
                showNotification(`WebSocket for ${deviceID} is not open`);
            }
        }

        // WebSocket event handlers for each device
        function setupWebSocket(ws, deviceID) {
            ws.onopen = () => {
                console.log(`Connected to ${deviceID}`);
                showNotification(`Connected to ${deviceID}`);
                updateConnectionStatus(true);
            };

                        // Store the last known statuses for each device
            const lastStatuses = {
                orientation: {},
                battery: {}
            };

            ws.onmessage = (event) => {
                try {
                    const data = JSON.parse(event.data);

                    if (data.deviceID === deviceID) {
                        const newPos = [data.lat, data.lng];
                        if (markers[deviceID]) {
                            markers[deviceID].setLatLng(newPos);
                        } else {
                            markers[deviceID] = L.marker(newPos, { title: deviceID }).addTo(map);
                        }

                        const popupContent = `
                            <div>
                                <b>${data.deviceID}</b><br>
                                Latitude: ${data.lat.toFixed(6)}<br>
                                Longitude: ${data.lng.toFixed(6)}<br>
                                Status: ${["Normal", "Tilted", "Flipped"][data.status]}<br>
                                Temperature: ${data.temp.toFixed(2)} °C<br>
                                Battery: ${["Critical", "Low", "Good"][data.battery]}
                            </div>
                        `;
                        markers[deviceID].bindPopup(popupContent);

                        positions[deviceID] = newPos;

                        // Check and log orientation status if changed
                        const orientationStatus = ["Normal", "Tilted", "Flipped"][data.status];
                        if (lastStatuses.orientation[deviceID] !== orientationStatus) {
                            showNotification(`${deviceID} orientation: ${orientationStatus}`, true);
                            lastStatuses.orientation[deviceID] = orientationStatus;
                        }

                        // Check and log battery status if changed
                        const batteryStatus = ["Critical", "Low", "Good"][data.battery];
                        if (lastStatuses.battery[deviceID] !== batteryStatus) {
                            showNotification(`${deviceID} battery status: ${batteryStatus}`, true);
                            lastStatuses.battery[deviceID] = batteryStatus;
                        }

                        updateDistanceDisplay();
                    }
                } catch (e) {
                    console.error(`Error parsing WebSocket message from ${deviceID}:`, e);
                }
            };

            ws.onclose = () => {
                console.log(`Disconnected from ${deviceID}`);
                showNotification(`Disconnected from ${deviceID}`);
                updateConnectionStatus(false);

                        // Update the popup to show "Waiting for connection"
                if (markers[deviceID]) {
                    markers[deviceID].bindPopup("Waiting for connection").openPopup();
                }
            };

            ws.onerror = (error) => {
                console.error(`WebSocket error for ${deviceID}:`, error);
                updateConnectionStatus(false);
            };
        }

        // Setup WebSocket connections
        setupWebSocket(wsSelf, "Self");
        setupWebSocket(wsFalcone1, "Falcone1");
        setupWebSocket(wsFalcone2, "Falcone2");

        // Distance calculation function (Haversine formula)
        function calculateDistance(lat1, lon1, lat2, lon2) {
            const earthRadius = 6371000; // Earth radius in meters

            // Convert degrees to radians
            const lat1Rad = lat1 * Math.PI / 180;
            const lat2Rad = lat2 * Math.PI / 180;
            const latDiffRad = (lat2 - lat1) * Math.PI / 180;
            const lonDiffRad = (lon2 - lon1) * Math.PI / 180;

            // Haversine formula
            const a = Math.sin(latDiffRad / 2) * Math.sin(latDiffRad / 2) +
                Math.cos(lat1Rad) * Math.cos(lat2Rad) *
                Math.sin(lonDiffRad / 2) * Math.sin(lonDiffRad / 2);

            const c = 2 * Math.atan2(Math.sqrt(a), Math.sqrt(1 - a));
            return earthRadius * c; // Distance in meters
        }

        // Update distance display
        function updateDistanceDisplay() {
            if (positions.Self && positions.Falcone1 && positions.Falcone2) {
                // Calculate distances
                const distSelfF1 = calculateDistance(
                    positions.Self[0], positions.Self[1],
                    positions.Falcone1[0], positions.Falcone1[1]
                );

                const distF1F2 = calculateDistance(
                    positions.Falcone1[0], positions.Falcone1[1],
                    positions.Falcone2[0], positions.Falcone2[1]
                );

                // // Update UI
                // document.getElementById('dist-self-f1').textContent = formatDistance(distSelfF1);
                // document.getElementById('dist-f1-f2').textContent = formatDistance(distF1F2);
            }
        }

        function formatDistance(meters) {
            if (meters >= 1000) {
                return (meters / 1000).toFixed(2) + ' km';
            } else {
                return Math.round(meters) + ' m';
            }
        }

        function createIcon(className) {
            return L.divIcon({
                className: `device-icon ${className}`,
                html: `<svg xmlns="http://www.w3.org/2000/svg" width="30" height="30" viewBox="0 0 16 16">
                    <path fill="currentColor" stroke="black" stroke-width="0.5"
                        d="m9.97 4.88.953 3.811C10.159 8.878 9.14 9 8 9s-2.158-.122-2.923-.309L6.03 4.88C6.635 4.957 7.3 5 8 5s1.365-.043 1.97-.12m-.245-.978L8.97.88C8.718-.13 7.282-.13 7.03.88L6.275 3.9C6.8 3.965 7.382 4 8 4s1.2-.036 1.725-.098m4.396 8.613a.5.5 0 0 1 .037.96l-6 2a.5.5 0 0 1-.316 0l-6-2a.5.5 0 0 1 .037-.96l2.391-.598.565-2.257c.862.212 1.964.339 3.165.339s2.303-.127 3.165-.339l.565 2.257z"/>
                    </svg>`,
                iconSize: [30, 30],
                iconAnchor: [15, 30]
            });
        }

        function showNotification(message) {
            console.log(message); // Log the notification
            const logList = document.getElementById('log-list');
            const logItem = document.createElement('li');
            logItem.textContent = message;
            logList.appendChild(logItem);
            logList.scrollTop = logList.scrollHeight;
        }

        function updateConnectionStatus(connected) {
            const statusElement = document.getElementById('connection-status');
            if (connected) {
                statusElement.textContent = "Connected";
                statusElement.className = "connected";
            } else {
                statusElement.textContent = "Disconnected";
                statusElement.className = "disconnected";
            }
        }

        function centerMap(deviceID) {
            if (positions[deviceID]) {
                map.setView(positions[deviceID], 18);
                showNotification(`Centered map on ${deviceID}`);
            } else {
                showNotification(`No position data available for ${deviceID}`);
            }
        }

        function startAlignment() {
            document.getElementById('alignment-screen').style.display = 'block';
            showNotification("Entered alignment mode");

            // Send the "START_ALIGNMENT" command to all devices
            sendCommand('Self', 'START_ALIGNMENT');
            sendCommand('Falcone1', 'START_ALIGNMENT');
            sendCommand('Falcone2', 'START_ALIGNMENT');
        }

        function exitAlignment() {
            document.getElementById('alignment-screen').style.display = 'none';
            showNotification("Exited alignment mode");

            // Send the "RESUME_GPS" command to all devices
            sendCommand('Self', 'RESUME_GPS');
            sendCommand('Falcone1', 'RESUME_GPS');
            sendCommand('Falcone2', 'RESUME_GPS');
        }

        function showNotification(message, playSound = false) {
            // Log the message with a timestamp
            const logList = document.getElementById('log-list');
            const logEntry = document.createElement('li');
            const timestamp = new Date().toLocaleTimeString();
            logEntry.textContent = `[${timestamp}] ${message}`;
            logList.appendChild(logEntry);

            // Keep the log container scrolled to the bottom
            const logContainer = document.getElementById('log-container');
            logContainer.scrollTop = logContainer.scrollHeight;

            // Play notification sound only if playSound is true
            if (playSound) {
                const audio = new Audio('https://github.com/Senchou-yujin/falcone-capstone_project/raw/refs/heads/main/Falcone/Tracking-Maps/sound.wav');
                audio.play().catch(e => console.log("Audio play failed:", e));
            }
        }
    </script>
</body>
</html>