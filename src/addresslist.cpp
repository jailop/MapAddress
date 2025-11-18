#include "addresslist.h"

AddressList::AddressList()
    : m_id(-1), m_name("Untitled List") {
}

AddressList::AddressList(int id, const QString& name)
    : m_id(id), m_name(name) {
}

void AddressList::addAddress(const Address& address) {
    m_addresses.append(address);
}

void AddressList::removeAddress(int addressId) {
    int index = findAddressIndex(addressId);
    if (index >= 0) {
        m_addresses.removeAt(index);
    }
}

void AddressList::updateAddress(const Address& address) {
    int index = findAddressIndex(address.getId());
    if (index >= 0) {
        m_addresses[index] = address;
    }
}

Address AddressList::getAddress(int addressId) const {
    int index = findAddressIndex(addressId);
    if (index >= 0) {
        return m_addresses[index];
    }
    return Address();
}

void AddressList::clear() {
    m_addresses.clear();
}

int AddressList::findAddressIndex(int addressId) const {
    for (int i = 0; i < m_addresses.size(); ++i) {
        if (m_addresses[i].getId() == addressId) {
            return i;
        }
    }
    return -1;
}
