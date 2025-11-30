#ifndef ADDRESS_H
#define ADDRESS_H

#include <QString>
#include <QMetaType>

class Address {
public:
    Address();
    Address(int id, const QString& street, const QString& city, 
            const QString& state, const QString& zip, const QString& country,
            double latitude, double longitude);
    
    int getId() const { return m_id; }
    void setId(int id) { m_id = id; }
    
    QString getStreet() const { return m_street; }
    void setStreet(const QString& street) { m_street = street; }
    
    QString getCity() const { return m_city; }
    void setCity(const QString& city) { m_city = city; }
    
    QString getState() const { return m_state; }
    void setState(const QString& state) { m_state = state; }
    
    QString getZip() const { return m_zip; }
    void setZip(const QString& zip) { m_zip = zip; }
    
    QString getCountry() const { return m_country; }
    void setCountry(const QString& country) { m_country = country; }
    
    double getLatitude() const { return m_latitude; }
    void setLatitude(double latitude) { m_latitude = latitude; }
    
    double getLongitude() const { return m_longitude; }
    void setLongitude(double longitude) { m_longitude = longitude; }
    
    QString getFullAddress() const;
    bool isValid() const;
    bool hasCoordinates() const;
    
private:
    int m_id;
    QString m_street;
    QString m_city;
    QString m_state;
    QString m_zip;
    QString m_country;
    double m_latitude;
    double m_longitude;
};

Q_DECLARE_METATYPE(Address)

#endif // ADDRESS_H
