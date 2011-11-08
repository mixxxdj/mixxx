/***************************************************************************
                             mididevice.cpp
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

#include <qapplication.h>   // For command line arguments
#include "mididevice.h"
#include "midimapping.h"
#include "midimessage.h"
#include "mixxxcontrol.h"
#include "controlobject.h"

#ifdef __MIDISCRIPT__
#include "midiscriptengine.h"
#endif

static QString toHex(QString numberStr) {
    return "0x" + QString("0" + QString::number(numberStr.toUShort(), 16).toUpper()).right(2);
}

MidiDevice::MidiDevice(MidiMapping* mapping) : QThread()
{

    m_bIsOutputDevice = false;
    m_bIsInputDevice = false;
    m_pMidiMapping = mapping;
    m_pCorrespondingOutputDevice = NULL;
    m_bIsOpen = false;
    m_bMidiLearn = false;
    m_bReceiveInhibit = false;

    if (m_pMidiMapping == NULL) {
        m_pMidiMapping = new MidiMapping(this);
    }

    // Get --midiDebug command line option
    QStringList commandLineArgs = QApplication::arguments();
    m_midiDebug = commandLineArgs.contains("--midiDebug", Qt::CaseInsensitive);

    connect(m_pMidiMapping, SIGNAL(midiLearningStarted()), this, SLOT(enableMidiLearn()));
    connect(m_pMidiMapping, SIGNAL(midiLearningFinished()), this, SLOT(disableMidiLearn()));
}

MidiDevice::~MidiDevice()
{
    QMutexLocker locker(&m_mutex);

    qDebug() << "MidiDevice: Deleting MidiMapping...";
    m_mappingPtrMutex.lock();
    delete m_pMidiMapping;
    m_mappingPtrMutex.unlock();
}

void MidiDevice::startup()
{
    QMutexLocker locker(&m_mappingPtrMutex);
#ifdef __MIDISCRIPT__
    setReceiveInhibit(true);
    m_pMidiMapping->startupScriptEngine();
    setReceiveInhibit(false);
#endif
}

void MidiDevice::shutdown()
{
    QMutexLocker locker(&m_mappingPtrMutex);
    //Stop us from processing any MIDI messages that are
    //received while we're trying to shut down the scripting engine.
    //This prevents a deadlock that can happen because we've locked
    //the MIDI mapping pointer mutex (in the line above). If a
    //MIDI message is received while this is locked, the script
    //engine can end up waiting for the MIDI message to be
    //mapped and we end up with a deadlock.
    //Similarly, if a MIDI message is sent from the scripting
    //engine while we've held this lock, we'll end up with
    //a similar deadlock. (Note that shutdownScriptEngine()
    //waits for the scripting engine thread to terminate,
    //which will never happen if it's stuck waiting for
    //m_mappingPtrMutex to unlock.
    setReceiveInhibit(true);
#ifdef __MIDISCRIPT__
    m_pMidiMapping->shutdownScriptEngine();
#endif
    setReceiveInhibit(false);
}

void MidiDevice::setMidiMapping(MidiMapping* mapping)
{
    m_mutex.lock();
    m_mappingPtrMutex.lock();
    m_pMidiMapping = mapping;

    if (m_pCorrespondingOutputDevice)
    {
        m_pCorrespondingOutputDevice->setMidiMapping(m_pMidiMapping);
    }
    m_mappingPtrMutex.unlock();
    m_mutex.unlock();
}

void MidiDevice::sendShortMsg(unsigned char status, unsigned char byte1, unsigned char byte2) {
    unsigned int word = (((unsigned int)byte2) << 16) |
                        (((unsigned int)byte1) << 8) | status;
    sendShortMsg(word);
}

void MidiDevice::sendShortMsg(unsigned int word) {
    m_mutex.lock();
    qDebug() << "MIDI short message sending not yet implemented for this API or platform";
    m_mutex.unlock();
}

void MidiDevice::sendSysexMsg(QList<int> data, unsigned int length) {
    m_mutex.lock();

    unsigned char * sysexMsg;
    sysexMsg = new unsigned char [length];

    for (unsigned int i=0; i<length; i++) {
        sysexMsg[i] = data.at(i);
//         qDebug() << "sysexMsg" << i << "=" << sysexMsg[i] << ", data=" << data.at(i);
    }

    sendSysexMsg(sysexMsg,length);
    delete[] sysexMsg;
    m_mutex.unlock();
}

void MidiDevice::sendSysexMsg(unsigned char data[], unsigned int length) {
    qDebug() << "MIDI system exclusive message sending not yet implemented for this API or platform";
}

bool MidiDevice::getMidiLearnStatus() {
    m_mutex.lock();
    bool learn = m_bMidiLearn;
    m_mutex.unlock();
    return learn;
}

void MidiDevice::enableMidiLearn() {
    m_mutex.lock();
    m_bMidiLearn = true;
    m_mutex.unlock();

    m_mappingPtrMutex.lock();
    connect(this, SIGNAL(midiEvent(MidiMessage)), m_pMidiMapping, SLOT(finishMidiLearn(MidiMessage)));
    m_mappingPtrMutex.unlock();
}

void MidiDevice::disableMidiLearn() {
    m_mutex.lock();
    m_bMidiLearn = false;
    m_mutex.unlock();

    m_mappingPtrMutex.lock();
    disconnect(this, SIGNAL(midiEvent(MidiMessage)), m_pMidiMapping, SLOT(finishMidiLearn(MidiMessage)));
    m_mappingPtrMutex.unlock();
}

void MidiDevice::receive(MidiStatusByte status, char channel, char control, char value)
{
    if (midiDebugging()) qDebug() << QString("MIDI status 0x%1 (ch %2, opcode 0x%3), ctrl 0x%4, val 0x%5")
      .arg(QString::number(status, 16).toUpper())
      .arg(QString::number(channel+1, 10))
      .arg(QString::number((status & 255)>>4, 16).toUpper())
      .arg(QString::number(control, 16).toUpper().rightJustified(2,'0'))
      .arg(QString::number(value, 16).toUpper().rightJustified(2,'0'));
    
    // some status bytes can have the channel encoded in them. Take out the
    // channel when necessary. We do this because later bits of this
    // function (and perhaps its callchain) assume the channel nibble to be
    // zero in its comparisons -- bkgood
    switch (status & 0xF0) {
    case MIDI_STATUS_NOTE_OFF:
    case MIDI_STATUS_NOTE_ON:
    case MIDI_STATUS_AFTERTOUCH:
    case MIDI_STATUS_CC:
    case MIDI_STATUS_PROGRAM_CH:
    case MIDI_STATUS_CH_AFTERTOUCH:
    case MIDI_STATUS_PITCH_BEND:
        status = (MidiStatusByte) (status & 0xF0);
    }
    QMutexLocker locker(&m_mutex); //Lots of returns in this function. Keeps things simple.

    MidiMessage inputCommand(status, control, channel);

    //If the receive inhibit flag is true, then we don't process any midi messages
    //that are received from the device. This is done in order to prevent a race
    //condition where the MidiMapping is accessed via isMidiMessageMapped() below
    //but it is already locked because it is being modified by the GUI thread.
    //(This happens when you hit apply in the preferences and then quickly push
    // a button on your controller.)
    // This also avoids deadlocks when the device is sending data while it's being
    //  initialized or shut down (more of a problem with scripted devices.)
    if (m_bReceiveInhibit)
        return;

    if (m_bMidiLearn) {
        emit(midiEvent(inputCommand));
        return; // Don't process midi messages further when MIDI learning
    }

    QMutexLocker mappingLocker(&m_mappingPtrMutex);

    // Only check for a mapping if the status byte is one we know how to handle
    if (status == MIDI_STATUS_NOTE_ON
         || status == MIDI_STATUS_NOTE_OFF
         || status == MIDI_STATUS_PITCH_BEND
         || status == MIDI_STATUS_CC) {
        // If there was no control bound to that MIDI command, return;
        if (!m_pMidiMapping->isMidiMessageMapped(inputCommand)) {
            return;
        }
    }

    MixxxControl mixxxControl = m_pMidiMapping->getInputMixxxControl(inputCommand);
    //qDebug() << "MidiDevice: " << mixxxControl.getControlObjectGroup() << mixxxControl.getControlObjectValue();

    ConfigKey configKey(mixxxControl.getControlObjectGroup(), mixxxControl.getControlObjectValue());
    
    MidiOption currMidiOption = mixxxControl.getMidiOption();

#ifdef __MIDISCRIPT__
    // Custom MixxxScript (QtScript) handler

    if (currMidiOption == MIDI_OPT_SCRIPT) {
        // qDebug() << "MidiDevice: Calling script function" << configKey.item << "with"
        //          << (int)channel << (int)control <<  (int)value << (int)status;

        //Unlock the mutex here to prevent a deadlock if a script needs to send a MIDI message
        //to the device. (sendShortMessage() would try to lock m_mutex...)
        locker.unlock();

        // This needs to be a signal because the MIDI Script Engine thread must execute
        //  script functions, not this MidiDevice one
        emit(callMidiScriptFunction(configKey.item, channel, control, value, status,
                                mixxxControl.getControlObjectGroup()));
        return;
    }
#endif

    ControlObject * p = ControlObject::getControl(configKey);

    if (p) //Only pass values on to valid ControlObjects.
    {
        double currMixxxControlValue = p->GetMidiValue();

        double newValue = value;

        // compute LSB and MSB for pitch bend messages
        if (status == MIDI_STATUS_PITCH_BEND) {
            unsigned int ivalue;
            ivalue = (value << 7) + control;

            newValue = m_pMidiMapping->ComputeValue(currMidiOption, currMixxxControlValue, ivalue);

            // normalize our value to 0-127
            newValue = (newValue / 0x3FFF) * 0x7F;
        } else if (currMidiOption != MIDI_OPT_SOFT_TAKEOVER) {
            newValue = m_pMidiMapping->ComputeValue(currMidiOption, currMixxxControlValue, value);
        }

        // ControlPushButton ControlObjects only accept NOTE_ON, so if the midi
        // mapping is <button> we override the Midi 'status' appropriately.
        switch (currMidiOption) {
            case MIDI_OPT_BUTTON:
            case MIDI_OPT_SWITCH: status = MIDI_STATUS_NOTE_ON; break; // Buttons and Switches are
                                                                       // treated the same, except
                                                                       // that their values are
                                                                       // computed differently.
            default: break;
        }
        
        // Soft-takeover is processed in addition to any other options
        if (currMidiOption == MIDI_OPT_SOFT_TAKEOVER) {
            m_st.enable(mixxxControl);  // This is the only place to enable it if it isn't already.
            if (m_st.ignore(mixxxControl,newValue,true)) return;
        }
        
        ControlObject::sync();

        //Super dangerous cast here... Should be fine once MidiCategory is replaced with MidiStatusByte permanently.
        p->queueFromMidi((MidiCategory)status, newValue);
    }

    return;
}

#ifdef __MIDISCRIPT__
// SysEx reception requires scripting
void MidiDevice::receive(const unsigned char data[], unsigned int length) {
    QMutexLocker locker(&m_mutex); //Lots of returns in this function. Keeps things simple.

    QString message = m_strDeviceName+": [";
    for(uint i=0; i<length; i++) {
        message += QString("%1%2")
                    .arg(data[i], 2, 16, QChar('0')).toUpper()
                    .arg((i<(length-1))?' ':']');
    }

    if (midiDebugging()) qDebug()<< message;

    MidiMessage inputCommand((MidiStatusByte)data[0]);

    //If the receive inhibit flag is true, then we don't process any midi messages
    //that are received from the device. This is done in order to prevent a race
    //condition where the MidiMapping is accessed via isMidiMessageMapped() below
    //but it is already locked because it is being modified by the GUI thread.
    //(This happens when you hit apply in the preferences and then quickly push
    // a button on your controller.)
    if (m_bReceiveInhibit)
        return;

    if (m_bMidiLearn)
        return; // Don't process custom midi messages when MIDI learning

    QMutexLocker mappingLocker(&m_mappingPtrMutex);

    MixxxControl mixxxControl = m_pMidiMapping->getInputMixxxControl(inputCommand);
    //qDebug() << "MidiDevice: " << mixxxControl.getControlObjectGroup() << mixxxControl.getControlObjectValue();

    ConfigKey configKey(mixxxControl.getControlObjectGroup(), mixxxControl.getControlObjectValue());

    // Custom MixxxScript (QtScript) handler

    if (mixxxControl.getMidiOption() == MIDI_OPT_SCRIPT) {
        // qDebug() << "MidiDevice: Calling script function" << configKey.item << "with"
        //          << (int)channel << (int)control <<  (int)value << (int)status;

        //Unlock the mutex here to prevent a deadlock if a script needs to send a MIDI message
        //to the device. (sendShortMessage() would try to lock m_mutex...)
        locker.unlock();

        if (!m_pMidiMapping->getMidiScriptEngine()->execute(configKey.item, data, length)) {
            qDebug() << "MidiDevice: Invalid script function" << configKey.item;
        }
        return;
    }
    qWarning() << "MidiDevice: No MIDI Script function found for" << message;
    return;
}
#endif

bool MidiDevice::midiDebugging()
{
    //Assumes a lock is already held. :/
    bool debug = m_midiDebug;
    return debug;
}

void MidiDevice::setReceiveInhibit(bool inhibit)
{
    //See comments for m_bReceiveInhibit.
    QMutexLocker locker(&m_mutex);
    m_bReceiveInhibit = inhibit;
}
