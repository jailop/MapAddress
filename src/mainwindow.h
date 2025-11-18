#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "mapwidget.h"
#include "geocodingservice.h"
#include "routingservice.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onAddAddress();
    void onEditAddress();
    void onDeleteAddress();
    void onNewList();
    void onRenameList();
    void onDeleteList();
    void onListChanged(int index);
    void onAddressSelected();
    void onSearchTextChanged(const QString& text);
    void onFitAllMarkers();
    void onZoomIn();
    void onZoomOut();
    void onImport();
    void onExport();
    void onAbout();
    void onSettings();
    void onMapMarkerClicked(int markerId);
    void onMapClicked(double latitude, double longitude);
    void onSetStartPoint();
    void onSetEndPoint();
    void onClearRoute();
    void onShowAddressContextMenu(const QPoint& pos);
    void onRouteCalculated(const QList<QPointF>& routePoints);
    void onRouteFailed(const QString& error);

private:
    Ui::MainWindow *ui;
    MapWidget* m_mapWidget;
    GeocodingService* m_geocodingService;
    RoutingService* m_routingService;
    int m_currentListId;
    int m_routeStartId;
    int m_routeEndId;
    Address m_mapClickStartAddr;
    Address m_mapClickEndAddr;

    void setupConnections();
    void setupMapWidget();
    void loadLists();
    void loadAddresses(int listId);
    void updateAddressButtons();
    void updateListButtons();
    void applySettings();
    void updateAddressListDisplay();
    void planRoute();
    void planMapClickRoute();
    void saveRouteInfo();
    void loadRouteInfo(int listId);
};

#endif // MAINWINDOW_H
