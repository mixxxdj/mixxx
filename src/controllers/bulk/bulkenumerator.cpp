#include "controllers/bulk/bulkenumerator.h"

#include <libusb.h>
#include <qglobal.h>

#ifdef __ANDROID__
#include <QtCore/private/qandroidextras_p.h>
#include <android/api-level.h>
#include <android/log.h>
#include <jni.h>

#include <QJniObject>
#include <QtJniTypes>
#endif

#include "controllers/bulk/bulkcontroller.h"
#include "controllers/bulk/bulksupported.h"
#include "moc_bulkenumerator.cpp"
#include "util/assert.h"

#ifndef __ANDROID__
BulkEnumerator::BulkEnumerator()
        : ControllerEnumerator(),
          m_pContext(nullptr) {
    int r;
    r = libusb_init(&m_pContext);
    VERIFY_OR_DEBUG_ASSERT(r == 0) {
        qCritical() << "libusb_init failed" << libusb_error_name(r);
        m_pContext = nullptr;
    }
}
#else
BulkEnumerator::BulkEnumerator() = default;
#endif

BulkEnumerator::~BulkEnumerator() {
    qDebug() << "Deleting USB Bulk devices...";
    while (m_devices.size() > 0) {
        delete m_devices.takeLast();
    }
#ifndef __ANDROID__
    if (m_pContext) {
        libusb_exit(m_pContext);
    }
#endif
}

static bool is_interesting(const uint16_t idVendor, const uint16_t idProduct) {
    return std::any_of(std::cbegin(bulk_supported),
            std::cend(bulk_supported),
            [&](const auto& dev) {
                return dev.key.vendor_id == idVendor && dev.key.product_id == idProduct;
            });
}

QList<Controller*> BulkEnumerator::queryDevices() {
    qDebug() << "Scanning USB Bulk devices:";
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
            "found %d USB devices for BULK enumerator",
            deviceList.size());

    for (const auto& pUsbDevice : deviceList) {
        const uint16_t idVendor = static_cast<unsigned short>(
                pUsbDevice->callMethod<jint>("getVendorId"));
        ;
        const uint16_t idProduct = static_cast<unsigned short>(
                pUsbDevice->callMethod<jint>("getProductId"));
        ;
        if (is_interesting(idVendor, idProduct)) {
            BulkController* pCurrentDevice =
                    new BulkController(pUsbDevice);
            m_devices.push_back(pCurrentDevice);
        }
    }
#else
    VERIFY_OR_DEBUG_ASSERT(m_pContext) {
        return {};
    }
    libusb_device** ppList;
    ssize_t cnt = libusb_get_device_list(m_pContext, &ppList);
    ssize_t i = 0;
    int err = 0;

    for (i = 0; i < cnt; i++) {
        libusb_device* pDevice = ppList[i];
        struct libusb_device_descriptor desc;

        libusb_get_device_descriptor(pDevice, &desc);
        if (is_interesting(desc.idVendor, desc.idProduct)) {
            struct libusb_device_handle* pHandle = nullptr;
            err = libusb_open(pDevice, &pHandle);
            if (err) {
                qWarning() << "Error opening a device:" << libusb_error_name(err);
                continue;
            }

            BulkController* pCurrentDevice =
                    new BulkController(m_pContext, pHandle, &desc);
            m_devices.push_back(pCurrentDevice);
        }
    }
    libusb_free_device_list(ppList, 1);
#endif
    return m_devices;
}
