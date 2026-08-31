#include "controllers/midi/libremidicontroller.h"

#include <QMetaObject>
#include <libremidi/libremidi.hpp>

#include "controllers/midi/midiutils.h"
#include "moc_libremidicontroller.cpp"
#include "util/duration.h"

LibremidiController::LibremidiController(std::string name)
        : MidiController(QString::fromStdString(name)) {
}

LibremidiController::~LibremidiController() {
    if (isOpen()) {
        close();
    }
}

void LibremidiController::removeInputPort() {
    m_pInputDevice.reset();
    setInputDevice(false);
}

void LibremidiController::setInputPort(libremidi::observer& observer, libremidi::input_port port) {
    m_pInputPort = port;

    setInputDevice(true);
    m_pInputDevice = libremidi::midi_in{
            libremidi::input_configuration{
                    .on_message = [this](const libremidi::message& m) {
                        this->onMessage(m);
                    },
            },
            libremidi::midi_in_configuration_for(observer)};
}

void LibremidiController::removeOutputPort() {
    m_pOutputDevice.reset();
    setOutputDevice(false);
}

void LibremidiController::setOutputPort(
        libremidi::observer& observer, libremidi::output_port port) {
    m_pOutputPort = std::move(port);
    m_pOutputDevice.emplace(libremidi::output_configuration{},
            libremidi::midi_out_configuration_for(observer));
    setOutputDevice(true);
}

void LibremidiController::onMessage(const libremidi::message& m) {
    auto status = m.bytes[0];
    auto timestamp = mixxx::Duration::fromMillis(m.timestamp);
    qDebug() << "LibremidiController::onMessage" << m.bytes[0] << m.bytes[1] << m.bytes[2];

    if ((status & 0xF8) == 0xF8) {
        // Handle real-time MIDI messages at any time
        QMetaObject::invokeMethod(this, [=, this] {
            receivedShortMessage(status, 0, 0, timestamp);
        });
        return;
    }

    bool is_sysex = status == 0xF0;
    if (is_sysex) {
        const auto* pData = reinterpret_cast<const char*>(m.bytes.data());
        receive(QByteArray::fromRawData(pData, m.size()), timestamp);
    } else {
        // unsigned char channel = status & 0x0F;
        unsigned char note = m.bytes[1];
        unsigned char velocity = m.bytes[2];
        QMetaObject::invokeMethod(this, [=, this] {
            receivedShortMessage(status, note, velocity, timestamp);
        });
    }
}

int LibremidiController::open(const QString& resourcePath) {
    if (isOpen()) {
        qCWarning(m_logBase) << "Libremidi device" << getName() << "already open";
        return -1;
    }

    if (getName() == MIXXX_LIBREMIDI_NO_DEVICE_STRING) {
        return -1;
    }

    if (m_pInputPort && isInputDevice()) {
        qCInfo(m_logBase) << "LibremidiController: Opening"
                          << m_pInputPort->port_name.c_str() << "for input";
        stdx::error error = m_pInputDevice->open_port(m_pInputPort.value());

        if (error.is_set()) {
            qCWarning(m_logBase) << "Libremidi error:" << error.message().data();
            return -2;
        }
    }
    if (m_pOutputPort && isOutputDevice()) {
        qCInfo(m_logBase) << "LibremidiController: Opening"
                          << m_pOutputPort->port_name.c_str() << "for output";

        stdx::error error = m_pOutputDevice->open_port(m_pOutputPort.value());
        if (error.is_set()) {
            qCWarning(m_logBase) << "Libremidi error:" << error.message().data();
            return -2;
        }
    }
    startEngine();
    applyMapping(resourcePath);
    setOpen(true);
    return 0;
}

int LibremidiController::close() {
    if (!isOpen()) {
        qCWarning(m_logBase) << "Libremidi device" << getName() << "already closed";
        return -1;
    }

    stopEngine();
    MidiController::close();

    int result = 0;

    if (m_pInputDevice && m_pInputDevice->is_port_connected()) {
        stdx::error error = m_pInputDevice->close_port();
        if (error.is_set()) {
            qCWarning(m_logBase) << "Libremidi error:" << error.message().data();
            result = -1;
        }
    }

    if (m_pOutputDevice && m_pOutputDevice->is_port_open()) {
        stdx::error error = m_pOutputDevice->close_port();
        if (error.is_set()) {
            qCWarning(m_logBase) << "Libremidi error:" << error.message().data();
            result = -1;
        }
    }

    setOpen(false);
    return result;
}

void LibremidiController::sendShortMsg(
        unsigned char status, unsigned char byte1, unsigned char byte2) {
    if (!m_pOutputPort || !m_pOutputDevice->is_port_open()) {
        return;
    }

    stdx::error error = m_pOutputDevice->send_message(status, byte1, byte2);
    if (error.is_set()) {
        // Use two qWarnings() to ensure line break works on all operating systems
        qCWarning(m_logOutput) << "Error sending short message"
                               << MidiUtils::formatMidiOpCode(getName(),
                                          status,
                                          byte1,
                                          byte2,
                                          MidiUtils::channelFromStatus(status),
                                          MidiUtils::opCodeFromStatus(status));
        qCWarning(m_logOutput) << "Libremidi error:" << error.message().data();
    } else {
        qDebug(m_logOutput) << QStringLiteral("outgoing: ")
                            << MidiUtils::formatMidiOpCode(getName(),
                                       status,
                                       byte1,
                                       byte2,
                                       MidiUtils::channelFromStatus(status),
                                       MidiUtils::opCodeFromStatus(status));
    }
}

bool LibremidiController::sendBytes(const QByteArray& data) {
    if (!m_pOutputPort || !m_pOutputDevice->is_port_open()) {
        return false;
    }

    stdx::error error = m_pOutputDevice->send_message(
            (unsigned char*)data.constData(), data.size());
    if (error.is_set()) {
        // Use two qWarnings() to ensure line break works on all operating systems
        qCWarning(m_logOutput) << "Error sending SysEx message:"
                               << MidiUtils::formatSysexMessage(getName(), data);
        qCWarning(m_logOutput) << "Libremidi error:" << error.message().data();
    } else {
        qCDebug(m_logOutput) << QStringLiteral("outgoing: ")
                             << MidiUtils::formatSysexMessage(getName(), data);
        return true;
    }
    return false;
}
