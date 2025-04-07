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
        .control-btn {
            padding: 10px 12px;
            background: white;
            border: 1px solid #ccc;
            border-radius: 4px;
            cursor: pointer;
            transition: all 0.2s;
            flex: 1 1 30%;
            text-align: center;
            font-size: 14px;
            min-width: 0;
        }
        .control-btn:hover {
            background: #e9e9e9;
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
        </div>
    </div>

    <script src="https://unpkg.com/leaflet@1.7.1/dist/leaflet.js"></script>
    <script>
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
            }).addTo(map),
            "Falcone2": L.marker([0, 0], {
                icon: createIcon('falcone2'),
                title: "Falcone3"
            }).addTo(map),
            "Self": L.marker([0, 0], {
                icon: createIcon('self'),
                title: "Falcone 1"
            }).addTo(map)
        };

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

        document.getElementById('center-self').addEventListener('click', () => centerOnDevice('Self'));
        document.getElementById('center-falcone1').addEventListener('click', () => centerOnDevice('Falcone1'));
        document.getElementById('center-falcone2').addEventListener('click', () => centerOnDevice('Falcone2'));

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

        window.addEventListener('resize', () => {
            map.invalidateSize();
        });
    </script>
</body>
</html>
