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
  public:
    LibremidiController(std::string name);
    ~LibremidiController() override;

    void setInputPort(libremidi::observer& observer, libremidi::input_port port);
    void setOutputPort(libremidi::observer& observer, libremidi::output_port port);
    void removeInputPort();
    void removeOutputPort();

    PhysicalTransportProtocol getPhysicalTransportProtocol() const override {
        return PhysicalTransportProtocol::UNKNOWN;
    }

    QString getVendorString() const override {
        if (m_pInputPort) {
            return m_pInputPort->manufacturer.c_str();
        }
        if (m_pOutputPort) {
            return m_pOutputPort->manufacturer.c_str();
        }
        return {};
    }

    QString getProductString() const override {
        if (m_pInputPort) {
            return m_pInputPort->product.c_str();
        }
        if (m_pOutputPort) {
            return m_pOutputPort->product.c_str();
        }
        return {};
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

    libremidi::midi_in* inputDevice() {
        if (m_pInputDevice) {
            return &m_pInputDevice.value();
        }
        return nullptr;
    }

    libremidi::midi_out* outputDevice() {
        if (m_pOutputDevice) {
            return &m_pOutputDevice.value();
        }
        return nullptr;
    }

    libremidi::input_port* inputPort() {
        if (m_pInputPort) {
            return &m_pInputPort.value();
        }
        return nullptr;
    }

    libremidi::output_port* outputPort() {
        if (m_pOutputPort) {
            return &m_pOutputPort.value();
        }
        return nullptr;
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
