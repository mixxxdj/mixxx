/**
* @file hidenumerator.cpp
* @author Sean Pappalardo spappalardo@mixxx.org
* @date Sat Apr 30 2011
* @brief This class handles discovery and enumeration of DJ controllers that use the USB-HID protocol
*/

#include <hidapi.h>

#include "controllers/hid/hidcontroller.h"
#include "controllers/hid/hidenumerator.h"
#include "controllers/hid/hidblacklist.h"

HidEnumerator::HidEnumerator() : ControllerEnumerator() {
}

HidEnumerator::~HidEnumerator() {
    qDebug() << "Deleting HID devices...";
    while (m_devices.size() > 0) {
        delete m_devices.takeLast();
    }
    hid_exit();
}

bool isDeviceBlacklisted(struct hid_device_info* cur_dev) {
    bool interface_number_valid = cur_dev->interface_number != -1;
    const int blacklist_len = sizeof(hid_blacklisted)/sizeof(hid_blacklisted[0]);
    for (int bl_index=0;bl_index<blacklist_len;bl_index++) {
        hid_blacklist_t blacklisted = hid_blacklisted[bl_index];
        // If vendor ids do not match, skip.
        if (cur_dev->vendor_id != blacklisted.vendor_id)
            continue;
        // If product IDs do not match, skip.
        if (cur_dev->product_id != blacklisted.product_id)
            continue;
        // Blacklist entry based on interface number
        if (blacklisted.interface_number != -1) {
            // Skip matching for devices without usage info.
            if (!interface_number_valid)
                continue;
            // If interface number is present and the interface numbers do not
            // match, skip.
            if (cur_dev->interface_number != blacklisted.interface_number) {
                continue;
            }
        } 
        // Blacklist entry based on usage_page and usage (both required)
        if (blacklisted.usage_page != 0 && blacklisted.usage != 0) {
            // Skip matching for devices with no usage_page/usage info.
            if (cur_dev->usage_page == 0 && cur_dev->usage == 0)
                continue;
            // If usage_page is different, skip.
            if (cur_dev->usage_page != blacklisted.usage_page)
                continue;
            // If usage is different, skip.
            if (cur_dev->usage != blacklisted.usage)
                continue;
        }
        return true;
    }
    return false;
}

QList<Controller*> HidEnumerator::queryDevices() {
    qDebug() << "Scanning HID devices:";

    struct hid_device_info *devs, *cur_dev;
    devs = hid_enumerate(0x0, 0x0);

    for (cur_dev = devs; cur_dev; cur_dev = cur_dev->next) {
        if (isDeviceBlacklisted(cur_dev)) {
            // OS/X and windows use usage_page/usage not interface_number
            qDebug() << "Blacklisting"
                     << cur_dev->manufacturer_string
                     << cur_dev->product_string
                     << QString("r%1").arg(cur_dev->release_number)
                     << "S/N" << cur_dev->serial_number
                     << (cur_dev->interface_number == -1 ? QString("Usage Page %1 Usage %2").arg(
                         QString::number(cur_dev->usage_page),
                         QString::number(cur_dev->usage)) :
                         QString("Interface %1").arg(cur_dev->interface_number));
            continue;
        }

        // OS/X and windows use usage_page/usage not interface_number
        qDebug() << "Found"
                 << cur_dev->manufacturer_string
                 << cur_dev->product_string
                 << QString("r%1").arg(cur_dev->release_number)
                 << "S/N" << cur_dev->serial_number
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
