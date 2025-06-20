#include "controllers/midi/portmidicontroller.h"

#include "controllers/midi/midiutils.h"
#include "moc_portmidicontroller.cpp"

namespace {
const QString kUnknownControllerName = QStringLiteral("Unknown PortMidiController");
} // namespace

PortMidiController::PortMidiController(const PmDeviceInfo* inputDeviceInfo,
        const PmDeviceInfo* outputDeviceInfo,
        int inputDeviceIndex,
        int outputDeviceIndex)
        : MidiController((inputDeviceInfo || outputDeviceInfo)
                          ? QString::fromLocal8Bit(inputDeviceInfo
                                            ? inputDeviceInfo->name
                                            : outputDeviceInfo->name)
                          : kUnknownControllerName),
          m_cReceiveMsg_index(0),
          m_bInSysex(false) {
    for (int k = 0; k < MIXXX_PORTMIDI_BUFFER_LEN; ++k) {
        m_midiBuffer[k] = {0, 0};
    }

    // Note: We prepend the input stream's index to the device's name to prevent
    // duplicate devices from causing mayhem.
    //setDeviceName(QString("%1. %2").arg(QString::number(m_iInputDeviceIndex), inputDeviceInfo->name));
    if (inputDeviceInfo) {
        setInputDevice(inputDeviceInfo->input);
        m_pInputDevice.reset(new PortMidiDevice(
            inputDeviceInfo, inputDeviceIndex));
    }
    if (outputDeviceInfo) {
        setOutputDevice(outputDeviceInfo->output);
        m_pOutputDevice.reset(new PortMidiDevice(
            outputDeviceInfo, outputDeviceIndex));
    }
}

PortMidiController::~PortMidiController() {
    if (isOpen()) {
        close();
    }
}

int PortMidiController::open(const QString& resourcePath) {
    if (isOpen()) {
        qCWarning(m_logBase) << "PortMIDI device" << getName() << "already open";
        return -1;
    }

    if (getName() == MIXXX_PORTMIDI_NO_DEVICE_STRING) {
        return -1;
    }

    m_bInSysex = false;
    m_cReceiveMsg_index = 0;

    if (m_pInputDevice && isInputDevice()) {
        qCInfo(m_logBase) << "PortMidiController: Opening"
                          << m_pInputDevice->info()->name << "index"
                          << m_pInputDevice->index() << "for input";
        PmError err = m_pInputDevice->openInput(MIXXX_PORTMIDI_BUFFER_LEN);

        if (err != pmNoError) {
            qCWarning(m_logBase) << "PortMidi error:" << Pm_GetErrorText(err);
            return -2;
        }
    }
    if (m_pOutputDevice && isOutputDevice()) {
        qCInfo(m_logBase) << "PortMidiController: Opening"
                          << m_pOutputDevice->info()->name << "index"
                          << m_pOutputDevice->index() << "for output";

        PmError err = m_pOutputDevice->openOutput();
        if (err != pmNoError) {
            qCWarning(m_logBase) << "PortMidi error:" << Pm_GetErrorText(err);
            return -2;
        }
    }
    startEngine();
    applyMapping(resourcePath);
    setOpen(true);
    return 0;
}

int PortMidiController::close() {
    if (!isOpen()) {
        qCWarning(m_logBase) << "PortMIDI device" << getName() << "already closed";
        return -1;
    }

    stopEngine();
    MidiController::close();

    int result = 0;

    if (m_pInputDevice && m_pInputDevice->isOpen()) {
        PmError err = m_pInputDevice->close();
        if (err != pmNoError) {
            qCWarning(m_logBase) << "PortMidi error:" << Pm_GetErrorText(err);
            result = -1;
        }
    }

    if (m_pOutputDevice && m_pOutputDevice->isOpen()) {
        PmError err = m_pOutputDevice->close();
        if (err != pmNoError) {
            qCWarning(m_logBase) << "PortMidi error:" << Pm_GetErrorText(err);
            result = -1;
        }
    }

    setOpen(false);
    return result;
}

