#pragma once

#include <QObject>
#include <QString>
#include <string>

#include "controllers/controller.h"
#include "controllers/hid/hidusagetables.h"

struct ProductInfo;
struct hid_device_info;

namespace mixxx {

namespace hid {

/// Detached copy of `struct hid_device_info`.
///
/// Stores a detached copy of hid_device_info and its contents.
///
/// All instances of hid_device_info are returned by the HIDAPI
/// library as a linked list. The memory of both the members
/// of this list as well as their contents are managed by the
/// library and freed immediately after iterating through this
/// list.
///
/// Includes some basic validations and implicit conversion to
/// QString if needed.
class DeviceInfo final {
  public:
    explicit DeviceInfo(
            const hid_device_info& device_info, intptr_t file_descriptor = -1, int interface = -1);

    // The VID.
    uint16_t getVendorId() const {
        return vendor_id;
    }
    // The PID.
    uint16_t getProductId() const {
        return product_id;
    }
    // The File Descriptor.
    intptr_t getFileDescriptor() const {
        return file_descriptor;
    }
    // The Interface.
    intptr_t getInterface() const {
        return interfaceIdx;
    }

    /// The releaseNumberBCD returns the version of the USB specification to
    /// which the device conforms. The bcdUSB field contains a BCD version
    /// number in the format 0xJJMN:
    /// - JJ: major version number
    /// - M: minor version number
    /// - N: sub-minor version number
    /// Examples:
    /// - 0200H represents USB 2.0 specification
    /// - 0300H represents USB 3.0 specification
    /// - 0310H represents USB 3.1 specification
    /// Note, that many devices have not set this field as intended
    uint16_t releaseNumberBCD() const {
        return release_number;
    }

    /// The raw path, needed for subsequent HIDAPI requests.
    const char* pathRaw() const {
        return m_pathRaw.c_str();
    }
    /// The raw serial number, needed for subsequent HIDAPI requests.
    const wchar_t* serialNumberRaw() const {
        return m_serialNumberRaw.c_str();
    }

    const QString& getVendorString() const {
        return m_manufacturerString;
    }
    const QString& getProductString() const {
        return m_productString;
    }
    const QString& getSerialNumber() const {
        return m_serialNumber;
    }

    std::optional<uint8_t> getUsbInterfaceNumber() const {
        if (m_usbInterfaceNumber == -1) {
            return std::nullopt;
        }
        return m_usbInterfaceNumber;
    }

    const PhysicalTransportProtocol& getPhysicalTransportProtocol() const {
        return m_physicalTransportProtocol;
    }

    uint16_t getUsagePage() const {
        return usage_page;
    }

    uint16_t getUsage() const {
        return usage;
    }

    QString getUsagePageDescription() const {
        return mixxx::hid::HidUsageTables::getUsagePageDescription(usage_page);
    }

    QString getUsageDescription() const {
        return mixxx::hid::HidUsageTables::getUsageDescription(usage_page, usage);
    }

    bool isValid() const {
        return !getProductString().isNull() && !getSerialNumber().isNull();
    }
    QString formatName() const;

    bool matchProductInfo(
            const ProductInfo& productInfo) const;

  private:
    friend class DeviceCategory;
    friend QDebug operator<<(
            QDebug dbg,
            const DeviceInfo& deviceInfo);

    // members from hid_device_info
    unsigned short vendor_id;
    unsigned short product_id;
    unsigned short release_number;
    unsigned short usage_page;
    unsigned short usage;
    intptr_t file_descriptor;
    int interfaceIdx;

    PhysicalTransportProtocol m_physicalTransportProtocol;
    int m_usbInterfaceNumber;

    std::string m_pathRaw;
    std::wstring m_serialNumberRaw;

    QString m_manufacturerString;
    QString m_productString;
    QString m_serialNumber;
};

} // namespace hid

} // namespace mixxx
