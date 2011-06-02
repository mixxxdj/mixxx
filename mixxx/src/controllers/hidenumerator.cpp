/***************************************************************************
                           hidenumerator.cpp
                      HID Device Enumerator Class
                      ---------------------------
    begin                : Sat Apr 30 2011
    copyright            : (C) 2011 Sean M. Pappalardo
    email                : spappalardo@mixxx.org

***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include <hidapi.h>

#include "hidcontroller.h"
#include "hidenumerator.h"

HidEnumerator::HidEnumerator() : ControllerEnumerator() {
}

HidEnumerator::~HidEnumerator() {
    qDebug() << "Deleting HID devices...";
    QListIterator<Controller*> dev_it(m_devices);
    while (dev_it.hasNext()) {
        delete dev_it.next();
    }    
//     Node::Shutdown();
}

// Enumerate the HID MIDI devices
QList<Controller*> HidEnumerator::queryDevices() {
    qDebug() << "Scanning HID devices:";
    
    struct hid_device_info *devs, *cur_dev;
    
    devs = hid_enumerate(0x0, 0x0);
    cur_dev = devs; 
    while (cur_dev) {
        printf("  Found %ls %ls r%hx, S/N %ls  Interface %d", cur_dev->manufacturer_string, 
               cur_dev->product_string, cur_dev->release_number, cur_dev->serial_number,
               cur_dev->interface_number);
        printf("\n");
        
        HidController *currentDevice = new HidController(*(cur_dev));
        m_devices.push_back((Controller*)currentDevice);
        
        cur_dev = cur_dev->next;
    }
    hid_free_enumeration(devs);

    return m_devices;
}
