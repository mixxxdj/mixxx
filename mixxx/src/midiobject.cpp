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
#include "configobject.h"
#include "controlobject.h"
#include <algorithm>
#include <signal.h>
#include "dlgprefmididevice.h"
#include "dlgprefmidibindings.h"

#ifdef __SCRIPT__
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
}

/* -------- ------------------------------------------------------
   Purpose: Deallocates midi buffer, and closes device
   Input:   -
   Output:  -
   -------- ------------------------------------------------------ */
MidiObject::~MidiObject()
{
}

#ifdef __SCRIPT__
/* -------- ------------------------------------------------------
   Purpose: Loads and processes MIDI scripts
   Input:   -
   Output:  -
   -------- ------------------------------------------------------ */
void MidiObject::loadScripts()
{
    m_pScriptEngine = new MidiScriptEngine();
    m_pScriptEngine->engineGlobalObject.setProperty("midi", m_pScriptEngine->getEngine()->newQObject(this));

    ConfigObject<ConfigValue> *m_pConfig = new ConfigObject<ConfigValue>(QDir::homePath().append("/").append(SETTINGS_FILE));
    
    // Individually load & evaluate script files in the QList (built by dlgPrefMidiBindings) to check for errors
    // so line numbers will be accurate per file (for syntax errors at least) to make troubleshooting much easier
    bool scriptError = false;
    
    for (int i=0; i<scriptFileNames.size(); i++) {
        m_pScriptEngine->clearCode();   // So line numbers will be correct
        
        QString filename = scriptFileNames.at(i);
        qDebug() << "MidiObject: Loading & testing MIDI script" << filename;
        m_pScriptEngine->loadScript(m_pConfig->getConfigPath().append("midi/").append(filename));
        
        m_pScriptEngine->evaluateScript();
        if (!m_pScriptEngine->checkException() && m_pScriptEngine->isGood()) qDebug() << "MidiObject: Success";
        else {
            // This is only included for completeness since checkException should pop a qCritical() itself if there's a problem
            qCritical() << "MidiObject: Failure evaluating MIDI script" << filename;
            scriptError = true;
        }
    }
    
    qDebug() << "MidiObject: Loading & evaluating all MIDI script code";
    m_pScriptEngine->clearCode();   // Start from scratch
    
    if (!scriptError) {
        while (!scriptFileNames.isEmpty()) {
            m_pScriptEngine->loadScript(m_pConfig->getConfigPath().append("midi/").append(scriptFileNames.takeFirst()));
        }
        
        m_pScriptEngine->evaluateScript();
        if (!m_pScriptEngine->checkException() && m_pScriptEngine->isGood()) qDebug() << "MidiObject: Script code evaluated successfully";
    
        // Call each script's init function if it exists
        while (!scriptFunctionPrefixes.isEmpty()) {
            QString initName = scriptFunctionPrefixes.takeFirst();
            if (initName!="") {
                initName.append(".init");
                qDebug() << "MidiObject: Executing" << initName;
                QScriptValue scriptFunction = m_pScriptEngine->execute(initName);
                if (!scriptFunction.isFunction()) qWarning() << "MidiObject: No" << initName << "function in script";
                else {
                    scriptFunction.call(QScriptValue());
                    m_pScriptEngine->checkException();
                }
            }
        }
    }
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
    dir.setNameFilter("*.midi.xml *.MIDI.XML");

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
    // qDebug() << "MidiObject::send() miditype: " << category << " ch: " << channel << ", ctrl: " << control << ", val: " << value;

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
/*
    qDebug() << QString("MidiObject::receive from device: %1, type: %2, catagory: %3, ch: %4, ctrl: %5, val: %6").arg(device)
    .arg(QString::number(type))
    .arg(QString::number(category, 16).toUpper())
    .arg(QString::number(channel, 16).toUpper())
    .arg(QString::number(control, 16).toUpper())
    .arg(QString::number(value, 16).toUpper());
*/
    if (midiLearn) {
        emit(midiEvent(new ConfigValueMidi(type,control,channel), device));
        return; // Don't pass on controls when in dialog
    }

    if (debug) {
        emit(debugInfo(new ConfigValueMidi(type,control,channel), value, device));
        return; // Don't pass on controls when in dialog
    }

    if (!m_pMidiConfig) {
    return;
    } // Used to be this:
    //Q_ASSERT(m_pMidiConfig);
    ConfigKey * pConfigKey = m_pMidiConfig->get(ConfigValueMidi(type,control,channel));

    if (!pConfigKey) return; // No configuration was retrieved for this input event, eject.
    // qDebug() << "MidiObject::receive ok" << pConfigKey->group << pConfigKey->item;
    ConfigOption<ConfigValueMidi> *c = m_pMidiConfig->get(*pConfigKey);

#ifdef __SCRIPT__
    // Custom MixxxScript (QtScript) handler
    if (((ConfigValueMidi *)c->val)->midioption==MIDI_OPT_SCRIPT) {
//         qDebug() << "MidiObject: Calling script function" << pConfigKey->item;
        QScriptValue scriptFunction = m_pScriptEngine->execute(pConfigKey->item);
        if (!scriptFunction.isFunction()) {
            qDebug() << "MidiObject: Invalid function" << pConfigKey->item;
            return;
        }
        else {
//             ScriptMidiMsg *tempMidiMsg = new ScriptMidiMsg(channel, control, value, device);
//             QScriptValue msg = m_pScriptEngine->getEngine().newQObject(tempMidiMsg);
//             m_pScriptEngine->engineGlobalObject.setProperty("msg", msg);
            QScriptValueList args;
            // Channel and Device should be in this object
            args << QScriptValue(m_pScriptEngine->getEngine(), channel);
            args << QScriptValue(m_pScriptEngine->getEngine(), device);
            // -----
            args << QScriptValue(m_pScriptEngine->getEngine(), control);
            args << QScriptValue(m_pScriptEngine->getEngine(), value);
            args << QScriptValue(m_pScriptEngine->getEngine(), category);

            scriptFunction.call(QScriptValue(),args);
            m_pScriptEngine->checkException();
            return;
        }
    }
#endif

    ControlObject * p = ControlObject::getControl(*pConfigKey);
    // qDebug() << "MidiObject::receive value:" << QString::number(value, 16).toUpper() << " c:" << c << "c->midioption:" << ((ConfigValueMidi *)c->val)->midioption << "p:" << p;

    // BJW: Apply any mapped (7-bit integer) translations
    if (c && p) {
        value = ((ConfigValueMidi *)c->val)->translateValue(value);
    }

    // BJW: newValue is the post-processed value, which may be floating point.
    double newValue = (double) value;

    // This is done separately from the switch above because it needs to go after translateValue()
    if (type == MIDI_PITCH) {
        unsigned int _14bit;
        // Pitch bend should be 14-bit control, so control and value are multiplexed
        // (value is the MSB, control the LSB)
        // and passed as a double within the 0-127 range, but with decimal info
        // BJW: Moved here from below so that the right numbers go into ComputeValue
        _14bit = value;
        _14bit <<= 7;
        _14bit |= (unsigned char) control;
        // qDebug() << "-- 14 bit pitch " << _14bit;
        // Need to force the centre point, otherwise the conversion formula maps it very slightly off-centre (not any more)
        if (_14bit == 8192)
            newValue = 64.0;
        else
            newValue = (double) _14bit * 127. / 16383.;
        // qDebug() << "-- converted to " << newValue;
    }

    if (c && p)
    {
        // qDebug() << "value going into ComputeValue: " << newValue;
        ControlObject::sync();
        newValue = ((ConfigValueMidi *)c->val)->ComputeValue(type, p->GetMidiValue(), newValue);
        // qDebug() << "value coming out ComputeValue: " << newValue;

        // I'm not sure entirely why buttons should be special here or what the difference is - Adam
        if (((ConfigValueMidi *)c->val)->midioption == MIDI_OPT_BUTTON || ((ConfigValueMidi *)c->val)->midioption == MIDI_OPT_SWITCH) {
            p->set(newValue);
            // qDebug() << "New Control Value: " << newValue << " (skipping queueFromMidi call)";
            return;
        }

        // qDebug() << "New Control Value: " << newValue << " ";
        p->queueFromMidi(category, newValue);
    }
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

    for (int i=0; i<length; i++) {
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

#ifdef __SCRIPT__
MidiScriptEngine * MidiObject::getMidiScriptEngine() {
    return m_pScriptEngine;
}
#endif
