/**
  * @file portmidicontroller.h
  * @author Albert Santoni alberts@mixxx.org
  * @author Sean M. Pappalardo  spappalardo@mixxx.org
  * @date Thu 15 Mar 2012
  * @brief PortMidi-based MIDI backend
  *
  */


/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "portmidicontroller.h"

PortMidiController::PortMidiController(const PmDeviceInfo* inputDeviceInfo,
                                       const PmDeviceInfo* outputDeviceInfo,
                                       int inputDeviceIndex,
                                       int outputDeviceIndex)
                                        : MidiController()
{
    m_pInputStream = NULL;
    m_pOutputStream = NULL;
    m_pInputDeviceInfo = inputDeviceInfo;
    m_pOutputDeviceInfo = outputDeviceInfo;
    m_iInputDeviceIndex = inputDeviceIndex;
    m_iOutputDeviceIndex = outputDeviceIndex;

    //Note: We prepend the input stream's index to the device's name to prevent duplicate devices from causing mayhem.
//     m_sDeviceName = QString("%1. %2").arg(QString::number(m_iInputDeviceIndex), inputDeviceInfo->name);
    m_sDeviceName = QString("%1").arg(inputDeviceInfo->name);

    if (inputDeviceInfo) {
        m_bIsInputDevice = m_pInputDeviceInfo->input;
    }
    if (outputDeviceInfo) {
        m_bIsOutputDevice = m_pOutputDeviceInfo->output;
    }
}

PortMidiController::~PortMidiController()
{
    close();
}

int PortMidiController::open()
{
    if (m_bIsOpen) {
        qDebug() << "PortMIDI device" << m_sDeviceName << "already open";
        return -1;
    }

    if (m_sDeviceName == MIXXX_PORTMIDI_NO_DEVICE_STRING)
        return -1;
    
    m_bInSysex = false;
    m_bEndSysex = false;
    m_cReceiveMsg_index = 0;
    
    PmError err = Pm_Initialize();
    if( err != pmNoError )
    {
        qDebug() << "PortMidi error:" << Pm_GetErrorText(err);
        return -1;
    }

    if (m_pInputDeviceInfo)
    {
        if (m_bIsInputDevice)
        {
            if (debugging()) qDebug() << "PortMidiController: Opening" << m_pInputDeviceInfo->name << "index" << m_iInputDeviceIndex << "for input";

            err = Pm_OpenInput( &m_pInputStream,
                    m_iInputDeviceIndex,
                    NULL, //No drive hacks
                    MIXXX_PORTMIDI_BUFFER_LEN,
                    NULL,
                    NULL);

            if( err != pmNoError )
            {
                qDebug() << "PortMidi error:" << Pm_GetErrorText(err);
                return -2;
            }
        }
    }
    if (m_pOutputDeviceInfo)
    {
        if (m_bIsOutputDevice)
        {
            if (debugging()) qDebug() << "PortMidiController: Opening" << m_pOutputDeviceInfo->name << "index" << m_iOutputDeviceIndex << "for output";

            err = Pm_OpenOutput( &m_pOutputStream,
                    m_iOutputDeviceIndex,
                    NULL, // No driver hacks
                    0,      // No buffering
                    NULL, // Use PortTime for timing
                    NULL, // No time info
                    0);   // No latency compensation.

            if( err != pmNoError )
            {
                qDebug() << "PortMidi error:" << Pm_GetErrorText(err);
                return -2;
            }
        }
    }

    m_bIsOpen = true;

    startEngine();

    return 0;

}

int PortMidiController::close()
{
    if (!m_bIsOpen) {
        qDebug() << "PortMIDI device" << m_sDeviceName << "already closed";
        return -1;
    }

    stopEngine();

    if (m_pInputStream)
    {
        PmError err = Pm_Close(m_pInputStream);
        if( err != pmNoError )
        {
            qDebug() << "PortMidi error:" << Pm_GetErrorText(err);
            return -1;
        }
    }

    if (m_pOutputStream)
    {
        PmError err = Pm_Close(m_pOutputStream);
        if( err != pmNoError )
        {
            qDebug() << "PortMidi error:" << Pm_GetErrorText(err);
            return -1;
        }
    }

    m_bIsOpen = false;

    return 0;
}

void PortMidiController::timerEvent(QTimerEvent *event, bool poll) {

    if (!poll) m_pEngine->timerEvent(event);
    else {
        // Poll the controller for new data
        int numEvents = 0;

        if (m_pInputStream) {
            PmError gotEvents = Pm_Poll(m_pInputStream);
            if (gotEvents == FALSE) return;
            if (gotEvents < 0) {
                qWarning() << "PortMidi error:" << Pm_GetErrorText(gotEvents);
                return;
            }
            
            numEvents = Pm_Read(m_pInputStream, m_midiBuffer, MIXXX_PORTMIDI_BUFFER_LEN);

            if (numEvents < 0) {
                qDebug() << "PortMidi error:" << Pm_GetErrorText((PmError)numEvents);

                // Don't process anything
                numEvents = 0;
            }

            for (int i = 0; i < numEvents; i++) {
                unsigned char status = Pm_MessageStatus(m_midiBuffer[i].message);

                if ((status & 0xF8) == 0xF8) {
                    // Handle real-time MIDI messages at any time
                    receive(status, 0, 0);
                }

                if (!m_bInSysex) {
                    if (status == 0xF0) {
                        m_bInSysex=true;
                        status = 0;
                    }
                    else {
//                         unsigned char channel = status & 0x0F;
                        unsigned char note = Pm_MessageData1(m_midiBuffer[i].message);
                        unsigned char velocity = Pm_MessageData2(m_midiBuffer[i].message);

                        receive(status, note, velocity);
                    }
                }

                if (m_bInSysex) {
                    int data = 0;
                    // Collect bytes from PmMessage
                    for (int shift = 0; shift < 32 && (data != MIDI_EOX); shift += 8) {
                        m_cReceiveMsg[m_cReceiveMsg_index++] = data =
                            (m_midiBuffer[i].message >> shift) & 0xFF;
                    }
                    // End System Exclusive message if the EOX byte or
                    //  a non-realtime status byte was received
                    if (data == MIDI_EOX || status > 0x7F) m_bEndSysex=true;
                }
            }

            if (m_bInSysex && m_bEndSysex) {
                MidiController::receive(m_cReceiveMsg, m_cReceiveMsg_index);
                m_bInSysex=false;
                m_bEndSysex=false;
                m_cReceiveMsg_index = 0;
            }
        }
    }
}

void PortMidiController::send(unsigned int word)
{
    if (m_pOutputStream)
    {
        PmError err = Pm_WriteShort(m_pOutputStream, 0, word);
        if( err != pmNoError ) qDebug() << "PortMidi sendShortMsg error:" << Pm_GetErrorText(err);
    }

}

// The sysex data must already contain the start byte 0xf0 and the end byte 0xf7.
void PortMidiController::send(unsigned char data[], unsigned int length)
{
    Q_UNUSED(length);   // We have to accept it even if we don't use it
                        // to be consistent with other MIDI APIs that do need it
    if (m_pOutputStream)
    {
        PmError err = Pm_WriteSysEx(m_pOutputStream, 0, data);
        if( err != pmNoError ) qDebug() << "PortMidi sm_bEndSysexMsg error:" << Pm_GetErrorText(err);
    }
}
