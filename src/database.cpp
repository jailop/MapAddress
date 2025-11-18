#include "database.h"
#include "logger.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QStandardPaths>
#include <QDir>

Database& Database::instance() {
    static Database instance;
    return instance;
}

Database::Database() {
}

Database::~Database() {
    close();
}

bool Database::initialize(const QString& dbPath) {
    QString path = dbPath;
    if (path.isEmpty()) {
        QString appData = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QDir dir(appData);
        if (!dir.exists()) {
            dir.mkpath(".");
        }
        path = appData + "/mapaddress.db";
    }
    
    LOG_INFO("Database path: " + path);
    
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(path);
    
    if (!m_db.open()) {
        QString error = "Failed to open database: " + m_db.lastError().text();
        setLastError(error);
        LOG_ERROR(error);
        return false;
    }
    
    LOG_INFO("Database opened successfully");
    return createTables();
}

bool Database::isOpen() const {
    return m_db.isOpen();
}

void Database::close() {
    if (m_db.isOpen()) {
        m_db.close();
    }
}

bool Database::createTables() {
    QSqlQuery query(m_db);
    
    QString createListsTable = R"(
        CREATE TABLE IF NOT EXISTS address_lists (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL
        )
    )";
    
    if (!query.exec(createListsTable)) {
        setLastError("Failed to create address_lists table: " + query.lastError().text());
        return false;
    }
    
    QString createAddressesTable = R"(
        CREATE TABLE IF NOT EXISTS addresses (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            list_id INTEGER NOT NULL,
            street TEXT,
            city TEXT,
            state TEXT,
            zip TEXT,
            country TEXT,
            latitude REAL,
            longitude REAL,
            FOREIGN KEY(list_id) REFERENCES address_lists(id) ON DELETE CASCADE
        )
    )";
    
    if (!query.exec(createAddressesTable)) {
        setLastError("Failed to create addresses table: " + query.lastError().text());
        return false;
    }
    
    // Create route info table for storing start/end points per list
    QString createRouteInfoTable = R"(
        CREATE TABLE IF NOT EXISTS route_info (
            list_id INTEGER PRIMARY KEY,
            start_address_id INTEGER,
            end_address_id INTEGER,
            start_lat REAL,
            start_lng REAL,
            start_label TEXT,
            end_lat REAL,
            end_lng REAL,
            end_label TEXT,
            FOREIGN KEY(list_id) REFERENCES address_lists(id) ON DELETE CASCADE
        )
    )";
    
    if (!query.exec(createRouteInfoTable)) {
        setLastError("Failed to create route_info table: " + query.lastError().text());
        return false;
    }
    
    return true;
}

int Database::createList(const QString& name) {
    QSqlQuery query(m_db);
    query.prepare("INSERT INTO address_lists (name) VALUES (?)");
    query.addBindValue(name);
    
    if (!query.exec()) {
        setLastError("Failed to create list: " + query.lastError().text());
        return -1;
    }
    
    return query.lastInsertId().toInt();
}

bool Database::deleteList(int listId) {
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM address_lists WHERE id = ?");
    query.addBindValue(listId);
    
    if (!query.exec()) {
        setLastError("Failed to delete list: " + query.lastError().text());
        return false;
    }
    
    query.prepare("DELETE FROM addresses WHERE list_id = ?");
    query.addBindValue(listId);
    
    if (!query.exec()) {
        setLastError("Failed to delete addresses: " + query.lastError().text());
        return false;
    }
    
    return true;
}

bool Database::updateList(int listId, const QString& name) {
    QSqlQuery query(m_db);
    query.prepare("UPDATE address_lists SET name = ? WHERE id = ?");
    query.addBindValue(name);
    query.addBindValue(listId);
    
    if (!query.exec()) {
        setLastError("Failed to update list: " + query.lastError().text());
        return false;
    }
    
    return true;
}

QList<AddressList> Database::getAllLists() {
    QList<AddressList> lists;
    QSqlQuery query(m_db);
    
    if (!query.exec("SELECT id, name FROM address_lists")) {
        setLastError("Failed to get lists: " + query.lastError().text());
        return lists;
    }
    
    while (query.next()) {
        int id = query.value(0).toInt();
        QString name = query.value(1).toString();
        AddressList list(id, name);
        
        QList<Address> addresses = getAddressesForList(id);
        for (const Address& addr : addresses) {
            list.addAddress(addr);
        }
        
        lists.append(list);
    }
    
    return lists;
}

AddressList Database::getList(int listId) {
    QSqlQuery query(m_db);
    query.prepare("SELECT id, name FROM address_lists WHERE id = ?");
    query.addBindValue(listId);
    
    if (!query.exec()) {
        setLastError("Failed to get list: " + query.lastError().text());
        return AddressList();
    }
    
    if (query.next()) {
        int id = query.value(0).toInt();
        QString name = query.value(1).toString();
        AddressList list(id, name);
        
        QList<Address> addresses = getAddressesForList(id);
        for (const Address& addr : addresses) {
            list.addAddress(addr);
        }
        
        return list;
    }
    
    return AddressList();
}

