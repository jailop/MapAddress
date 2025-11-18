#ifndef GEOCODINGSERVICE_H
#define GEOCODINGSERVICE_H

#include <QObject>
#include <QString>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include "address.h"

class GeocodingService : public QObject {
    Q_OBJECT

public:
    explicit GeocodingService(QObject* parent = nullptr);
    ~GeocodingService() override;

    void geocode(const Address& address);
    void reverseGeocode(double latitude, double longitude);

signals:
    void geocodingCompleted(double latitude, double longitude, const QString& formattedAddress);
    void geocodingFailed(const QString& error);
    void reverseGeocodingCompleted(const QString& street, const QString& city,
                                   const QString& state, const QString& country);
    void reverseGeocodingFailed(const QString& error);

private slots:
    void onGeocodeFinished();
    void onReverseGeocodeFinished();

private:
    QNetworkAccessManager* m_networkManager;
    QString buildGeocodeUrl(const Address& address) const;
    QString buildReverseGeocodeUrl(double latitude, double longitude) const;
};

#endif // GEOCODINGSERVICE_H
