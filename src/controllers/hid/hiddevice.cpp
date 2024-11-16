#include "controllers/hid/hiddevice.h"

#include <hidapi.h>

#include <QDebugStateSaver>

#include "controllers/controllermappinginfo.h"
#include "util/string.h"

namespace {

constexpr std::size_t kDeviceInfoStringMaxLength = 512;

} // namespace

namespace mixxx {

namespace hid {

DeviceInfo::DeviceInfo(
        const hid_device_info& device_info, const HidUsageTables& hidUsageTables)
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
                  m_serialNumberRaw.data(), m_serialNumberRaw.size())),
          m_hidUsageTables(hidUsageTables) {
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

    auto formatHex = [](unsigned short value) {
        return QString::number(value, 16).toUpper().rightJustified(4, '0');
    };

    parts.append(QStringLiteral("VID:%1 PID:%2")
                    .arg(formatHex(deviceInfo.getVendorId()),
                            formatHex(deviceInfo.getProductId())));

    static const QMap<PhysicalTransportProtocol, QString> protocolMap = {
            {PhysicalTransportProtocol::USB, QStringLiteral("USB")},
            {PhysicalTransportProtocol::BlueTooth, QStringLiteral("Bluetooth")},
            {PhysicalTransportProtocol::I2C, QStringLiteral("I2C")},
            {PhysicalTransportProtocol::SPI, QStringLiteral("SPI")},
            {PhysicalTransportProtocol::FireWire, QStringLiteral("Firewire")},
            {PhysicalTransportProtocol::UNKNOWN, QStringLiteral("Unknown")}};

    parts.append(QStringLiteral("Physical: %1")
                    .arg(protocolMap.value(
                            deviceInfo.getPhysicalTransportProtocol(),
                            QStringLiteral("Unknown"))));

    parts.append(QStringLiteral("Usage-Page: %1 %2")
                    .arg(formatHex(deviceInfo.getUsagePage()),
                            deviceInfo.getUsagePageDescription()));
    parts.append(QStringLiteral("Usage: %1 %2")
                    .arg(formatHex(deviceInfo.getUsage()), deviceInfo.getUsageDescription()));

    if (deviceInfo.getUsbInterfaceNumber()) {
        parts.append(QStringLiteral("Interface: #%1")
                        .arg(deviceInfo.getUsbInterfaceNumber().value()));
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
