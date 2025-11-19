#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "address.h"
#include "addresslist.h"
#include "addressdialog.h"
#include "settingsdialog.h"
#include "database.h"
#include "logger.h"
#include "routingservice.h"
#include <QMessageBox>
#include <QInputDialog>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QSettings>
#include <QMenu>
#include <QRegularExpression>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_mapWidget(nullptr)
    , m_geocodingService(nullptr)
    , m_routingService(nullptr)
    , m_currentListId(-1)
    , m_routeStartId(-1)
    , m_routeEndId(-1)
    , m_pendingAddressLat(0.0)
    , m_pendingAddressLng(0.0)
{
    ui->setupUi(this);
    
    // Initialize services
    m_geocodingService = new GeocodingService(this);
    m_routingService = new RoutingService(this);
    
    setupMapWidget();
    setupConnections();
    applySettings();
    loadLists();
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::setupConnections()
{
    // List management
    connect(ui->newListButton, &QPushButton::clicked, this, &MainWindow::onNewList);
    connect(ui->renameListButton, &QPushButton::clicked, this, &MainWindow::onRenameList);
    connect(ui->deleteListButton, &QPushButton::clicked, this, &MainWindow::onDeleteList);
    connect(ui->listComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onListChanged);
    
    // Address management
    connect(ui->addAddressButton, &QPushButton::clicked, this, &MainWindow::onAddAddress);
    connect(ui->editAddressButton, &QPushButton::clicked, this, &MainWindow::onEditAddress);
    connect(ui->deleteAddressButton, &QPushButton::clicked, this, &MainWindow::onDeleteAddress);
    connect(ui->addressListWidget, &QListWidget::itemSelectionChanged, this, &MainWindow::onAddressSelected);
    connect(ui->searchLineEdit, &QLineEdit::textChanged, this, &MainWindow::onSearchTextChanged);
    
    // Context menu for address list
    ui->addressListWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->addressListWidget, &QListWidget::customContextMenuRequested, 
            this, &MainWindow::onShowAddressContextMenu);
    
    // Menu and toolbar actions
    connect(ui->actionAddAddress, &QAction::triggered, this, &MainWindow::onAddAddress);
    connect(ui->actionEditAddress, &QAction::triggered, this, &MainWindow::onEditAddress);
    connect(ui->actionDeleteAddress, &QAction::triggered, this, &MainWindow::onDeleteAddress);
    connect(ui->actionFitAllMarkers, &QAction::triggered, this, &MainWindow::onFitAllMarkers);
    connect(ui->actionZoomIn, &QAction::triggered, this, &MainWindow::onZoomIn);
    connect(ui->actionZoomOut, &QAction::triggered, this, &MainWindow::onZoomOut);
    connect(ui->actionImport, &QAction::triggered, this, &MainWindow::onImport);
    connect(ui->actionExport, &QAction::triggered, this, &MainWindow::onExport);
    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::onAbout);
    connect(ui->actionSettings, &QAction::triggered, this, &MainWindow::onSettings);
    
    // Routing service
    connect(m_routingService, &RoutingService::routeCalculated, 
            this, &MainWindow::onRouteCalculated);
    connect(m_routingService, &RoutingService::routeFailed,
            this, &MainWindow::onRouteFailed);
    
    // Geocoding service for reverse geocoding
    connect(m_geocodingService, &GeocodingService::reverseGeocodingCompleted,
            this, &MainWindow::onReverseGeocodeCompleted);
    connect(m_geocodingService, &GeocodingService::reverseGeocodingFailed,
            this, &MainWindow::onReverseGeocodeFailed);
    
    // Map provider selector
    connect(ui->mapProviderComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, [this](int index) {
        if (m_mapWidget) {
            MapProvider::ProviderType type = (index == 0) ? MapProvider::OpenStreetMap : MapProvider::GoogleMaps;
            m_mapWidget->setMapProvider(type);
            ui->statusbar->showMessage(QString("Switched to %1").arg(ui->mapProviderComboBox->currentText()), 2000);
        }
    });
}

void MainWindow::setupMapWidget()
{
    m_mapWidget = new MapWidget(this);
    m_geocodingService = new GeocodingService(this);
    
    ui->mapLayout->addWidget(m_mapWidget);
    
    connect(m_mapWidget, &MapWidget::markerClicked, this, &MainWindow::onMapMarkerClicked);
    connect(m_mapWidget, &MapWidget::mapClicked, this, &MainWindow::onMapClicked);
}

