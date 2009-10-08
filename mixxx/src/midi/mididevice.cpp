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

MidiDevice::MidiDevice(MidiMapping* mapping) : QObject()
{
    m_bIsOutputDevice = false;
    m_bIsInputDevice = false;
    m_pMidiMapping = mapping;
    m_pCorrespondingOutputDevice = NULL;
    m_bIsOpen = false;
    m_bMidiLearn = false;

    if (m_pMidiMapping == NULL)
        m_pMidiMapping = new MidiMapping(this);
        
#ifndef __MIDISCRIPT__
     m_pMidiMapping->loadInitialPreset();    // Only do this here if NOT using MIDI scripting
#endif
    // Get --midiDebug command line option
    QStringList commandLineArgs = QApplication::arguments();
    m_midiDebug = commandLineArgs.indexOf("--midiDebug");
    
    connect(m_pMidiMapping, SIGNAL(midiLearningStarted()), this, SLOT(enableMidiLearn()));
    connect(m_pMidiMapping, SIGNAL(midiLearningFinished()), this, SLOT(disableMidiLearn()));
    
}

MidiDevice::~MidiDevice()
{
    qDebug() << "MidiDevice: Deleting MidiMapping...";
    delete m_pMidiMapping;

}

void MidiDevice::initialize()
{
    qDebug() << "MidiDevice::initialize()";
    m_pMidiMapping->scriptInitialize();
}

void MidiDevice::shutdown()
{
    m_pMidiMapping->scriptShutdown();
}

void MidiDevice::setMidiMapping(MidiMapping* mapping)
{
    m_pMidiMapping = mapping;
    
    if (m_pCorrespondingOutputDevice)
    {
        m_pCorrespondingOutputDevice->setMidiMapping(m_pMidiMapping);
    }
}

void MidiDevice::setOutputDevice(MidiDevice* outputDevice)
{
    Q_ASSERT(this->isInputDevice()); //The object we're in should support input.
    Q_ASSERT(outputDevice->isOutputDevice());
    
    m_pCorrespondingOutputDevice = outputDevice;
    // This IF avoids endless recursion if the input and output are the same MidiDevice
    if (outputDevice != this) m_pCorrespondingOutputDevice->setMidiMapping(m_pMidiMapping);
}


void MidiDevice::sendShortMsg(unsigned char status, unsigned char byte1, unsigned char byte2) {
    unsigned int word = (((unsigned int)byte2) << 16) |
                        (((unsigned int)byte1) << 8) | status;
    sendShortMsg(word);
}

void MidiDevice::sendShortMsg(unsigned int word) {
    qDebug() << "MIDI short message sending not yet implemented for this API or platform";
}

void MidiDevice::sendSysexMsg(QList<int> data, unsigned int length) {
    unsigned char * sysexMsg;
    sysexMsg = new unsigned char [length];

    for (unsigned int i=0; i<length; i++) {
        sysexMsg[i] = data.at(i);
//         qDebug() << "sysexMsg" << i << "=" << sysexMsg[i] << ", data=" << data.at(i);
    }

    sendSysexMsg(sysexMsg,length);
    delete[] sysexMsg;
}

void MidiDevice::sendSysexMsg(unsigned char data[], unsigned int length) {
    qDebug() << "MIDI system exclusive message sending not yet implemented for this API or platform";
}

bool MidiDevice::getMidiLearnStatus() {
    return m_bMidiLearn;
}

void MidiDevice::enableMidiLearn() {
    m_bMidiLearn = true;
    connect(this, SIGNAL(midiEvent(MidiMessage)), m_pMidiMapping, SLOT(finishMidiLearn(MidiMessage)));

}

void MidiDevice::disableMidiLearn() {
    m_bMidiLearn = false;
    disconnect(this, SIGNAL(midiEvent(MidiMessage)), m_pMidiMapping, SLOT(finishMidiLearn(MidiMessage)));
}

void MidiDevice::receive(MidiStatusByte status, char channel, char control, char value)
{
    if (midiDebugging()) qDebug() << QString("MIDI ch %1: status: %2, ctrl: %3, val: %4")
        .arg(QString::number(channel+1, 16).toUpper())
        .arg(QString::number(status & 255, 16).toUpper())
        .arg(QString::number(control, 16).toUpper())
        .arg(QString::number(value, 16).toUpper());

    MidiMessage inputCommand(status, control, channel);

    if (m_bMidiLearn) {
        emit(midiEvent(inputCommand));
        return; // Don't process midi messages further when MIDI learning
    }

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
    //FIXME: SEAN - The script engine will live inside the MidiMapping! Update this code accordingly.
    
    if (mixxxControl.getMidiOption() == MIDI_OPT_SCRIPT) {
        // qDebug() << "MidiDevice: Calling script function" << configKey.item << "with" << (int)channel << (int)control <<  (int)value << (int)status;

        if (!m_pMidiMapping->getMidiScriptEngine()->execute(configKey.item, channel, control, value, status)) {
            qDebug() << "MidiDevice: Invalid script function" << configKey.item;
        }
        return;
    }
#endif

    ControlObject * p = ControlObject::getControl(configKey);

    if (p) //Only pass values on to valid ControlObjects.
    {
      double newValue = m_pMidiMapping->ComputeValue(mixxxControl.getMidiOption(), p->GetMidiValue(), value);

      // ControlPushButton ControlObjects only accept NOTE_ON, so if the midi mapping is <button> we override the Midi 'status' appropriately.
      switch (mixxxControl.getMidiOption()) {
              case MIDI_OPT_BUTTON:
              case MIDI_OPT_SWITCH: status = MIDI_STATUS_NOTE_ON; break; // Buttons and Switches are treated the same, except that their values are computed differently.
              default: break;
      }

      ControlObject::sync();
    
        //Super dangerous cast here... Should be fine once MidiCategory is replaced with MidiStatusByte permanently.
      p->queueFromMidi((MidiCategory)status, newValue);
    }

    return;
}

bool MidiDevice::midiDebugging() {
    if (m_midiDebug != -1) return true;
    else return false;
}