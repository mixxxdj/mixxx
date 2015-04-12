/**
* @file hidenumerator.h
* @author Sean Pappalardo spappalardo@mixxx.org
* @date Sat Apr 30 2011
* @brief This class handles discovery and enumeration of DJ controllers that use the USB-HID protocol
*/

#ifndef HIDENUMERATOR_H
#define HIDENUMERATOR_H

#include <hidapi.h>

#include "controllers/controllerenumerator.h"

class HidEnumerator : public ControllerEnumerator {
  public:
    HidEnumerator();
    virtual ~HidEnumerator();

    QList<Controller*> queryDevices();

  private:
    QList<Controller*> m_devices;
    bool isDuplicate(hid_device_info *dev);
};

#endif