void MainWindow::loadLists()
{
    ui->listComboBox->clear();
    auto lists = Database::instance().getAllLists();
    
    for (const auto& list : lists) {
        ui->listComboBox->addItem(list.getName(), list.getId());
    }
    
    if (ui->listComboBox->count() > 0) {
        ui->listComboBox->setCurrentIndex(0);
    } else {
        updateAddressButtons();
        updateListButtons();
    }
}

void MainWindow::loadAddresses(int listId)
{
    // Save current route info before switching
    if (m_currentListId != -1 && m_currentListId != listId) {
        saveRouteInfo();
    }
    
    ui->addressListWidget->clear();
    
    auto addresses = Database::instance().getAddressesForList(listId);
    
    if (m_mapWidget->getCurrentProvider()) {
        m_mapWidget->getCurrentProvider()->clearMarkers();
        m_mapWidget->clearRoute();
    }
    
    for (const auto& address : addresses) {
        QString displayText = QString("%1, %2, %3")
            .arg(address.getStreet())
            .arg(address.getCity())
            .arg(address.getState());
        
        auto item = new QListWidgetItem(displayText, ui->addressListWidget);
        item->setData(Qt::UserRole, address.getId());
        
        if (address.getLatitude() != 0.0 && address.getLongitude() != 0.0 && m_mapWidget->getCurrentProvider()) {
            m_mapWidget->getCurrentProvider()->addMarker(address.getId(), address.getLatitude(), address.getLongitude(), displayText);
        }
    }
    
    updateAddressButtons();
    
    // Load route info for this list
    loadRouteInfo(listId);
    
    // Fit map bounds and refresh display
    if (!addresses.isEmpty() && m_mapWidget->getCurrentProvider()) {
        m_mapWidget->getCurrentProvider()->fitBounds(addresses);
    }
    
    // Reload the map to display all markers
    if (m_mapWidget) {
        m_mapWidget->loadMap();
    }
}

void MainWindow::updateAddressButtons()
{
    bool hasSelection = ui->addressListWidget->currentItem() != nullptr;
    bool hasList = m_currentListId != -1;
    
    ui->addAddressButton->setEnabled(hasList);
    ui->editAddressButton->setEnabled(hasSelection);
    ui->deleteAddressButton->setEnabled(hasSelection);
    ui->actionAddAddress->setEnabled(hasList);
    ui->actionEditAddress->setEnabled(hasSelection);
    ui->actionDeleteAddress->setEnabled(hasSelection);
}

void MainWindow::updateListButtons()
{
    bool hasList = ui->listComboBox->count() > 0;
    ui->renameListButton->setEnabled(hasList);
    ui->deleteListButton->setEnabled(hasList);
}

void MainWindow::onAddAddress()
{
    if (m_currentListId == -1) return;
    
    AddressDialog dialog(this);
    dialog.setWindowTitle("Add Address");
    dialog.setListId(m_currentListId);
    
    if (dialog.exec() == QDialog::Accepted) {
        Address address = dialog.getAddress();
        int addressId = Database::instance().addAddress(m_currentListId, address);
        
        if (addressId > 0) {
            address.setId(addressId);
            
            QString displayText = QString("%1, %2, %3")
                .arg(address.getStreet())
                .arg(address.getCity())
                .arg(address.getState());
            
            auto item = new QListWidgetItem(displayText, ui->addressListWidget);
            item->setData(Qt::UserRole, addressId);
            
            if (address.hasCoordinates() && m_mapWidget->getCurrentProvider()) {
                m_mapWidget->getCurrentProvider()->addMarker(addressId, 
                    address.getLatitude(), address.getLongitude(), displayText);
                
                // Refit map to show all markers including the new one
                auto addresses = Database::instance().getAddressesForList(m_currentListId);
                m_mapWidget->getCurrentProvider()->fitBounds(addresses);
                m_mapWidget->loadMap();
            }
            
            ui->statusbar->showMessage("Address added", 2000);
        } else {
            QMessageBox::warning(this, "Error", "Failed to add address to database");
        }
    }
}

