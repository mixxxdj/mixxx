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
    m_mappingMutex.lock();
    delete m_pMidiMapping;
    m_mappingMutex.unlock();
}

void MidiDevice::startup()
{
    m_mappingMutex.lock();
#ifdef __MIDISCRIPT__
    m_pMidiMapping->startupScriptEngine();
#endif
    m_mappingMutex.unlock();
}

void MidiDevice::shutdown()
{
    m_mappingMutex.lock();
#ifdef __MIDISCRIPT__
    m_pMidiMapping->shutdownScriptEngine();
#endif
    m_mappingMutex.unlock();
}

void MidiDevice::setMidiMapping(MidiMapping* mapping)
{
    m_mutex.lock();
    m_mappingMutex.lock();
    m_pMidiMapping = mapping;
    
    if (m_pCorrespondingOutputDevice)
    {
        m_pCorrespondingOutputDevice->setMidiMapping(m_pMidiMapping);
    }
    m_mappingMutex.unlock();
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

    m_mappingMutex.lock();
    connect(this, SIGNAL(midiEvent(MidiMessage)), m_pMidiMapping, SLOT(finishMidiLearn(MidiMessage)));
    m_mappingMutex.unlock();
}

void MidiDevice::disableMidiLearn() {
    m_mutex.lock();
    m_bMidiLearn = false;
    m_mutex.unlock();

    m_mappingMutex.lock();
    disconnect(this, SIGNAL(midiEvent(MidiMessage)), m_pMidiMapping, SLOT(finishMidiLearn(MidiMessage)));
    m_mappingMutex.unlock();
}

void MidiDevice::receive(MidiStatusByte status, char channel, char control, char value)
{
    QMutexLocker locker(&m_mutex); //Lots of returns in this function. Keeps things simple.
    if (midiDebugging()) qDebug() << QString("MIDI ch %1: status: %2, ctrl: %3, val: %4")
        .arg(QString::number(channel+1, 16).toUpper())
        .arg(QString::number(status & 255, 16).toUpper())
        .arg(QString::number(control, 16).toUpper())
        .arg(QString::number(value, 16).toUpper());

    MidiMessage inputCommand(status, control, channel);

    //If the receive inhibit flag is true, then we don't process any midi messages
    //that are received from the device. This is done in order to prevent a race
    //condition where the MidiMapping is accessed via isMidiMessageMapped() below
    //but it is already locked because it is being modified by the GUI thread.
    //(This happens when you hit apply in the preferences and then quickly push
    // a button on your controller.)
    if (m_bReceiveInhibit)
        return;

    if (m_bMidiLearn) {
        emit(midiEvent(inputCommand));
        return; // Don't process midi messages further when MIDI learning
    }

    QMutexLocker mappingLocker(&m_mappingMutex);

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

#ifdef __MIDISCRIPT__
    // Custom MixxxScript (QtScript) handler
    
    if (mixxxControl.getMidiOption() == MIDI_OPT_SCRIPT) {
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
        double newValue = m_pMidiMapping->ComputeValue(mixxxControl.getMidiOption(), p->GetMidiValue(), value);

        // ControlPushButton ControlObjects only accept NOTE_ON, so if the midi 
        // mapping is <button> we override the Midi 'status' appropriately.
        switch (mixxxControl.getMidiOption()) {
            case MIDI_OPT_BUTTON:
            case MIDI_OPT_SWITCH: status = MIDI_STATUS_NOTE_ON; break; // Buttons and Switches are 
                                                                       // treated the same, except 
                                                                       // that their values are 
                                                                       // computed differently.
            default: break;
        }

        ControlObject::sync();

        //Super dangerous cast here... Should be fine once MidiCategory is replaced with MidiStatusByte permanently.
        p->queueFromMidi((MidiCategory)status, newValue);
    }

    return;
}

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
