#ifndef MAPWIDGET_H
#define MAPWIDGET_H

#include <QWidget>
#include <QWebEngineView>
#include <QWebEnginePage>
#include <QWebChannel>
#include "mapprovider.h"
#include "googlemapsprovider.h"
#include "openstreetmapprovider.h"

// Custom page to capture console messages
class ConsolePage : public QWebEnginePage {
    Q_OBJECT
public:
    explicit ConsolePage(QObject* parent = nullptr) : QWebEnginePage(parent) {}
    
protected:
    void javaScriptConsoleMessage(JavaScriptConsoleMessageLevel level, const QString &message,
                                  int lineNumber, const QString &sourceID) override;
};

class MapBridge : public QObject {
    Q_OBJECT

public:
    explicit MapBridge(QObject* parent = nullptr) : QObject(parent) {}

public slots:
    void markerClicked(int markerId) {
        emit markerClickedSignal(markerId);
    }

    void mapClicked(double lat, double lng) {
        emit mapClickedSignal(lat, lng);
    }

signals:
    void markerClickedSignal(int markerId);
    void mapClickedSignal(double lat, double lng);
};

class MapWidget : public QWidget {
    Q_OBJECT

public:
    explicit MapWidget(QWidget* parent = nullptr);
    ~MapWidget() override;

    void setMapProvider(MapProvider::ProviderType type);
    MapProvider* getCurrentProvider() const { return m_currentProvider; }
    
    void displayAddresses(const QList<Address>& addresses);
    void centerOnAddress(const Address& address);
    void clearMap();
    void loadMap();
    void zoomIn();
    void zoomOut();
    void highlightMarker(int markerId);
    void showRoute(const Address& start, const Address& end);
    void showRoutePolyline(const QList<QPointF>& routePoints);
    void clearRoute();

signals:
    void markerClicked(int markerId);
    void mapClicked(double latitude, double longitude);

private:
    QWebEngineView* m_webView;
    MapProvider* m_currentProvider;
    GoogleMapsProvider* m_googleMapsProvider;
    OpenStreetMapProvider* m_osmProvider;
    MapBridge* m_bridge;
    QWebChannel* m_channel;

    void setupProviders();
};

#endif // MAPWIDGET_H
