#ifndef DATABASE_H
#define DATABASE_H

#include "address.h"
#include "addresslist.h"
#include <QSqlDatabase>
#include <QString>
#include <QList>

class Database {
public:
    static Database& instance();
    
    bool initialize(const QString& dbPath = "");
    bool isOpen() const;
    void close();
    
    // AddressList operations
    int createList(const QString& name);
    bool deleteList(int listId);
    bool updateList(int listId, const QString& name);
    QList<AddressList> getAllLists();
    AddressList getList(int listId);
    
    // Address operations
    int addAddress(int listId, const Address& address);
    bool deleteAddress(int addressId);
    bool updateAddress(const Address& address);
    QList<Address> getAddressesForList(int listId);
    
    // Route info operations
    void saveRouteInfo(int listId, int startAddressId, int endAddressId,
                      const Address& startPoint, const Address& endPoint);
    bool loadRouteInfo(int listId, int& startAddressId, int& endAddressId,
                      Address& startPoint, Address& endPoint);
    void clearRouteInfo(int listId);
    
    QString getLastError() const { return m_lastError; }
    
private:
    Database();
    ~Database();
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;
    
    bool createTables();
    void setLastError(const QString& error);
    
    QSqlDatabase m_db;
    QString m_lastError;
};

#endif // DATABASE_H
