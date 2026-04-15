#include "controllers/hid/hidenumerator.h"

#ifdef __ANDROID__
#include <android/api-level.h>
#include <android/log.h>
#include <hidapi_libusb.h>
#include <jni.h>
#include <libusb.h>

#include <QJniObject>
#else
#include <hidapi.h>
#endif

#include "controllers/hid/hidcontroller.h"
#include "controllers/hid/hiddenylist.h"
#include "moc_hidenumerator.cpp"
#include "util/cmdlineargs.h"

namespace mixxx {

namespace hid {

#ifndef __ANDROID__
constexpr unsigned short kGenericDesktopUsagePage = 0x01;

constexpr unsigned short kGenericDesktopMouseUsage = 0x02;
constexpr unsigned short kGenericDesktopKeyboardUsage = 0x06;
#endif

// Apple has two two different vendor IDs which are used for different devices.
constexpr unsigned short kAppleVendorId = 0x5ac;
constexpr unsigned short kAppleIncVendorId = 0x004c;

} // namespace hid

} // namespace mixxx

namespace {

bool recognizeDevice(const mixxx::hid::DeviceInfo& deviceInfo) {
    const unsigned short vendor_id = deviceInfo.getVendorId();
    const unsigned short product_id = deviceInfo.getProductId();
    const auto usbInterfaceNumber = deviceInfo.getUsbInterfaceNumber();
    const int interface_number = usbInterfaceNumber.has_value()
            ? static_cast<int>(*usbInterfaceNumber)
            : kInvalidInterfaceNumber;
// On Android, usage_page and usage are only accessible when permission is
// granted to the device, so we don't use it for device detection.
#ifndef __ANDROID__
    const unsigned short usage_page = deviceInfo.getUsagePage();
    const unsigned short usage = deviceInfo.getUsage();
    // Skip mice and keyboards. Users can accidentally disable their mouse
    // and/or keyboard by enabling them as HID controllers in Mixxx.
    // https://github.com/mixxxdj/mixxx/issues/10498
    if (!CmdlineArgs::Instance().getDeveloper() &&
            usage_page == mixxx::hid::kGenericDesktopUsagePage &&
            (usage == mixxx::hid::kGenericDesktopMouseUsage ||
                    usage == mixxx::hid::kGenericDesktopKeyboardUsage)) {
        return false;
    }
#endif

    // Apple includes a variety of HID devices in their computers, not all of which
    // match the filter above for keyboards and mice, for example "Magic Trackpad",
    // "Internal Keyboard", and "T1 Controller". Apple is likely to keep changing
    // these devices in future computers and none of these devices are DJ controllers,
    // so skip all Apple HID devices rather than maintaining a list of specific devices
    // to skip.
    if (vendor_id == mixxx::hid::kAppleVendorId || vendor_id == mixxx::hid::kAppleIncVendorId) {
        return false;
    }

    // Exclude specific devices from the denylist.
    for (const hid_denylist_t& denylisted : kHidDenyList) {
        // If vendor ids are specified and do not match, skip.
        if (denylisted.vendor_id != kAnyValue &&
                vendor_id != denylisted.vendor_id) {
            continue;
        }
        // If product IDs are specified and do not match, skip.
        if (denylisted.product_id != kAnyValue &&
                product_id != denylisted.product_id) {
            continue;
        }
        // Denylist entry based on interface number
        // If interface number is present and the interface numbers do not
        // match, skip.
        if (denylisted.interface_number != kInvalidInterfaceNumber &&
                interface_number != denylisted.interface_number) {
            continue;
        }
#ifndef __ANDROID__
        // Denylist entry based on usage_page and usage (both required)
        if (denylisted.usage_page != kAnyValue && denylisted.usage != kAnyValue) {
            // If usage_page is different, skip.
            if (usage_page != denylisted.usage_page) {
                continue;
            }
            // If usage is different, skip.
            if (usage != denylisted.usage) {
                continue;
            }
        }
#endif
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

#ifdef __ANDROID__
    QJniObject context = QNativeInterface::QAndroidApplication::context();
    QJniObject USB_SERVICE =
            QJniObject::getStaticObjectField(
                    "android/content/Context",
                    "USB_SERVICE",
                    "Ljava/lang/String;");
    auto usbManager = context.callObjectMethod("getSystemService",
            "(Ljava/lang/String;)Ljava/lang/Object;",
            USB_SERVICE.object());
    if (!usbManager.isValid()) {
        qDebug() << "usbManager invalid";
        return {};
    }

    QJniObject deviceListObject =
            usbManager.callMethod<QJniObject>("getDeviceList", "()Ljava/util/HashMap;");
    deviceListObject = deviceListObject.callMethod<jobject>("values", "()Ljava/util/Collection;");
    QJniArray<QJniObject> deviceList = QJniArray<QJniObject>(
            deviceListObject.callMethod<jobjectArray>("toArray"));
    __android_log_print(ANDROID_LOG_VERBOSE,
            "mixxx",
            "found %d USB devices for HID enumerator",
            deviceList.size());

    for (const auto& usbDevice : deviceList) {
        for (jint ifaceIdx = 0;
                ifaceIdx < usbDevice->callMethod<jint>("getInterfaceCount");
                ifaceIdx++) {
            auto usbInterface = usbDevice->callMethod<jobject>("getInterface",
                    "(I)Landroid/hardware/usb/UsbInterface;",
                    ifaceIdx);
            if (usbInterface.callMethod<jint>("getInterfaceClass") == LIBUSB_CLASS_HID) {
                auto deviceInfo = mixxx::hid::DeviceInfo(usbDevice, usbInterface);

                if (!recognizeDevice(deviceInfo)) {
                    qInfo() << "Excluding HID device" << deviceInfo;
                    continue;
                }

                qInfo() << "Found HID device:" << deviceInfo;

                if (!deviceInfo.isValid()) {
                    qWarning() << "HID device permissions problem or device error."
                               << "Your account needs write access to HID controllers.";
                    continue;
                }

                HidController* newDevice = new HidController(std::move(deviceInfo));
                m_devices.push_back(newDevice);
            }
        }
    }
#else

    QStringList enumeratedDevices;
    hid_device_info* p_device_info_list = hid_enumerate(0x0, 0x0);
    for (const auto* p_device_info = p_device_info_list;
            p_device_info;
            p_device_info = p_device_info->next) {
        auto deviceInfo = mixxx::hid::DeviceInfo(*p_device_info);
        // The hidraw backend of hidapi on Linux returns many duplicate hid_device_info's from hid_enumerate,
        // so filter them out.
        // https://github.com/libusb/hidapi/issues/298
        if (enumeratedDevices.contains(deviceInfo.pathRaw())) {
            qInfo() << "Duplicate HID device, excluding" << deviceInfo;
            continue;
        }
        enumeratedDevices.append(QString(deviceInfo.pathRaw()));

        if (!recognizeDevice(deviceInfo)) {
            qInfo() << "Excluding HID device" << deviceInfo;
            continue;
        }
        qInfo() << "Found HID device:" << deviceInfo;

        if (!deviceInfo.isValid()) {
            qWarning() << "HID device permissions problem or device error."
                       << "Your account needs write access to HID controllers.";
            continue;
        }

        HidController* newDevice = new HidController(std::move(deviceInfo));
        m_devices.push_back(newDevice);
    }
    hid_free_enumeration(p_device_info_list);
#endif

    return m_devices;
}
