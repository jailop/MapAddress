#include "mapwidget.h"
#include <QVBoxLayout>
#include <QWebEngineSettings>
#include <QWebEnginePage>
#include <QFile>
#include <QDebug>
#include <QSizePolicy>
#include <QTimer>
#include "logger.h"

// ConsolePage implementation
void ConsolePage::javaScriptConsoleMessage(JavaScriptConsoleMessageLevel level, const QString &message,
                                           int lineNumber, const QString &sourceID) {
    QString levelStr;
    switch (level) {
        case InfoMessageLevel:
            levelStr = "INFO";
            LOG_INFO(QString("[JS Console] %1:%2 - %3").arg(sourceID).arg(lineNumber).arg(message));
            break;
        case WarningMessageLevel:
            levelStr = "WARNING";
            LOG_WARNING(QString("[JS Console] %1:%2 - %3").arg(sourceID).arg(lineNumber).arg(message));
            break;
        case ErrorMessageLevel:
            levelStr = "ERROR";
            LOG_ERROR(QString("[JS Console] %1:%2 - %3").arg(sourceID).arg(lineNumber).arg(message));
            break;
    }
    qDebug() << QString("[JS %1] %2:%3 - %4").arg(levelStr).arg(sourceID).arg(lineNumber).arg(message);
}

MapWidget::MapWidget(QWidget* parent)
    : QWidget(parent),
      m_webView(new QWebEngineView(this)),
      m_currentProvider(nullptr),
      m_bridge(new MapBridge(this)),
      m_channel(new QWebChannel(this)) {

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(m_webView);
    setLayout(layout);
    
    // Set minimum size for the map
    setMinimumSize(400, 300);
    
    // Force the webview to be visible and expand
    m_webView->setVisible(true);
    m_webView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    
    // Set custom page to capture console messages
    m_webView->setPage(new ConsolePage(this));

    // Enable web features
    m_webView->settings()->setAttribute(QWebEngineSettings::JavascriptEnabled, true);
    m_webView->settings()->setAttribute(QWebEngineSettings::LocalContentCanAccessRemoteUrls, true);
    
    // Enable developer tools for debugging
    m_webView->settings()->setAttribute(QWebEngineSettings::JavascriptCanOpenWindows, true);
    m_webView->page()->settings()->setAttribute(QWebEngineSettings::PluginsEnabled, true);
    
    // Enable console output
    connect(m_webView->page(), &QWebEnginePage::loadFinished, this, [this](bool ok) {
        qDebug() << "Page load finished:" << ok;
        if (ok) {
            // Force a JavaScript execution to verify it's working
            m_webView->page()->runJavaScript("console.log('MapWidget: Page loaded successfully'); typeof L !== 'undefined' ? 'Leaflet loaded' : 'Leaflet NOT loaded'", 
                [](const QVariant &result) {
                    qDebug() << "JavaScript check:" << result.toString();
                });
        }
    });
    
    setupProviders();
    
    m_channel->registerObject("qtBridge", m_bridge);
    m_webView->page()->setWebChannel(m_channel);

    connect(m_bridge, &MapBridge::markerClickedSignal, 
            this, &MapWidget::markerClicked);
    connect(m_bridge, &MapBridge::mapClickedSignal,
            this, &MapWidget::mapClicked);
    
    // Load map after a short delay to ensure widget is visible
    QTimer::singleShot(100, this, [this]() {
        loadMap();
    });
}

MapWidget::~MapWidget() {
}

void MapWidget::setupProviders() {
    m_googleMapsProvider = new GoogleMapsProvider(this);
    m_osmProvider = new OpenStreetMapProvider(this);
    
    m_googleMapsProvider->initialize();
    m_osmProvider->initialize();
    
    m_currentProvider = m_osmProvider;
    
    // Don't load map here - wait until widget is shown
}

void MapWidget::setMapProvider(MapProvider::ProviderType type) {
    if (type == MapProvider::GoogleMaps) {
        m_currentProvider = m_googleMapsProvider;
    } else {
        m_currentProvider = m_osmProvider;
    }
    loadMap();
}

