/**
  * @file hidcontroller.h
  * @author Sean M. Pappalardo	spappalardo@mixxx.org
  * @date Sun May 1 2011
  * @brief HID controller backend
  *
  */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef HIDCONTROLLER_H
#define HIDCONTROLLER_H

// #include <QtCore>
#include <hidapi.h>
#include "controller.h"

class HidController : public Controller {
    public:
        HidController(const hid_device_info deviceInfo);
        ~HidController();
        int open();
        int close();
        //  For devices which only support a single report, reportID must be set to 0x0.
        void send(unsigned char data[], unsigned int length, unsigned int reportID = 0);

    protected:
        hid_device_info m_deviceInfo;
        QString m_sUID;
        static QList<QString> m_deviceList;
        
    private:
        hid_device* m_pHidDevice;
};

#endif
