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
            <button id="center-self" class="control-btn">Falcone1</button>
            <button id="center-falcone1" class="control-btn">Falcone2</button>
            <button id="center-falcone2" class="control-btn">Falcone3</button>
            <button id="toggle-map" class="control-btn">Satellite View</button>
            <button onclick="startAlignment()" class="control-btn" style="flex: 1 1 100%;">Start Alignment</button>
        </div>
        <div id="log-container" style="margin-top: 10px; max-height: 200px; overflow-y: auto; border: 1px solid #ccc; padding: 10px; background: #f9f9f9; font-size: 14px;">
            <strong>Status Log:</strong>
            <ul id="log-list" style="list-style: none; padding: 0; margin: 0;"></ul>
        </div>
    </div>
    <div id="alignment-screen" style="display: none; position: absolute; top: 0; left: 0; width: 100%; height: 100%; background: #f0f0f0; z-index: 1000; padding: 20px; text-align: center;">
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
        // Initialize WebSocket at the top
    const ws = new WebSocket(`ws://${window.location.hostname}:81/`);

        // WebSocket event handlers
        const statusElement = document.getElementById('connection-status');

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

        const positions = {
            "Self": null,
            "Falcone1": null,
            "Falcone2": null
        };

        const markers = {
            "Falcone1": L.marker([0, 0], {
                icon: createIcon('falcone1'),
                title: "Falcone2"
            }).addTo(map).bindPopup("Waiting for data..."),

            "Falcone2": L.marker([0, 0], {
                icon: createIcon('falcone2'),
                title: "Falcone3"
            }).addTo(map).bindPopup("Waiting for data..."),

            "Self": L.marker([0, 0], {
                icon: createIcon('self'),
                title: "Falcone1"
            }).addTo(map).bindPopup("Waiting for data..."),
        };

        // Distance calculation function (Haversine formula)
        function calculateDistance(lat1, lon1, lat2, lon2) {
            const earthRadius = 6371000; // Earth radius in meters
            
            // Convert degrees to radians
            const lat1Rad = lat1 * Math.PI / 180;
            const lat2Rad = lat2 * Math.PI / 180;
            const latDiffRad = (lat2 - lat1) * Math.PI / 180;
            const lonDiffRad = (lon2 - lon1) * Math.PI / 180;

            // Haversine formula
            const a = Math.sin(latDiffRad/2) * Math.sin(latDiffRad/2) +
                    Math.cos(lat1Rad) * Math.cos(lat2Rad) *
                    Math.sin(lonDiffRad/2) * Math.sin(lonDiffRad/2);
            
            const c = 2 * Math.atan2(Math.sqrt(a), Math.sqrt(1-a));
            return earthRadius * c; // Distance in meters
        }

        function startAlignment() {
            if (ws && ws.readyState === WebSocket.OPEN) {
                ws.send("Self:START_ALIGN"); // Send the command in the correct format
                showNotification("Alignment started.");
                showAlignmentScreen(); // Show the alignment screen immediately
            } else {
                showNotification("WebSocket is not connected.");
            }
        }

        function stopAlignment() {
            if (ws && ws.readyState === WebSocket.OPEN) {
                ws.send("STOP_ALIGN");
                showNotification("Alignment stopped by Self.");
            } else {
                showNotification("WebSocket is not connected.");
            }
        }

        // Update distance display
        function updateDistanceDisplay() {
            if (positions.Self && positions.Falcone1 && positions.Falcone2) {
                // Calculate Self to Falcone1 distance
                const distSelfF1 = calculateDistance(
                    positions.Self[0], positions.Self[1],
                    positions.Falcone1[0], positions.Falcone1[1]
                );
                
                // Calculate Falcone1 to Falcone2 distance
                const distF1F2 = calculateDistance(
                    positions.Falcone1[0], positions.Falcone1[1],
                    positions.Falcone2[0], positions.Falcone2[1]
                );

                // Update UI
                document.getElementById('dist-self-f1').textContent = formatDistance(distSelfF1);
                document.getElementById('dist-f1-f2').textContent = formatDistance(distF1F2);

                // Update popups with distance info
                updatePopupsWithDistances(distSelfF1, 0, distF1F2); // 0 for unused Self-Falcone2 distance
            }
        }
        function formatDistance(meters) {
            if (meters >= 1000) {
                return (meters / 1000).toFixed(2) + ' km';
            } else {
                return Math.round(meters) + ' m';
            }
        }


        function updatePopupsWithDistances(distSelfF1, distSelfF2, distF1F2) {
            // Update Self popup
            markers.Self.setPopupContent(
                markers.Self.getPopup().getContent().replace('</div>', 
                `<hr>Distance to Falcone1: <b>${formatDistance(distSelfF1)}</b></div>`)
            );

            // Update Falcone1 popup
            markers.Falcone1.setPopupContent(
                markers.Falcone1.getPopup().getContent().replace('</div>', 
                `<hr>Distance to Self: <b>${formatDistance(distSelfF1)}</b><br>
                Distance to Falcone2: <b>${formatDistance(distF1F2)}</b></div>`)
            );

            // Update Falcone2 popup
            markers.Falcone2.setPopupContent(
                markers.Falcone2.getPopup().getContent().replace('</div>', 
                `<hr>Distance to Falcone1: <b>${formatDistance(distF1F2)}</b></div>`)
            );
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

        function centerOnDevice(deviceId) {
            if (positions[deviceId]) {
                map.flyTo(positions[deviceId], 18, {
                    duration: 0.5
                });
            }
        }

        function showAlignmentScreen() {
            document.getElementById('alignment-screen').style.display = 'block';
        }

        function exitAlignment() {
            document.getElementById('alignment-screen').style.display = 'none';
            // Optionally send a message to the leader to stop alignment mode
            if (ws && ws.readyState === WebSocket.OPEN) {
                ws.send("STOP_ALIGN");
            }
        }

        function controlCone(coneId) {
            console.log(`Controlling ${coneId}`);
            // Send control commands to the leader ESP32
            if (ws && ws.readyState === WebSocket.OPEN) {
                ws.send(`CONTROL:${coneId}`);
            }
        }

        function sendCommand(cmd) {
            if (ws && ws.readyState === WebSocket.OPEN) {
                ws.send(cmd);
                console.log(`Command sent: ${cmd}`);
            } else {
                console.error("WebSocket is not connected.");
            }
        }

        function sendCommand(target, command) {
            if (ws && ws.readyState === WebSocket.OPEN) {
                const fullCommand = `${target}:${command}`;
                ws.send(fullCommand);
                console.log(`Command sent: ${fullCommand}`);
            } else {
                console.error("WebSocket is not connected.");
            }
        }

        // Show the alignment screen when confirmation is received
        ws.onmessage = (event) => {
            const message = event.data;
            if (message === "ALIGN_START") {
                showAlignmentScreen();
            } else if (message.startsWith("ALIGN:")) {
                const yaw = parseFloat(message.split(":")[1]);
                console.log("Alignment correction received:", yaw);
                // Optionally update the UI with alignment data
            }
        };

        document.getElementById('center-self').addEventListener('click', () => centerOnDevice('Self'));
        document.getElementById('center-falcone1').addEventListener('click', () => centerOnDevice('Falcone1'));
        document.getElementById('center-falcone2').addEventListener('click', () => centerOnDevice('Falcone2'));

        ws.onopen = () => {
            statusElement.textContent = "Connected";
            statusElement.className = "connected";
        };

        ws.onerror = (error) => {
            statusElement.textContent = "Connection Error";
            statusElement.className = "disconnected";
        };

        ws.onclose = () => {
            statusElement.textContent = "Disconnected";
            statusElement.className = "disconnected";
        };

        ws.onmessage = (event) => {
            const message = event.data;
            if (message.startsWith("ALIGN:")) {
                const yaw = parseFloat(message.split(":")[1]);
                console.log("Alignment correction received:", yaw);
                // Update UI or take action based on yaw
            }
        };

        // Function to display a popup notification
        function showNotification(message) {
            // Create a notification container if it doesn't exist
            let notificationContainer = document.getElementById('notification-container');
            if (!notificationContainer) {
                notificationContainer = document.createElement('div');
                notificationContainer.id = 'notification-container';
                notificationContainer.style.position = 'fixed';
                notificationContainer.style.top = '50%';
                notificationContainer.style.left = '50%';
                notificationContainer.style.transform = 'translate(-50%, -50%)';
                notificationContainer.style.zIndex = '1000';
                notificationContainer.style.maxWidth = '300px';
                notificationContainer.style.padding = '15px';
                notificationContainer.style.backgroundColor = '#333';
                notificationContainer.style.color = '#fff';
                notificationContainer.style.borderRadius = '8px';
                notificationContainer.style.boxShadow = '0 0 15px rgba(0, 0, 0, 0.5)';
                notificationContainer.style.fontSize = '16px';
                notificationContainer.style.textAlign = 'center';
                notificationContainer.style.transition = 'opacity 0.5s ease';
                document.body.appendChild(notificationContainer);
            }

            // Set the message and make the notification visible
            notificationContainer.textContent = message;
            notificationContainer.style.opacity = '1';

            // Play a sound
            const audio = new Audio('https://github.com/Senchou-yujin/falcone-capstone_project/raw/refs/heads/main/Falcone/Tracking-Maps/sound.wav'); // Replace with your desired sound URL
            audio.play();

            // Hide the notification after 5 seconds
            setTimeout(() => {
                notificationContainer.style.opacity = '0';
            }, 5000);

            // Log the message with a timestamp
            const logList = document.getElementById('log-list');
            const logEntry = document.createElement('li');
            const timestamp = new Date().toLocaleString();
            logEntry.textContent = `[${timestamp}] ${message}`;
            logList.appendChild(logEntry);

            // Keep the log container scrolled to the bottom
            const logContainer = document.getElementById('log-container');
            logContainer.scrollTop = logContainer.scrollHeight;
        }

        // Track the last known statuses for comparison
        const lastStatuses = {
            "Self": { tilt: null, battery: null },
            "Falcone1": { tilt: null, battery: null },
            "Falcone2": { tilt: null, battery: null }
        };

        // WebSocket message handler
        ws.onmessage = (event) => {
            try {
                const data = JSON.parse(event.data);
                data.devices.forEach(device => {
                    if (device.lat !== 0 && device.lng !== 0) {
                        const newPos = [device.lat, device.lng];
                        markers[device.id].setLatLng(newPos);
                        positions[device.id] = newPos;

                        let popupContent = `<div style="min-width: 150px"><b>${device.id}</b><br>`;
                        popupContent += `Lat: ${device.lat.toFixed(6)}<br>`;
                        popupContent += `Lng: ${device.lng.toFixed(6)}<br>`;

                        // Optional temperature
                        if (device.temp !== undefined) {
                            popupContent += `Temp: ${device.temp.toFixed(1)}°C<br>`;
                        }

                        // Status info
                        const tiltStatus = ["Normal", "Tilted", "Flipped"][device.status] || "Unknown";
                        const batteryStatus = ["Critical", "Low", "Good"][device.battery] || "Unknown";

                        popupContent += `Tilt: <b>${tiltStatus}</b><br>`;
                        popupContent += `Battery: <b>${batteryStatus}</b></div>`;

                        markers[device.id].setPopupContent(popupContent);

                        // Check for changes in tilt or battery status
                        if (lastStatuses[device.id].tilt !== device.status) {
                            showNotification(`${device.id} orientation changed to ${tiltStatus}`);
                        }
                        if (lastStatuses[device.id].battery !== device.battery) {
                            showNotification(`${device.id} battery status changed to ${batteryStatus}`);
                        }

                        // Update the last known statuses
                        lastStatuses[device.id].tilt = device.status;
                        lastStatuses[device.id].battery = device.battery;
                    }
                });

                // Update distances after all positions are updated
                updateDistanceDisplay();

            } catch (e) {
                console.error("Data error:", e);
            }
        };

        window.addEventListener('resize', () => {
            map.invalidateSize();
        });
    </script>
</body>
</html>