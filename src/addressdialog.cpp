#include "addressdialog.h"
#include "ui_addressdialog.h"
#include <QMessageBox>

AddressDialog::AddressDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AddressDialog)
    , m_geocodingService(new GeocodingService(this))
    , m_listId(-1)
    , m_geocoded(false)
{
    ui->setupUi(this);
    
    connect(ui->geocodeButton, &QPushButton::clicked, this, &AddressDialog::onGeocodeClicked);
    connect(m_geocodingService, &GeocodingService::geocodingCompleted, this, &AddressDialog::onGeocodeFinished);
    connect(m_geocodingService, &GeocodingService::geocodingFailed, this, &AddressDialog::onGeocodeFailed);
    
    // Disable OK button until address is geocoded
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
}

AddressDialog::~AddressDialog()
{
    delete ui;
}

void AddressDialog::setAddress(const Address& address)
{
    m_address = address;
    ui->addressLineEdit->setText(address.getFullAddress());
    updateCoordinatesDisplay();
    
    if (address.hasCoordinates()) {
        m_geocoded = true;
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
        ui->statusLabel->setText("<font color='green'>✓ Address verified</font>");
    }
}

Address AddressDialog::getAddress() const
{
    return m_address;
}

void AddressDialog::onGeocodeClicked()
{
    QString addressText = ui->addressLineEdit->text().trimmed();
    
    // Validate input
    if (addressText.isEmpty()) {
        QMessageBox::warning(this, "Invalid Input", "Please enter an address.");
        return;
    }
    
    if (addressText.length() < 5) {
        QMessageBox::warning(this, "Invalid Input", 
            "Address is too short. Please enter a complete address.");
        return;
    }
    
    // Check for minimum components
    if (!addressText.contains(',')) {
        QMessageBox::warning(this, "Invalid Format", 
            "Please use format: Street, City, State ZIP\n"
            "Example: 123 Main St, Springfield, IL 62701");
        return;
    }
    
    ui->statusLabel->setText("Geocoding address...");
    ui->geocodeButton->setEnabled(false);
    
    // Parse the address string into components
    // Simple parsing: assume format like "street, city, state zip"
    QStringList parts = addressText.split(',');
    
    Address tempAddress;
    if (parts.size() >= 1) {
        tempAddress.setStreet(parts[0].trimmed());
    }
    if (parts.size() >= 2) {
        tempAddress.setCity(parts[1].trimmed());
    }
    if (parts.size() >= 3) {
        QStringList stateZip = parts[2].trimmed().split(' ', Qt::SkipEmptyParts);
        if (stateZip.size() >= 1) {
            tempAddress.setState(stateZip[0]);
        }
        if (stateZip.size() >= 2) {
            tempAddress.setZip(stateZip[1]);
        }
    }
    
    tempAddress.setCountry("USA"); // Default
    m_address = tempAddress;
    
    m_geocodingService->geocode(tempAddress);
}

void AddressDialog::onGeocodeFinished(double latitude, double longitude, const QString& formattedAddress)
{
    m_address.setLatitude(latitude);
    m_address.setLongitude(longitude);
    m_geocoded = true;
    
    updateCoordinatesDisplay();
    
    ui->statusLabel->setText(QString("<font color='green'>✓ Address verified: %1</font>")
        .arg(formattedAddress));
    ui->geocodeButton->setEnabled(true);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
}

void AddressDialog::onGeocodeFailed(const QString& error)
{
    ui->statusLabel->setText(QString("<font color='red'>✗ %1</font>").arg(error));
    ui->geocodeButton->setEnabled(true);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    
    QMessageBox::warning(this, "Geocoding Failed", 
        QString("Could not verify address: %1\n\nPlease check the address and try again.").arg(error));
}

void AddressDialog::updateCoordinatesDisplay()
{
    if (m_address.hasCoordinates()) {
        ui->coordinatesGroup->setEnabled(true);
        ui->latitudeLineEdit->setText(QString::number(m_address.getLatitude(), 'f', 6));
        ui->longitudeLineEdit->setText(QString::number(m_address.getLongitude(), 'f', 6));
    } else {
        ui->coordinatesGroup->setEnabled(false);
        ui->latitudeLineEdit->clear();
        ui->longitudeLineEdit->clear();
    }
}
