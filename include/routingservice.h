#ifndef ROUTINGSERVICE_H
#define ROUTINGSERVICE_H

#include <QObject>
#include <QString>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QList>
#include <QPointF>
#include "address.h"

class RoutingService : public QObject {
    Q_OBJECT

public:
    explicit RoutingService(QObject* parent = nullptr);
    ~RoutingService() override;

    // Calculate route through multiple waypoints
    void calculateRoute(const QList<Address>& waypoints);

signals:
    void routeCalculated(const QList<QPointF>& routePoints);
    void routeFailed(const QString& error);

private slots:
    void onRouteRequestFinished();

private:
    QNetworkAccessManager* m_networkManager;
    QString buildRouteUrl(const QList<Address>& waypoints) const;
    QList<QPointF> parseRouteResponse(const QByteArray& data) const;
};

#endif // ROUTINGSERVICE_H
