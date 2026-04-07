#pragma once

#include <libremidi/libremidi.hpp>

#include "controllers/midi/midicontroller.h"

// String to display for no MIDI devices present
#define MIXXX_LIBREMIDI_NO_DEVICE_STRING "None"

/// Libremidi-based implementation of MidiController
///
/// This class is represents a MIDI device, either physical or software.
/// It uses the Libremidi API to send and receive MIDI messages to/from the device.
/// It's important to note that Libremidi only addresses MIDI input and output ports
/// in the API. In this class, we wrap those together into a single device, which is
/// why the constructor takes both arguments pertaining to both input and output ports.
class LibremidiController : public MidiController {
    Q_OBJECT

    friend class LibremidiEnumerator;

  public:
    LibremidiController(const libremidi::input_port* pInputPort,
            const libremidi::output_port* pOutputPort);
    ~LibremidiController() override;

    void setInputPort(std::optional<libremidi::input_port> port);
    void setOutputPort(std::optional<libremidi::output_port> port);

    PhysicalTransportProtocol getPhysicalTransportProtocol() const override {
        return PhysicalTransportProtocol::UNKNOWN;
    }

    QString getVendorString() const override {
        return QString();
    }
    QString getProductString() const override {
        if (m_pInputPort) {
            return QString::fromLocal8Bit(m_pInputPort->port_name);
        }
        if (m_pOutputPort) {
            return QString::fromLocal8Bit(m_pOutputPort->port_name);
        }
        return QString();
    }
    std::optional<uint16_t> getVendorId() const override {
        return std::nullopt;
    }
    std::optional<uint16_t> getProductId() const override {
        return std::nullopt;
    }
    QString getSerialNumber() const override {
        return QString();
    }

    std::optional<uint8_t> getUsbInterfaceNumber() const override {
        return std::nullopt;
    }

  protected:
    void sendShortMsg(unsigned char status, unsigned char byte1, unsigned char byte2) override;

  private:
    int open(const QString& resourcePath) override;
    int close() override;

    bool sendBytes(const QByteArray& data) override;

    bool isPolling() const override {
        return false;
    }

    void onMessage(const libremidi::message& m);

    std::optional<libremidi::midi_in> m_pInputDevice;
    std::optional<libremidi::midi_out> m_pOutputDevice;
    std::optional<libremidi::input_port> m_pInputPort;
    std::optional<libremidi::output_port> m_pOutputPort;
};
