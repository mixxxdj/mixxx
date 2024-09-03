#include "controllers/hid/hidenumerator.h"

#include <hidapi.h>

#include <memory>
#include <unordered_set>

#include "controllers/hid/hidcontroller.h"
#include "controllers/hid/hiddenylist.h"
#include "controllers/hid/hiddevice.h"
#include "moc_hidenumerator.cpp"
#include "util/cmdlineargs.h"

namespace {

bool recognizeDevice(const hid_device_info& device_info) {
    // Skip mice and keyboards. Users can accidentally disable their mouse
    // and/or keyboard by enabling them as HID controllers in Mixxx.
    // https://github.com/mixxxdj/mixxx/issues/10498
    if (!CmdlineArgs::Instance().getDeveloper() &&
            device_info.usage_page == mixxx::hid::kGenericDesktopUsagePage &&
            (device_info.usage == mixxx::hid::kGenericDesktopMouseUsage ||
                    device_info.usage == mixxx::hid::kGenericDesktopKeyboardUsage)) {
        return false;
    }

    // Apple includes a variety of HID devices in their computers, not all of which
    // match the filter above for keyboards and mice, for example "Magic Trackpad",
    // "Internal Keyboard", and "T1 Controller". Apple is likely to keep changing
    // these devices in future computers and none of these devices are DJ controllers,
    // so skip all Apple HID devices rather than maintaining a list of specific devices
    // to skip.
    if (device_info.vendor_id == mixxx::hid::kAppleVendorId
          || device_info.vendor_id == mixxx::hid::kAppleIncVendorId) {
        return false;
    }

    // Exclude specific devices from the denylist.
    for (const hid_denylist_t& denylisted : hid_denylisted) {
        // If vendor ids are specified and do not match, skip.
        if (denylisted.vendor_id != kBlanketValue &&
                device_info.vendor_id != denylisted.vendor_id) {
            continue;
        }
        // If product IDs are specified and do not match, skip.
        if (denylisted.product_id != kBlanketValue &&
                device_info.product_id != denylisted.product_id) {
            continue;
        }
        // Denylist entry based on interface number
        // If interface number is present and the interface numbers do not
        // match, skip.
        if (denylisted.interface_number != kInvalidInterfaceNumber &&
                device_info.interface_number != denylisted.interface_number) {
            continue;
        }
        // Denylist entry based on usage_page and usage (both required)
        if (denylisted.usage_page != kBlanketValue && denylisted.usage != kBlanketValue) {
            // Skip matching for devices with no usage_page/usage info.
            if (device_info.usage_page == kBlanketValue && device_info.usage == kBlanketValue) {
                continue;
            }
            // If usage_page is different, skip.
            if (device_info.usage_page != denylisted.usage_page) {
                continue;
            }
            // If usage is different, skip.
            if (device_info.usage != denylisted.usage) {
                continue;
            }
        }
        return false;
    }
    return true;
}

} // namespace

HidEnumerator::~HidEnumerator() {
    qDebug() << "Deleting HID devices...";
    // devices must be released before hid_exit
    m_devices.clear();
    hid_exit();
}

QList<Controller*> HidEnumerator::queryDevices() {
    qInfo() << "Scanning USB HID devices";

    std::unordered_set<std::string> enumeratedDevices;
    auto device_info_list = std::unique_ptr<hid_device_info,
            decltype([](hid_device_info* dev) { hid_free_enumeration(dev); })>(
            hid_enumerate(0x0, 0x0));
    for (const auto* device_info = device_info_list.get();
            device_info;
            device_info = device_info->next) {
        auto deviceInfo = mixxx::hid::DeviceInfo(*device_info);
        // The hidraw backend of hidapi on Linux returns many duplicate hid_device_info's from hid_enumerate,
        // so filter them out.
        // https://github.com/libusb/hidapi/issues/298
        bool duplicateDevice = !enumeratedDevices.emplace(deviceInfo.pathRaw()).second;
        if (duplicateDevice) {
            qInfo() << "Duplicate HID device, excluding" << deviceInfo;
            continue;
        }

        if (!recognizeDevice(*device_info)) {
            qInfo()
                    << "Excluding HID device"
                    << deviceInfo;
            continue;
        }
        qInfo() << "Found HID device:"
                << deviceInfo;

        if (!deviceInfo.isValid()) {
            qWarning() << "HID device permissions problem or device error."
                       << "Your account needs write access to HID controllers.";
            continue;
        }

        m_devices.push_back(std::make_unique<HidController>(std::move(deviceInfo)));
    }
    QList<Controller*> devices;
    devices.reserve(m_devices.size());
    for (const auto& pDevice : m_devices) {
        devices.push_back(pDevice.get());
    }

    return devices;
}
