/**
* @file hidenumerator.cpp
* @author Sean Pappalardo spappalardo@mixxx.org
* @date Sat Apr 30 2011
* @brief This class handles discovery and enumeration of DJ controllers that use the USB-HID protocol
*/

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
        if (device_info.vendor_id != denylisted.vendor_id)
            continue;
        // If product IDs do not match, skip.
        if (device_info.product_id != denylisted.product_id)
            continue;
        // Denylist entry based on interface number
        if (denylisted.interface_number != -1) {
            // Skip matching for devices without usage info.
            if (!interface_number_valid)
                continue;
            // If interface number is present and the interface numbers do not
            // match, skip.
            if (device_info.interface_number != denylisted.interface_number) {
                continue;
            }
        }
        // Denylist entry based on usage_page and usage (both required)
        if (denylisted.usage_page != 0 && denylisted.usage != 0) {
            // Skip matching for devices with no usage_page/usage info.
            if (device_info.usage_page == 0 && device_info.usage == 0)
                continue;
            // If usage_page is different, skip.
            if (device_info.usage_page != denylisted.usage_page)
                continue;
            // If usage is different, skip.
            if (device_info.usage != denylisted.usage)
                continue;
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
    qDebug() << "Scanning HID devices:";

    struct hid_device_info *devs, *cur_dev;
    devs = hid_enumerate(0x0, 0x0);

    for (cur_dev = devs; cur_dev; cur_dev = cur_dev->next) {
        if (!recognizeDevice(*cur_dev)) {
            // OS/X and windows use usage_page/usage not interface_number
            qDebug()
                    << "Skipping"
                    << HidController::safeDecodeWideString(
                               cur_dev->manufacturer_string, 512)
                    << HidController::safeDecodeWideString(
                               cur_dev->product_string, 512)
                    << QString("r%1").arg(cur_dev->release_number) << "S/N"
                    << HidController::safeDecodeWideString(
                               cur_dev->serial_number, 512)
                    << (cur_dev->interface_number == -1
                                       ? QString("Usage Page %1 Usage %2")
                                                 .arg(QString::number(
                                                              cur_dev->usage_page),
                                                         QString::number(
                                                                 cur_dev->usage))
                                       : QString("Interface %1")
                                                 .arg(cur_dev->interface_number));
            continue;
        }

        // OS/X and windows use usage_page/usage not interface_number
        qDebug() << "Found"
                 << HidController::safeDecodeWideString(cur_dev->manufacturer_string, 512)
                 << HidController::safeDecodeWideString(cur_dev->product_string, 512)
                 << QString("r%1").arg(cur_dev->release_number)
                 << "S/N" << HidController::safeDecodeWideString(cur_dev->serial_number, 512)
                 << (cur_dev->interface_number == -1 ? QString("Usage Page %1 Usage %2").arg(
                     QString::number(cur_dev->usage_page),
                     QString::number(cur_dev->usage)) :
                     QString("Interface %1").arg(cur_dev->interface_number));

        if (!cur_dev->serial_number && !cur_dev->product_string) {
            qWarning() << "USB permissions problem (or device error.) Your account needs write access to USB HID controllers.";
            continue;
        }

        HidController* currentDevice = new HidController(*cur_dev);
        m_devices.push_back(currentDevice);
    }
    hid_free_enumeration(devs);

    return m_devices;
}
