#include "geocodingservice.h"
#include "logger.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrlQuery>
#include <QUrl>
#include <QTimer>

GeocodingService::GeocodingService(QObject* parent)
    : QObject(parent), m_networkManager(new QNetworkAccessManager(this)) {
    // Set timeout for network requests
    m_networkManager->setTransferTimeout(15000); // 15 seconds
}

GeocodingService::~GeocodingService() {
}

void GeocodingService::geocode(const Address& address, int maxResults) {
    m_maxResults = maxResults;
    QString url = buildGeocodeUrl(address, maxResults);
    QString fullAddress = address.getFullAddress();
    LOG_INFO("Geocoding address: " + fullAddress);
    LOG_DEBUG("Geocoding URL: " + url);
    
    QNetworkRequest request(url);
    // Nominatim requires a User-Agent header
    request.setHeader(QNetworkRequest::UserAgentHeader, "MapAddress/1.0 Qt Application");
    
    QNetworkReply* reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, &GeocodingService::onGeocodeFinished);
}

void GeocodingService::reverseGeocode(double latitude, double longitude) {
    QString url = buildReverseGeocodeUrl(latitude, longitude);
    LOG_INFO(QString("Reverse geocoding coordinates: %1, %2").arg(latitude).arg(longitude));
    LOG_DEBUG("Reverse geocoding URL: " + url);
    
    QNetworkRequest request(url);
    // Nominatim requires a User-Agent header
    request.setHeader(QNetworkRequest::UserAgentHeader, "MapAddress/1.0 Qt Application");
    
    QNetworkReply* reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, &GeocodingService::onReverseGeocodeFinished);
}

QString GeocodingService::buildGeocodeUrl(const Address& address, int maxResults) const {
    // Build a structured query for better results
    QStringList queryParts;
    
    if (!address.getStreet().isEmpty()) {
        queryParts << address.getStreet();
    }
    if (!address.getCity().isEmpty()) {
        queryParts << address.getCity();
    }
    if (!address.getState().isEmpty()) {
        queryParts << address.getState();
    }
    if (!address.getZip().isEmpty()) {
        queryParts << address.getZip();
    }
    if (!address.getCountry().isEmpty()) {
        queryParts << address.getCountry();
    }
    
    QString query = queryParts.join(", ");
    QString encodedQuery = QUrl::toPercentEncoding(query);
    
    // Add countrycodes parameter if country is specified to improve results
    QString url = QString("https://nominatim.openstreetmap.org/search?q=%1&format=json&limit=%2&addressdetails=1")
        .arg(encodedQuery).arg(maxResults);
    
    return url;
}

QString GeocodingService::buildReverseGeocodeUrl(double latitude, double longitude) const {
    return QString("https://nominatim.openstreetmap.org/reverse?lat=%1&lon=%2&format=json")
        .arg(latitude).arg(longitude);
}

