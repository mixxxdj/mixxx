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

#include "controllers/controller.h"
#include "controllers/hidcontrollerpreset.h"
#include "controllers/hidcontrollerpresetfilehandler.h"

class HidReader : public QThread {
    Q_OBJECT    // For signals
    public:
        HidReader(hid_device* device);
        ~HidReader();
        void stop() { m_bStop=true; };

    signals:
        void incomingData(QByteArray data);

    protected:
        void run();

    private:
        hid_device* m_pHidDevice;
        bool m_bStop;
};

class HidController : public Controller {
    Q_OBJECT    // For Q_INVOKABLE
    public:
        HidController(const hid_device_info deviceInfo);
        virtual ~HidController();

        virtual const ControllerPreset* getPreset() const {
            // TODO(XXX) clone the preset
            return &m_preset;
        }

        virtual bool savePreset(const QString fileName) const;

        virtual ControllerPresetFileHandler* getFileHandler() const {
            return new HidControllerPresetFileHandler();
        }

        virtual void visit(const MidiControllerPreset* preset);
        virtual void visit(const HidControllerPreset* preset);

    protected:
        Q_INVOKABLE void send(QList<int> data, unsigned int length, unsigned int reportID = 0);

    private slots:
        int open();
        int close();

    private:
        //  For devices which only support a single report, reportID must be set to 0x0.
        void send(QByteArray data, unsigned int reportID = 0);

        // Local copies of things we need from hid_device_info
        int hid_interface_number;
        unsigned short hid_vendor_id;
        unsigned short hid_product_id;
        char * hid_path;
        wchar_t *hid_serial;
        QString hid_manufacturer;
        QString hid_product;

        QString m_sUID;
        static QList<QString> m_deviceList;
        hid_device* m_pHidDevice;
        HidReader* m_pReader;
        HidControllerPreset m_preset;
};

#endif
