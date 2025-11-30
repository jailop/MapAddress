#include <QtTest/QtTest>
#include "addresslist.h"

class TestAddressList : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    
    void testConstructor();
    void testAddAddress();
    void testRemoveAddress();
    void testUpdateAddress();
    void testGetAddress();
    void testGetAddresses();
    void testClear();
    void testAddressCount();
};

void TestAddressList::initTestCase()
{
}

void TestAddressList::cleanupTestCase()
{
}

void TestAddressList::testConstructor()
{
    // Default constructor
    AddressList list1;
    QCOMPARE(list1.getId(), -1);  // Default ID is -1
    QCOMPARE(list1.getName(), QString("Untitled List"));  // Default name
    QCOMPARE(list1.getAddressCount(), 0);
    
    // Parameterized constructor
    AddressList list2(1, "My List");
    QCOMPARE(list2.getId(), 1);
    QCOMPARE(list2.getName(), QString("My List"));
    QCOMPARE(list2.getAddressCount(), 0);
}

void TestAddressList::testAddAddress()
{
    AddressList list(1, "Test List");
    
    Address addr1(1, "123 Main St", "Springfield", "IL", "62701", "USA", 0, 0);
    list.addAddress(addr1);
    
    QCOMPARE(list.getAddressCount(), 1);
    
    Address addr2(2, "456 Oak Ave", "Chicago", "IL", "60601", "USA", 0, 0);
    list.addAddress(addr2);
    
    QCOMPARE(list.getAddressCount(), 2);
}

void TestAddressList::testRemoveAddress()
{
    AddressList list(1, "Test List");
    
    Address addr1(1, "123 Main St", "Springfield", "IL", "62701", "USA", 0, 0);
    Address addr2(2, "456 Oak Ave", "Chicago", "IL", "60601", "USA", 0, 0);
    
    list.addAddress(addr1);
    list.addAddress(addr2);
    QCOMPARE(list.getAddressCount(), 2);
    
    list.removeAddress(1);
    QCOMPARE(list.getAddressCount(), 1);
}

void TestAddressList::testUpdateAddress()
{
    AddressList list(1, "Test List");
    
    Address addr(1, "123 Main St", "Springfield", "IL", "62701", "USA", 0, 0);
    list.addAddress(addr);
    
    Address updatedAddr(1, "456 Oak Ave", "Chicago", "IL", "60601", "USA", 0, 0);
    list.updateAddress(updatedAddr);
    
    Address retrieved = list.getAddress(1);
    QCOMPARE(retrieved.getStreet(), QString("456 Oak Ave"));
    QCOMPARE(retrieved.getCity(), QString("Chicago"));
}

void TestAddressList::testGetAddress()
{
    AddressList list(1, "Test List");
    
    Address addr(1, "123 Main St", "Springfield", "IL", "62701", "USA", 0, 0);
    list.addAddress(addr);
    
    Address retrieved = list.getAddress(1);
    QCOMPARE(retrieved.getId(), 1);
    QCOMPARE(retrieved.getStreet(), QString("123 Main St"));
}

void TestAddressList::testGetAddresses()
{
    AddressList list(1, "Test List");
    
    Address addr1(1, "123 Main St", "Springfield", "IL", "62701", "USA", 0, 0);
    Address addr2(2, "456 Oak Ave", "Chicago", "IL", "60601", "USA", 0, 0);
    
    list.addAddress(addr1);
    list.addAddress(addr2);
    
    QList<Address> addresses = list.getAddresses();
    QCOMPARE(addresses.size(), 2);
}

void TestAddressList::testClear()
{
    AddressList list(1, "Test List");
    
    Address addr1(1, "123 Main St", "Springfield", "IL", "62701", "USA", 0, 0);
    Address addr2(2, "456 Oak Ave", "Chicago", "IL", "60601", "USA", 0, 0);
    
    list.addAddress(addr1);
    list.addAddress(addr2);
    QCOMPARE(list.getAddressCount(), 2);
    
    list.clear();
    QCOMPARE(list.getAddressCount(), 0);
}

void TestAddressList::testAddressCount()
{
    AddressList list(1, "Test List");
    QCOMPARE(list.getAddressCount(), 0);
    
    Address addr1(1, "123 Main St", "Springfield", "IL", "62701", "USA", 0, 0);
    list.addAddress(addr1);
    QCOMPARE(list.getAddressCount(), 1);
    
    Address addr2(2, "456 Oak Ave", "Chicago", "IL", "60601", "USA", 0, 0);
    list.addAddress(addr2);
    QCOMPARE(list.getAddressCount(), 2);
    
    list.removeAddress(1);
    QCOMPARE(list.getAddressCount(), 1);
}

QTEST_MAIN(TestAddressList)
#include "test_addresslist.moc"
