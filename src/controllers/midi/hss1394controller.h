#pragma once

#include <hss1394/HSS1394.h>

#include "controllers/midi/midicontroller.h"
#include "util/duration.h"

#define MIXXX_HSS1394_BUFFER_LEN 64 /**Number of MIDI messages to buffer*/
#define MIXXX_HSS1394_NO_DEVICE_STRING "None" /**String to display for no HSS1394 devices present */

/// HSS1394-based MIDI backend
///
/// This class represents a physical HSS1394 device. (HSS1394 is simply a way
/// to send MIDI messages at high speed over IEEE1394 (FireWire)) It uses the
/// HSS1394 API to send and receive MIDI messages to/from the device.
class DeviceChannelListener : public QObject, public hss1394::ChannelListener {
    Q_OBJECT
  public:
    DeviceChannelListener(QObject* pParent, QString name);
    virtual ~DeviceChannelListener();
    // Called when data has arrived. This call will occur inside a separate
    // thread.
    void Process(const hss1394::uint8 *pBuffer, hss1394::uint uBufferSize);
    void Disconnected();
    void Reconnected();
  signals:
    void incomingData(unsigned char status, unsigned char control, unsigned char value,
                      mixxx::Duration timestamp);
    void incomingData(const QByteArray& data, mixxx::Duration timestamp);

  private:
    QString m_sName;
};

class Hss1394Controller : public MidiController {
    Q_OBJECT
  public:
    Hss1394Controller(
            const hss1394::TNodeInfo& deviceInfo,
            int deviceIndex,
            UserSettingsPointer pConfig);
    ~Hss1394Controller() override;

  private slots:
    int open() override;
    int close() override;

  protected:
    void sendShortMsg(unsigned char status, unsigned char byte1,
                      unsigned char byte2) override;

  private:
    // The sysex data must already contain the start byte 0xf0 and the end byte
    // 0xf7.
    void sendBytes(const QByteArray& data) override;

    hss1394::TNodeInfo m_deviceInfo;
    int m_iDeviceIndex;
    static QList<QString> m_deviceList;
    hss1394::Channel* m_pChannel;
    DeviceChannelListener *m_pChannelListener;
};
