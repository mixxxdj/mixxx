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

QList<Controller*> HidEnumerator::queryDevices() {
    qDebug() << "Scanning HID devices:";

    int bl_index,is_blacklisted;
    hid_blacklist_t blacklisted;
    int blacklist_len = sizeof(hid_blacklisted)/sizeof(hid_blacklist_t);

    struct hid_device_info *devs, *cur_dev;

    devs = hid_enumerate(0x0, 0x0);

    for (cur_dev = devs; cur_dev; cur_dev = cur_dev->next) {

        is_blacklisted = 0;
        for (bl_index=0;bl_index<blacklist_len;bl_index++) {
            blacklisted = hid_blacklisted[bl_index];
            if (cur_dev->vendor_id != blacklisted.vendor_id) continue;
            if (cur_dev->product_id != blacklisted.product_id) continue;
            if (blacklisted.interface_number!=-1) {
                if (cur_dev->interface_number==blacklisted.interface_number) continue;
            }
            if (cur_dev->usage_page!=0 && blacklisted.usage_page!=0) {
                if (cur_dev->usage_page!=blacklisted.usage_page) continue;
            }
            if (cur_dev->usage==0 || blacklisted.usage==0) {
                if (cur_dev->usage!=blacklisted.usage) continue;
            }
            is_blacklisted = 1;
            break;
        }
        if (is_blacklisted) {
            // OS/X and windows use usage_page/usage not interface_number (linux)
            if (cur_dev->interface_number==-1) {
                printf("  Blacklisted %ls %ls r%hx, S/N %ls  Usage Page %d Usage %d\n", cur_dev->manufacturer_string,
                       cur_dev->product_string, cur_dev->release_number, cur_dev->serial_number,
                       cur_dev->usage_page, cur_dev->usage);
            } else {
                printf("  Blacklisted %ls %ls r%hx, S/N %ls  Interface %d\n", cur_dev->manufacturer_string,
                    cur_dev->product_string, cur_dev->release_number, cur_dev->serial_number,
                    cur_dev->interface_number);
            }
            continue;
        }

        // OS/X and windows use usage_page/usage not interface_number (linux)
        if (cur_dev->interface_number==-1) {
            printf("  Found %ls %ls r%hx, S/N %ls  Usage Page %d Usage %d\n", cur_dev->manufacturer_string,
                       cur_dev->product_string, cur_dev->release_number, cur_dev->serial_number,
                       cur_dev->usage_page, cur_dev->usage);
        } else {
            printf("  Found %ls %ls r%hx, S/N %ls  Interface %d\n", cur_dev->manufacturer_string,
                       cur_dev->product_string, cur_dev->release_number, cur_dev->serial_number,
                       cur_dev->interface_number);
        }

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
