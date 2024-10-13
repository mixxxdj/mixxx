#include "controllers/hid/hiddevice.h"

#include <hidapi.h>

#include <QDebugStateSaver>

#include "controllers/controllermappinginfo.h"
#include "util/path.h" // for PATH_MAX on Windows
#include "util/string.h"

namespace {

constexpr unsigned short kGenericDesktopPointerUsage = 0x01;
constexpr unsigned short kGenericDesktopJoystickUsage = 0x04;
constexpr unsigned short kGenericDesktopGamePadUsage = 0x05;
constexpr unsigned short kGenericDesktopKeypadUsage = 0x07;
constexpr unsigned short kGenericDesktopMultiaxisControllerUsage = 0x08;

constexpr unsigned short kAppleInfraredControlProductId = 0x8242;

constexpr std::size_t kDeviceInfoStringMaxLength = 512;

} // namespace

namespace mixxx {

namespace hid {

DeviceInfo::DeviceInfo(
        const hid_device_info& device_info)
        : vendor_id(device_info.vendor_id),
          product_id(device_info.product_id),
          release_number(device_info.release_number),
          usage_page(device_info.usage_page),
          usage(device_info.usage),
          m_usbInterfaceNumber(device_info.interface_number),
          m_pathRaw(device_info.path, mixxx::strnlen_s(device_info.path, PATH_MAX)),
          m_serialNumberRaw(device_info.serial_number,
                  mixxx::wcsnlen_s(device_info.serial_number,
                          kDeviceInfoStringMaxLength)),
          m_manufacturerString(mixxx::convertWCStringToQString(
                  device_info.manufacturer_string,
                  kDeviceInfoStringMaxLength)),
          m_productString(mixxx::convertWCStringToQString(device_info.product_string,
                  kDeviceInfoStringMaxLength)),
          m_serialNumber(mixxx::convertWCStringToQString(
                  m_serialNumberRaw.data(), m_serialNumberRaw.size())) {
    switch (device_info.bus_type) {
    case HID_API_BUS_USB:
        m_physicalTransportProtocol = PhysicalTransportProtocol::USB;
        break;
    case HID_API_BUS_BLUETOOTH:
        m_physicalTransportProtocol = PhysicalTransportProtocol::BlueTooth;
        break;
    case HID_API_BUS_I2C:
        m_physicalTransportProtocol = PhysicalTransportProtocol::I2C;
        break;
    case HID_API_BUS_SPI:
        m_physicalTransportProtocol = PhysicalTransportProtocol::SPI;
        break;
    default:
        m_physicalTransportProtocol = PhysicalTransportProtocol::UNKNOWN;
        break;
    }
}

QString DeviceInfo::formatInterface() const {
    if (m_usbInterfaceNumber < 0) {
        return QString();
    }
    return QChar('#') + QString::number(m_usbInterfaceNumber);
}

QString DeviceInfo::formatVID() const {
    return QStringLiteral("%1").arg(vendor_id, 4, 16, QLatin1Char('0'));
}

QString DeviceInfo::formatPID() const {
    return QStringLiteral("%1").arg(product_id, 4, 16, QLatin1Char('0'));
}

QString DeviceInfo::formatReleaseNumber() const {
    return QString::number(releaseNumberBCD(), 16);
}

QString DeviceInfo::formatUsage() const {
    if (usage_page == 0 && usage == 0) {
        return QString();
    }
    return QStringLiteral("%1").arg(usage_page, 4, 16, QLatin1Char('0')) +
            QLatin1Char(':') +
            QStringLiteral("%1").arg(usage, 4, 16, QLatin1Char('0'));
}

QString DeviceInfo::formatName() const {
    // We include the last 4 digits of the serial number and the
    // interface number to allow the user (and Mixxx!) to keep
    // track of which is which
    const auto serialSuffix = getSerialNumber().right(4);
    if (m_usbInterfaceNumber >= 0) {
        return getProductString() +
                QChar(' ') +
                serialSuffix +
                QChar('_') +
                QString::number(m_usbInterfaceNumber);
    } else {
        return getProductString() +
                QChar(' ') +
                serialSuffix;
    }
}

QDebug operator<<(QDebug dbg, const DeviceInfo& deviceInfo) {
    QStringList parts;
    parts.reserve(8);
    // "VID:PID vReleaseNumber"
    parts.append(deviceInfo.formatVID() +
            QLatin1Char(':') +
            deviceInfo.formatPID() +
            QLatin1String(" r") +
            deviceInfo.formatReleaseNumber());
    const QString usage = deviceInfo.formatUsage();
    if (!usage.isEmpty()) {
        parts.append(QStringLiteral("Usage: ") + usage);
    }
    const QString interfaceId = deviceInfo.formatInterface();
    if (!interfaceId.isEmpty()) {
        parts.append(QStringLiteral("Interface: ") + interfaceId);
    }
    if (!deviceInfo.getVendorString().isEmpty()) {
        parts.append(QStringLiteral("Manufacturer: ") + deviceInfo.getVendorString());
    }
    if (!deviceInfo.getProductString().isEmpty()) {
        parts.append(QStringLiteral("Product: ") + deviceInfo.getProductString());
    }
    if (!deviceInfo.getSerialNumber().isEmpty()) {
        parts.append(QStringLiteral("S/N: ") + deviceInfo.getSerialNumber());
    }
    const auto dbgState = QDebugStateSaver(dbg);
    return dbg.nospace().noquote()
            << QStringLiteral("{ ")
            << parts.join(QStringLiteral(" | "))
            << QStringLiteral(" }");
}

bool DeviceInfo::matchProductInfo(
        const ProductInfo& product) const {
    bool ok;
    // Product and vendor match is always required
    if (vendor_id != product.vendor_id.toInt(&ok, 16) || !ok) {
        return false;
    }
    if (product_id != product.product_id.toInt(&ok, 16) || !ok) {
        return false;
    }

    // Optionally check against m_usbInterfaceNumber / usage_page && usage
    if (m_usbInterfaceNumber >= 0) {
        if (m_usbInterfaceNumber != product.interface_number.toInt(&ok, 16) || !ok) {
            return false;
        }
    } else {
        if (usage_page != product.usage_page.toInt(&ok, 16) || !ok) {
            return false;
        }
        if (usage != product.usage.toInt(&ok, 16) || !ok) {
            return false;
        }
    }
    // Match found
    return true;
}

} // namespace hid

} // namespace mixxx
