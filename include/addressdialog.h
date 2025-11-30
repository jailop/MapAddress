#ifndef ADDRESSDIALOG_H
#define ADDRESSDIALOG_H

#include <QDialog>
#include "address.h"
#include "geocodingservice.h"

namespace Ui {
class AddressDialog;
}

class AddressDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddressDialog(QWidget *parent = nullptr);
    ~AddressDialog();
    
    void setAddress(const Address& address);
    Address getAddress() const;
    void setListId(int listId) { m_listId = listId; }

private slots:
    void onGeocodeClicked();
    void onGeocodeFinished(double latitude, double longitude, const QString& formattedAddress);
    void onGeocodeFailed(const QString& error);
    void onMultipleCandidates(const QVector<GeocodingCandidate>& candidates);
    void onCandidateSelected(int index);

private:
    Ui::AddressDialog *ui;
    GeocodingService* m_geocodingService;
    Address m_address;
    int m_listId;
    bool m_geocoded;
    QVector<GeocodingCandidate> m_candidates;
    
    void updateCoordinatesDisplay();
};

#endif // ADDRESSDIALOG_H
