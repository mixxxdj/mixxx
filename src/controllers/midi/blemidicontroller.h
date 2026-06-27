#pragma once

#ifdef Q_OS_ANDROID

#include <QJniObject>
#include <QString>

#include "controllers/midi/midicontroller.h"

/// BLE MIDI controller implementation for Android.
///
/// This controller communicates with BLE MIDI devices using the
/// standard BLE MIDI service (UUID 03B80E5A-EDE8-4B33-A751-6CE34EC4C700)
/// via Android BluetoothGatt APIs through JNI.
///
/// The DDJ-FLX4 in Bluetooth mode advertises this service UUID.
/// To enter Bluetooth mode on the DDJ-FLX4, hold SHIFT + BROWSE.
class BleMidiController : public MidiController {
    Q_OBJECT
  public:
    explicit BleMidiController(const QString& deviceName,
            const QString& deviceAddress);
    ~BleMidiController() override;

    PhysicalTransportProtocol getPhysicalTransportProtocol() const override;
    QString getVendorString() const override;
    QString getProductString() const override;
    std::optional<uint16_t> getVendorId() const override;
    std::optional<uint16_t> getProductId() const override;
    QString getSerialNumber() const override;
    std::optional<uint8_t> getUsbInterfaceNumber() const override;

    /// Connect to the BLE MIDI device via GATT
    bool connectDevice();
    /// Disconnect from the BLE MIDI device
    void disconnectDevice();

    /// Called from Java when a MIDI characteristic notification is received
    static void onMidiDataReceived(const QByteArray& data);

  private slots:
    bool poll() override;

  protected:
    void sendShortMsg(unsigned char status,
            unsigned char byte1,
            unsigned char byte2) override;

  private:
    int open(const QString& resourcePath) override;
    int close() override;
    bool sendBytes(const QByteArray& data) override;
    bool isPolling() const override;

    QString m_deviceAddress;
    QString m_vendorString;
    QString m_productString;
    QString m_serialNumber;
    std::optional<uint16_t> m_vendorId;
    std::optional<uint16_t> m_productId;
    bool m_connected;

    // MIDI parser state for BLE MIDI packets
    // BLE MIDI uses a timestamp-based framing over BLE characteristics
    QByteArray m_pendingMidiData;
    unsigned char m_midiBuffer[1024];
    int m_midiBufferIndex;
    bool m_inSysex;
};

#endif // Q_OS_ANDROID
