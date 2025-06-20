#include "controllers/bulk/bulkenumerator.h"

#include <libusb.h>

#include "controllers/bulk/bulkcontroller.h"
#include "controllers/bulk/bulksupported.h"
#include "moc_bulkenumerator.cpp"

BulkEnumerator::BulkEnumerator()
        : ControllerEnumerator(),
          m_context(nullptr) {
    libusb_init(&m_context);
}

BulkEnumerator::~BulkEnumerator() {
    qDebug() << "Deleting USB Bulk devices...";
    while (m_devices.size() > 0) {
        delete m_devices.takeLast();
    }
    libusb_exit(m_context);
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
    return m_devices;
}
