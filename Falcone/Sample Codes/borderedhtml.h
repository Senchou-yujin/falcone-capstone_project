<!DOCTYPE html>
<html>
<head>
    <title>FALCONE Tracker</title>
    <link rel="stylesheet" href="https://unpkg.com/leaflet@1.7.1/dist/leaflet.css" />
    <style>
        body {
            margin: 0;
            font-family: Arial, sans-serif;
            background: #f0f0f0;
            padding: 20px;
        }
        #container {
            border: 2px solid #333;
            border-radius: 8px;
            overflow: hidden;
            box-shadow: 0 0 15px rgba(0,0,0,0.1);
            height: calc(100vh - 40px);
            display: flex;
            flex-direction: column;
        }
        #map { 
            width: 100%; 
            height: calc(100% - 60px);
            flex-grow: 1;
        }
        .falcone1 { color: red; }
        .falcone2 { color: blue; }
        .self { color: green; }
        
        /* Control Panel */
        #control-panel {
            display: flex;
            justify-content: center;
            gap: 10px;
            padding: 10px;
            background: #f5f5f5;
            border-top: 1px solid #ddd;
        }
        .control-btn {
            padding: 8px 15px;
            background: white;
            border: 1px solid #ccc;
            border-radius: 4px;
            cursor: pointer;
            transition: all 0.2s;
            min-width: 100px;
            text-align: center;
        }
        .control-btn:hover {
            background: #e9e9e9;
            transform: translateY(-1px);
        }
        #center-self { border-left: 4px solid green; }
        #center-falcone1 { border-left: 4px solid red; }
        #center-falcone2 { border-left: 4px solid blue; }
        
        /* Status indicator */
        #connection-status {
            position: absolute;
            top: 30px;
            right: 30px;
            background: white;
            padding: 5px 10px;
            border-radius: 4px;
            z-index: 1000;
            box-shadow: 0 0 5px rgba(0,0,0,0.2);
            border: 1px solid #eee;
        }
        .connected { color: green; }
        .disconnected { color: red; }
        
        /* Title bar */
        #title-bar {
            background: #333;
            color: white;
            padding: 8px 15px;
            font-weight: bold;
        }
    </style>
</head>
<body>
    <div id="container">
        <div id="title-bar">FALCONE TRACKING SYSTEM</div>
        <div id="connection-status" class="disconnected">Disconnected</div>
        <div id="map"></div>
        
        <div id="control-panel">
            <button id="center-self" class="control-btn">Center: Self</button>
            <button id="center-falcone1" class="control-btn">Center: Falcone1</button>
            <button id="center-falcone2" class="control-btn">Center: Falcone2</button>
        </div>
    </div>

    <script src="https://unpkg.com/leaflet@1.7.1/dist/leaflet.js"></script>
    <script>
        // Initialize map
        const map = L.map('map').setView([0, 0], 2); // Default zoomed out view
        
        // Base map layer
        L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
            attribution: '&copy; <a href="https://www.openstreetmap.org/copyright">OpenStreetMap</a>'
        }).addTo(map);
        
        // Store last known positions
        const positions = {
            "Self": null,
            "Falcone1": null,
            "Falcone2": null
        };

        // Create markers
        const markers = {
            "Falcone1": L.marker([0, 0], { 
                icon: createIcon('falcone1'),
                title: "Falcone1"
            }).addTo(map),
            "Falcone2": L.marker([0, 0], {
                icon: createIcon('falcone2'),
                title: "Falcone2"
            }).addTo(map),
            "Self": L.marker([0, 0], {
                icon: createIcon('self'),
                title: "Self (Leader)"
            }).addTo(map)
        };

        function createIcon(className) {
            return L.divIcon({
                className: `device-icon ${className}`,
                html: `<svg xmlns="http://www.w3.org/2000/svg" width="40" height="40" viewBox="0 0 16 16">
                    <path fill="currentColor" stroke="black" stroke-width="0.5"
                        d="m9.97 4.88.953 3.811C10.159 8.878 9.14 9 8 9s-2.158-.122-2.923-.309L6.03 4.88C6.635 4.957 7.3 5 8 5s1.365-.043 1.97-.12m-.245-.978L8.97.88C8.718-.13 7.282-.13 7.03.88L6.275 3.9C6.8 3.965 7.382 4 8 4s1.2-.036 1.725-.098m4.396 8.613a.5.5 0 0 1 .037.96l-6 2a.5.5 0 0 1-.316 0l-6-2a.5.5 0 0 1 .037-.96l2.391-.598.565-2.257c.862.212 1.964.339 3.165.339s2.303-.127 3.165-.339l.565 2.257z"/>
                    </svg>`,
                iconSize: [40, 40],
                iconAnchor: [20, 40]
            });
        }

        // Control Panel Functions
        function centerOnDevice(deviceId) {
            if (positions[deviceId]) {
                map.setView(positions[deviceId], 18);
                flashButton(`center-${deviceId.toLowerCase()}`);
            }
        }

        document.getElementById('center-self').addEventListener('click', () => centerOnDevice('Self'));
        document.getElementById('center-falcone1').addEventListener('click', () => centerOnDevice('Falcone1'));
        document.getElementById('center-falcone2').addEventListener('click', () => centerOnDevice('Falcone2'));

        // Button flash animation
        function flashButton(btnId) {
            const btn = document.getElementById(btnId);
            btn.style.backgroundColor = '#d4edda';
            setTimeout(() => {
                btn.style.backgroundColor = 'white';
            }, 300);
        }

        // WebSocket connection
        const statusElement = document.getElementById('connection-status');
        const ws = new WebSocket(`ws://${window.location.hostname}:81/`);

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
            try {
                const data = JSON.parse(event.data);
                
                data.devices.forEach(device => {
                    if (device.lat !== 0 && device.lng !== 0) {
                        const newPos = [device.lat, device.lng];
                        markers[device.id].setLatLng(newPos);
                        positions[device.id] = newPos;
                        
                        markers[device.id].setPopupContent(`
                            <div style="min-width: 150px">
                                <b>${device.id}</b><br>
                                Lat: ${device.lat.toFixed(6)}<br>
                                Lng: ${device.lng.toFixed(6)}
                            </div>
                        `);
                    }
                });
                
            } catch (e) {
                console.error("Data error:", e);
            }
        };
    </script>
</body>
</html>