#include "controllers/bulk/bulkenumerator.h"

#include <QtCore/private/qandroidextras_p.h>
#include <libusb.h>
#include <qglobal.h>

#include <QJniObject>
#include <QtJniTypes>

#include "controllers/bulk/bulkcontroller.h"
#include "controllers/bulk/bulksupported.h"
#include "moc_bulkenumerator.cpp"
#include "util/assert.h"

BulkEnumerator::BulkEnumerator()
        : ControllerEnumerator(),
          m_context(nullptr) {
    int r;
#ifdef __ANDROID__
    r = libusb_set_option(nullptr, LIBUSB_OPTION_NO_DEVICE_DISCOVERY);
    VERIFY_OR_DEBUG_ASSERT(r == 0) {
        qCritical()
                << "libusb_set_option LIBUSB_OPTION_NO_DEVICE_DISCOVERY failed"
                << libusb_error_name(r);
        m_context = nullptr;
        return;
    }
#else
#endif
    r = libusb_init(&m_context);
    VERIFY_OR_DEBUG_ASSERT(r == 0) {
        qCritical() << "libusb_init failed" << libusb_error_name(r);
        m_context = nullptr;
    }
}

BulkEnumerator::~BulkEnumerator() {
    qDebug() << "Deleting USB Bulk devices...";
    while (m_devices.size() > 0) {
        delete m_devices.takeLast();
    }
    if (m_context) {
        libusb_exit(m_context);
    }
}

static bool is_interesting(const libusb_device_descriptor& desc) {
    return std::any_of(std::cbegin(bulk_supported),
            std::cend(bulk_supported),
            [&](const auto& dev) {
                return dev.key.vendor_id == desc.idVendor && dev.key.product_id == desc.idProduct;
            });
}

QList<Controller*> BulkEnumerator::queryDevices() {
    qDebug() << "Scanning USB Bulk devices:";
    VERIFY_OR_DEBUG_ASSERT(m_context) {
        return {};
    }

#ifdef __ANDROID__
    return {};

    // QJniObject javaNotification = QJniObject::fromString(m_notification);
    // QJniObject::callStaticMethod<void>(
    //                 "org/qtproject/example/androidnotifier/NotificationClient",
    //                 "notify",
    //                 "(Landroid/content/Context;Ljava/lang/String;)V",
    //                 QNativeInterface::QAndroidApplication::context(),
    //                 javaNotification.object<jstring>());

#else
    libusb_device **list;
    ssize_t cnt = libusb_get_device_list(m_context, &list);
    ssize_t i = 0;
    int err = 0;

    for (i = 0; i < cnt; i++) {
        libusb_device *device = list[i];
        struct libusb_device_descriptor desc;

        libusb_get_device_descriptor(device, &desc);
        if (is_interesting(desc)) {
            struct libusb_device_handle* handle = nullptr;
            err = libusb_open(device, &handle);
            if (err) {
                qWarning() << "Error opening a device:" << libusb_error_name(err);
                continue;
            }

            BulkController* currentDevice =
                    new BulkController(m_context, handle, &desc);
            m_devices.push_back(currentDevice);
        }
    }
    libusb_free_device_list(list, 1);
#endif
    return m_devices;
}
