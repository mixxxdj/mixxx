/**
 * @file portmidicontroller.h
 * @author Albert Santoni alberts@mixxx.org
 * @author Sean M. Pappalardo  spappalardo@mixxx.org
 * @date Thu 15 Mar 2012
 * @brief PortMidi-based MIDI backend
 *
 */

#include "controllers/midi/portmidicontroller.h"

PortMidiController::PortMidiController(const PmDeviceInfo* inputDeviceInfo,
                                       const PmDeviceInfo* outputDeviceInfo,
                                       int inputDeviceIndex,
                                       int outputDeviceIndex)
        : MidiController(),
          m_pInputDeviceInfo(inputDeviceInfo),
          m_pOutputDeviceInfo(outputDeviceInfo),
          m_iInputDeviceIndex(inputDeviceIndex),
          m_iOutputDeviceIndex(outputDeviceIndex),
          m_pInputStream(NULL),
          m_pOutputStream(NULL) {
    // Note: We prepend the input stream's index to the device's name to prevent
    // duplicate devices from causing mayhem.
    //setDeviceName(QString("%1. %2").arg(QString::number(m_iInputDeviceIndex), inputDeviceInfo->name));
    setDeviceName(QString("%1").arg(inputDeviceInfo->name));

    if (m_pInputDeviceInfo) {
        setInputDevice(m_pInputDeviceInfo->input);
    }
    if (m_pOutputDeviceInfo) {
        setOutputDevice(m_pOutputDeviceInfo->output);
    }
}

PortMidiController::~PortMidiController() {
    close();
}

int PortMidiController::open() {
    if (isOpen()) {
        qDebug() << "PortMIDI device" << getName() << "already open";
        return -1;
    }

    if (getName() == MIXXX_PORTMIDI_NO_DEVICE_STRING)
        return -1;

    m_bInSysex = false;
    m_cReceiveMsg_index = 0;

    PmError err = Pm_Initialize();
    if (err != pmNoError) {
        qDebug() << "PortMidi error:" << Pm_GetErrorText(err);
        return -1;
    }

    if (m_pInputDeviceInfo) {
        if (isInputDevice()) {
            if (debugging()) {
                qDebug() << "PortMidiController: Opening"
                         << m_pInputDeviceInfo->name << "index"
                         << m_iInputDeviceIndex << "for input";
            }

            err = Pm_OpenInput(&m_pInputStream,
                               m_iInputDeviceIndex,
                               NULL, //No drive hacks
                               MIXXX_PORTMIDI_BUFFER_LEN,
                               NULL,
                               NULL);

            if (err != pmNoError) {
                qDebug() << "PortMidi error:" << Pm_GetErrorText(err);
                return -2;
            }
        }
    }
    if (m_pOutputDeviceInfo) {
        if (isOutputDevice()) {
            if (debugging()) {
                qDebug() << "PortMidiController: Opening"
                         << m_pOutputDeviceInfo->name << "index"
                         << m_iOutputDeviceIndex << "for output";
            }

            err = Pm_OpenOutput(&m_pOutputStream,
                                m_iOutputDeviceIndex,
                                NULL, // No driver hacks
                                0,      // No buffering
                                NULL, // Use PortTime for timing
                                NULL, // No time info
                                0);   // No latency compensation.

            if (err != pmNoError) {
                qDebug() << "PortMidi error:" << Pm_GetErrorText(err);
                return -2;
            }
        }
    }

    setOpen(true);
    startEngine();
    return 0;
}

int PortMidiController::close() {
    if (!isOpen()) {
        qDebug() << "PortMIDI device" << getName() << "already closed";
        return -1;
    }

    stopEngine();
    MidiController::close();

    if (m_pInputStream) {
        PmError err = Pm_Close(m_pInputStream);
        if (err != pmNoError) {
            qDebug() << "PortMidi error:" << Pm_GetErrorText(err);
            return -1;
        }
    }

    if (m_pOutputStream) {
        PmError err = Pm_Close(m_pOutputStream);
        if (err != pmNoError) {
            qDebug() << "PortMidi error:" << Pm_GetErrorText(err);
            return -1;
        }
    }

    setOpen(false);
    return 0;
}

bool PortMidiController::poll() {
    // Poll the controller for new data, if it's an input device
    if (!m_pInputStream)
        return false;

    PmError gotEvents = Pm_Poll(m_pInputStream);
    if (gotEvents == FALSE) {
        return false;
    }
    if (gotEvents < 0) {
        qWarning() << "PortMidi error:" << Pm_GetErrorText(gotEvents);
        return false;
    }

    int numEvents = Pm_Read(m_pInputStream, m_midiBuffer, MIXXX_PORTMIDI_BUFFER_LEN);

    if (numEvents < 0) {
        qWarning() << "PortMidi error:" << Pm_GetErrorText((PmError)numEvents);
        return false;
    }

    for (int i = 0; i < numEvents; i++) {
        unsigned char status = Pm_MessageStatus(m_midiBuffer[i].message);

        if ((status & 0xF8) == 0xF8) {
            // Handle real-time MIDI messages at any time
            receive(status, 0, 0);
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
                receive(status, note, velocity);
            }
        }

        if (m_bInSysex) {
            // Abort (drop) the current System Exclusive message if a
            //  non-realtime status byte was received
            if (status > 0x7F && status < 0xF7) {
                m_bInSysex = false;
                m_cReceiveMsg_index = 0;
                qWarning() << "Buggy MIDI device: SysEx interrupted!";
                goto reprocessMessage;    // Don't lose the new message
            }

            // Collect bytes from PmMessage
            int data = 0;
            for (int shift = 0; shift < 32 && (data != MIDI_EOX); shift += 8) {
                if ((data & 0xF8) == 0xF8) {
                    // Handle real-time messages at any time
                    receive(data, 0, 0);
                } else {
                    m_cReceiveMsg[m_cReceiveMsg_index++] = data =
                        (m_midiBuffer[i].message >> shift) & 0xFF;
                }
            }

            // End System Exclusive message if the EOX byte was received
            if (data == MIDI_EOX) {
                m_bInSysex = false;
                const char* buffer = reinterpret_cast<const char*>(m_cReceiveMsg);
                receive(QByteArray::fromRawData(buffer, m_cReceiveMsg_index));
                m_cReceiveMsg_index = 0;
            }
        }
    }
    return numEvents > 0;
}

void PortMidiController::send(unsigned int word) {
    if (m_pOutputStream) {
        PmError err = Pm_WriteShort(m_pOutputStream, 0, word);
        if (err != pmNoError) {
            qDebug() << "PortMidi sendShortMsg error:" << Pm_GetErrorText(err);
        }
    }

}

void PortMidiController::send(QByteArray data) {
    if (m_pOutputStream) {
        PmError err = Pm_WriteSysEx(m_pOutputStream, 0, (unsigned char*)data.constData());
        if (err != pmNoError) {
            qDebug() << "PortMidi sendSysexMsg error:"
                     << Pm_GetErrorText(err);
        }
    }
}
