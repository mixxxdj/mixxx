/**
 * @file portmidicontroller.h
 * @author Albert Santoni alberts@mixxx.org
 * @author Sean M. Pappalardo  spappalardo@mixxx.org
 * @date Thu 15 Mar 2012
 * @brief PortMidi-based MIDI backend
 *
 * This class is represents a MIDI device, either physical or software.
 * It uses the PortMidi API to send and receive MIDI messages to/from the device.
 * It's important to note that PortMidi treats input and output on a single
 * physical device as two separate half-duplex devices. In this class, we wrap
 * those together into a single device, which is why the constructor takes
 * both arguments pertaining to both input and output "devices".
 *
 */

#ifndef PORTMIDICONTROLLER_H
#define PORTMIDICONTROLLER_H

#include <portmidi.h>

#include <QScopedPointer>

#include "controllers/midi/midicontroller.h"
#include "controllers/midi/portmididevice.h"

// Mixxx completely stops responding to the controller if more than this number of messages queue up.
//  Don't lower this (much.) The SCS.1d accumulated 500 messages in a single poll during stress-testing.
#define MIXXX_PORTMIDI_BUFFER_LEN 1024 /**Number of MIDI messages to buffer*/
#define MIXXX_PORTMIDI_NO_DEVICE_STRING "None" /**String to display for no MIDI devices present */

/** A PortMidi-based implementation of MidiController */
class PortMidiController : public MidiController {
    Q_OBJECT
  public:
    PortMidiController(const PmDeviceInfo* inputDeviceInfo,
                       const PmDeviceInfo* outputDeviceInfo,
                       int inputDeviceIndex,
                       int outputDeviceIndex);
    virtual ~PortMidiController();

  private slots:
    virtual int open();
    virtual int close();
    virtual bool poll();

  private:
    void sendWord(unsigned int word);
    // The sysex data must already contain the start byte 0xf0 and the end byte
    // 0xf7.
    void send(QByteArray data);

    virtual bool isPolling() const {
        return true;
    }

    // For testing only so that test fixtures can install mock PortMidiDevices.
    void setPortMidiInputDevice(PortMidiDevice* device) {
        m_pInputDevice.reset(device);
    }
    void setPortMidiOutputDevice(PortMidiDevice* device) {
        m_pOutputDevice.reset(device);
    }

    QScopedPointer<PortMidiDevice> m_pInputDevice;
    QScopedPointer<PortMidiDevice> m_pOutputDevice;

    PmEvent m_midiBuffer[MIXXX_PORTMIDI_BUFFER_LEN];

    // Storage for SysEx messages
    unsigned char m_cReceiveMsg[1024];
    int m_cReceiveMsg_index;
    bool m_bInSysex;

    friend class PortMidiControllerTest;
};

#endif
