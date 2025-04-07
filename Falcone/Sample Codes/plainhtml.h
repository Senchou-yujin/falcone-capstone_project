<!DOCTYPE html>
<html>
<head>
    <title>FALCONE Tracker</title>
    <link rel="stylesheet" href="https://unpkg.com/leaflet@1.7.1/dist/leaflet.css" />
    <style>
        #map { width: 100%; height: 900px; }
        .falcone1 { color: red; }  /* Red for Falcone1 */
        .falcone2 { color: blue; } /* Blue for Falcone2 */
        .self { color: green; }    /* Green for Self */
    </style>
</head>
<body>
    <div id="map"></div>

    <script src="https://unpkg.com/leaflet@1.7.1/dist/leaflet.js"></script>
    <script>
        // Initialize map (centered at 0,0 by default)
        const map = L.map('map').setView([0, 0], 15);
        L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png').addTo(map);

        // Create markers for each device
        const markers = {
            "Falcone1": L.marker([0, 0], { icon: createIcon('falcone1') }).addTo(map),
            "Falcone2": L.marker([0, 0], { icon: createIcon('falcone2') }).addTo(map),
            "Self": L.marker([0, 0], { icon: createIcon('self') }).addTo(map)
        };

        // Function to create cone-shaped icons
        function createIcon(className) {
            return L.divIcon({
                className: `device-icon ${className}`,
                html: `
                    <svg xmlns="http://www.w3.org/2000/svg" width="40" height="40" viewBox="0 0 16 16">
                        <path fill="currentColor" stroke="black" stroke-width="0.5"
                            d="m9.97 4.88.953 3.811C10.159 8.878 9.14 9 8 9s-2.158-.122-2.923-.309L6.03 4.88C6.635 4.957 7.3 5 8 5s1.365-.043 1.97-.12m-.245-.978L8.97.88C8.718-.13 7.282-.13 7.03.88L6.275 3.9C6.8 3.965 7.382 4 8 4s1.2-.036 1.725-.098m4.396 8.613a.5.5 0 0 1 .037.96l-6 2a.5.5 0 0 1-.316 0l-6-2a.5.5 0 0 1 .037-.96l2.391-.598.565-2.257c.862.212 1.964.339 3.165.339s2.303-.127 3.165-.339l.565 2.257z"/>
                    </svg>`,
                iconSize: [40, 40],
                iconAnchor: [20, 40]
            });
        }

        // Connect to ESP32 WebSocket
        const ws = new WebSocket(`ws://${window.location.hostname}:81/`);

        // Update markers when new data arrives
        ws.onmessage = (event) => {
            const data = JSON.parse(event.data);
            data.devices.forEach(device => {
                if (device.lat !== 0 && device.lng !== 0) {
                    markers[device.id].setLatLng([device.lat, device.lng]);
                    markers[device.id].bindPopup(`
                        <b>${device.id}</b><br>
                        Lat: ${device.lat.toFixed(6)}<br>
                        Lng: ${device.lng.toFixed(6)}<br>
                        Alt: ${device.alt.toFixed(2)}m
                    `);
                    if (device.id === "Self") map.setView([device.lat, device.lng], 15);
                }
            });
        };
    </script>
</body>
</html>