void MainWindow::onEditAddress()
{
    auto currentItem = ui->addressListWidget->currentItem();
    if (!currentItem || m_currentListId == -1) return;
    
    int addressId = currentItem->data(Qt::UserRole).toInt();
    
    // Find the current address
    auto addresses = Database::instance().getAddressesForList(m_currentListId);
    Address currentAddress;
    bool found = false;
    
    for (const auto& addr : addresses) {
        if (addr.getId() == addressId) {
            currentAddress = addr;
            found = true;
            break;
        }
    }
    
    if (!found) {
        QMessageBox::warning(this, "Error", "Address not found");
        return;
    }
    
    AddressDialog dialog(this);
    dialog.setWindowTitle("Edit Address");
    dialog.setListId(m_currentListId);
    dialog.setAddress(currentAddress);
    
    if (dialog.exec() == QDialog::Accepted) {
        Address updatedAddress = dialog.getAddress();
        updatedAddress.setId(addressId);
        
        if (Database::instance().updateAddress(updatedAddress)) {
            QString displayText = QString("%1, %2, %3")
                .arg(updatedAddress.getStreet())
                .arg(updatedAddress.getCity())
                .arg(updatedAddress.getState());
            
            currentItem->setText(displayText);
            
            if (m_mapWidget->getCurrentProvider()) {
                m_mapWidget->getCurrentProvider()->removeMarker(addressId);
                
                if (updatedAddress.hasCoordinates()) {
                    m_mapWidget->getCurrentProvider()->addMarker(addressId, 
                        updatedAddress.getLatitude(), updatedAddress.getLongitude(), displayText);
                }
                
                // Refit map to show all markers after update
                auto addresses = Database::instance().getAddressesForList(m_currentListId);
                if (!addresses.isEmpty()) {
                    m_mapWidget->getCurrentProvider()->fitBounds(addresses);
                }
                m_mapWidget->loadMap();
            }
            
            ui->statusbar->showMessage("Address updated", 2000);
        } else {
            QMessageBox::warning(this, "Error", "Failed to update address");
        }
    }
}

void MainWindow::onDeleteAddress()
{
    auto currentItem = ui->addressListWidget->currentItem();
    if (!currentItem) return;
    
    int addressId = currentItem->data(Qt::UserRole).toInt();
    
    auto reply = QMessageBox::question(this, "Delete Address",
        "Are you sure you want to delete this address?",
        QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        if (Database::instance().deleteAddress(addressId)) {
            if (m_mapWidget->getCurrentProvider()) {
                m_mapWidget->getCurrentProvider()->removeMarker(addressId);
                
                // Refit map to show remaining markers after deletion
                auto addresses = Database::instance().getAddressesForList(m_currentListId);
                if (!addresses.isEmpty()) {
                    m_mapWidget->getCurrentProvider()->fitBounds(addresses);
                }
                m_mapWidget->loadMap();
            }
            delete currentItem;
            ui->detailsLabel->setText("Select an address to view details");
            ui->statusbar->showMessage("Address deleted", 2000);
        } else {
            QMessageBox::warning(this, "Error", "Failed to delete address");
        }
    }
}

void MainWindow::onNewList()
{
    bool ok;
    QString name = QInputDialog::getText(this, "New List",
        "Enter list name:", QLineEdit::Normal, "", &ok);
    
    if (ok && !name.isEmpty()) {
        int listId = Database::instance().createList(name);
        if (listId > 0) {
            ui->listComboBox->addItem(name, listId);
            ui->listComboBox->setCurrentIndex(ui->listComboBox->count() - 1);
            ui->statusbar->showMessage("List created", 2000);
        } else {
            QMessageBox::warning(this, "Error", "Failed to create list");
        }
    }
}

void MainWindow::onRenameList()
{
    if (m_currentListId == -1) return;
    
    bool ok;
    QString currentName = ui->listComboBox->currentText();
    QString name = QInputDialog::getText(this, "Rename List",
        "Enter new name:", QLineEdit::Normal, currentName, &ok);
    
    if (ok && !name.isEmpty() && name != currentName) {
        if (Database::instance().updateList(m_currentListId, name)) {
            ui->listComboBox->setItemText(ui->listComboBox->currentIndex(), name);
            ui->statusbar->showMessage("List renamed", 2000);
        } else {
            QMessageBox::warning(this, "Error", "Failed to rename list");
        }
    }
}

void MainWindow::onDeleteList()
{
    if (m_currentListId == -1) return;
    
    auto reply = QMessageBox::question(this, "Delete List",
        "Are you sure you want to delete this list and all its addresses?",
        QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        if (Database::instance().deleteList(m_currentListId)) {
            ui->listComboBox->removeItem(ui->listComboBox->currentIndex());
            ui->statusbar->showMessage("List deleted", 2000);
        } else {
            QMessageBox::warning(this, "Error", "Failed to delete list");
        }
    }
}

void MainWindow::onListChanged(int index)
{
    if (index >= 0) {
        m_currentListId = ui->listComboBox->itemData(index).toInt();
        loadAddresses(m_currentListId);
    } else {
        m_currentListId = -1;
        ui->addressListWidget->clear();
        ui->detailsLabel->setText("Select an address to view details");
        if (m_mapWidget->getCurrentProvider()) {
            m_mapWidget->getCurrentProvider()->clearMarkers();
        }
    }
    updateAddressButtons();
    updateListButtons();
}

