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

#ifdef __ANDROID__
#include "controllers/android.h"
#include "controllers/midi/androidusbmidicontroller.h"
#endif

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
#ifdef __ANDROID__
        if (denylisted.vendor_id == kAnyValue &&
                denylisted.product_id == kAnyValue) {
            // these wildcard entries would deny any devices on Android, skip
            continue;
        }
#endif
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

#ifdef __ANDROID__
    // On Android, the DDJ-FLX4 BLE chip (2B73:0045) exposes two HID
    // interfaces (5 & 6). Interface 6 is already denied by the deny list
    // above. Interface 5 is a HID class interface that only carries
    // Bluetooth data, not deck control data. The actual deck HID is on
    // the main chip (08E4:017B) interface 10 (vendor-specific class 0xFF)
    // but is invisible on Android. Skip interface 5 to avoid a dead HID
    // controller that would override the working MIDI controller.
    if (vendor_id == 0x2b73 && product_id == 0x0045 &&
            interface_number == 0x5) {
        return false;
    }
#endif
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
    if (!context.isValid()) {
        qWarning() << "HID enumerator: Android context is null";
        return {};
    }
    QJniObject USB_SERVICE =
            QJniObject::getStaticObjectField(
                    "android/content/Context",
                    "USB_SERVICE",
                    "Ljava/lang/String;");
    auto usbManager = context.callObjectMethod("getSystemService",
            "(Ljava/lang/String;)Ljava/lang/Object;",
            USB_SERVICE.object());
    if (!usbManager.isValid()) {
        qWarning() << "HID enumerator: usbManager invalid (no USB_SERVICE?)";
        return {};
    }

    QJniObject deviceListObject =
            usbManager.callMethod<QJniObject>("getDeviceList", "()Ljava/util/HashMap;");
    deviceListObject = deviceListObject.callMethod<QJniObject>(
            "values", "()Ljava/util/Collection;");
    if (!deviceListObject.isValid()) {
        qWarning() << "HID enumerator: getDeviceList returned null";
        return {};
    }
    QJniArray<QJniObject> deviceList = QJniArray<QJniObject>(
            deviceListObject.callMethod<QJniArray<QJniObject>>("toArray"));
    qInfo() << "HID enumerator: found" << deviceList.size() << "USB devices";
    if (deviceList.size() == 0) {
        qInfo() << "HID enumerator: no USB devices with permission. "
                   "Unplug/replug controller and grant permission.";
        QThread::msleep(500);
        deviceListObject = usbManager.callMethod<QJniObject>(
                "getDeviceList", "()Ljava/util/HashMap;");
        deviceListObject = deviceListObject.callMethod<QJniObject>(
                "values", "()Ljava/util/Collection;");
        if (deviceListObject.isValid()) {
            deviceList = QJniArray<QJniObject>(
                    deviceListObject.callMethod<QJniArray<QJniObject>>(
                            "toArray"));
            qInfo() << "HID enumerator: retry found" << deviceList.size()
                    << "USB devices";
        }
    }

    for (const auto& usbDevice : deviceList) {
        if (!usbDevice->isValid()) {
            continue;
        }
        QString productName =
                usbDevice->callObjectMethod("getProductName",
                                 "()Ljava/lang/String;")
                        .toString();
        jint vid = usbDevice->callMethod<jint>("getVendorId");
        jint pid = usbDevice->callMethod<jint>("getProductId");
        jint ifaceCount = usbDevice->callMethod<jint>("getInterfaceCount");
        qInfo() << "HID enumerator: USB device" << productName
                << "VID=0x" << Qt::hex << vid << "PID=0x" << pid
                << Qt::dec << "interfaces=" << ifaceCount;
        for (jint ifaceIdx = 0;
                ifaceIdx < usbDevice->callMethod<jint>("getInterfaceCount");
                ifaceIdx++) {
            auto usbInterface = usbDevice->callMethod<QJniObject>("getInterface",
                    "(I)Landroid/hardware/usb/UsbInterface;",
                    ifaceIdx);
            jint ifaceClass = usbInterface.callMethod<jint>("getInterfaceClass");
            jint ifaceSubclass = usbInterface.callMethod<jint>("getInterfaceSubclass");
            qInfo() << "HID enumerator: iface" << ifaceIdx
                    << "class=" << ifaceClass
                    << "subclass=" << ifaceSubclass;
            if (ifaceClass == LIBUSB_CLASS_HID
#ifdef __ANDROID__
                    || ifaceClass == 0xFF // vendor-specific — DDJ-FLX4 deck HID on Android
#endif
            ) {
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
#ifdef __ANDROID__
            } else if (ifaceClass == 1 && // LIBUSB_CLASS_AUDIO
                    ifaceSubclass == 3) {
                // MIDI Streaming interface (class 1 subclass 3)
                // Found on DDJ-FLX4 main chip interface 3
                // Use direct bulkTransfer instead of MidiManager
                qInfo() << "Found USB MIDI interface" << ifaceIdx
                        << "on device"
                        << usbDevice->callObjectMethod("getProductName",
                                            "()Ljava/lang/String;")
                                   .toString();

                // Open device and claim interface for raw MIDI access
                auto usbManager = context.callObjectMethod("getSystemService",
                        "(Ljava/lang/String;)Ljava/lang/Object;",
                        USB_SERVICE.object());
                if (!usbManager.isValid()) {
                    qWarning() << "Cannot get UsbManager for MIDI device";
                    continue;
                }

                if (!usbManager.callMethod<jboolean>("hasPermission",
                            "(Landroid/hardware/usb/UsbDevice;)Z",
                            usbDevice)) {
                    auto pendingIntent = mixxx::android::getIntent();
                    usbManager.callMethod<void>("requestPermission",
                            "(Landroid/hardware/usb/UsbDevice;Landroid/app/"
                            "PendingIntent;)V",
                            usbDevice,
                            pendingIntent);
                    if (!mixxx::android::waitForPermission(usbDevice)) {
                        qWarning() << "MIDI device permission denied";
                        continue;
                    }
                }

                auto usbConnection = usbManager.callMethod<QJniObject>(
                        "openDevice",
                        "(Landroid/hardware/usb/UsbDevice;)"
                        "Landroid/hardware/usb/UsbDeviceConnection;",
                        usbDevice);
                if (!usbConnection.isValid()) {
                    qWarning() << "Failed to open MIDI USB connection";
                    continue;
                }

                // Get the file descriptor — we'll use libusb to claim the
                // interface and do bulk transfers, because Android's JNI
                // claimInterface fails on composite audio/MIDI devices
                // (the audio subsystem owns the kernel driver).
                jint fd = usbConnection.callMethod<jint>("getFileDescriptor");
                int ifaceId = usbInterface.callMethod<jint>("getId");
                qInfo() << "HID enumerator: MIDI USB device opened FD="
                        << fd << "interface=" << ifaceId;

                // Discover bulk endpoint addresses (we need the raw addresses
                // for libusb_bulk_transfer, not JNI endpoint objects)
                uint8_t bulkInEp = 0, bulkOutEp = 0;
                jint epCount = usbInterface.callMethod<jint>("getEndpointCount");
                qInfo() << "HID enumerator: MIDI iface" << ifaceId
                        << "has" << epCount << "endpoints";
                for (jint i = 0; i < epCount; i++) {
                    auto ep = usbInterface.callMethod<QJniObject>(
                            "getEndpoint",
                            "(I)Landroid/hardware/usb/UsbEndpoint;",
                            i);
                    jint epAddr = ep.callMethod<jint>("getAddress");
                    jint epType = ep.callMethod<jint>("getType");
                    jint epMaxPkt = ep.callMethod<jint>("getMaxPacketSize");
                    jint epDir = ep.callMethod<jint>("getDirection");
                    qInfo() << "HID enumerator:   ep" << i
                            << "addr=0x" << Qt::hex << epAddr
                            << "type=" << Qt::dec << epType
                            << "dir=" << epDir
                            << "maxPkt=" << epMaxPkt;
                    if (epType != 2)
                        continue;
                    if (epAddr & 0x80) {
                        bulkInEp = static_cast<uint8_t>(epAddr);
                    } else {
                        bulkOutEp = static_cast<uint8_t>(epAddr);
                    }
                }

                if (!bulkInEp && !bulkOutEp) {
                    qWarning() << "No bulk endpoints found on MIDI interface";
                    continue;
                }

                QString productName =
                        usbDevice->callObjectMethod("getProductName",
                                         "()Ljava/lang/String;")
                                .toString();
                QString manufacturerName =
                        usbDevice->callObjectMethod("getManufacturerName",
                                         "()Ljava/lang/String;")
                                .toString();
                jint vendorId = usbDevice->callMethod<jint>("getVendorId");
                jint productId = usbDevice->callMethod<jint>("getProductId");
                int ifaceNum =
                        usbInterface.callMethod<jint>("getId");

                // Create a unique name including interface number
                QString devName = QString("%1 M%2")
                                          .arg(productName)
                                          .arg(ifaceNum);

                auto* midiDevice = new AndroidUsbMidiController(
                        devName, vendorId, productId, manufacturerName, productName);
                midiDevice->setAndroidDevice(
                        QJniObject(*usbDevice),
                        fd,
                        ifaceId,
                        bulkInEp,
                        bulkOutEp,
                        static_cast<uint16_t>(vendorId),
                        static_cast<uint16_t>(productId));
                m_devices.push_back(midiDevice);
#endif
            } else {
                qInfo() << "Skipping non-HID interface" << ifaceIdx << "class"
                        << ifaceClass << "on device"
                        << usbDevice->callObjectMethod("getProductName",
                                            "()Ljava/lang/String;")
                                   .toString();
            }
        }
    }
#else

    // Explicitly initialise hidapi before hid_enumerate so the hidraw backend
    // is fully set up on the main thread. The background fetch threads spawned
    // by HidController::fetchReportDescriptorInBackground() also call hid_open*
    // (which internally calls hid_enumerate); those calls are serialised by
    // s_hidOpenMutex in hidcontroller.cpp to avoid concurrent udev access.
    if (hid_init() != 0) {
        qWarning() << "Failed to initialise hidapi";
    }

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
