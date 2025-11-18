#ifndef OPENSTREETMAPPROVIDER_H
#define OPENSTREETMAPPROVIDER_H

#include "mapprovider.h"
#include <QMap>

class OpenStreetMapProvider : public MapProvider {
    Q_OBJECT

public:
    explicit OpenStreetMapProvider(QObject* parent = nullptr);
    ~OpenStreetMapProvider() override = default;

    void initialize() override;
    void setCenter(double latitude, double longitude, int zoom = 13) override;
    void addMarker(int id, double latitude, double longitude, const QString& title) override;
    void removeMarker(int id) override;
    void clearMarkers() override;
    void fitBounds(const QList<Address>& addresses) override;
    QString getHtml() const override;
    ProviderType getType() const override { return OpenStreetMap; }

private:
    QMap<int, QString> m_markers;
    double m_centerLat;
    double m_centerLng;
    int m_zoom;

    QString generateHtml() const;
};

#endif // OPENSTREETMAPPROVIDER_H
