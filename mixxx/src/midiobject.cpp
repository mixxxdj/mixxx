/***************************************************************************
                          midiobject.cpp  -  description
                             -------------------
    copyright            : (C) 2002 by Tue and Ken Haste Andersen
    email                :
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include <QtDebug>
#include "midiobject.h"
#include "midimapping.h"
#include "midimessage.h"
#include "configobject.h"
#include "controlobject.h"
#include <algorithm>
#include <signal.h>

#ifdef __MIDISCRIPT__
#include "script/midiscriptengine.h"
#endif

static QString toHex(QString numberStr) {
    return "0x" + QString("0" + QString::number(numberStr.toUShort(), 16).toUpper()).right(2);
}

/* -------- ------------------------------------------------------
   Purpose: Initialize midi, and start parsing loop
   Input:   None. Automatically selects default midi input sound
            card and device.
   Output:  -
   -------- ------------------------------------------------------ */
MidiObject::MidiObject()
{
    no = 0;
    requestStop = false;
    m_bMidiLearn = false;


#ifdef __MIDISCRIPT__
    //Start the scripting engine.
    m_pScriptEngine = NULL;
    restartScriptEngine();

#endif
    m_pMidiMapping = new MidiMapping(*this);
#ifndef __MIDISCRIPT__
     m_pMidiMapping->loadInitialPreset();	// Only do this here if NOT using MIDI scripting
#endif
    connect(m_pMidiMapping, SIGNAL(midiLearningStarted()), this, SLOT(enableMidiLearn()));
    connect(m_pMidiMapping, SIGNAL(midiLearningFinished()), this, SLOT(disableMidiLearn()));

}

/* -------- ------------------------------------------------------
   Purpose: Deallocates midi buffer, and closes device
   Input:   -
   Output:  -
   -------- ------------------------------------------------------ */
MidiObject::~MidiObject()
{
}

#ifdef __MIDISCRIPT__

void MidiObject::restartScriptEngine()
{

    //qDebug() << QString("MidiObject: Creating MidiScriptEngine in Thread ID=%1").arg(QThread::currentThreadId(),0,16);
    
    if(m_pScriptEngine) {
        MidiScriptEngine *engine = m_pScriptEngine;
        m_pScriptEngine = NULL;
        delete engine;
    }

    m_pScriptEngine = new MidiScriptEngine(this);
    
    m_pScriptEngine->moveToThread(m_pScriptEngine);
    
    connect(m_pScriptEngine, SIGNAL(initialized()),
            this, SLOT(slotScriptEngineReady()),
            Qt::DirectConnection);
    m_scriptEngineInitializedMutex.lock();
    m_pScriptEngine->start();
    // Wait until the script engine is initialized
    m_scriptEngineInitializedCondition.wait(&m_scriptEngineInitializedMutex);
    m_scriptEngineInitializedMutex.unlock();
    
}
#endif

void MidiObject::slotScriptEngineReady() {
#ifdef __MIDISCRIPT__	// Can't ifdef slots in the .h file, so we just do the body.

    // The lock prevents us from waking before the main thread is waiting on the
    // condition.
    m_scriptEngineInitializedMutex.lock();
    m_scriptEngineInitializedCondition.wakeAll();
    m_scriptEngineInitializedMutex.unlock();

#endif
}



/* -------- ------------------------------------------------------
   Purpose: Deletes MIDI mapping and stops script engine, to be called
            by the child destructor
   Input:   -
   Output:  -
   -------- ------------------------------------------------------ */
void MidiObject::shutdown()
{
    qDebug() << "MidiObject: Deleting MidiMapping...";
    delete m_pMidiMapping;
#ifdef __MIDISCRIPT__
    qDebug() << "MidiObject: Deleting MIDI script engine...";
    delete m_pScriptEngine;
#endif
}

#ifdef __MIDISCRIPT__
/* -------- ------------------------------------------------------
   Purpose: Loads the script files & executes their init functions
   Input:   -
   Output:  -
   -------- ------------------------------------------------------ */
void MidiObject::run()
{

    unsigned static id = 0; //the id of this thread, for debugging purposes //XXX copypasta (should factor this out somehow), -kousu 2/2009
    QThread::currentThread()->setObjectName(QString("MidiObject %1").arg(++id));
    
    m_pMidiMapping->loadInitialPreset();    // Do this here so the script's init() function can run correctly
}
#endif

void MidiObject::reopen(QString device)
{
    devClose();
    devOpen(device);
}

/* -------- ------------------------------------------------------
   Purpose: Add a control ready to recieve midi events.
   Input:   a pointer to the control. The second argument
            is the method in the control to call when the button
        has been moved.
   Output:  -
   -------- ------------------------------------------------------ */
void MidiObject::add(ControlObject * c)
{
    no++;
    controlList.resize(no);
    controlList.insert(no-1,(const ControlObject *)c);
    // qDebug() << "Registered midi control" << c->print();
}