void MainWindow::onAddressSelected()
{
    updateAddressButtons();
    
    auto currentItem = ui->addressListWidget->currentItem();
    if (currentItem) {
        int addressId = currentItem->data(Qt::UserRole).toInt();
        
        // Find the address to get full details
        auto addresses = Database::instance().getAddressesForList(m_currentListId);
        for (const auto& address : addresses) {
            if (address.getId() == addressId) {
                // Show address details
                QString details = QString(
                    "<b>Street:</b> %1<br>"
                    "<b>City:</b> %2<br>"
                    "<b>State:</b> %3<br>"
                    "<b>ZIP:</b> %4<br>"
                    "<b>Country:</b> %5<br>"
                    "<b>Coordinates:</b> %6, %7"
                ).arg(address.getStreet())
                 .arg(address.getCity())
                 .arg(address.getState())
                 .arg(address.getZip())
                 .arg(address.getCountry())
                 .arg(address.getLatitude(), 0, 'f', 6)
                 .arg(address.getLongitude(), 0, 'f', 6);
                
                ui->detailsLabel->setText(details);
                
                // Highlight marker on map with auto-refresh
                if (m_mapWidget && address.hasCoordinates()) {
                    m_mapWidget->highlightMarker(addressId);
                }
                break;
            }
        }
    } else {
        ui->detailsLabel->setText("Select an address to view details");
    }
}

void MainWindow::onSearchTextChanged(const QString& text)
{
    for (int i = 0; i < ui->addressListWidget->count(); ++i) {
        auto item = ui->addressListWidget->item(i);
        bool matches = text.isEmpty() || item->text().contains(text, Qt::CaseInsensitive);
        item->setHidden(!matches);
    }
}

void MainWindow::onFitAllMarkers()
{
    if (m_currentListId != -1 && m_mapWidget->getCurrentProvider()) {
        auto addresses = Database::instance().getAddressesForList(m_currentListId);
        if (!addresses.isEmpty()) {
            m_mapWidget->getCurrentProvider()->fitBounds(addresses);
            m_mapWidget->loadMap();
            ui->statusbar->showMessage("Fitted all markers", 1000);
        } else {
            ui->statusbar->showMessage("No addresses to fit", 2000);
        }
    }
}

void MainWindow::onZoomIn()
{
    if (m_mapWidget) {
        m_mapWidget->zoomIn();
        ui->statusbar->showMessage("Zoomed in", 1000);
    }
}

void MainWindow::onZoomOut()
{
    if (m_mapWidget) {
        m_mapWidget->zoomOut();
        ui->statusbar->showMessage("Zoomed out", 1000);
    }
}

void MainWindow::onImport()
{
    if (m_currentListId == -1) {
        QMessageBox::information(this, "No List Selected", 
            "Please select or create a list before importing addresses.");
        return;
    }
    
    QString fileName = QFileDialog::getOpenFileName(this, "Import Addresses", "", "CSV Files (*.csv)");
    if (fileName.isEmpty()) {
        return;
    }
    
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Import Error", 
            "Could not open file: " + file.errorString());
        return;
    }
    
    QTextStream in(&file);
    int importedCount = 0;
    int errorCount = 0;
    bool isFirstLine = true;
    
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        
        // Skip header line if it looks like a header
        if (isFirstLine) {
            isFirstLine = false;
            if (line.toLower().contains("street") || line.toLower().contains("address")) {
                continue;
            }
        }
        
        if (line.isEmpty()) {
            continue;
        }
        
        // Parse CSV line (format: street, city, state, zip, country, latitude, longitude)
        QStringList parts = line.split(',');
        
        if (parts.size() < 5) {
            errorCount++;
            continue;
        }
        
        Address address;
        address.setStreet(parts[0].trimmed());
        address.setCity(parts[1].trimmed());
        address.setState(parts[2].trimmed());
        address.setZip(parts[3].trimmed());
        address.setCountry(parts[4].trimmed());
        
        // Optional latitude and longitude
        if (parts.size() >= 7) {
            bool latOk, lngOk;
            double lat = parts[5].trimmed().toDouble(&latOk);
            double lng = parts[6].trimmed().toDouble(&lngOk);
            if (latOk && lngOk) {
                address.setLatitude(lat);
                address.setLongitude(lng);
            }
        }
        
        int addressId = Database::instance().addAddress(m_currentListId, address);
        if (addressId > 0) {
            importedCount++;
        } else {
            errorCount++;
        }
    }
    
    file.close();
    
    // Reload addresses to display imported ones
    loadAddresses(m_currentListId);
    
    QString message = QString("Import complete: %1 addresses imported").arg(importedCount);
    if (errorCount > 0) {
        message += QString(", %1 errors").arg(errorCount);
    }
    
    QMessageBox::information(this, "Import Complete", message);
    ui->statusbar->showMessage(message, 5000);
}

