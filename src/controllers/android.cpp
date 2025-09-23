#include "android.h"

#include <libusb.h>

#include <QtJniTypes>
#include <cstddef>

Q_DECLARE_JNI_CLASS(MainActivityClass, "org/mixxx/MyAppActivity")

namespace mixxx {
namespace android {

std::vector<DeviceIface> devices = {};
libusb_context* libusb_ctx = nullptr;
bool devicesReady = false;
std::mutex m_deviceMutex = {};
std::condition_variable m_deviceReadyWaitCond = {};

void register_usb_device(
        intptr_t usb_device_file_descriptor) {
    int r;

    if (libusb_ctx == nullptr) {
        r = libusb_set_option(nullptr, LIBUSB_OPTION_NO_DEVICE_DISCOVERY);
        qDebug() << "libusb_set_option LIBUSB_OPTION_NO_DEVICE_DISCOVERY" << r;

        r = libusb_init(&libusb_ctx);
        if (r < 0) {
            qCritical() << "Android - Unable to perform libusb_init" << r << libusb_error_name(r);
            return;
        }
    }

    struct libusb_device_handle* dev_handle;
    r = libusb_wrap_sys_device(libusb_ctx, usb_device_file_descriptor, &dev_handle);
    if (r < 0) {
        qDebug() << "libusb_wrap_sys_device" << r;
        libusb_exit(libusb_ctx);
        return;
    }

    libusb_device* dev = libusb_get_device(dev_handle);
    struct libusb_device_descriptor desc;
    r = libusb_get_device_descriptor(dev, &desc);
    if (r < 0) {
        qWarning() << "Android - Unable to perform libusb_get_device_descriptor"
                   << r << libusb_error_name(r);
        return;
    }

    qDebug() << "Android - USB Device:" << desc.idVendor << desc.idProduct
             << "- Config count:" << desc.bNumConfigurations;

    for (int configIdx = 0; configIdx < desc.bNumConfigurations; configIdx++) {
        struct libusb_config_descriptor* config;
        r = libusb_get_config_descriptor(dev, configIdx, &config);
        if (r < 0) {
            qWarning() << "Android - Unable to perform "
                          "libusb_get_config_descriptor on"
                       << configIdx << libusb_error_name(r);
            continue;
        }

        qDebug() << "Android - USB Device:" << desc.idVendor << desc.idProduct
                 << "- Interface count:" << config->bNumInterfaces;

        for (int ifaceIdx = 0; ifaceIdx < config->bNumInterfaces; ifaceIdx++) {
            const auto& iface = config->interface[ifaceIdx];
            for (int i = 0; i < iface.num_altsetting; i++) {
                const struct libusb_interface_descriptor& altsetting = iface.altsetting[i];
                // qDebug() << QString::asprintf("  Interface: %d\n",
                // altsetting.bInterfaceNumber); qDebug() << QString::asprintf("
                // Class: %02X\n", altsetting.bInterfaceClass); qDebug() <<
                // QString::asprintf("    SubClass: %02X\n",
                // altsetting.bInterfaceSubClass); qDebug() <<
                // QString::asprintf("    Protocol: %02X\n",
                // altsetting.bInterfaceProtocol);

                if (altsetting.bInterfaceClass == LIBUSB_CLASS_HID) {
                    // qDebug() << QString::asprintf("    **HID Interface**\n");
                    devices.push_back({
                            .fd = usb_device_file_descriptor,
                            .num = ifaceIdx,
                            .type = DeviceType::HID,
                    });
                    continue;
                }

                for (int j = 0; j < altsetting.bNumEndpoints; j++) {
                    const auto& ep = altsetting.endpoint[j];
                    if ((ep.bmAttributes & LIBUSB_TRANSFER_TYPE_MASK) ==
                            LIBUSB_TRANSFER_TYPE_BULK) {
                        devices.push_back({
                                .fd = usb_device_file_descriptor,
                                .num = ifaceIdx,
                                .type = DeviceType::BULK,
                        });
                        break;
                    }
                    // qDebug() << QString::asprintf("    Endpoint: %02X\n",
                    // ep.bEndpointAddress); qDebug() << QString::asprintf("
                    // Type: %s\n",
                    //     (ep.bmAttributes & LIBUSB_TRANSFER_TYPE_MASK) ==
                    //     LIBUSB_TRANSFER_TYPE_BULK ? "Bulk" : (ep.bmAttributes
                    //     & LIBUSB_TRANSFER_TYPE_MASK) ==
                    //     LIBUSB_TRANSFER_TYPE_INTERRUPT ? "Interrupt" :
                    //     (ep.bmAttributes & LIBUSB_TRANSFER_TYPE_MASK) ==
                    //     LIBUSB_TRANSFER_TYPE_ISOCHRONOUS ? "Isochronous" :
                    //     (ep.bmAttributes & LIBUSB_TRANSFER_TYPE_MASK) ==
                    //     LIBUSB_TRANSFER_TYPE_CONTROL ? "Control" :
                    //     "Unknown");
                }
            }
        }
        libusb_free_config_descriptor(config);
    }

    libusb_close(dev_handle);

    qDebug() << "Android - successfully parsed device" << usb_device_file_descriptor;
}

bool wait_for_ready() {
    // std::unique_lock lock(m_deviceMutex);
    // return m_deviceReadyWaitCond.wait_for(lock, std::chrono::seconds(10), [&]
    // { return devicesReady; });
    return true;
}

void finishUsbRegistering() {
    std::unique_lock lock(m_deviceMutex);
    devicesReady = true;
    if (libusb_ctx == nullptr) {
        libusb_exit(libusb_ctx);
        libusb_ctx = nullptr;
    }
    qDebug() << "Android - USB Devices ready. Number of devices: " << devices.size();
    m_deviceReadyWaitCond.notify_one();
}

} // namespace android
} // namespace mixxx

void registerUsbDevice(JNIEnv*, jobject, jint usb_device_file_descriptor) {
    mixxx::android::register_usb_device((intptr_t)usb_device_file_descriptor);
}
void finishUsbRegistering(JNIEnv*, jobject) {
    mixxx::android::finishUsbRegistering();
}

Q_DECLARE_JNI_NATIVE_METHOD(registerUsbDevice)
Q_DECLARE_JNI_NATIVE_METHOD(finishUsbRegistering)

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM*, void*) {
    QJniEnvironment env;
    env.registerNativeMethods<QtJniTypes::MainActivityClass>({
            Q_JNI_NATIVE_METHOD(registerUsbDevice),
            Q_JNI_NATIVE_METHOD(finishUsbRegistering),
    });
    return JNI_VERSION_1_6;
}
