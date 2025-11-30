#include <QtTest/QtTest>
#include "address.h"

class TestAddress : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    
    void testConstructor();
    void testGettersSetters();
    void testFullAddress();
    void testIsValid();
    void testHasCoordinates();
    void testCoordinateValidation();
};

void TestAddress::initTestCase()
{
    // Setup before all tests
}

void TestAddress::cleanupTestCase()
{
    // Cleanup after all tests
}

void TestAddress::testConstructor()
{
    // Test default constructor
    Address addr1;
    QCOMPARE(addr1.getId(), -1);  // Default ID is -1
    QVERIFY(addr1.getStreet().isEmpty());
    
    // Test parameterized constructor
    Address addr2(1, "123 Main St", "Springfield", "IL", "62701", "USA", 39.781721, -89.650148);
    QCOMPARE(addr2.getId(), 1);
    QCOMPARE(addr2.getStreet(), QString("123 Main St"));
    QCOMPARE(addr2.getCity(), QString("Springfield"));
    QCOMPARE(addr2.getState(), QString("IL"));
    QCOMPARE(addr2.getZip(), QString("62701"));
    QCOMPARE(addr2.getCountry(), QString("USA"));
    QCOMPARE(addr2.getLatitude(), 39.781721);
    QCOMPARE(addr2.getLongitude(), -89.650148);
}

void TestAddress::testGettersSetters()
{
    Address addr;
    
    addr.setId(42);
    QCOMPARE(addr.getId(), 42);
    
    addr.setStreet("456 Oak Ave");
    QCOMPARE(addr.getStreet(), QString("456 Oak Ave"));
    
    addr.setCity("Chicago");
    QCOMPARE(addr.getCity(), QString("Chicago"));
    
    addr.setState("IL");
    QCOMPARE(addr.getState(), QString("IL"));
    
    addr.setZip("60601");
    QCOMPARE(addr.getZip(), QString("60601"));
    
    addr.setCountry("USA");
    QCOMPARE(addr.getCountry(), QString("USA"));
    
    addr.setLatitude(41.878113);
    QCOMPARE(addr.getLatitude(), 41.878113);
    
    addr.setLongitude(-87.629799);
    QCOMPARE(addr.getLongitude(), -87.629799);
}

void TestAddress::testFullAddress()
{
    Address addr(1, "123 Main St", "Springfield", "IL", "62701", "USA", 0, 0);
    QString fullAddr = addr.getFullAddress();
    
    QVERIFY(fullAddr.contains("123 Main St"));
    QVERIFY(fullAddr.contains("Springfield"));
    QVERIFY(fullAddr.contains("IL"));
    QVERIFY(fullAddr.contains("62701"));
}

void TestAddress::testIsValid()
{
    // Valid address
    Address addr1(1, "123 Main St", "Springfield", "IL", "62701", "USA", 0, 0);
    QVERIFY(addr1.isValid());
    
    // Invalid address (missing required fields)
    Address addr2;
    QVERIFY(!addr2.isValid());
}

void TestAddress::testHasCoordinates()
{
    // Address with coordinates
    Address addr1(1, "123 Main St", "Springfield", "IL", "62701", "USA", 39.781721, -89.650148);
    QVERIFY(addr1.hasCoordinates());
    
    // Address without coordinates
    Address addr2(1, "123 Main St", "Springfield", "IL", "62701", "USA", 0, 0);
    QVERIFY(!addr2.hasCoordinates());
}

void TestAddress::testCoordinateValidation()
{
    Address addr;
    
    // Valid coordinates
    addr.setLatitude(39.781721);
    addr.setLongitude(-89.650148);
    QVERIFY(addr.hasCoordinates());
    
    // Invalid coordinates (zeros)
    addr.setLatitude(0.0);
    addr.setLongitude(0.0);
    QVERIFY(!addr.hasCoordinates());
}

QTEST_MAIN(TestAddress)
#include "test_address.moc"
