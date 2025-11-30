#ifndef MAPPROVIDER_H
#define MAPPROVIDER_H

#include <QObject>
#include <QString>
#include <QList>
#include "address.h"

class MapProvider : public QObject {
    Q_OBJECT

public:
    enum ProviderType {
        GoogleMaps,
        OpenStreetMap
    };

    explicit MapProvider(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~MapProvider() = default;

    virtual void initialize() = 0;
    virtual void setCenter(double latitude, double longitude, int zoom = 13) = 0;
    virtual void addMarker(int id, double latitude, double longitude, const QString& title) = 0;
    virtual void removeMarker(int id) = 0;
    virtual void clearMarkers() = 0;
    virtual void fitBounds(const QList<Address>& addresses) = 0;
    virtual QString getHtml() const = 0;
    virtual ProviderType getType() const = 0;

signals:
    void markerClicked(int markerId);
    void mapClicked(double latitude, double longitude);
};

#endif // MAPPROVIDER_H