void MainWindow::onExport()
{
    if (m_currentListId == -1) {
        QMessageBox::information(this, "No List Selected", 
            "Please select a list to export addresses.");
        return;
    }
    
    auto addresses = Database::instance().getAddressesForList(m_currentListId);
    if (addresses.isEmpty()) {
        QMessageBox::information(this, "No Addresses", 
            "The current list has no addresses to export.");
        return;
    }
    
    QString fileName = QFileDialog::getSaveFileName(this, "Export Addresses", "", "CSV Files (*.csv)");
    if (fileName.isEmpty()) {
        return;
    }
    
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Export Error", 
            "Could not create file: " + file.errorString());
        return;
    }
    
    QTextStream out(&file);
    
    // Write header
    out << "Street,City,State,ZIP,Country,Latitude,Longitude\n";
    
    // Write addresses
    for (const auto& address : addresses) {
        out << QString("\"%1\",\"%2\",\"%3\",\"%4\",\"%5\",%6,%7\n")
            .arg(address.getStreet())
            .arg(address.getCity())
            .arg(address.getState())
            .arg(address.getZip())
            .arg(address.getCountry())
            .arg(address.getLatitude(), 0, 'f', 6)
            .arg(address.getLongitude(), 0, 'f', 6);
    }
    
    file.close();
    
    QString message = QString("Exported %1 addresses to %2").arg(addresses.size()).arg(fileName);
    QMessageBox::information(this, "Export Complete", message);
    ui->statusbar->showMessage(message, 5000);
}

void MainWindow::onAbout()
{
    QMessageBox::about(this, "About Map Address",
        "Map Address v1.0.0\n\n"
        "A Qt application for managing address lists with map visualization.\n\n"
        "Supports Google Maps and OpenStreetMap.");
}

void MainWindow::onSettings()
{
    SettingsDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        applySettings();
        ui->statusbar->showMessage("Settings applied", 2000);
    }
}

void MainWindow::applySettings()
{
    QSettings settings("DataInquiry", "MapAddress");
    
    // Block signals to prevent multiple map loads
    ui->mapProviderComboBox->blockSignals(true);
    
    // Apply default map provider
    QString defaultProvider = settings.value("Map/DefaultProvider", "osm").toString();
    if (defaultProvider == "osm") {
        ui->mapProviderComboBox->setCurrentIndex(0);  // OpenStreetMap is now index 0
    } else {
        ui->mapProviderComboBox->setCurrentIndex(1);  // Google Maps is now index 1
    }
    
    ui->mapProviderComboBox->blockSignals(false);
    
    // Apply log level
    int logLevel = settings.value("General/LogLevel", 1).toInt();
    Logger::instance().setLogLevel(static_cast<Logger::Level>(logLevel));
    
    // Setup map provider selector connection after setting initial value
    static bool connected = false;
    if (!connected) {
        connect(ui->mapProviderComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), 
                this, [this](int index) {
            if (m_mapWidget) {
                MapProvider::ProviderType type = (index == 0) ? MapProvider::OpenStreetMap : MapProvider::GoogleMaps;
                m_mapWidget->setMapProvider(type);
                ui->statusbar->showMessage(QString("Switched to %1").arg(ui->mapProviderComboBox->currentText()), 2000);
            }
        });
        connected = true;
    }
}

void MainWindow::onMapMarkerClicked(int markerId)
{
    for (int i = 0; i < ui->addressListWidget->count(); ++i) {
        auto item = ui->addressListWidget->item(i);
        if (item->data(Qt::UserRole).toInt() == markerId) {
            ui->addressListWidget->setCurrentItem(item);
            break;
        }
    }
}