int Database::addAddress(int listId, const Address& address) {
    QSqlQuery query(m_db);
    query.prepare(R"(
        INSERT INTO addresses (list_id, street, city, state, zip, country, latitude, longitude)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?)
    )");
    
    query.addBindValue(listId);
    query.addBindValue(address.getStreet());
    query.addBindValue(address.getCity());
    query.addBindValue(address.getState());
    query.addBindValue(address.getZip());
    query.addBindValue(address.getCountry());
    query.addBindValue(address.getLatitude());
    query.addBindValue(address.getLongitude());
    
    if (!query.exec()) {
        setLastError("Failed to add address: " + query.lastError().text());
        return -1;
    }
    
    return query.lastInsertId().toInt();
}

bool Database::deleteAddress(int addressId) {
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM addresses WHERE id = ?");
    query.addBindValue(addressId);
    
    if (!query.exec()) {
        setLastError("Failed to delete address: " + query.lastError().text());
        return false;
    }
    
    return true;
}

bool Database::updateAddress(const Address& address) {
    QSqlQuery query(m_db);
    query.prepare(R"(
        UPDATE addresses 
        SET street = ?, city = ?, state = ?, zip = ?, country = ?, latitude = ?, longitude = ?
        WHERE id = ?
    )");
    
    query.addBindValue(address.getStreet());
    query.addBindValue(address.getCity());
    query.addBindValue(address.getState());
    query.addBindValue(address.getZip());
    query.addBindValue(address.getCountry());
    query.addBindValue(address.getLatitude());
    query.addBindValue(address.getLongitude());
    query.addBindValue(address.getId());
    
    if (!query.exec()) {
        setLastError("Failed to update address: " + query.lastError().text());
        return false;
    }
    
    return true;
}

QList<Address> Database::getAddressesForList(int listId) {
    QList<Address> addresses;
    QSqlQuery query(m_db);
    query.prepare(R"(
        SELECT id, street, city, state, zip, country, latitude, longitude
        FROM addresses WHERE list_id = ?
    )");
    query.addBindValue(listId);
    
    if (!query.exec()) {
        setLastError("Failed to get addresses: " + query.lastError().text());
        return addresses;
    }
    
    while (query.next()) {
        Address addr(
            query.value(0).toInt(),
            query.value(1).toString(),
            query.value(2).toString(),
            query.value(3).toString(),
            query.value(4).toString(),
            query.value(5).toString(),
            query.value(6).toDouble(),
            query.value(7).toDouble()
        );
        addresses.append(addr);
    }
    
    return addresses;
}

void Database::setLastError(const QString& error) {
    m_lastError = error;
}

// Route info operations
void Database::saveRouteInfo(int listId, int startAddressId, int endAddressId, 
                             const Address& startPoint, const Address& endPoint) {
    QSqlQuery query(m_db);
    
    // Delete existing route info for this list
    query.prepare("DELETE FROM route_info WHERE list_id = ?");
    query.addBindValue(listId);
    query.exec();
    
    // Insert new route info
    query.prepare(R"(
        INSERT INTO route_info (list_id, start_address_id, end_address_id, 
                               start_lat, start_lng, start_label,
                               end_lat, end_lng, end_label)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)
    )");
    
    query.addBindValue(listId);
    query.addBindValue(startAddressId > 0 ? startAddressId : QVariant());
    query.addBindValue(endAddressId > 0 ? endAddressId : QVariant());
    query.addBindValue(startPoint.hasCoordinates() ? startPoint.getLatitude() : QVariant());
    query.addBindValue(startPoint.hasCoordinates() ? startPoint.getLongitude() : QVariant());
    query.addBindValue(startPoint.hasCoordinates() ? startPoint.getStreet() : QVariant());
    query.addBindValue(endPoint.hasCoordinates() ? endPoint.getLatitude() : QVariant());
    query.addBindValue(endPoint.hasCoordinates() ? endPoint.getLongitude() : QVariant());
    query.addBindValue(endPoint.hasCoordinates() ? endPoint.getStreet() : QVariant());
    
    if (!query.exec()) {
        LOG_ERROR("Failed to save route info: " + query.lastError().text());
    }
}

bool Database::loadRouteInfo(int listId, int& startAddressId, int& endAddressId,
                             Address& startPoint, Address& endPoint) {
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM route_info WHERE list_id = ?");
    query.addBindValue(listId);
    
    if (!query.exec() || !query.next()) {
        return false;
    }
    
    // Load address IDs
    startAddressId = query.value("start_address_id").isNull() ? -1 : query.value("start_address_id").toInt();
    endAddressId = query.value("end_address_id").isNull() ? -1 : query.value("end_address_id").toInt();
    
    // Load map-clicked start point
    if (!query.value("start_lat").isNull()) {
        startPoint.setLatitude(query.value("start_lat").toDouble());
        startPoint.setLongitude(query.value("start_lng").toDouble());
        startPoint.setStreet(query.value("start_label").toString());
        startPoint.setId(-100);
    }
    
    // Load map-clicked end point
    if (!query.value("end_lat").isNull()) {
        endPoint.setLatitude(query.value("end_lat").toDouble());
        endPoint.setLongitude(query.value("end_lng").toDouble());
        endPoint.setStreet(query.value("end_label").toString());
        endPoint.setId(-200);
    }
    
    return true;
}

void Database::clearRouteInfo(int listId) {
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM route_info WHERE list_id = ?");
    query.addBindValue(listId);
    
    if (!query.exec()) {
        LOG_ERROR("Failed to clear route info: " + query.lastError().text());
    }
}
