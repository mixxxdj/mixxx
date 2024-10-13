#pragma once

#include <QObject>
#include <QString>
#include <string>

#include "controllers/controller.h"

struct ProductInfo;
struct hid_device_info;

namespace mixxx {

namespace hid {

constexpr unsigned short kGenericDesktopUsagePage = 0x01;

constexpr unsigned short kGenericDesktopMouseUsage = 0x02;
constexpr unsigned short kGenericDesktopKeyboardUsage = 0x06;

// Apple has two two different vendor IDs which are used for different devices.
constexpr unsigned short kAppleVendorId = 0x5ac;
constexpr unsigned short kAppleIncVendorId = 0x004c;

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
            const hid_device_info& device_info);

    // The VID.
    unsigned short getVendorId() const {
        return vendor_id;
    }
    // The PID.
    unsigned short getProductId() const {
        return product_id;
    }
    /// The release number as a binary-coded decimal (BCD).
    unsigned short releaseNumberBCD() const {
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

    bool isValid() const {
        return !getProductString().isNull() && !getSerialNumber().isNull();
    }

    QString formatVID() const;
    QString formatPID() const;
    QString formatReleaseNumber() const;
    QString formatInterface() const;
    QString formatUsage() const;
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