void MainWindow::onMapClicked(double latitude, double longitude)
{
    // Show context menu for map click actions
    QMenu contextMenu(this);
    
    QAction* setStartHereAction = contextMenu.addAction("Set Start Point Here");
    QAction* setEndHereAction = contextMenu.addAction("Set End Point Here");
    contextMenu.addSeparator();
    QAction* addAddressAction = contextMenu.addAction("Add Address at Location");
    
    QAction* selectedAction = contextMenu.exec(QCursor::pos());
    
    if (selectedAction == setStartHereAction) {
        // Create temporary address for start point
        Address tempAddr;
        tempAddr.setId(-100); // Temporary ID for map-clicked start
        tempAddr.setLatitude(latitude);
        tempAddr.setLongitude(longitude);
        tempAddr.setStreet(QString("Start: %1, %2").arg(latitude, 0, 'f', 5).arg(longitude, 0, 'f', 5));
        
        // Store as route start
        m_mapClickStartAddr = tempAddr;
        
        // Add marker on map
        if (m_mapWidget && m_mapWidget->getCurrentProvider()) {
            m_mapWidget->getCurrentProvider()->addMarker(-100, latitude, longitude, "Start Point");
            m_mapWidget->loadMap();
        }
        
        saveRouteInfo();
        ui->statusbar->showMessage("Start point set on map", 2000);
        
        // Check if we can plan route
        if (m_mapClickEndAddr.hasCoordinates()) {
            planMapClickRoute();
        }
    } 
    else if (selectedAction == setEndHereAction) {
        // Create temporary address for end point
        Address tempAddr;
        tempAddr.setId(-200); // Temporary ID for map-clicked end
        tempAddr.setLatitude(latitude);
        tempAddr.setLongitude(longitude);
        tempAddr.setStreet(QString("End: %1, %2").arg(latitude, 0, 'f', 5).arg(longitude, 0, 'f', 5));
        
        // Store as route end
        m_mapClickEndAddr = tempAddr;
        
        // Add marker on map
        if (m_mapWidget && m_mapWidget->getCurrentProvider()) {
            m_mapWidget->getCurrentProvider()->addMarker(-200, latitude, longitude, "End Point");
            m_mapWidget->loadMap();
        }
        
        saveRouteInfo();
        ui->statusbar->showMessage("End point set on map", 2000);
        
        // Check if we can plan route
        if (m_mapClickStartAddr.hasCoordinates()) {
            planMapClickRoute();
        }
    }
    else if (selectedAction == addAddressAction) {
        // Store coordinates for later use
        m_pendingAddressLat = latitude;
        m_pendingAddressLng = longitude;
        
        // Start reverse geocoding to get address details
        ui->statusbar->showMessage("Getting address information...", 0);
        m_geocodingService->reverseGeocode(latitude, longitude);
    }
}

void MainWindow::onShowAddressContextMenu(const QPoint& pos)
{
    auto item = ui->addressListWidget->itemAt(pos);
    if (!item) return;
    
    int addressId = item->data(Qt::UserRole).toInt();
    
    QMenu contextMenu(this);
    
    QAction* setStartAction = contextMenu.addAction(QIcon(":/icons/icons/add.svg"), "Set as Start Point");
    QAction* setEndAction = contextMenu.addAction(QIcon(":/icons/icons/add.svg"), "Set as End Point");
    contextMenu.addSeparator();
    QAction* planAllAction = contextMenu.addAction("Plan Route Through All Addresses");
    contextMenu.addSeparator();
    QAction* clearRouteAction = contextMenu.addAction("Clear Route");
    
    // Disable if already set
    if (addressId == m_routeStartId) {
        setStartAction->setEnabled(false);
        setStartAction->setText("✓ Start Point");
    }
    if (addressId == m_routeEndId) {
        setEndAction->setEnabled(false);
        setEndAction->setText("✓ End Point");
    }
    
    clearRouteAction->setEnabled(m_routeStartId != -1 || m_routeEndId != -1 || m_mapWidget);
    
    QAction* selectedAction = contextMenu.exec(ui->addressListWidget->mapToGlobal(pos));
    
    if (selectedAction == setStartAction) {
        m_routeStartId = addressId;
        onSetStartPoint();
    } else if (selectedAction == setEndAction) {
        m_routeEndId = addressId;
        onSetEndPoint();
    } else if (selectedAction == planAllAction) {
        planRoute();
    } else if (selectedAction == clearRouteAction) {
        onClearRoute();
    }
}

void MainWindow::onSetStartPoint()
{
    if (m_routeStartId == -1) return;
    
    updateAddressListDisplay();
    saveRouteInfo();
    ui->statusbar->showMessage("Start point set", 2000);
    
    // If both points are set, plan route
    if (m_routeStartId != -1 && m_routeEndId != -1) {
        planRoute();
    }
}

void MainWindow::onSetEndPoint()
{
    if (m_routeEndId == -1) return;
    
    updateAddressListDisplay();
    saveRouteInfo();
    ui->statusbar->showMessage("End point set", 2000);
    
    // If both points are set, plan route
    if (m_routeStartId != -1 && m_routeEndId != -1) {
        planRoute();
    }
}