void MidiObject::remove(ControlObject * c)
{
    int i = controlList.find(c,0);
    if (i>=0)
    {
        controlList.remove(i);
        no--;
    }
    else
        qDebug() << "MidiObject: Control which is requested for removal does not exist.";
}

QStringList * MidiObject::getDeviceList()
{
    updateDeviceList();
    qDebug() << "getting midi device list, size " << devices.size() << " and: " << devices.join(", ");
    return &devices;
}

QStringList * MidiObject::getConfigList(QString path)
{
    // Make sure list is empty
    configs.clear();

    // Get list of available midi configurations
    QDir dir(path);
    dir.setFilter(QDir::Files);
    dir.setNameFilters(QString("*.midi.xml *.MIDI.XML").split(' ')); //there should be a way to pass Qt::CaseInsensitive here..

    //const QFileInfoList *list = dir.entryInfoList();
    //if (dir.entryInfoList().empty())
    {
        //QFileInfoListIterator it(*list);        // create list iterator
        QListIterator<QFileInfo> it(dir.entryInfoList());
        QFileInfo fi;                          // pointer for traversing
        while (it.hasNext())
        {
            fi = it.next();
            configs.append(fi.fileName());
        }
    }

    return &configs;
}

QString MidiObject::getOpenDevice()
{
    return m_deviceName;
}

/* -------- ------------------------------------------------------
   Purpose: Receive a MIDI event from the backend, do value conversion as required, and queue event to mixxx control
   Input:   Values as received from MIDI
   Output:  -
   -------- ------------------------------------------------------ */
void MidiObject::receive(MidiStatusByte status, char channel, char control, char value)
{
//    qDebug() << "MidiObject::receive() miditype: " << toHex(QString::number((int)status)) << " ch: " << toHex(QString::number((int)channel)) << ", ctrl: " << toHex(QString::number((int)control)) << ", val: " << toHex(QString::number((int)value));

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
//         qDebug() << "MidiObject: " << mixxxControl.getControlObjectGroup() << mixxxControl.getControlObjectValue();

    ConfigKey configKey(mixxxControl.getControlObjectGroup(), mixxxControl.getControlObjectValue());


#ifdef __MIDISCRIPT__
    // Custom MixxxScript (QtScript) handler
    if (mixxxControl.getMidiOption() == MIDI_OPT_SCRIPT) {
        // qDebug() << "MidiObject: Calling script function" << configKey.item << "with" << (int)channel << (int)control <<  (int)value << (int)status;

        if (!m_pScriptEngine->execute(configKey.item, channel, control, value, status)) {
            qDebug() << "MidiObject: Invalid script function" << configKey.item;
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

void MidiObject::stop()
{
    requestStop = true;
}

void abortRead(int)
{
    // Reinstall default handler
    signal(SIGINT,SIG_DFL);

#ifndef QT3_SUPPORT
    // End thread execution
    QThread::exit();
#endif
}

void MidiObject::sendShortMsg(unsigned char status, unsigned char byte1, unsigned char byte2) {
    unsigned int word = (((unsigned int)byte2) << 16) |
                        (((unsigned int)byte1) << 8) | status;
    sendShortMsg(word);
}

void MidiObject::sendShortMsg(unsigned int word) {
    qDebug() << "MIDI short message sending not yet implemented on this platform";
}

void MidiObject::sendSysexMsg(QList<int> data, unsigned int length) {
    unsigned char * sysexMsg;
    sysexMsg = new unsigned char [length];

    for (unsigned int i=0; i<length; i++) {
        sysexMsg[i] = data.at(i);
//         qDebug() << "sysexMsg" << i << "=" << sysexMsg[i] << ", data=" << data.at(i);
    }

    sendSysexMsg(sysexMsg,length);
    delete[] sysexMsg;
}

void MidiObject::sendSysexMsg(unsigned char data[], unsigned int length) {
    qDebug() << "MIDI system exclusive message sending not yet implemented on this platform";
}

bool MidiObject::getMidiLearnStatus() {
    return m_bMidiLearn;
}

void MidiObject::enableMidiLearn() {
    m_bMidiLearn = true;
    connect(this, SIGNAL(midiEvent(MidiMessage)), m_pMidiMapping, SLOT(finishMidiLearn(MidiMessage)));

}

void MidiObject::disableMidiLearn() {
    m_bMidiLearn = false;
    disconnect(this, SIGNAL(midiEvent(MidiMessage)), m_pMidiMapping, SLOT(finishMidiLearn(MidiMessage)));
}

#ifdef __MIDISCRIPT__
MidiScriptEngine * MidiObject::getMidiScriptEngine() {
    return m_pScriptEngine;
}
#endif

MidiMapping * MidiObject::getMidiMapping() {
    return m_pMidiMapping;
}
