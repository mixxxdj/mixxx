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
#include "configobject.h"
#include "controlobject.h"
#include <algorithm>
#include <signal.h>
#include "dlgprefmididevice.h"
#include "dlgprefmidibindings.h"

#ifdef __MIDISCRIPT__
#include "script/midiscriptengine.h"
#endif

/* -------- ------------------------------------------------------
   Purpose: Initialize midi, and start parsing loop
   Input:   None. Automatically selects default midi input sound
            card and device.
   Output:  -
   -------- ------------------------------------------------------ */
MidiObject::MidiObject()
{
    m_pMidiConfig = 0;
    no = 0;
    requestStop = false;
    midiLearn = false;
    debug = false;

#ifdef __MIDISCRIPT__
    //qDebug() << QString("MidiObject: Creating MidiScriptEngine in Thread ID=%1").arg(this->thread()->currentThreadId(),0,16);
    m_pScriptEngine = new MidiScriptEngine();

    m_pScriptEngine->start();
    // Wait for the m_pScriptEngine to initialize
    while(!m_pScriptEngine->isReady()) ;
    m_pScriptEngine->moveToThread(m_pScriptEngine);


    m_pScriptEngine->engineGlobalObject.setProperty("midi", m_pScriptEngine->getEngine()->newQObject(this));
    m_pMidiMapping = new MidiMapping(*this);
    m_pMidiMapping->loadInitialPreset();
#endif

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
/* -------- ------------------------------------------------------
   Purpose: Allows the child MIDI thread to create the ScriptEngine
            & load the script files
   Input:   -
   Output:  -
   -------- ------------------------------------------------------ */
void MidiObject::run()
{

    unsigned static id = 0; //the id of this thread, for debugging purposes //XXX copypasta (should factor this out somehow), -kousu 2/2009
    QThread::currentThread()->setObjectName(QString("MidiObject %1").arg(++id));
}
#endif

void MidiObject::setMidiConfig(ConfigObject<ConfigValueMidi> * pMidiConfig)
{
    m_pMidiConfig = pMidiConfig;
}

void MidiObject::reopen(QString device)
{
    devClose(device);
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

QStringList MidiObject::getOpenDevices()
{
    return openDevices;
}

/* -------- ------------------------------------------------------
   Purpose: Receive a MIDI event from the backend, do value conversion as required, and queue event to mixxx control
   Input:   Values as received from MIDI
   Output:  -
   -------- ------------------------------------------------------ */
void MidiObject::receive(MidiCategory category, char channel, char control, char value, QString device)
{
    // qDebug() << "Device:" << device << "RxEnabled:"<< RxEnabled[device];
    // if (!RxEnabled[device]) return;

    // BJW: From this point onwards, use human (1-based) channel numbers
    channel++;

//     qDebug() << "MidiObject::receive() miditype: " << (int)category << " ch: " << (int)channel << ", ctrl: " << (int)control << ", val: " << (int)value;

    MidiType type = MIDI_EMPTY;
    switch (category) {
    case NOTE_OFF:
        // BJW: Not clear why this is done.
        value = 1;
        // NB Fall-through
    case NOTE_ON:
        type = MIDI_KEY;
        break;
    case CTRL_CHANGE:
        type = MIDI_CTRL;
        break;
    case PITCH_WHEEL:
        type = MIDI_PITCH;
        break;
    default:
        type = MIDI_EMPTY;
    }

    //m_pMidiMappingtype,control,channel));
//     qDebug() << "MidiObject: type:" << (int)type << "control:" << (int)control << "channel:" << (int)channel;
    
    if (midiLearn) {
        emit(midiEvent(new ConfigValueMidi(type,control,channel), device));
        return; // Don't pass on controls when in dialog
    }

    if (debug) {
        emit(debugInfo(new ConfigValueMidi(type,control,channel), value, device));
        return; // Don't pass on controls when in dialog
    }
    
    MidiCommand inputCommand(type, control, channel);

    //If there was no control bound to that MIDI command, return;
    if (!m_pMidiMapping->isMidiCommandMapped(inputCommand))
        return;

    MidiControl midiControl = m_pMidiMapping->getInputMidiControl(inputCommand);
//         qDebug() << "MidiObject: " << midiControl.getControlObjectGroup() << midiControl.getControlObjectValue();

    ConfigKey configKey(midiControl.getControlObjectGroup(), midiControl.getControlObjectValue());


#ifdef __MIDISCRIPT__
    // Custom MixxxScript (QtScript) handler
    if (midiControl.getMidiOption() == MIDI_OPT_SCRIPT) {
//         qDebug() << "MidiObject: Calling script function" << configKey.item;

        if (!m_pScriptEngine->execute(configKey.item, channel, device, control, value, category)) {
            qDebug() << "MidiObject: Invalid script function" << configKey.item;
        }
        return;
    }
#endif

    ControlObject * p = ControlObject::getControl(configKey);

    if (p) //Only pass values on to valid ControlObjects.
    {
        double newValue = (double)value;
        m_pMidiMapping->ComputeValue(midiControl.getMidiOption(), p->GetMidiValue(), newValue);
        // qDebug() << "value coming out ComputeValue: " << newValue;

        ControlObject::sync();

        p->queueFromMidi(category, newValue);
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

void MidiObject::sendShortMsg(unsigned char status, unsigned char byte1, unsigned char byte2, QString device) {
//    if (!TxEnabled[device]) { qDebug() << "Device:"<< device << "is not enabled for transmit."; return; }
    unsigned int word = (((unsigned int)byte2) << 16) |
                        (((unsigned int)byte1) << 8) | status;
    sendShortMsg(word);
}

void MidiObject::sendShortMsg(unsigned int /* word */) {
    // This warning comes out rather frequently now we're using LEDs with VuMeters
    // qDebug() << "MIDI message sending not implemented yet on this platform";
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

bool MidiObject::getRxStatus(QString device) {
    return RxEnabled[device];
}

bool MidiObject::getTxStatus(QString device) {
    return TxEnabled[device];
}

void MidiObject::setRxStatus(QString device, bool status) {
    RxEnabled[device] = status;
}

void MidiObject::setTxStatus(QString device, bool status) {
    TxEnabled[device] = status;
}

bool MidiObject::getDebugStatus() {
    return debug;
}

void MidiObject::enableDebug(DlgPrefMidiDevice *dlgDevice) {
    debug = true;
    this->dlgDevice = dlgDevice;
    connect(this, SIGNAL(debugInfo(ConfigValueMidi *, char, QString)), dlgDevice, SLOT(slotDebug(ConfigValueMidi *, char, QString)));
}

void MidiObject::disableDebug() {
    debug = false;
}

bool MidiObject::getMidiLearnStatus() {
    return midiLearn;
}

void MidiObject::enableMidiLearn(DlgPrefMidiBindings *dlgBindings) {
    midiLearn = true;
    this->dlgBindings = dlgBindings;
    connect(this, SIGNAL(midiEvent(ConfigValueMidi *, QString)), dlgBindings, SLOT(singleLearn(ConfigValueMidi *, QString)));
    connect(this, SIGNAL(midiEvent(ConfigValueMidi *, QString)), dlgBindings, SLOT(groupLearn(ConfigValueMidi *, QString)));
}

void MidiObject::disableMidiLearn() {
    midiLearn = false;
}

#ifdef __MIDISCRIPT__
MidiScriptEngine * MidiObject::getMidiScriptEngine() {
    return m_pScriptEngine;
}
#endif

MidiMapping * MidiObject::getMidiMapping() {
    return m_pMidiMapping;
}