void MainWindow::onClearRoute()
{
    m_routeStartId = -1;
    m_routeEndId = -1;
    
    // Clear map-clicked points
    if (m_mapClickStartAddr.hasCoordinates() && m_mapWidget && m_mapWidget->getCurrentProvider()) {
        m_mapWidget->getCurrentProvider()->removeMarker(-100);
    }
    if (m_mapClickEndAddr.hasCoordinates() && m_mapWidget && m_mapWidget->getCurrentProvider()) {
        m_mapWidget->getCurrentProvider()->removeMarker(-200);
    }
    
    m_mapClickStartAddr = Address();
    m_mapClickEndAddr = Address();
    
    if (m_mapWidget) {
        m_mapWidget->clearRoute();
        m_mapWidget->loadMap();
    }
    
    updateAddressListDisplay();
    
    // Clear from database
    if (m_currentListId != -1) {
        Database::instance().clearRouteInfo(m_currentListId);
    }
    
    ui->statusbar->showMessage("Route cleared", 2000);
}

void MainWindow::updateAddressListDisplay()
{
    // Update display to show start/end indicators
    for (int i = 0; i < ui->addressListWidget->count(); ++i) {
        auto item = ui->addressListWidget->item(i);
        int addressId = item->data(Qt::UserRole).toInt();
        QString text = item->text();
        
        // Remove existing markers
        text.remove(QRegularExpression("^[🚩🏁] "));
        
        // Add markers
        if (addressId == m_routeStartId) {
            text = "🚩 " + text;
        } else if (addressId == m_routeEndId) {
            text = "🏁 " + text;
        }
        
        item->setText(text);
    }
}

void MainWindow::planRoute()
{
    if (m_currentListId == -1) return;
    
    // Get all addresses in the list with coordinates
    auto allAddresses = Database::instance().getAddressesForList(m_currentListId);
    QList<Address> waypoints;
    
    // Add addresses in order, filtering those with coordinates
    for (const auto& addr : allAddresses) {
        if (addr.hasCoordinates()) {
            waypoints.append(addr);
        }
    }
    
    if (waypoints.size() < 2) {
        QMessageBox::information(this, "Route Planning", 
            "Need at least 2 addresses with coordinates for route planning.");
        return;
    }
    
    // Calculate route through all waypoints
    ui->statusbar->showMessage(QString("Calculating route through %1 waypoints...").arg(waypoints.size()), 0);
    m_routingService->calculateRoute(waypoints);
}

void MainWindow::onRouteCalculated(const QList<QPointF>& routePoints)
{
    if (routePoints.isEmpty()) return;
    
    // Display route on map
    if (m_mapWidget) {
        m_mapWidget->showRoutePolyline(routePoints);
    }
    
    ui->statusbar->showMessage(QString("Route calculated (%1 km)").arg(routePoints.size() * 0.1, 0, 'f', 1), 3000);
}

void MainWindow::onRouteFailed(const QString& error)
{
    ui->statusbar->showMessage("Route calculation failed", 3000);
    QMessageBox::warning(this, "Route Planning Failed", error);
}

void MainWindow::planMapClickRoute()
{
    if (!m_mapClickStartAddr.hasCoordinates() || !m_mapClickEndAddr.hasCoordinates()) {
        return;
    }
    
    // Create waypoint list with start and end
    QList<Address> waypoints;
    waypoints.append(m_mapClickStartAddr);
    waypoints.append(m_mapClickEndAddr);
    
    // Calculate route
    ui->statusbar->showMessage("Calculating route between map points...", 0);
    m_routingService->calculateRoute(waypoints);
}

void MainWindow::saveRouteInfo()
{
    if (m_currentListId == -1) return;
    
    Database::instance().saveRouteInfo(
        m_currentListId,
        m_routeStartId,
        m_routeEndId,
        m_mapClickStartAddr,
        m_mapClickEndAddr
    );
}

void MainWindow::loadRouteInfo(int listId)
{
    // Reset route state
    m_routeStartId = -1;
    m_routeEndId = -1;
    m_mapClickStartAddr = Address();
    m_mapClickEndAddr = Address();
    
    // Load from database
    if (Database::instance().loadRouteInfo(listId, m_routeStartId, m_routeEndId,
                                          m_mapClickStartAddr, m_mapClickEndAddr)) {
        
        // Restore address-based start/end markers
        updateAddressListDisplay();
        
        // Restore map-clicked markers
        if (m_mapClickStartAddr.hasCoordinates() && m_mapWidget && m_mapWidget->getCurrentProvider()) {
            m_mapWidget->getCurrentProvider()->addMarker(
                -100,
                m_mapClickStartAddr.getLatitude(),
                m_mapClickStartAddr.getLongitude(),
                m_mapClickStartAddr.getStreet()
            );
        }
        
        if (m_mapClickEndAddr.hasCoordinates() && m_mapWidget && m_mapWidget->getCurrentProvider()) {
            m_mapWidget->getCurrentProvider()->addMarker(
                -200,
                m_mapClickEndAddr.getLatitude(),
                m_mapClickEndAddr.getLongitude(),
                m_mapClickEndAddr.getStreet()
            );
        }
        
        // Reload map to show restored markers
        if (m_mapWidget) {
            m_mapWidget->loadMap();
        }
    }
}

