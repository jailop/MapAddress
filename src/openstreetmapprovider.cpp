#include "openstreetmapprovider.h"

OpenStreetMapProvider::OpenStreetMapProvider(QObject* parent)
    : MapProvider(parent), m_centerLat(0.0), m_centerLng(0.0), m_zoom(13) {
}

void OpenStreetMapProvider::initialize() {
    m_markers.clear();
    m_centerLat = 0.0;
    m_centerLng = 0.0;
    m_zoom = 2;
}

void OpenStreetMapProvider::setCenter(double latitude, double longitude, int zoom) {
    m_centerLat = latitude;
    m_centerLng = longitude;
    m_zoom = zoom;
}

void OpenStreetMapProvider::addMarker(int id, double latitude, double longitude, const QString& title) {
    QString markerData = QString("%1,%2,%3").arg(latitude).arg(longitude).arg(title);
    m_markers[id] = markerData;
}

void OpenStreetMapProvider::removeMarker(int id) {
    m_markers.remove(id);
}

void OpenStreetMapProvider::clearMarkers() {
    m_markers.clear();
}

void OpenStreetMapProvider::fitBounds(const QList<Address>& addresses) {
    if (addresses.isEmpty()) return;

    double minLat = 90.0, maxLat = -90.0;
    double minLng = 180.0, maxLng = -180.0;

    for (const Address& addr : addresses) {
        if (addr.hasCoordinates()) {
            double lat = addr.getLatitude();
            double lng = addr.getLongitude();
            if (lat < minLat) minLat = lat;
            if (lat > maxLat) maxLat = lat;
            if (lng < minLng) minLng = lng;
            if (lng > maxLng) maxLng = lng;
        }
    }

    m_centerLat = (minLat + maxLat) / 2.0;
    m_centerLng = (minLng + maxLng) / 2.0;
    
    // Calculate appropriate zoom level based on bounds
    double latDiff = maxLat - minLat;
    double lngDiff = maxLng - minLng;
    double maxDiff = qMax(latDiff, lngDiff);
    
    // Estimate zoom level (rough approximation)
    if (maxDiff > 180) m_zoom = 1;
    else if (maxDiff > 90) m_zoom = 2;
    else if (maxDiff > 45) m_zoom = 3;
    else if (maxDiff > 22) m_zoom = 4;
    else if (maxDiff > 11) m_zoom = 5;
    else if (maxDiff > 5) m_zoom = 6;
    else if (maxDiff > 2.5) m_zoom = 7;
    else if (maxDiff > 1.2) m_zoom = 8;
    else if (maxDiff > 0.6) m_zoom = 9;
    else if (maxDiff > 0.3) m_zoom = 10;
    else if (maxDiff > 0.15) m_zoom = 11;
    else if (maxDiff > 0.075) m_zoom = 12;
    else if (maxDiff > 0.035) m_zoom = 13;
    else if (maxDiff > 0.015) m_zoom = 14;
    else m_zoom = 15;
}

QString OpenStreetMapProvider::getHtml() const {
    return generateHtml();
}

