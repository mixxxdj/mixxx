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
#include "controllers/midi/midicontroller.h"

#define MIXXX_PORTMIDI_BUFFER_LEN 64 /**Number of MIDI messages to buffer*/
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
    void send(unsigned int word);
    // The sysex data must already contain the start byte 0xf0 and the end byte
    // 0xf.7
    void send(QByteArray data);

    virtual bool isPolling() const {
        return true;
    }

    const PmDeviceInfo* m_pInputDeviceInfo;
    const PmDeviceInfo* m_pOutputDeviceInfo;
    int m_iInputDeviceIndex;
    int m_iOutputDeviceIndex;
    PortMidiStream *m_pInputStream;
    PortMidiStream *m_pOutputStream;
    PmEvent m_midiBuffer[MIXXX_PORTMIDI_BUFFER_LEN];

    // Storage for SysEx messages
    unsigned char m_cReceiveMsg[1024];
    int m_cReceiveMsg_index;
    bool m_bInSysex;
};

#endif
