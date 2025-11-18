#include <QtTest/QtTest>
#include "../src/database.h"
#include <QTemporaryDir>

class TestDatabase : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
    
    void testInitialize();
    void testCreateList();
    void testUpdateList();
    void testDeleteList();
    void testGetAllLists();
    void testAddAddress();
    void testUpdateAddress();
    void testDeleteAddress();
    void testGetAddressesForList();
    void testCascadeDelete();

private:
    QTemporaryDir* m_tempDir;
    QString m_dbPath;
};

void TestDatabase::initTestCase()
{
    m_tempDir = new QTemporaryDir();
    QVERIFY(m_tempDir->isValid());
}

void TestDatabase::cleanupTestCase()
{
    delete m_tempDir;
}

void TestDatabase::init()
{
    m_dbPath = m_tempDir->path() + "/test.db";
    QVERIFY(Database::instance().initialize(m_dbPath));
}

void TestDatabase::cleanup()
{
    Database::instance().close();
    QFile::remove(m_dbPath);
}

void TestDatabase::testInitialize()
{
    QVERIFY(Database::instance().isOpen());
}

void TestDatabase::testCreateList()
{
    int listId = Database::instance().createList("Test List");
    QVERIFY(listId > 0);
    
    AddressList list = Database::instance().getList(listId);
    QCOMPARE(list.getName(), QString("Test List"));
}

void TestDatabase::testUpdateList()
{
    int listId = Database::instance().createList("Original Name");
    QVERIFY(listId > 0);
    
    bool success = Database::instance().updateList(listId, "Updated Name");
    QVERIFY(success);
    
    AddressList list = Database::instance().getList(listId);
    QCOMPARE(list.getName(), QString("Updated Name"));
}

void TestDatabase::testDeleteList()
{
    int listId = Database::instance().createList("Test List");
    QVERIFY(listId > 0);
    
    bool success = Database::instance().deleteList(listId);
    QVERIFY(success);
    
    AddressList list = Database::instance().getList(listId);
    QCOMPARE(list.getId(), -1); // Invalid list returns -1
}

void TestDatabase::testGetAllLists()
{
    Database::instance().createList("List 1");
    Database::instance().createList("List 2");
    Database::instance().createList("List 3");
    
    QList<AddressList> lists = Database::instance().getAllLists();
    QCOMPARE(lists.size(), 3);
}

void TestDatabase::testAddAddress()
{
    int listId = Database::instance().createList("Test List");
    
    Address addr(0, "123 Main St", "Springfield", "IL", "62701", "USA", 39.781721, -89.650148);
    int addressId = Database::instance().addAddress(listId, addr);
    
    QVERIFY(addressId > 0);
    
    QList<Address> addresses = Database::instance().getAddressesForList(listId);
    QCOMPARE(addresses.size(), 1);
    QCOMPARE(addresses[0].getStreet(), QString("123 Main St"));
}

void TestDatabase::testUpdateAddress()
{
    int listId = Database::instance().createList("Test List");
    
    Address addr(0, "123 Main St", "Springfield", "IL", "62701", "USA", 39.781721, -89.650148);
    int addressId = Database::instance().addAddress(listId, addr);
    QVERIFY(addressId > 0);
    
    Address updatedAddr(addressId, "456 Oak Ave", "Chicago", "IL", "60601", "USA", 41.878113, -87.629799);
    bool success = Database::instance().updateAddress(updatedAddr);
    QVERIFY(success);
    
    QList<Address> addresses = Database::instance().getAddressesForList(listId);
    QCOMPARE(addresses[0].getStreet(), QString("456 Oak Ave"));
    QCOMPARE(addresses[0].getCity(), QString("Chicago"));
}

void TestDatabase::testDeleteAddress()
{
    int listId = Database::instance().createList("Test List");
    
    Address addr(0, "123 Main St", "Springfield", "IL", "62701", "USA", 0, 0);
    int addressId = Database::instance().addAddress(listId, addr);
    
    bool success = Database::instance().deleteAddress(addressId);
    QVERIFY(success);
    
    QList<Address> addresses = Database::instance().getAddressesForList(listId);
    QCOMPARE(addresses.size(), 0);
}

void TestDatabase::testGetAddressesForList()
{
    int listId = Database::instance().createList("Test List");
    
    Address addr1(0, "123 Main St", "Springfield", "IL", "62701", "USA", 0, 0);
    Address addr2(0, "456 Oak Ave", "Chicago", "IL", "60601", "USA", 0, 0);
    
    Database::instance().addAddress(listId, addr1);
    Database::instance().addAddress(listId, addr2);
    
    QList<Address> addresses = Database::instance().getAddressesForList(listId);
    QCOMPARE(addresses.size(), 2);
}

void TestDatabase::testCascadeDelete()
{
    int listId = Database::instance().createList("Test List");
    
    Address addr1(0, "123 Main St", "Springfield", "IL", "62701", "USA", 0, 0);
    Address addr2(0, "456 Oak Ave", "Chicago", "IL", "60601", "USA", 0, 0);
    
    Database::instance().addAddress(listId, addr1);
    Database::instance().addAddress(listId, addr2);
    
    // Delete the list
    Database::instance().deleteList(listId);
    
    // Addresses should be deleted too (cascade)
    QList<Address> addresses = Database::instance().getAddressesForList(listId);
    QCOMPARE(addresses.size(), 0);
}

QTEST_MAIN(TestDatabase)
#include "test_database.moc"