QString OpenStreetMapProvider::generateHtml() const {
    QString markers;
    for (auto it = m_markers.constBegin(); it != m_markers.constEnd(); ++it) {
        QStringList parts = it.value().split(',');
        if (parts.size() >= 3) {
            QString title = parts.mid(2).join(',');
            // Escape single quotes in title
            QString escapedTitle = title;
            escapedTitle.replace("'", "\\'");
            markers += QString(R"(
                {
                    id: %1,
                    lat: %2,
                    lng: %3,
                    title: '%4'
                },
            )").arg(it.key()).arg(parts[0]).arg(parts[1]).arg(escapedTitle);
        }
    }

    QString html = QString(R"(
<!DOCTYPE html>
<html>
<head>
    <meta charset="utf-8">
    <link rel="stylesheet" href="https://unpkg.com/leaflet@1.9.4/dist/leaflet.css" />
    <script src="https://unpkg.com/leaflet@1.9.4/dist/leaflet.js"></script>
    <style>
        html, body, #map { height: 100%; margin: 0; padding: 0; }
        .custom-marker {
            background-color: #2563eb;
            border: 2px solid #ffffff;
            border-radius: 50%;
            width: 16px;
            height: 16px;
            transition: all 0.3s ease;
        }
        .custom-marker.highlighted {
            background-color: #ef4444;
            width: 24px;
            height: 24px;
            box-shadow: 0 0 10px rgba(239, 68, 68, 0.8);
        }
    </style>
</head>
<body>
    <div id="map"></div>
    <script>
        // Initialize map immediately - don't wait for anything
        console.log('Starting map initialization...');
        console.log('Map container element:', document.getElementById('map'));
        console.log('Leaflet available:', typeof L !== 'undefined');
        const map = L.map('map').setView([%1, %2], %3);
        console.log('Map created:', map);

        L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
            attribution: '&copy; <a href="https://www.openstreetmap.org/copyright">OpenStreetMap</a> contributors',
            maxZoom: 19
        }).addTo(map);
        console.log('Tile layer added');
        
        // Force map to recalculate size after a short delay
        setTimeout(function() {
            map.invalidateSize();
            console.log('Map size invalidated, tiles should load now');
        }, 100);

        // Custom marker icon
        const customIcon = L.divIcon({
            className: 'custom-marker',
            iconSize: [16, 16],
            iconAnchor: [8, 8],
            popupAnchor: [0, -8]
        });
        
        const highlightedIcon = L.divIcon({
            className: 'custom-marker highlighted',
            iconSize: [24, 24],
            iconAnchor: [12, 12],
            popupAnchor: [0, -12]
        });

        const markerData = [%4];
        const markers = {};
        let currentHighlighted = null;

        markerData.forEach(data => {
            const marker = L.marker([data.lat, data.lng], { icon: customIcon })
                .addTo(map)
                .bindPopup('<div style="min-width: 150px;"><strong>' + data.title + '</strong></div>');

            // Show tooltip on hover
            marker.bindTooltip(data.title, {
                permanent: false,
                direction: 'top',
                offset: [0, -10]
            });

            marker.on('click', () => {
                if (window.qtBridge) {
                    window.qtBridge.markerClicked(data.id);
                }
            });

            markers[data.id] = marker;
        });
        
        // Function to highlight a specific marker
        window.highlightMarker = function(markerId) {
            // Reset previous highlight
            if (currentHighlighted !== null && markers[currentHighlighted]) {
                markers[currentHighlighted].setIcon(customIcon);
            }
            
            // Highlight new marker
            if (markers[markerId]) {
                markers[markerId].setIcon(highlightedIcon);
                markers[markerId].openPopup();
                currentHighlighted = markerId;
            }
        };

        map.on('click', (e) => {
            if (window.qtBridge) {
                window.qtBridge.mapClicked(e.latlng.lat, e.latlng.lng);
            }
        });
        
        // Route planning
        let routeLayer = null;
        
        window.showRoute = function(startLat, startLng, endLat, endLng) {
            // Clear existing route
            if (routeLayer) {
                map.removeLayer(routeLayer);
            }
            
            // Draw straight line route
            const latlngs = [
                [startLat, startLng],
                [endLat, endLng]
            ];
            
            routeLayer = L.polyline(latlngs, {
                color: '#ef4444',
                weight: 4,
                opacity: 0.7,
                dashArray: '10, 5'
            }).addTo(map);
            
            // Fit bounds to show the route
            map.fitBounds(routeLayer.getBounds(), { padding: [50, 50] });
            
            console.log('Route displayed from', startLat, startLng, 'to', endLat, endLng);
        };
        
        window.showRoutePolyline = function(routePoints) {
            // Clear existing route
            if (routeLayer) {
                map.removeLayer(routeLayer);
            }
            
            // Draw road-based route through multiple waypoints
            routeLayer = L.polyline(routePoints, {
                color: '#3b82f6',
                weight: 5,
                opacity: 0.8
            }).addTo(map);
            
            // Fit bounds to show the entire route
            map.fitBounds(routeLayer.getBounds(), { padding: [50, 50] });
            
            console.log('Route polyline displayed with', routePoints.length, 'points');
        };
        
        window.clearRoute = function() {
            if (routeLayer) {
                map.removeLayer(routeLayer);
                routeLayer = null;
                console.log('Route cleared');
            }
        };
    </script>
    <script src="qrc:///qtwebchannel/qwebchannel.js"></script>
    <script>
        // Initialize QWebChannel when available
        if (typeof QWebChannel !== 'undefined' && typeof qt !== 'undefined') {
            new QWebChannel(qt.webChannelTransport, function(channel) {
                window.qtBridge = channel.objects.qtBridge;
                console.log('QWebChannel initialized successfully');
            });
        } else {
            console.log('QWebChannel not available (running outside Qt)');
        }
    </script>
</body>
</html>
    )").arg(m_centerLat).arg(m_centerLng).arg(m_zoom).arg(markers);

    return html;
}
