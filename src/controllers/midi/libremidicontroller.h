#pragma once

#include <libremidi/libremidi.hpp>

#include "controllers/midi/midicontroller.h"

// Note:
// A standard Midi device runs at 31.25 kbps, with 10 bits / byte
// 1 byte / 320 microseconds
// a usual Midi message has 3 byte which results to
// 1042.6 messages per second
//
// The MIDI over IEEE-1394:
// http://www.midi.org/techspecs/rp27v10spec%281394%29.pdf
// which is also used for USB defines 3 speeds:
// 1 byte / 320 microseconds
// 2 bytes / 320 microseconds
// 3 bytes / 320 microseconds
// which results in up to 3125 messages per second
// if we assume normal 3 Byte messages.
//
// For instants the SCS.1d, uses the 3 x speed
//
// Due to a bug Mixxx completely stops responding to the controller
// if more than this number of messages queue up. Don't lower this (much.)
// The SCS.1d a 3x Speed device
// accumulated 500 messages in a single poll during stress-testing.
// A midi message contains 1 .. 4 bytes.
// a 1024 messages buffer will buffer ~327 ms Midi-Stream

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
  public:
    LibremidiController(const libremidi::input_port* inputDevicePort,
            const libremidi::output_port* outputDevicePort);
    ~LibremidiController() override;

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

    std::optional<libremidi::midi_in> m_pInputDevice;
    std::optional<libremidi::midi_out> m_pOutputDevice;
    std::optional<libremidi::input_port> m_pInputPort;
    std::optional<libremidi::output_port> m_pOutputPort;
};
