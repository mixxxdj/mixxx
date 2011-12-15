/***************************************************************************
                             mididevice.h
                           MIDI Device Class
                           -------------------
    begin                : Thu Dec 18 2008
    copyright            : (C) 2008 Albert Santoni
    email                : alberts@mixxx.org

    This class represents a physical or virtual MIDI device. A MIDI device
    processes MIDI messages that are received from it in the receive()
    function and MIDI messages can be sent back to it using the send*()
    functions. This is a base class that cannot be instantiated on its own,
    it must be inherited by a class that implements it on top of some API.
    (See MidiDevicePortMidi, as of Nov 2009.)

    This class is thread safe and must remain thread safe because parts of
    it may be accessed by the MIDI script engine thread as well as the
    MIDI thread concurrently.

***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef MIDIDEVICE_H
#define MIDIDEVICE_H

#include <QtCore>
#include "midimessage.h"
#include "softtakeover.h"

//Forward declarations
class MidiMapping;
#ifdef __MIDISCRIPT__
class MidiScriptEngine;
#endif

class MidiDevice : public QThread
{
Q_OBJECT
    public:
        MidiDevice(MidiMapping* mapping);
        virtual ~MidiDevice();
        virtual int open() = 0;
        virtual int close() = 0;
        virtual void run() = 0;
        void startup();
        void shutdown();
        bool isOpen() { return m_bIsOpen; };
        bool isOutputDevice() { return m_bIsOutputDevice; };
        bool isInputDevice() { return m_bIsInputDevice; };
        QString getName() { return m_strDeviceName; };
        void setMidiMapping(MidiMapping* mapping);
        MidiMapping* getMidiMapping() { return m_pMidiMapping; };
        Q_INVOKABLE void sendShortMsg(unsigned char status, unsigned char byte1, unsigned char byte2);
        virtual void sendShortMsg(unsigned int word);
        virtual void sendSysexMsg(unsigned char data[], unsigned int length);
        Q_INVOKABLE void sendSysexMsg(QList<int> data, unsigned int length);
        bool getMidiLearnStatus();
        void receive(MidiStatusByte status, char channel, char control, char value);
#ifdef __MIDISCRIPT__
        /** Receives System Exclusive (and other unhandled and/or arbitrary-length)
            messages and passes them straight to a script function. */
        void receive(const unsigned char data[], unsigned int length);
#endif
        /** Specifies whether or not we should dump MIDI messages to the console at runtime. This is useful
            for end-user debugging and to help people map their controllers. */
        bool midiDebugging();
        void setReceiveInhibit(bool inhibit);
    public slots:
        void disableMidiLearn();
        void enableMidiLearn();

    signals:
        void midiEvent(MidiMessage message);
        void callMidiScriptFunction(QString function, char channel, char control, char value, MidiStatusByte status, QString group);

    protected:
        /** Verbose device name, in format "[index]. [device name]". Suitable for display in GUI. */
        QString m_strDeviceName;
        /** Flag indicating if this device supports MIDI output */
        bool m_bIsOutputDevice;
        /** Flag indicating if this device supports MIDI input */
        bool m_bIsInputDevice;
        /** MIDI Mapping for this MIDI device, maps MIDI messages onto Mixxx controls */
        MidiMapping* m_pMidiMapping;
        /** Indicates whether or not the MIDI device has been opened for input/output. */
        bool m_bIsOpen;
        /** Indicates whether MIDI learning is currently enabled or not */
        bool m_bMidiLearn;
        /** Pointer to the output device that corresponds to this physical input device. MIDI is a half-duplex
            protocol that treats input and output ports on a single device as complete separate entities. This
            helps us group those back together for sanity/usability. */
        MidiDevice* m_pCorrespondingOutputDevice;
        /** Specifies whether or not we should dump MIDI messages to the console at runtime. This is useful
            for end-user debugging and to help people map their controllers. */
        bool m_midiDebug;
        /** Mutex to protect against concurrent access to member variables */
        QMutex m_mutex;
        /** Mutex to protect against concurrent access to the m_pMidiMapping _pointer. Note that MidiMapping itself is thread-safe, so we just need to protect the pointer!. */
        QMutex m_mappingPtrMutex;
        /** A flag to inhibit the reception of messages from this device. This is used to prevent
            a race condition when a MIDI message is received and looked up in the MidiMapping while
            the MidiMapping is being modified (and is already locked).  */
        bool m_bReceiveInhibit;
        SoftTakeover m_st;
};

#endif
