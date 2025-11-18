#include "googlemapsprovider.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcessEnvironment>
#include <QSettings>

GoogleMapsProvider::GoogleMapsProvider(QObject* parent)
    : MapProvider(parent), m_centerLat(0.0), m_centerLng(0.0), m_zoom(13) {
}

void GoogleMapsProvider::initialize() {
    m_markers.clear();
    m_centerLat = 0.0;
    m_centerLng = 0.0;
    m_zoom = 2;
}

void GoogleMapsProvider::setCenter(double latitude, double longitude, int zoom) {
    m_centerLat = latitude;
    m_centerLng = longitude;
    m_zoom = zoom;
}

void GoogleMapsProvider::addMarker(int id, double latitude, double longitude, const QString& title) {
    QString markerData = QString("%1,%2,%3").arg(latitude).arg(longitude).arg(title);
    m_markers[id] = markerData;
}

void GoogleMapsProvider::removeMarker(int id) {
    m_markers.remove(id);
}

void GoogleMapsProvider::clearMarkers() {
    m_markers.clear();
}

void GoogleMapsProvider::fitBounds(const QList<Address>& addresses) {
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

QString GoogleMapsProvider::getHtml() const {
    return generateHtml();
}

QString GoogleMapsProvider::generateHtml() const {
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
                    position: { lat: %2, lng: %3 },
                    title: '%4'
                },
            )").arg(it.key()).arg(parts[0]).arg(parts[1]).arg(escapedTitle);
        }
    }

    // Get API key from settings (fallback to environment variable)
    QSettings settings("DataInquiry", "MapAddress");
    QString apiKey = settings.value("Map/GoogleMapsApiKey", "").toString();
    
    // Fallback to environment variable if not in settings
    if (apiKey.isEmpty()) {
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        apiKey = env.value("GOOGLE_MAPS_API_KEY", "");
    }
    
    if (apiKey.isEmpty()) {
        qWarning("Google Maps API key not set in settings or GOOGLE_MAPS_API_KEY environment variable");
    }

    QString html = QString(R"(
<!DOCTYPE html>
<html>
<head>
    <meta charset="utf-8">
    <style>
        html, body, #map { height: 100%; margin: 0; padding: 0; }
        .custom-marker {
            background-color: #EA4335;
            border: 2px solid #ffffff;
            border-radius: 50%;
            width: 20px;
            height: 20px;
            cursor: pointer;
            transition: all 0.3s ease;
        }
        .custom-marker.highlighted {
            background-color: #ef4444;
            width: 28px;
            height: 28px;
            box-shadow: 0 0 15px rgba(239, 68, 68, 0.9);
        }
    </style>
</head>
<body>
    <div id="map"></div>
    <script>
        let map;
        let markers = {};
        let infoWindow;
        let currentHighlighted = null;
        
        const normalIcon = {
            path: google.maps.SymbolPath.CIRCLE,
            scale: 8,
            fillColor: '#EA4335',
            fillOpacity: 1,
            strokeColor: '#ffffff',
            strokeWeight: 2
        };
        
        const highlightedIcon = {
            path: google.maps.SymbolPath.CIRCLE,
            scale: 12,
            fillColor: '#ef4444',
            fillOpacity: 1,
            strokeColor: '#ffffff',
            strokeWeight: 3
        };

        function initMap() {
            map = new google.maps.Map(document.getElementById('map'), {
                center: { lat: %1, lng: %2 },
                zoom: %3,
                mapTypeControl: true,
                streetViewControl: true,
                fullscreenControl: true
            });

            infoWindow = new google.maps.InfoWindow();

            const markerData = [%4];

            markerData.forEach(data => {
                const marker = new google.maps.Marker({
                    position: data.position,
                    map: map,
                    title: data.title,
                    animation: google.maps.Animation.DROP,
                    icon: normalIcon
                });

                // Add info window on marker hover
                marker.addListener('mouseover', () => {
                    infoWindow.setContent('<div style="padding: 5px;"><strong>' + data.title + '</strong></div>');
                    infoWindow.open(map, marker);
                });

                marker.addListener('mouseout', () => {
                    infoWindow.close();
                });

                marker.addListener('click', () => {
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
                    markers[currentHighlighted].setIcon(normalIcon);
                    markers[currentHighlighted].setAnimation(null);
                }
                
                // Highlight new marker
                if (markers[markerId]) {
                    markers[markerId].setIcon(highlightedIcon);
                    markers[markerId].setAnimation(google.maps.Animation.BOUNCE);
                    setTimeout(() => markers[markerId].setAnimation(null), 2000);
                    infoWindow.setContent('<div style="padding: 5px;"><strong>' + markers[markerId].getTitle() + '</strong></div>');
                    infoWindow.open(map, markers[markerId]);
                    currentHighlighted = markerId;
                }
            };

            map.addListener('click', (e) => {
                if (window.qtBridge) {
                    window.qtBridge.mapClicked(e.latLng.lat(), e.latLng.lng());
                }
            });
            
            // Route planning
            let routeLine = null;
            
            window.showRoute = function(startLat, startLng, endLat, endLng) {
                // Clear existing route
                if (routeLine) {
                    routeLine.setMap(null);
                }
                
                // Draw straight line route
                routeLine = new google.maps.Polyline({
                    path: [
                        {lat: startLat, lng: startLng},
                        {lat: endLat, lng: endLng}
                    ],
                    geodesic: true,
                    strokeColor: '#ef4444',
                    strokeOpacity: 0.7,
                    strokeWeight: 4,
                    map: map
                });
                
                // Fit bounds to show the route
                const bounds = new google.maps.LatLngBounds();
                bounds.extend({lat: startLat, lng: startLng});
                bounds.extend({lat: endLat, lng: endLng});
                map.fitBounds(bounds);
                
                console.log('Route displayed');
            };
            
            window.showRoutePolyline = function(routePoints) {
                // Clear existing route
                if (routeLine) {
                    routeLine.setMap(null);
                }
                
                // Convert to Google Maps format
                const path = routePoints.map(p => ({lat: p[0], lng: p[1]}));
                
                // Draw road-based route
                routeLine = new google.maps.Polyline({
                    path: path,
                    geodesic: true,
                    strokeColor: '#3b82f6',
                    strokeOpacity: 0.8,
                    strokeWeight: 5,
                    map: map
                });
                
                // Fit bounds to show entire route
                const bounds = new google.maps.LatLngBounds();
                path.forEach(p => bounds.extend(p));
                map.fitBounds(bounds);
                
                console.log('Route polyline displayed with', routePoints.length, 'points');
            };
            
            window.clearRoute = function() {
                if (routeLine) {
                    routeLine.setMap(null);
                    routeLine = null;
                    console.log('Route cleared');
                }
            };
        }

        window.onerror = function(msg, url, line) {
            console.error('Error: ' + msg + ' at line ' + line);
        };
    </script>
    <script>
        // Load QWebChannel asynchronously - don't block if it fails
        (function() {
            var script = document.createElement('script');
            script.src = 'qrc:///qtwebchannel/qwebchannel.js';
            script.onload = function() {
                if (typeof QWebChannel !== 'undefined') {
                    new QWebChannel(qt.webChannelTransport, function(channel) {
                        window.qtBridge = channel.objects.qtBridge;
                        console.log('QWebChannel initialized');
                    });
                }
            };
            script.onerror = function() {
                console.log('QWebChannel not available (running outside Qt)');
            };
            document.head.appendChild(script);
        })();
    </script>
    <script async defer
        src="https://maps.googleapis.com/maps/api/js?key=%5&callback=initMap">
    </script>
</body>
</html>
    )").arg(m_centerLat).arg(m_centerLng).arg(m_zoom).arg(markers).arg(apiKey);

    return html;
}
