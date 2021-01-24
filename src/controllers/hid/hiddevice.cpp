#include "controllers/hid/hiddevice.h"

#include <QDebugStateSaver>

#include "controllers/controllermappinginfo.h"
#include "moc_hiddevice.cpp"
#include "util/path.h" // for PATH_MAX on Windows
#include "util/string.h"

namespace {

constexpr unsigned short kGenericDesktopUsagePage = 0x01;

constexpr unsigned short kGenericDesktopPointerUsage = 0x01;
constexpr unsigned short kGenericDesktopMouseUsage = 0x02;
constexpr unsigned short kGenericDesktopJoystickUsage = 0x04;
constexpr unsigned short kGenericDesktopGamePadUsage = 0x05;
constexpr unsigned short kGenericDesktopKeyboardUsage = 0x06;
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
          interface_number(device_info.interface_number),
          m_pathRaw(device_info.path, mixxx::strnlen(device_info.path, PATH_MAX)),
          m_serialNumberRaw(device_info.serial_number,
                  mixxx::wcsnlen(device_info.serial_number,
                          kDeviceInfoStringMaxLength)),
          m_manufacturerString(mixxx::convertWCStringToQString(
                  device_info.manufacturer_string,
                  mixxx::wcsnlen(device_info.manufacturer_string,
                          kDeviceInfoStringMaxLength))),
          m_productString(mixxx::convertWCStringToQString(device_info.product_string,
                  mixxx::wcsnlen(device_info.product_string,
                          kDeviceInfoStringMaxLength))),
          m_serialNumber(mixxx::convertWCStringToQString(
                  m_serialNumberRaw.data(), m_serialNumberRaw.size())) {
}

QString DeviceInfo::formatInterface() const {
    if (interface_number < 0) {
        return QString();
    }
    return QChar('#') + QString::number(interface_number);
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
        DEBUG_ASSERT(!formatInterface().isEmpty());
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
    const auto serialSuffix = serialNumber().right(4);
    if (interface_number >= 0) {
        return productString() +
                QChar(' ') +
                serialSuffix +
                QChar('_') +
                QString::number(interface_number);
    } else {
        return productString() +
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
    const QString interface = deviceInfo.formatInterface();
    if (!interface.isEmpty()) {
        parts.append(QStringLiteral("Interface: ") + interface);
    }
    if (!deviceInfo.manufacturerString().isEmpty()) {
        parts.append(QStringLiteral("Manufacturer: ") + deviceInfo.manufacturerString());
    }
    if (!deviceInfo.productString().isEmpty()) {
        parts.append(QStringLiteral("Product: ") + deviceInfo.productString());
    }
    if (!deviceInfo.serialNumber().isEmpty()) {
        parts.append(QStringLiteral("S/N: ") + deviceInfo.serialNumber());
    }
    const auto dbgState = QDebugStateSaver(dbg);
    return dbg.nospace().noquote()
            << QStringLiteral("{ ")
            << parts.join(QStringLiteral(" | "))
            << QStringLiteral(" }");
}

QString DeviceCategory::guessFromDeviceInfoImpl(
        const DeviceInfo& deviceInfo) const {
    // This should be done somehow else, I know. But at least we get started with
    // the idea of mapping this information
    const QString interface = deviceInfo.formatInterface();
    if (!interface.isEmpty()) {
        // TODO: Guess linux device types somehow as well
        // or maybe just fill in the interface number?
        return tr("HID Interface %1: ").arg(interface) + deviceInfo.formatUsage();
    }
    if (deviceInfo.usage_page == kGenericDesktopUsagePage) {
        switch (deviceInfo.usage) {
        case kGenericDesktopPointerUsage:
            return tr("Generic HID Pointer");
        case kGenericDesktopMouseUsage:
            return tr("Generic HID Mouse");
        case kGenericDesktopJoystickUsage:
            return tr("Generic HID Joystick");
        case kGenericDesktopGamePadUsage:
            return tr("Generic HID Game Pad");
        case kGenericDesktopKeyboardUsage:
            return tr("Generic HID Keyboard");
        case kGenericDesktopKeypadUsage:
            return tr("Generic HID Keypad");
        case kGenericDesktopMultiaxisControllerUsage:
            return tr("Generic HID Multi-axis Controller");
        default:
            return tr("Unknown HID Desktop Device: ") + deviceInfo.formatUsage();
        }
    } else if (deviceInfo.vendor_id == kAppleVendorId) {
        // Apple laptop special HID devices
        if (deviceInfo.product_id == kAppleInfraredControlProductId) {
            return tr("Apple HID Infrared Control");
        } else {
            return tr("Unknown Apple HID Device: ") + deviceInfo.formatUsage();
        }
    } else {
        return tr("Unknown HID Device: ") + deviceInfo.formatUsage();
    }
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

    // Optionally check against interface_number / usage_page && usage
    if (interface_number >= 0) {
        if (interface_number != product.interface_number.toInt(&ok, 16) || !ok) {
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