void GeocodingService::onGeocodeFinished() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;

    reply->deleteLater();

    // Handle network errors with user-friendly messages
    if (reply->error() != QNetworkReply::NoError) {
        QString errorMsg;
        switch (reply->error()) {
            case QNetworkReply::ConnectionRefusedError:
                errorMsg = "Connection refused. Please check your internet connection.";
                break;
            case QNetworkReply::HostNotFoundError:
                errorMsg = "Geocoding service not found. Please check your internet connection.";
                break;
            case QNetworkReply::TimeoutError:
                errorMsg = "Request timed out. Please try again.";
                break;
            case QNetworkReply::NetworkSessionFailedError:
                errorMsg = "Network session failed. Please check your internet connection.";
                break;
            default:
                errorMsg = "Network error: " + reply->errorString();
        }
        LOG_ERROR("Geocoding network error: " + errorMsg);
        emit geocodingFailed(errorMsg);
        return;
    }

    QByteArray data = reply->readAll();
    LOG_DEBUG("Geocoding response: " + QString(data));
    
    QJsonDocument doc = QJsonDocument::fromJson(data);

    if (!doc.isArray() || doc.array().isEmpty()) {
        LOG_WARNING("Geocoding failed: Address not found in Nominatim response");
        emit geocodingFailed("Address not found. Please verify the address and try again.");
        return;
    }

    QJsonArray results = doc.array();
    
    // If requesting multiple results, emit all candidates (even if just one)
    if (m_maxResults > 1) {
        QVector<GeocodingCandidate> candidates;
        
        for (const QJsonValue &value : results) {
            QJsonObject result = value.toObject();
            
            if (!result.contains("lat") || !result.contains("lon")) {
                continue;
            }
            
            bool latOk, lonOk;
            double lat = result["lat"].toString().toDouble(&latOk);
            double lon = result["lon"].toString().toDouble(&lonOk);
            
            if (!latOk || !lonOk) {
                continue;
            }
            
            GeocodingCandidate candidate;
            candidate.latitude = lat;
            candidate.longitude = lon;
            candidate.displayName = result["display_name"].toString();
            candidate.type = result["type"].toString();
            candidate.importance = result["importance"].toDouble();
            
            candidates.append(candidate);
        }
        
        if (candidates.isEmpty()) {
            LOG_ERROR("No valid candidates found in response");
            emit geocodingFailed("No valid addresses found. Please try again.");
            return;
        }
        
        LOG_INFO(QString("Geocoding returned %1 candidate(s)").arg(candidates.size()));
        emit geocodingMultipleResults(candidates);
        return;
    }
    
    // Single result - use existing behavior
    QJsonObject result = results.first().toObject();
    
    // Validate response data
    if (!result.contains("lat") || !result.contains("lon")) {
        LOG_ERROR("Invalid geocoding response: missing lat/lon fields");
        emit geocodingFailed("Invalid geocoding response. Please try again.");
        return;
    }
    
    bool latOk, lonOk;
    double lat = result["lat"].toString().toDouble(&latOk);
    double lon = result["lon"].toString().toDouble(&lonOk);
    
    if (!latOk || !lonOk) {
        LOG_ERROR(QString("Invalid coordinates: lat=%1 lon=%2").arg(result["lat"].toString()).arg(result["lon"].toString()));
        emit geocodingFailed("Invalid coordinates received. Please try again.");
        return;
    }
    
    QString displayName = result["display_name"].toString();
    LOG_INFO(QString("Geocoding successful: %1, %2 - %3").arg(lat).arg(lon).arg(displayName));

    emit geocodingCompleted(lat, lon, displayName);
}

void GeocodingService::onReverseGeocodeFinished() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;

    reply->deleteLater();

    // Handle network errors with user-friendly messages
    if (reply->error() != QNetworkReply::NoError) {
        QString errorMsg;
        switch (reply->error()) {
            case QNetworkReply::ConnectionRefusedError:
                errorMsg = "Connection refused. Please check your internet connection.";
                break;
            case QNetworkReply::HostNotFoundError:
                errorMsg = "Geocoding service not found. Please check your internet connection.";
                break;
            case QNetworkReply::TimeoutError:
                errorMsg = "Request timed out. Please try again.";
                break;
            default:
                errorMsg = "Network error: " + reply->errorString();
        }
        emit reverseGeocodingFailed(errorMsg);
        return;
    }

    QByteArray data = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);

    if (!doc.isObject()) {
        emit reverseGeocodingFailed("Invalid response from geocoding service.");
        return;
    }

    QJsonObject result = doc.object();
    
    if (!result.contains("address")) {
        emit reverseGeocodingFailed("No address information found for these coordinates.");
        return;
    }
    
    QJsonObject addressObj = result["address"].toObject();

    QString street = addressObj["road"].toString();
    if (street.isEmpty()) {
        street = addressObj["suburb"].toString();
    }
    QString city = addressObj["city"].toString();
    if (city.isEmpty()) {
        city = addressObj["town"].toString();
    }
    if (city.isEmpty()) {
        city = addressObj["village"].toString();
    }
    QString state = addressObj["state"].toString();
    QString country = addressObj["country"].toString();

    emit reverseGeocodingCompleted(street, city, state, country);
}
