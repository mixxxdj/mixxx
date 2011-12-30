/**
  * @file mididevicehss1394.h
  * @author Sean M. Pappalardo	spappalardo@mixxx.org
  * @date Thu Feb 25 2010
  * @brief hss1394-based MIDI backend
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

#ifndef MIDIDEVICEHSS1394_H
#define MIDIDEVICEHSS1394_H

#include <QtCore>
#include <hss1394/HSS1394.h>
#include "mididevice.h"

#define MIXXX_HSS1394_BUFFER_LEN 64 /**Number of MIDI messages to buffer*/
#define MIXXX_HSS1394_NO_DEVICE_STRING "None" /**String to display for no HSS1394 devices present */
/**
  *@author Sean M. Pappalardo
  */

class DeviceChannelListener : public hss1394::ChannelListener {
    public:
        DeviceChannelListener(int id, QString name, MidiDevice* midiDevice);
        void Process(const hss1394::uint8 *pBuffer, hss1394::uint uBufferSize);
        void Disconnected();
        void Reconnected();
    private:
        int m_iId;
        QString m_sName;
        MidiDevice *m_pMidiDevice;
};

/** An HSS1394-based implementation of MidiDevice */
class MidiDeviceHss1394 : public MidiDevice {
    public:
        MidiDeviceHss1394(MidiMapping* mapping,
                           const hss1394::TNodeInfo deviceInfo,
                           int deviceIndex);
        ~MidiDeviceHss1394();
        int open();
        int close();
        void sendShortMsg(unsigned int word);
        void sendSysexMsg(unsigned char data[], unsigned int length);

    protected:
        void run() { };
        hss1394::TNodeInfo m_deviceInfo;
        int m_iDeviceIndex;
        static QList<QString> m_deviceList;
        QMutex m_mutex;         /** Protects access to this object. Makes it thread safe. */
        static QMutex m_sHSSLock;    // HSS1394 is not thread-safe, so we need to only allow one thread at a time
        hss1394::Channel* m_pChannel;
        DeviceChannelListener *m_pChannelListener;
};

#endif
