#include "routingservice.h"
#include "logger.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrlQuery>
#include <QUrl>

RoutingService::RoutingService(QObject* parent)
    : QObject(parent), m_networkManager(new QNetworkAccessManager(this)) {
    m_networkManager->setTransferTimeout(30000); // 30 seconds
}

RoutingService::~RoutingService() {
}

void RoutingService::calculateRoute(const QList<Address>& waypoints) {
    if (waypoints.size() < 2) {
        emit routeFailed("At least 2 waypoints are required for routing");
        return;
    }

    // Validate all waypoints have coordinates
    for (const Address& addr : waypoints) {
        if (!addr.hasCoordinates()) {
            emit routeFailed("All waypoints must have valid coordinates");
            return;
        }
    }

    QString url = buildRouteUrl(waypoints);
    LOG_INFO(QString("Requesting route with %1 waypoints").arg(waypoints.size()));
    LOG_DEBUG("Routing URL: " + url);
    
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, "MapAddress/1.0 Qt Application");
    
    QNetworkReply* reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, &RoutingService::onRouteRequestFinished);
}

QString RoutingService::buildRouteUrl(const QList<Address>& waypoints) const {
    // Using OpenRouteService (ORS) API
    // Alternative: OSRM (https://router.project-osrm.org/route/v1/driving/)
    
    // Build coordinates string: lon,lat;lon,lat;...
    QStringList coordinates;
    for (const Address& addr : waypoints) {
        coordinates << QString("%1,%2")
            .arg(addr.getLongitude(), 0, 'f', 6)
            .arg(addr.getLatitude(), 0, 'f', 6);
    }
    
    // Using public OSRM demo server (for testing - consider hosting your own)
    QString url = QString("https://router.project-osrm.org/route/v1/driving/%1?overview=full&geometries=geojson")
        .arg(coordinates.join(";"));
    
    return url;
}

void RoutingService::onRouteRequestFinished() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;

    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        QString errorMsg;
        switch (reply->error()) {
            case QNetworkReply::ConnectionRefusedError:
            case QNetworkReply::HostNotFoundError:
                errorMsg = "Cannot reach routing service. Please check your internet connection.";
                break;
            case QNetworkReply::TimeoutError:
                errorMsg = "Routing request timed out. Please try again.";
                break;
            default:
                errorMsg = "Routing error: " + reply->errorString();
        }
        LOG_ERROR("Routing request failed: " + errorMsg);
        emit routeFailed(errorMsg);
        return;
    }

    QByteArray data = reply->readAll();
    LOG_DEBUG("Routing response: " + QString(data));
    
    QList<QPointF> routePoints = parseRouteResponse(data);
    
    if (routePoints.isEmpty()) {
        LOG_WARNING("No route found in response");
        emit routeFailed("No route found between the waypoints");
        return;
    }
    
    LOG_INFO(QString("Route calculated with %1 points").arg(routePoints.size()));
    emit routeCalculated(routePoints);
}

QList<QPointF> RoutingService::parseRouteResponse(const QByteArray& data) const {
    QList<QPointF> points;
    
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) {
        LOG_ERROR("Invalid routing response: not a JSON object");
        return points;
    }
    
    QJsonObject root = doc.object();
    
    // Check for errors
    if (root.contains("code")) {
        QString code = root["code"].toString();
        if (code != "Ok") {
            LOG_ERROR("Routing service returned error: " + code);
            return points;
        }
    }
    
    // Parse OSRM GeoJSON response
    if (!root.contains("routes") || !root["routes"].isArray()) {
        LOG_ERROR("No routes in response");
        return points;
    }
    
    QJsonArray routes = root["routes"].toArray();
    if (routes.isEmpty()) {
        LOG_ERROR("Routes array is empty");
        return points;
    }
    
    QJsonObject route = routes[0].toObject();
    if (!route.contains("geometry") || !route["geometry"].isObject()) {
        LOG_ERROR("No geometry in route");
        return points;
    }
    
    QJsonObject geometry = route["geometry"].toObject();
    if (!geometry.contains("coordinates") || !geometry["coordinates"].isArray()) {
        LOG_ERROR("No coordinates in geometry");
        return points;
    }
    
    QJsonArray coordinates = geometry["coordinates"].toArray();
    
    // Coordinates are in [lon, lat] format
    for (const QJsonValue& coord : coordinates) {
        if (coord.isArray()) {
            QJsonArray c = coord.toArray();
            if (c.size() >= 2) {
                double lon = c[0].toDouble();
                double lat = c[1].toDouble();
                points.append(QPointF(lon, lat));
            }
        }
    }
    
    return points;
}
