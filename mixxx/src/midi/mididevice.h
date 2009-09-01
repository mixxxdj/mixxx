/***************************************************************************
                             mididevice.h
                           MIDI Device Class
                           -------------------
    begin                : Thu Dec 18 2008
    copyright            : (C) 2008 Albert Santoni
    email                : alberts@mixxx.org

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

//Forward declarations
class MidiMapping;
#ifdef __MIDISCRIPT__
class MidiScriptEngine;
#endif

class MidiDevice : public QObject
{
Q_OBJECT
	public:
		MidiDevice(MidiMapping* mapping);
		virtual ~MidiDevice();
		virtual int open() = 0;
		virtual int close() = 0;
		bool isOpen() { return m_bIsOpen; };
		bool isOutputDevice() { return m_bIsOutputDevice; };
		bool isInputDevice() { return m_bIsInputDevice; };
		void setOutputDevice(MidiDevice* outputDevice); 
		QString getName() { return m_strDeviceName; };
		void setMidiMapping(MidiMapping* mapping);
		MidiMapping* getMidiMapping() { return m_pMidiMapping; };
		Q_INVOKABLE void sendShortMsg(unsigned char status, unsigned char byte1, unsigned char byte2);
   	 	virtual void sendShortMsg(unsigned int word);
	    virtual void sendSysexMsg(unsigned char data[], unsigned int length);
	    Q_INVOKABLE void sendSysexMsg(QList<int> data, unsigned int length);
	    bool getMidiLearnStatus();
	    void receive(MidiStatusByte status, char channel, char control, char value);

	public slots:
	    void disableMidiLearn();
	    void enableMidiLearn();

    signals:
        void midiEvent(MidiMessage message);	    	
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


};

#endif
