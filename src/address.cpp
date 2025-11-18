#include "address.h"
#include <QStringList>

Address::Address()
    : m_id(-1), m_latitude(0.0), m_longitude(0.0) {
}

Address::Address(int id, const QString& street, const QString& city,
                 const QString& state, const QString& zip, const QString& country,
                 double latitude, double longitude)
    : m_id(id), m_street(street), m_city(city), m_state(state),
      m_zip(zip), m_country(country), m_latitude(latitude), m_longitude(longitude) {
}

QString Address::getFullAddress() const {
    QStringList parts;
    if (!m_street.isEmpty()) parts << m_street;
    if (!m_city.isEmpty()) parts << m_city;
    if (!m_state.isEmpty()) parts << m_state;
    if (!m_zip.isEmpty()) parts << m_zip;
    if (!m_country.isEmpty()) parts << m_country;
    return parts.join(", ");
}

bool Address::isValid() const {
    return !m_street.isEmpty() || !m_city.isEmpty() || hasCoordinates();
}

bool Address::hasCoordinates() const {
    return m_latitude != 0.0 || m_longitude != 0.0;
}
