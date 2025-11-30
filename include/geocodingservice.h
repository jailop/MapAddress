#ifndef GEOCODINGSERVICE_H
#define GEOCODINGSERVICE_H

#include "address.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QString>
#include <QVector>

struct GeocodingCandidate {
  double latitude;
  double longitude;
  QString displayName;
  QString type;
  double importance;
};

class GeocodingService : public QObject {
  Q_OBJECT

public:
  explicit GeocodingService(QObject *parent = nullptr);
  ~GeocodingService() override;

  void geocode(const Address &address, int maxResults = 1);
  void reverseGeocode(double latitude, double longitude);

signals:
  void geocodingCompleted(double latitude, double longitude,
                          const QString &formattedAddress);
  void geocodingMultipleResults(const QVector<GeocodingCandidate> &candidates);
  void geocodingFailed(const QString &error);
  void reverseGeocodingCompleted(const QString &street, const QString &city,
                                 const QString &state, const QString &country);
  void reverseGeocodingFailed(const QString &error);

private slots:
  void onGeocodeFinished();
  void onReverseGeocodeFinished();

private:
  QNetworkAccessManager *m_networkManager;
  int m_maxResults;
  QString buildGeocodeUrl(const Address &address, int maxResults) const;
  QString buildReverseGeocodeUrl(double latitude, double longitude) const;
};

#endif // GEOCODINGSERVICE_H
