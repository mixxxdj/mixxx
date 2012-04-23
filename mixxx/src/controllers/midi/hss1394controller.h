/**
  * @file hss1394controller.h
  * @author Sean M. Pappalardo  spappalardo@mixxx.org
  * @date Thu 15 Mar 2012
  * @brief HSS1394-based MIDI backend
  *
  * This class represents a physical HSS1394 device.
  * (HSS1394 is simply a way to send MIDI messages at high speed over
  * IEEE1394 (FireWire))
  * It uses the HSS1394 API to send and receive MIDI messages to/from
  * the device.
  */

#ifndef HSS1394CONTROLLER_H
#define HSS1394CONTROLLER_H

#include <hss1394/HSS1394.h>
#include "midicontroller.h"

#define MIXXX_HSS1394_BUFFER_LEN 64 /**Number of MIDI messages to buffer*/
#define MIXXX_HSS1394_NO_DEVICE_STRING "None" /**String to display for no HSS1394 devices present */

class DeviceChannelListener : public QObject, public hss1394::ChannelListener {
Q_OBJECT    // For signals
    public:
        DeviceChannelListener(int id, QString name, MidiController* controller);
        void Process(const hss1394::uint8 *pBuffer, hss1394::uint uBufferSize);
        void Disconnected();
        void Reconnected();
    signals:
        void incomingData(unsigned char status, unsigned char control, unsigned char value);
        void incomingData(QByteArray data);
    private:
        int m_iId;
        QString m_sName;
        MidiController *m_pController;
};

class Hss1394Controller : public MidiController {
    public:
        Hss1394Controller(const hss1394::TNodeInfo deviceInfo, int deviceIndex);
        ~Hss1394Controller();
        int open();
        int close();
        void send(unsigned int word);
        void send(QByteArray data);

    protected:
        hss1394::TNodeInfo m_deviceInfo;
        int m_iDeviceIndex;
        static QList<QString> m_deviceList;
        hss1394::Channel* m_pChannel;
        DeviceChannelListener *m_pChannelListener;
};

#endif
