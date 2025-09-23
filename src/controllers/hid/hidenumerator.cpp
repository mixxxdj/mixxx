#include "controllers/hid/hidenumerator.h"

#ifdef __ANDROID__
#include <hidapi_libusb.h>

#include "controllers/android.h"
#else
#include <hidapi.h>
#endif

#include "controllers/hid/hidcontroller.h"
#include "controllers/hid/hiddenylist.h"
#include "controllers/hid/hidusagetables.h"
#include "moc_hidenumerator.cpp"
#include "util/cmdlineargs.h"

namespace mixxx {

namespace hid {

constexpr unsigned short kGenericDesktopUsagePage = 0x01;

constexpr unsigned short kGenericDesktopMouseUsage = 0x02;
constexpr unsigned short kGenericDesktopKeyboardUsage = 0x06;

// Apple has two two different vendor IDs which are used for different devices.
constexpr unsigned short kAppleVendorId = 0x5ac;
constexpr unsigned short kAppleIncVendorId = 0x004c;

} // namespace hid

} // namespace mixxx

bool HidEnumerator::recognizeDevice(const hid_device_info& device_info) const {
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
        if (denylisted.vendor_id != kAnyValue &&
                device_info.vendor_id != denylisted.vendor_id) {
            continue;
        }
        // If product IDs are specified and do not match, skip.
        if (denylisted.product_id != kAnyValue &&
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
        if (denylisted.usage_page != kAnyValue && denylisted.usage != kAnyValue) {
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

HidEnumerator::~HidEnumerator() {
    qDebug() << "Deleting HID devices...";
    while (m_devices.size() > 0) {
        delete m_devices.takeLast();
    }
    hid_exit();
}

QList<Controller*> HidEnumerator::queryDevices() {
    qInfo() << "Scanning USB HID devices";

    QStringList enumeratedDevices;

    auto process = [this, &enumeratedDevices](
                           const hid_device_info* device_info,
                           mixxx::hid::DeviceInfo deviceInfo) {
        // The hidraw backend of hidapi on Linux returns many duplicate hid_device_info's from hid_enumerate,
        // so filter them out.
        // https://github.com/libusb/hidapi/issues/298
        if (enumeratedDevices.contains(deviceInfo.pathRaw())) {
            qInfo() << "Duplicate HID device, excluding" << deviceInfo;
            return;
        }
        enumeratedDevices.append(QString(deviceInfo.pathRaw()));

        if (!recognizeDevice(*device_info)) {
            qInfo()
                    << "Excluding HID device"
                    << deviceInfo;
            return;
        }
        qInfo() << "Found HID device:"
                << deviceInfo;

        if (!deviceInfo.isValid()) {
            qWarning() << "HID device permissions problem or device error."
                       << "Your account needs write access to HID controllers.";
            return;
        }

        HidController* newDevice = new HidController(std::move(deviceInfo));
        m_devices.push_back(newDevice);
    };

#ifdef __ANDROID__
    qInfo() << "Android - Waiting for device to be ready";
    mixxx::android::wait_for_ready();
    qInfo() << "Android - Device are ready to query";

    for (const auto& device : mixxx::android::devices) {
        if (device.type != mixxx::android::DeviceType::HID) {
            continue;
        }
        qInfo() << "Android - Proceed with" << device.fd;

        hid_device* hid_device = hid_libusb_wrap_sys_device(device.fd, device.num);
        if (!hid_device) {
            qWarning() << "Android - hid_libusb_wrap_sys_device failed on "
                       << device.fd << hid_error(nullptr);
            continue;
        }

        auto* device_info = hid_get_device_info(hid_device);
        if (!device_info) {
            qWarning() << "Android - hid_get_device_info failed on "
                       << device.fd << hid_error(hid_device);
            continue;
        }
        qDebug() << "Android - processing HID device";
        process(device_info, mixxx::hid::DeviceInfo(*device_info, device.fd, device.num));
    }
#else
    hid_device_info* device_info_list = hid_enumerate(0x0, 0x0);
    for (const auto* device_info = device_info_list;
            device_info;
            device_info = device_info->next) {
        process(device_info, mixxx::hid::DeviceInfo(*device_info));
    }
    hid_free_enumeration(device_info_list);
#endif

    return m_devices;
}