void MainWindow::onReverseGeocodeCompleted(const QString& street, const QString& city,
                                          const QString& state, const QString& country)
{
    if (m_currentListId == -1) {
        ui->statusbar->showMessage("Please select a list first", 3000);
        return;
    }
    
    // Create address with geocoded information
    Address address;
    address.setStreet(street);
    address.setCity(city);
    address.setState(state);
    address.setCountry(country);
    address.setLatitude(m_pendingAddressLat);
    address.setLongitude(m_pendingAddressLng);
    
    // Open the address dialog with pre-filled information
    AddressDialog dialog(this);
    dialog.setWindowTitle("Add Address from Map");
    dialog.setListId(m_currentListId);
    dialog.setAddress(address);
    
    ui->statusbar->clearMessage();
    
    if (dialog.exec() == QDialog::Accepted) {
        Address finalAddress = dialog.getAddress();
        int addressId = Database::instance().addAddress(m_currentListId, finalAddress);
        
        if (addressId > 0) {
            finalAddress.setId(addressId);
            
            QString displayText = QString("%1, %2, %3")
                .arg(finalAddress.getStreet())
                .arg(finalAddress.getCity())
                .arg(finalAddress.getState());
            
            auto item = new QListWidgetItem(displayText, ui->addressListWidget);
            item->setData(Qt::UserRole, addressId);
            
            if (finalAddress.hasCoordinates() && m_mapWidget->getCurrentProvider()) {
                m_mapWidget->getCurrentProvider()->addMarker(addressId, 
                    finalAddress.getLatitude(), finalAddress.getLongitude(), displayText);
                
                // Refit map to show all markers including the new one
                auto addresses = Database::instance().getAddressesForList(m_currentListId);
                m_mapWidget->getCurrentProvider()->fitBounds(addresses);
                m_mapWidget->loadMap();
            }
            
            ui->statusbar->showMessage("Address added from map location", 2000);
        } else {
            QMessageBox::warning(this, "Error", "Failed to add address to database");
        }
    }
}

void MainWindow::onReverseGeocodeFailed(const QString& error)
{
    if (m_currentListId == -1) {
        ui->statusbar->showMessage("Please select a list first", 3000);
        return;
    }
    
    // Even if reverse geocoding fails, allow user to add address manually with coordinates
    Address address;
    address.setLatitude(m_pendingAddressLat);
    address.setLongitude(m_pendingAddressLng);
    address.setStreet(QString("Location: %1, %2")
        .arg(m_pendingAddressLat, 0, 'f', 5)
        .arg(m_pendingAddressLng, 0, 'f', 5));
    
    // Open the address dialog with coordinates
    AddressDialog dialog(this);
    dialog.setWindowTitle("Add Address from Map");
    dialog.setListId(m_currentListId);
    dialog.setAddress(address);
    
    ui->statusbar->showMessage("Could not get address details. Please enter manually.", 3000);
    
    if (dialog.exec() == QDialog::Accepted) {
        Address finalAddress = dialog.getAddress();
        int addressId = Database::instance().addAddress(m_currentListId, finalAddress);
        
        if (addressId > 0) {
            finalAddress.setId(addressId);
            
            QString displayText = QString("%1, %2, %3")
                .arg(finalAddress.getStreet())
                .arg(finalAddress.getCity())
                .arg(finalAddress.getState());
            
            auto item = new QListWidgetItem(displayText, ui->addressListWidget);
            item->setData(Qt::UserRole, addressId);
            
            if (finalAddress.hasCoordinates() && m_mapWidget->getCurrentProvider()) {
                m_mapWidget->getCurrentProvider()->addMarker(addressId, 
                    finalAddress.getLatitude(), finalAddress.getLongitude(), displayText);
                
                // Refit map to show all markers including the new one
                auto addresses = Database::instance().getAddressesForList(m_currentListId);
                m_mapWidget->getCurrentProvider()->fitBounds(addresses);
                m_mapWidget->loadMap();
            }
            
            ui->statusbar->showMessage("Address added from map location", 2000);
        } else {
            QMessageBox::warning(this, "Error", "Failed to add address to database");
        }
    }
}
