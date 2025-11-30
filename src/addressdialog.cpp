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
    connect(m_geocodingService, &GeocodingService::geocodingMultipleResults, this, &AddressDialog::onMultipleCandidates);
    connect(ui->candidatesComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AddressDialog::onCandidateSelected);
    
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
    
    // Hide candidates dropdown when starting new geocode
    ui->candidatesComboBox->setVisible(false);
    ui->candidatesComboBox->clear();
    m_candidates.clear();
    
    m_geocodingService->geocode(tempAddress, 5); // Request up to 5 results
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
    ui->candidatesComboBox->setVisible(false);
    
    QMessageBox::warning(this, "Geocoding Failed", 
        QString("Could not verify address: %1\n\nPlease check the address and try again.").arg(error));
}

void AddressDialog::onMultipleCandidates(const QVector<GeocodingCandidate>& candidates)
{
    m_candidates = candidates;
    ui->candidatesComboBox->clear();
    
    if (candidates.size() > 1) {
        ui->statusLabel->setText(QString("<font color='orange'>⚠ Multiple addresses found (%1). Please select the correct one:</font>")
            .arg(candidates.size()));
    } else {
        ui->statusLabel->setText("<font color='green'>✓ Address found. Please confirm:</font>");
    }
    
    for (const auto& candidate : candidates) {
        QString displayText = QString("%1 (Type: %2)")
            .arg(candidate.displayName)
            .arg(candidate.type);
        ui->candidatesComboBox->addItem(displayText);
    }
    
    ui->candidatesComboBox->setVisible(true);
    ui->geocodeButton->setEnabled(true);
    
    // Auto-select first candidate
    if (!candidates.isEmpty()) {
        onCandidateSelected(0);
    }
}

void AddressDialog::onCandidateSelected(int index)
{
    if (index < 0 || index >= m_candidates.size()) {
        return;
    }
    
    const GeocodingCandidate& candidate = m_candidates[index];
    
    m_address.setLatitude(candidate.latitude);
    m_address.setLongitude(candidate.longitude);
    m_geocoded = true;
    
    updateCoordinatesDisplay();
    
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
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
