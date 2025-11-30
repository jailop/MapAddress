#ifndef ADDRESSLIST_H
#define ADDRESSLIST_H

#include "address.h"
#include <QString>
#include <QList>

class AddressList {
public:
    AddressList();
    AddressList(int id, const QString& name);
    
    int getId() const { return m_id; }
    void setId(int id) { m_id = id; }
    
    QString getName() const { return m_name; }
    void setName(const QString& name) { m_name = name; }
    
    void addAddress(const Address& address);
    void removeAddress(int addressId);
    void updateAddress(const Address& address);
    Address getAddress(int addressId) const;
    QList<Address> getAddresses() const { return m_addresses; }
    
    int getAddressCount() const { return m_addresses.size(); }
    void clear();
    
private:
    int m_id;
    QString m_name;
    QList<Address> m_addresses;
    
    int findAddressIndex(int addressId) const;
};

#endif // ADDRESSLIST_H