void MapWidget::loadMap() {
    if (m_currentProvider) {
        QString html = m_currentProvider->getHtml();
        qDebug() << "Loading map HTML, length:" << html.length();
        qDebug() << "WebView size:" << m_webView->size();
        qDebug() << "WebView visible:" << m_webView->isVisible();
        qDebug() << "MapWidget size:" << this->size();
        
        // Save HTML to file for debugging
        QFile debugFile("/tmp/mapaddress_debug.html");
        if (debugFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            debugFile.write(html.toUtf8());
            debugFile.close();
            qDebug() << "HTML saved to /tmp/mapaddress_debug.html for inspection";
        }
        
        // Set base URL to allow loading Qt resources and external content
        m_webView->setHtml(html, QUrl("qrc:///"));
    } else {
        qWarning() << "No current provider set!";
    }
}

void MapWidget::displayAddresses(const QList<Address>& addresses) {
    if (!m_currentProvider) return;

    m_currentProvider->clearMarkers();
    
    for (const Address& addr : addresses) {
        if (addr.hasCoordinates()) {
            m_currentProvider->addMarker(
                addr.getId(),
                addr.getLatitude(),
                addr.getLongitude(),
                addr.getFullAddress()
            );
        }
    }

    if (!addresses.isEmpty()) {
        m_currentProvider->fitBounds(addresses);
    }

    loadMap();
}

void MapWidget::centerOnAddress(const Address& address) {
    if (!m_currentProvider || !address.hasCoordinates()) return;

    m_currentProvider->setCenter(
        address.getLatitude(),
        address.getLongitude(),
        15
    );
    
    loadMap();
}

void MapWidget::clearMap() {
    if (m_currentProvider) {
        m_currentProvider->clearMarkers();
        loadMap();
    }
}

void MapWidget::zoomIn() {
    if (m_webView && m_webView->page()) {
        m_webView->page()->runJavaScript("if (typeof map !== 'undefined') { map.zoomIn(); }");
    }
}

void MapWidget::zoomOut() {
    if (m_webView && m_webView->page()) {
        m_webView->page()->runJavaScript("if (typeof map !== 'undefined') { map.zoomOut(); }");
    }
}

void MapWidget::highlightMarker(int markerId) {
    if (m_webView && m_webView->page()) {
        QString js = QString("if (typeof window.highlightMarker !== 'undefined') { window.highlightMarker(%1); }").arg(markerId);
        m_webView->page()->runJavaScript(js);
    }
}

void MapWidget::showRoute(const Address& start, const Address& end) {
    if (!m_webView || !m_webView->page()) return;
    if (!start.hasCoordinates() || !end.hasCoordinates()) return;
    
    QString js = QString(
        "if (typeof window.showRoute !== 'undefined') {"
        "  window.showRoute(%1, %2, %3, %4);"
        "}"
    ).arg(start.getLatitude())
     .arg(start.getLongitude())
     .arg(end.getLatitude())
     .arg(end.getLongitude());
    
    m_webView->page()->runJavaScript(js);
}

void MapWidget::showRoutePolyline(const QList<QPointF>& routePoints) {
    if (!m_webView || !m_webView->page() || routePoints.isEmpty()) return;
    
    // Convert route points to JSON array format [[lat, lon], ...]
    QStringList pointsJson;
    for (const QPointF& point : routePoints) {
        pointsJson << QString("[%1, %2]").arg(point.y(), 0, 'f', 6).arg(point.x(), 0, 'f', 6);
    }
    
    QString js = QString(
        "if (typeof window.showRoutePolyline !== 'undefined') {"
        "  window.showRoutePolyline([%1]);"
        "}"
    ).arg(pointsJson.join(", "));
    
    m_webView->page()->runJavaScript(js);
}

void MapWidget::clearRoute() {
    if (!m_webView || !m_webView->page()) return;
    
    QString js = "if (typeof window.clearRoute !== 'undefined') { window.clearRoute(); }";
    m_webView->page()->runJavaScript(js);
}
