#include "controllers/bulk/bulkenumerator.h"

#include <libusb.h>

#include <QDebug>
#include <algorithm>
#include <memory>
#include <span>

#include "controllers/bulk/bulkcontroller.h"
#include "controllers/bulk/bulksupported.h"
#include "moc_bulkenumerator.cpp"
#include "util/assert.h"

BulkEnumerator::BulkEnumerator(UserSettingsPointer pConfig)
        : ControllerEnumerator(),
          m_context(nullptr, &libusb_exit),
          m_pConfig(pConfig) {
    // TODO: use C++23 std::out_ptr instead
    libusb_context* ctx;
    libusb_init(&ctx);
    m_context.reset(ctx);
}

BulkEnumerator::~BulkEnumerator() {
    qDebug() << "Deleting USB Bulk devices...";
}

static bool is_interesting(const libusb_device_descriptor& desc) {
    auto vendorId = desc.idVendor;
    auto productId = desc.idProduct;
    return std::any_of(std::cbegin(bulk_supported),
            std::cend(bulk_supported),
            [=](const auto& dev) {
                return dev.vendor_id == vendorId && dev.product_id == productId;
            });
}

QList<Controller*> BulkEnumerator::queryDevices() {
    qDebug() << "Scanning USB Bulk devices:";
    libusb_device** listPtr; // contiguous array of pointers to libusb_device
    ssize_t cnt = libusb_get_device_list(m_context.get(), &listPtr);
    VERIFY_OR_DEBUG_ASSERT(cnt >= 0) {
        // this assumes that if libusb_get_device_list returned an error, no memory was allocated
        // and listPtr is still uninitialized. Unfortunately the libusb docs don't specify the state
        // of the arguments in the case of errors.
        qWarning() << "Error retrieving devicelist: " << libusb_error_name(cnt);
    }
    auto list = std::unique_ptr<libusb_device*[],
            decltype([](libusb_device** list) {
                libusb_free_device_list(list, true);
            })>(listPtr);

    for (libusb_device* device : std::span(list.get(), cnt)) {
        struct libusb_device_descriptor desc;

        libusb_get_device_descriptor(device, &desc);
        if (!is_interesting(desc)) {
            continue;
        }
        struct libusb_device_handle* handle = nullptr;
        int err = libusb_open(device, &handle);
        if (err) {
            qWarning() << "Error opening a device:" << libusb_error_name(err);
            continue;
        }
        m_devices.push_back(std::make_unique<BulkController>(m_context.get(), handle, &desc));
    }
    QList<Controller*> devices;
    devices.reserve(m_devices.size());
    for (const auto& pController : std::as_const(m_devices)) {
        devices.push_back(pController.get());
    }
    return devices;
}
