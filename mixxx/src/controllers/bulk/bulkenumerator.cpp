/**
 * @file bulkenumerator.cpp
 * @author Neale Picket  neale@woozle.org
 * @date Thu Jun 28 2012
 * @brief USB Bulk controller backend
 */

#include <libusb.h>

#include "controllers/bulk/bulkcontroller.h"
#include "controllers/bulk/bulkenumerator.h"
#include "controllers/bulk/bulksupported.h"

BulkEnumerator::BulkEnumerator() : ControllerEnumerator() {
}

BulkEnumerator::~BulkEnumerator() {
    qDebug() << "Deleting USB Bulk devices...";
    while (m_devices.size() > 0) {
        delete m_devices.takeLast();
    }
    libusb_exit(NULL);
}

static int
is_interesting(struct libusb_device_descriptor *desc)
{
    int i;

    for (i = 0; bulk_supported[i].vendor_id; i += 1) {
        if ((bulk_supported[i].vendor_id == desc->idVendor) && 
            (bulk_supported[i].product_id == desc->idProduct)) {
            return 1;
        }
    }

    return 0;
}

QList<Controller*> BulkEnumerator::queryDevices() {
    qDebug() << "Scanning USB Bulk devices:";

    libusb_device **list;
    ssize_t cnt = libusb_get_device_list(NULL, &list);
    ssize_t i = 0;
    int err = 0;

    for (i = 0; i < cnt; i++) {
        libusb_device *device = list[i];
        struct libusb_device_descriptor desc;

        libusb_get_device_descriptor(device, &desc);
        if (is_interesting(&desc)) {
            struct libusb_device_handle *handle;

            err = libusb_open(device, &handle);
            if (err) {
                qDebug() << "Error opening a device";
                continue;
            }

            BulkController* currentDevice = new BulkController(handle, &desc);
            m_devices.push_back(currentDevice);
        }
    }

    libusb_free_device_list(list, 1);

    return m_devices;
}
