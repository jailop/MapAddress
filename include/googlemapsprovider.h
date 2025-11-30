#ifndef GOOGLEMAPSPROVIDER_H
#define GOOGLEMAPSPROVIDER_H

#include "mapprovider.h"
#include <QMap>

class GoogleMapsProvider : public MapProvider {
    Q_OBJECT

public:
    explicit GoogleMapsProvider(QObject* parent = nullptr);
    ~GoogleMapsProvider() override = default;

    void initialize() override;
    void setCenter(double latitude, double longitude, int zoom = 13) override;
    void addMarker(int id, double latitude, double longitude, const QString& title) override;
    void removeMarker(int id) override;
    void clearMarkers() override;
    void fitBounds(const QList<Address>& addresses) override;
    QString getHtml() const override;
    ProviderType getType() const override { return GoogleMaps; }

private:
    QString m_htmlTemplate;
    QMap<int, QString> m_markers;
    double m_centerLat;
    double m_centerLng;
    int m_zoom;

    QString generateHtml() const;
};

#endif // GOOGLEMAPSPROVIDER_H