bool PortMidiController::poll() {
    // Poll the controller for new data if it's an input device
    if (m_pInputDevice.isNull() || !m_pInputDevice->isOpen()) {
        return false;
    }

    int numEvents = m_pInputDevice->read(m_midiBuffer, MIXXX_PORTMIDI_BUFFER_LEN);

    //qDebug() << "PortMidiController::poll()" << numEvents;

    if (numEvents < 0) {
        qCWarning(m_logInput) << "PortMidi error:" << Pm_GetErrorText((PmError)numEvents);
        return false;
    }

    for (int i = 0; i < numEvents; i++) {
        unsigned char status = Pm_MessageStatus(m_midiBuffer[i].message);
        mixxx::Duration timestamp = mixxx::Duration::fromMillis(m_midiBuffer[i].timestamp);

        if ((status & 0xF8) == 0xF8) {
            // Handle real-time MIDI messages at any time
            receivedShortMessage(status, 0, 0, timestamp);
            continue;
        }

        reprocessMessage:

        if (!m_bInSysex) {
            if (status == 0xF0) {
                m_bInSysex = true;
                status = 0;
            } else {
                //unsigned char channel = status & 0x0F;
                unsigned char note = Pm_MessageData1(m_midiBuffer[i].message);
                unsigned char velocity = Pm_MessageData2(m_midiBuffer[i].message);
                receivedShortMessage(status, note, velocity, timestamp);
            }
        }

        if (m_bInSysex) {
            // Abort (drop) the current System Exclusive message if a
            //  non-realtime status byte was received
            if (status > 0x7F && status < 0xF7) {
                m_bInSysex = false;
                m_cReceiveMsg_index = 0;
                qCWarning(m_logInput) << "Buggy MIDI device: SysEx interrupted!";
                goto reprocessMessage;    // Don't lose the new message
            }

            // Collect bytes from PmMessage
            uint8_t data = 0;
            for (int shift = 0; shift < 32 &&
                    (data != MidiUtils::opCodeValue(MidiOpCode::EndOfExclusive));
                    shift += 8) {
                // TODO(rryan): This prevents buffer overflow if the sysex is
                // larger than 1024 bytes. I don't want to radically change
                // anything before the 2.0 release so this will do for now.
                data = (m_midiBuffer[i].message >> shift) & 0xFF;
                if (m_cReceiveMsg_index < MIXXX_SYSEX_BUFFER_LEN) {
                    m_cReceiveMsg[m_cReceiveMsg_index++] = data;
                }
            }

            // End System Exclusive message if the EOX byte was received
            if (data == MidiUtils::opCodeValue(MidiOpCode::EndOfExclusive)) {
                m_bInSysex = false;
                const char* buffer = reinterpret_cast<const char*>(m_cReceiveMsg);
                receive(QByteArray::fromRawData(buffer, m_cReceiveMsg_index),
                        timestamp);
                m_cReceiveMsg_index = 0;
            }
        }
    }
    return numEvents > 0;
}

void PortMidiController::sendShortMsg(unsigned char status, unsigned char byte1,
                                      unsigned char byte2) {
    if (m_pOutputDevice.isNull() || !m_pOutputDevice->isOpen()) {
        return;
    }

    unsigned int word = (((unsigned int)byte2) << 16) |
                         (((unsigned int)byte1) << 8) | status;

    PmError err = m_pOutputDevice->writeShort(word);
    if (err == pmNoError) {
        qCDebug(m_logOutput) << QStringLiteral("outgoing: ")
                             << MidiUtils::formatMidiOpCode(getName(),
                                        status,
                                        byte1,
                                        byte2,
                                        MidiUtils::channelFromStatus(status),
                                        MidiUtils::opCodeFromStatus(status));
    } else {
        // Use two qWarnings() to ensure line break works on all operating systems
        qCWarning(m_logOutput) << "Error sending short message"
                               << MidiUtils::formatMidiOpCode(getName(),
                                          status,
                                          byte1,
                                          byte2,
                                          MidiUtils::channelFromStatus(status),
                                          MidiUtils::opCodeFromStatus(status));
        qCWarning(m_logOutput) << "PortMidi error:" << Pm_GetErrorText(err);
    }
}

bool PortMidiController::sendBytes(const QByteArray& data) {
    // PortMidi does not receive a length argument for the buffer we provide to
    // Pm_WriteSysEx. Instead, it scans for a MidiOpCode::EndOfExclusive byte
    // to know when the message is over. If one is not provided, it will
    // overflow the buffer and cause a segfault.
    if (!data.endsWith(MidiUtils::opCodeValue(MidiOpCode::EndOfExclusive))) {
        qCDebug(m_logOutput) << "SysEx message does not end with 0xF7 -- ignoring.";
        return false;
    }

    if (m_pOutputDevice.isNull() || !m_pOutputDevice->isOpen()) {
        return false;
    }

    PmError err = m_pOutputDevice->writeSysEx((unsigned char*)data.constData());
    if (err == pmNoError) {
        qCDebug(m_logOutput) << QStringLiteral("outgoing: ")
                             << MidiUtils::formatSysexMessage(getName(), data);
        return true;
    } else {
        // Use two qWarnings() to ensure line break works on all operating systems
        qCWarning(m_logOutput) << "Error sending SysEx message:"
                               << MidiUtils::formatSysexMessage(getName(), data);
        qCWarning(m_logOutput) << "PortMidi error:" << Pm_GetErrorText(err);
    }
    return false;
}
