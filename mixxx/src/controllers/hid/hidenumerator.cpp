/**
* @file hidenumerator.cpp
* @author Sean Pappalardo spappalardo@mixxx.org
* @date Sat Apr 30 2011
* @brief This class handles discovery and enumeration of DJ controllers that use the USB-HID protocol
*/

#include <hidapi.h>

#include "controllers/hid/hidcontroller.h"
#include "controllers/hid/hidenumerator.h"

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

    struct hid_device_info *devs, *cur_dev;

    devs = hid_enumerate(0x0, 0x0);

    for (cur_dev = devs; cur_dev; cur_dev = cur_dev->next) {
        printf("  Found %ls %ls r%hx, S/N %ls  Interface %d\n", cur_dev->manufacturer_string,
               cur_dev->product_string, cur_dev->release_number, cur_dev->serial_number,
               cur_dev->interface_number);

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
