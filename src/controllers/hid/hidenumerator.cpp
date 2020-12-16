#include "controllers/hid/hidenumerator.h"

#include <hidapi.h>

#include "controllers/hid/hidcontroller.h"
#include "controllers/hid/hiddenylist.h"

namespace {

bool recognizeDevice(const hid_device_info& device_info) {
    bool interface_number_valid = device_info.interface_number != -1;
    const int denylist_len = sizeof(hid_denylisted) / sizeof(hid_denylisted[0]);
    for (int bl_index = 0; bl_index < denylist_len; bl_index++) {
        hid_denylist_t denylisted = hid_denylisted[bl_index];
        // If vendor ids do not match, skip.
        if (device_info.vendor_id != denylisted.vendor_id) {
            continue;
        }
        // If product IDs do not match, skip.
        if (device_info.product_id != denylisted.product_id) {
            continue;
        }
        // Denylist entry based on interface number
        if (denylisted.interface_number != -1) {
            // Skip matching for devices without usage info.
            if (!interface_number_valid) {
                continue;
            }
            // If interface number is present and the interface numbers do not
            // match, skip.
            if (device_info.interface_number != denylisted.interface_number) {
                continue;
            }
        }
        // Denylist entry based on usage_page and usage (both required)
        if (denylisted.usage_page != 0 && denylisted.usage != 0) {
            // Skip matching for devices with no usage_page/usage info.
            if (device_info.usage_page == 0 && device_info.usage == 0) {
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
    while (m_devices.size() > 0) {
        delete m_devices.takeLast();
    }
    hid_exit();
}

QList<Controller*> HidEnumerator::queryDevices() {
    qInfo() << "Scanning USB HID devices";

    hid_device_info* device_info_list = hid_enumerate(0x0, 0x0);
    for (const auto* device_info = device_info_list;
            device_info;
            device_info = device_info->next) {
        auto deviceInfo = mixxx::hid::DeviceInfo(*device_info);
        if (!recognizeDevice(*device_info)) {
            qInfo()
                    << "Excluding USB HID device"
                    << deviceInfo;
            continue;
        }
        qInfo() << "Found USB HID device:"
                << deviceInfo;

        if (!deviceInfo.isValid()) {
            qWarning() << "USB permissions problem or device error."
                       << "Your account needs write access to USB HID controllers.";
            continue;
        }

        HidController* newDevice = new HidController(std::move(deviceInfo));
        m_devices.push_back(newDevice);
    }
    hid_free_enumeration(device_info_list);

    return m_devices;
}
