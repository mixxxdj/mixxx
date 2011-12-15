/***************************************************************************
                             midimapping.cpp - "Wow, I wrote more new code!"
                           MIDI Mapping Class
                           -------------------
    begin                : Sat Jan 17 2009
    copyright            : (C) 2009 Sean M. Pappalardo
                           (C) 2009 Albert Santoni
    email                : pegasus@c64.org

***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include <qapplication.h>
#include "defs_version.h"
#include "widget/wwidget.h"    // FIXME: This should be xmlparse.h
#include "mixxxcontrol.h"
#include "midimessage.h"
#include "defs.h"
#include "midiinputmappingtablemodel.h"
#include "midioutputmappingtablemodel.h"
#include "midimapping.h"
#include "mididevicedummy.h"
#include "midiledhandler.h"
#include "configobject.h"
#include "errordialoghandler.h"

#define REQUIRED_SCRIPT_FILE "midi-mappings-scripts.js"
#define XML_SCHEMA_VERSION "1"
#define DEFAULT_DEVICE_PRESET BINDINGS_PATH.append(m_deviceName.right(m_deviceName.size()-m_deviceName.indexOf(" ")-1).replace(" ", "_") + MIDI_MAPPING_EXTENSION)

// static QString toHex(QString numberStr) {
//     return "0x" + QString("0" + QString::number(numberStr.toUShort(), 16).toUpper()).right(2);
// }

MidiMapping::MidiMapping(MidiDevice* outputMidiDevice)
        : QObject(),
          m_mappingLock(QMutex::Recursive) {
    // If BINDINGS_PATH doesn't exist, create it
    if (!QDir(BINDINGS_PATH).exists()) {
        qDebug() << "Creating new MIDI mapping directory" << BINDINGS_PATH;
        QDir().mkpath(BINDINGS_PATH);
    }
    // Likewise, if LPRESETS_PATH doesn't exist, create that too
    if (!QDir(LPRESETS_PATH).exists()) {
        qDebug() << "Creating new MIDI presets directory" <<LPRESETS_PATH;
        QDir().mkpath(LPRESETS_PATH);
    }
    // So we can signal the MidiScriptEngine and pass a QList
    qRegisterMetaType<QList<QString> >("QList<QString>");

    //Q_ASSERT(outputMidiDevice);

#ifdef __MIDISCRIPT__
    //Start the scripting engine.
    m_pScriptEngine = NULL;
    m_pOutputMidiDevice = outputMidiDevice;
    if (m_pOutputMidiDevice)
        m_deviceName = m_pOutputMidiDevice->getName(); //Name of the device to look for the <controller> block for in the XML.

    startupScriptEngine();
#endif
    m_pMidiInputMappingTableModel = new MidiInputMappingTableModel(this);
    m_pMidiOutputMappingTableModel = new MidiOutputMappingTableModel(this);
}

MidiMapping::~MidiMapping() {
    delete m_pMidiInputMappingTableModel;
    delete m_pMidiOutputMappingTableModel;
}

#ifdef __MIDISCRIPT__
void MidiMapping::startupScriptEngine() {
    QMutexLocker Locker(&m_mappingLock);

    if(m_pScriptEngine) return;

    //XXX FIXME: Deadly hack attack:
    if (m_pOutputMidiDevice == NULL) {
        m_pOutputMidiDevice = new MidiDeviceDummy(this); //Just make some dummy device :(
        /* Why can't this be the same as the input MIDI device? Most of the time,
            the script engine thinks of it as the input device. Scripting is useful
            even if the MIDI device has no outputs - Sean 4/19/10   */
    }
    //XXX Memory leak :(

    qDebug () << "Starting script engine with output device" << m_pOutputMidiDevice->getName();

    m_pScriptEngine = new MidiScriptEngine(m_pOutputMidiDevice);

    m_pScriptEngine->moveToThread(m_pScriptEngine);

    // Let the script engine tell us when it's done with each task
    connect(m_pScriptEngine, SIGNAL(initialized()),
            this, SLOT(slotScriptEngineReady()),
            Qt::DirectConnection);
    m_scriptEngineInitializedMutex.lock();
    m_pScriptEngine->start();
    // Wait until the script engine is initialized
    m_scriptEngineInitializedCondition.wait(&m_scriptEngineInitializedMutex);
    m_scriptEngineInitializedMutex.unlock();

    // Allow this object to signal the MidiScriptEngine to load the list of script files
    connect(this, SIGNAL(loadMidiScriptFiles(QList<QString>)), m_pScriptEngine, SLOT(loadScriptFiles(QList<QString>)));
    // Allow this object to signal the MidiScriptEngine to run the initialization
    //  functions in the loaded scripts
    connect(this, SIGNAL(initMidiScripts(QList<QString>)), m_pScriptEngine, SLOT(initializeScripts(QList<QString>)));
    // Allow this object to signal the MidiScriptEngine to run its shutdown routines
    connect(this, SIGNAL(shutdownMidiScriptEngine(QList<QString>)), m_pScriptEngine, SLOT(gracefulShutdown(QList<QString>)));

    // Allow this object to signal the MidiScriptEngine to call functions
    connect(this, SIGNAL(callMidiScriptFunction(QString)),
            m_pScriptEngine, SLOT(execute(QString)));
    connect(this, SIGNAL(callMidiScriptFunction(QString, QString)),
            m_pScriptEngine, SLOT(execute(QString, QString)));

    // Allow the MidiScriptEngine to tell us if it needs to reset the controller (on errors)
    connect(m_pScriptEngine, SIGNAL(resetController()), this, SLOT(reset()));
}

void MidiMapping::loadScriptCode() {
    QMutexLocker Locker(&m_mappingLock);
    if(m_pScriptEngine) {
        m_scriptEngineInitializedMutex.lock();
        // Tell the script engine to run the init function in all loaded scripts
        emit(loadMidiScriptFiles(m_scriptFileNames));
        // Wait until it's done
        m_scriptEngineInitializedCondition.wait(&m_scriptEngineInitializedMutex);
        m_scriptEngineInitializedMutex.unlock();
    }
}

void MidiMapping::initializeScripts() {
    QMutexLocker Locker(&m_mappingLock);
    if(m_pScriptEngine) {
        m_scriptEngineInitializedMutex.lock();
        // Tell the script engine to run the init function in all loaded scripts
        emit(initMidiScripts(m_scriptFunctionPrefixes));
        // Wait until it's done
        m_scriptEngineInitializedCondition.wait(&m_scriptEngineInitializedMutex);
        m_scriptEngineInitializedMutex.unlock();
    }
}

void MidiMapping::shutdownScriptEngine() {
    QMutexLocker Locker(&m_mappingLock);
    if(m_pScriptEngine) {
        // Tell the script engine to do its shutdown sequence
        emit(shutdownMidiScriptEngine(m_scriptFunctionPrefixes));
        // ...and wait for it to finish
        m_pScriptEngine->wait();

        MidiScriptEngine *engine = m_pScriptEngine;
        m_pScriptEngine = NULL;
        delete engine;
    }
}
#endif

void MidiMapping::setOutputMidiDevice(MidiDevice* outputMidiDevice)
{
    m_mappingLock.lock();
    m_pOutputMidiDevice = outputMidiDevice;
    m_mappingLock.unlock();
#ifdef __MIDISCRIPT__
    //Restart the script engine so it gets its pointer to the output MIDI device updated.
    restartScriptEngine();
#endif
}

/* ============================== MIDI Input Mapping Modifiers
                                    (Part of QT MVC wrapper)
*/

/*
 * Return the total number of current input mappings.
 */
int MidiMapping::numInputMidiMessages() {
    m_mappingLock.lock();
    int value = internalNumInputMidiMessages();
    m_mappingLock.unlock();
    return value;
}
int MidiMapping::internalNumInputMidiMessages() {
    return m_inputMapping.size();;
}

/*
 * Return true if the index corresponds to an input mapping key.
 */
bool MidiMapping::isInputIndexValid(int index) {
    if(index < 0 || index >= numInputMidiMessages()) {
        return false;
    }
    return true;
}

bool MidiMapping::internalIsInputIndexValid(int index) {
    if(index < 0 || index >= internalNumInputMidiMessages()) {
        return false;
    }
    return true;
}

bool MidiMapping::isMidiMessageMapped(MidiMessage command) {
    m_mappingLock.lock();
    bool value = m_inputMapping.contains(command);
    m_mappingLock.unlock();
    return value;
}

/*
 * Lookup the MidiMessage corresponding to a given index.
 */
MidiMessage MidiMapping::getInputMidiMessage(int index) {
    m_mappingLock.lock();
    MidiMessage message;
    if (internalIsInputIndexValid(index)) {
        message = m_inputMapping.keys().at(index);
    }
    m_mappingLock.unlock();
    return message;
}

/*
 * Lookup the MixxxControl mapped to a given MidiMessage (by index).
 */
MixxxControl MidiMapping::getInputMixxxControl(int index) {
    m_mappingLock.lock();
    MixxxControl control;
    if (internalIsInputIndexValid(index)) {
        MidiMessage key = m_inputMapping.keys().at(index);
        control = m_inputMapping.value(key);
    }
    m_mappingLock.unlock();
    return control;
}

/*
 * Lookup the MixxxControl mapped to a given MidiMessage.
 */
MixxxControl MidiMapping::getInputMixxxControl(MidiMessage command) {
    m_mappingLock.lock();
    MixxxControl control;
    if (m_inputMapping.contains(command)) {
        control = m_inputMapping.value(command);
    }
    m_mappingLock.unlock();
    return control;
}

/*
 * Set a MidiMessage -> MixxxControl mapping, replacing an existing one
 * if necessary.
 */
void MidiMapping::setInputMidiMapping(MidiMessage command, MixxxControl control) {
    m_mappingLock.lock();
    internalSetInputMidiMapping(command, control, false);
    m_mappingLock.unlock();
    emit(inputMappingChanged());
}

void MidiMapping::internalSetInputMidiMapping(MidiMessage command, MixxxControl control, bool shouldEmit) {
    // If the command is already in the mapping, it will be replaced
    m_inputMapping.insert(command,control);
    if (shouldEmit)
        emit(inputMappingChanged());
}


/*
 * Clear a specific mapping for a MidiMessage by index.
 */
void MidiMapping::clearInputMidiMapping(int index) {
    bool valid = false;
    m_mappingLock.lock();
    if (internalIsInputIndexValid(index)) {
        MidiMessage key = m_inputMapping.keys().at(index);
        m_inputMapping.remove(key);
        valid = true;
    }
    m_mappingLock.unlock();

    if (valid)
        emit(inputMappingChanged());
}

/*
 * Clear a specific mapping for a MidiMessage.
 */
void MidiMapping::clearInputMidiMapping(MidiMessage command) {
    m_mappingLock.lock();
    int changed = m_inputMapping.remove(command);
    m_mappingLock.unlock();

    if(changed > 0)
        emit(inputMappingChanged());
}

/*
 * Clears a range of input mappings. (This really only exists so that
 * a caller can atomically remove a range of rows)
 *
 */
void MidiMapping::clearInputMidiMapping(int index, int count) {
    m_mappingLock.lock();
    QList<MidiMessage> keys = m_inputMapping.keys();
    int changed = 0;
    for(int i=index; i < index+count; i++) {
        MidiMessage command = keys.at(i);
        changed += m_inputMapping.remove(command);
    }
    m_mappingLock.unlock();
    if(changed > 0)
        emit(inputMappingChanged());
}

/*==== End of MIDI input mapping modifiers (part of QT MVC wrapper) */



/* ============================== MIDI ***Output*** Mapping Modifiers
                                    (Part of QT MVC wrapper)
*/

/*
 * Return the total number of current output mappings.
 */
int MidiMapping::numOutputMixxxControls() {
    m_mappingLock.lock();
    int value = internalNumOutputMixxxControls();
    m_mappingLock.unlock();
    return value;
}

int MidiMapping::internalNumOutputMixxxControls() {
    return m_outputMapping.size();
}


/*
 * Return true if the index corresponds to an input mapping key.
 */
bool MidiMapping::isOutputIndexValid(int index) {
    m_mappingLock.lock();
    bool result = internalIsOutputIndexValid(index);
    m_mappingLock.unlock();
    return result;
}

bool MidiMapping::internalIsOutputIndexValid(int index) {
    if(index < 0 || index >= internalNumOutputMixxxControls()) {
        return false;
    }
    return true;
}


bool MidiMapping::isMixxxControlMapped(MixxxControl control) {
    m_mappingLock.lock();
    bool result = m_outputMapping.contains(control);
    m_mappingLock.unlock();
    return result;
}

/*
 * Lookup the MidiMessage corresponding to a given index.
 */
MixxxControl MidiMapping::getOutputMixxxControl(int index) {
    m_mappingLock.lock();
    MixxxControl control;
    if (!internalIsOutputIndexValid(index)) {
        control = m_outputMapping.keys().at(index);
    }
    m_mappingLock.unlock();
    return control;
}

/*
 * Lookup the MixxxControl mapped to a given MidiMessage (by index).
 */
MidiMessage MidiMapping::getOutputMidiMessage(int index) {
    qDebug() << "getOutputMidiMessage" << index;
    m_mappingLock.lock();
    MidiMessage message;
    if (internalIsOutputIndexValid(index)) {
        MixxxControl key = m_outputMapping.keys().at(index);
        message = m_outputMapping.value(key);
    }
    m_mappingLock.unlock();
    return message;
}

/*
 * Lookup the MixxxControl mapped to a given MidiMessage.
 */
MidiMessage MidiMapping::getOutputMidiMessage(MixxxControl control) {
    m_mappingLock.lock();
    MidiMessage message;
    if (m_outputMapping.contains(control)) {
        message = m_outputMapping.value(control);
    }
    m_mappingLock.unlock();
    return message;
}

/*
 * Set a MidiMessage -> MixxxControl mapping, replacing an existing one
 * if necessary.
 */
void MidiMapping::setOutputMidiMapping(MixxxControl control, MidiMessage command) {
    m_mappingLock.lock();
    internalSetOutputMidiMapping(control, command, false);
    m_mappingLock.unlock();
    emit(outputMappingChanged());
}

void MidiMapping::internalSetOutputMidiMapping(MixxxControl control,
                                               MidiMessage command,
                                               bool shouldEmit) {
    // If the command is already in the mapping, it will be replaced
    m_outputMapping.insert(control, command);

    if (shouldEmit)
        emit(outputMappingChanged());
}

/*
 * Clear a specific mapping for a MidiMessage by index.
 */
void MidiMapping::clearOutputMidiMapping(int index) {
    m_mappingLock.lock();
    bool changed = false;
    if (internalIsOutputIndexValid(index)) {
        qDebug() << m_outputMapping.size();
        qDebug() << "MidiMapping: removing" << index;
        MixxxControl key = m_outputMapping.keys().at(index);
        m_outputMapping.remove(key);
        qDebug() << m_outputMapping.size();
        changed = true;
    }
    m_mappingLock.unlock();

    if (changed)
        emit(outputMappingChanged());
}

/*
 * Clear a specific mapping for a MidiMessage.
 */
void MidiMapping::clearOutputMidiMapping(MixxxControl control) {
    m_mappingLock.lock();
    int changed = m_outputMapping.remove(control);
    m_mappingLock.unlock();

    if(changed > 0)
        emit(outputMappingChanged());
}

/*
 * Clears a range of input mappings. (This really only exists so that
 * a caller can atomically remove a range of rows)
 *
 */
void MidiMapping::clearOutputMidiMapping(int index, int count) {
    m_mappingLock.lock();
    QList<MixxxControl> keys = m_outputMapping.keys();
    int changed = 0;
    for(int i=index; i < index+count; i++) {
        MixxxControl control = keys.at(i);
        changed += m_outputMapping.remove(control);
    }
    m_mappingLock.unlock();

    if(changed > 0)
        emit(outputMappingChanged());
}

/*==== End of MIDI output mapping modifiers (part of QT MVC wrapper) */


#ifdef __MIDISCRIPT__
/* -------- ------------------------------------------------------
   Purpose: Adds an entry to the list of script file names
            & associated list of function prefixes
   Input:   QString file name, QString function prefix
   Output:  -
   -------- ------------------------------------------------------ */
void MidiMapping::addScriptFile(QString filename, QString functionprefix) {
    QMutexLocker Locker(&m_mappingLock);
    m_scriptFileNames.append(filename);
    m_scriptFunctionPrefixes.append(functionprefix);
}
#endif

/* setName(QString)
 * Sets the controller name this mapping corresponds to
 * @param name The controller name this mapping is hooked to
 */
void MidiMapping::setName(QString name) {
    QMutexLocker Locker(&m_mappingLock);
    m_deviceName = name;
}

/* loadPreset()
 * Overloaded function for convenience, uses the default device path
 * @param forceLoad Forces the MIDI mapping to be loaded, regardless of whether or not the controller id
 *        specified in the mapping matches the device this MidiMapping object is hooked up to.
 */
void MidiMapping::loadPreset(bool forceLoad) {
    loadPreset(DEFAULT_DEVICE_PRESET, forceLoad);
}

/* loadPreset(QString)
 * Overloaded function for convenience
 * @param path The path to a MIDI mapping XML file.
 * @param forceLoad Forces the MIDI mapping to be loaded, regardless of whether or not the controller id
 *        specified in the mapping matches the device this MidiMapping object is hooked up to.
 */
void MidiMapping::loadPreset(QString path, bool forceLoad) {
    qDebug() << "MidiMapping: Loading MIDI preset from" << path;
    loadPreset(WWidget::openXMLFile(path, "controller"), forceLoad);
}

/* loadPreset(QDomElement)
 * Loads a set of MIDI bindings from a QDomElement structure.
 * @param root The root node of the XML document for the MIDI mapping.
 * @param forceLoad Forces the MIDI mapping to be loaded, regardless of whether or not the controller id
 *        specified in the mapping matches the device this MidiMapping object is hooked up to.
 */
void MidiMapping::loadPreset(QDomElement root, bool forceLoad) {
    //qDebug() << QString("MidiMapping: loadPreset() called in thread ID=%1").arg(this->thread()->currentThreadId(),0,16);

    if (root.isNull()) return;

    m_pMidiInputMappingTableModel->removeRows(0, m_pMidiInputMappingTableModel->rowCount());
    m_pMidiOutputMappingTableModel->removeRows(0, m_pMidiOutputMappingTableModel->rowCount());

    // Note, the lock comes after these two lines. We mustn't touch the
    // *MappingTableModel's after we are locked because they have pointers to us
    // so when we make a call to them they might in turn call us, causing a
    // deadlock.
    m_mappingLock.lock();

#ifdef __MIDISCRIPT__
    m_scriptFileNames.clear();
    m_scriptFunctionPrefixes.clear();
#endif

    // For each controller in the DOM
    m_Bindings = root;
    QDomElement controller = m_Bindings.firstChildElement("controller");

    // For each controller in the MIDI mapping XML...
    //(Only parse the <controller> block if it's id matches our device name, otherwise
    //keep looking at the next controller blocks....)
    QString device;
    while (!controller.isNull()) {
        // Get deviceid
        device = controller.attribute("id","");
        if (device != m_deviceName && !forceLoad) {
            controller = controller.nextSiblingElement("controller");
        }
        else
            break;
    }

    if (!controller.isNull()) {

        qDebug() << device << " settings found";
#ifdef __MIDISCRIPT__
        // Build a list of MIDI script files to load

        QDomElement scriptFile = controller.firstChildElement("scriptfiles").firstChildElement("file");

        // Default currently required file
        addScriptFile(REQUIRED_SCRIPT_FILE,"");

        // Look for additional ones
        while (!scriptFile.isNull()) {
            QString functionPrefix = scriptFile.attribute("functionprefix","");
            QString filename = scriptFile.attribute("filename","");
            addScriptFile(filename, functionPrefix);

            scriptFile = scriptFile.nextSiblingElement("file");
        }

        loadScriptCode();   // Actually load code from the list built above

        QStringList scriptFunctions;
        if (m_pScriptEngine != NULL) {
            scriptFunctions = m_pScriptEngine->getScriptFunctions();
        }

#endif

        QDomElement control = controller.firstChildElement("controls").firstChildElement("control");

        //Iterate through each <control> block in the XML
        while (!control.isNull()) {

            //Unserialize these objects from the XML
            MidiMessage midiMessage(control);
            MixxxControl mixxxControl(control);
#ifdef __MIDISCRIPT__
            // Verify script functions are loaded
            if (mixxxControl.getMidiOption()==MIDI_OPT_SCRIPT &&
                scriptFunctions.indexOf(mixxxControl.getControlObjectValue())==-1) {

                QString status = QString("%1").arg(midiMessage.getMidiStatusByte(), 0, 16).toUpper();
                status = "0x"+status;
                QString byte2 = QString("%1").arg(midiMessage.getMidiNo(), 0, 16).toUpper();
                byte2 = "0x"+byte2;

                // If status is MIDI pitch, the 2nd byte is part of the payload so don't display it
                if (midiMessage.getMidiStatusByte() == 0xE0) byte2 = "";

                QString errorLog = QString("MIDI script function \"%1\" not found. "
                                    "(Mapped to MIDI message %2 %3)")
                                    .arg(mixxxControl.getControlObjectValue())
                                    .arg(status)
                                    .arg(byte2);

                if (m_pOutputMidiDevice != NULL
                    && m_pOutputMidiDevice->midiDebugging()) {
                        qCritical() << errorLog;
                }
                else {
                    qWarning() << errorLog;
                    ErrorDialogProperties* props = ErrorDialogHandler::instance()->newDialogProperties();
                    props->setType(DLG_WARNING);
                    props->setTitle(tr("MIDI script function not found"));
                    props->setText(QString(tr("The MIDI script function '%1' was not "
                                    "found in loaded scripts."))
                                    .arg(mixxxControl.getControlObjectValue()));
                    props->setInfoText(QString(tr("The MIDI message %1 %2 will not be bound."
                                    "\n(Click Show Details for hints.)"))
                                    .arg(status)
                                    .arg(byte2));
                    QString detailsText = QString(tr("* Check to see that the "
                        "function name is spelled correctly in the mapping "
                        "file (.xml) and script file (.js)\n"));
                    detailsText += QString(tr("* Check to see that the script "
                        "file name (.js) is spelled correctly in the mapping "
                        "file (.xml)"));
                    props->setDetails(detailsText);

                    ErrorDialogHandler::instance()->requestErrorDialog(props);
                }
            } else {
#endif
                //Add to the input mapping.
                internalSetInputMidiMapping(midiMessage, mixxxControl, true);
                /*Old code: m_inputMapping.insert(midiMessage, mixxxControl);
                  Reason why this is bad: Don't want to access this directly because the
                  model doesn't get notified about the update */
#ifdef __MIDISCRIPT__
            }
#endif
            control = control.nextSiblingElement("control");
        }

        qDebug() << "MidiMapping: Input parsed!";

        QDomElement output = controller.firstChildElement("outputs").firstChildElement("output");

        //Iterate through each <control> block in the XML
        while (!output.isNull()) {
            //Unserialize these objects from the XML
            MidiMessage midiMessage(output);
            MixxxControl mixxxControl(output, true);

            //Add to the output mapping.
            internalSetOutputMidiMapping(mixxxControl, midiMessage, true);
            /*Old code: m_outputMapping.insert(mixxxControl, midiMessage);
              Reason why this is bad: Don't want to access this directly because the
                                      model doesn't get notified about the update */

            output = output.nextSiblingElement("output");
        }

        qDebug() << "MidiMapping: Output parsed!";
        //controller = controller.nextSiblingElement("controller"); //FIXME: Remove this line of code permanently - Albert
    }

    m_mappingLock.unlock();

}   // END loadPreset(QDomElement)

/* savePreset()
 * Saves the current table of bindings to the default device XML file.
 */
void MidiMapping::savePreset() {
    savePreset(DEFAULT_DEVICE_PRESET);
}

/* savePreset(QString)
 * Given a path, saves the current table of bindings to an XML file.
 */
void MidiMapping::savePreset(QString path) {
    qDebug() << "Writing MIDI preset file" << path;
    QMutexLocker locker(&m_mappingLock);
    QFile output(path);
    if (!output.open(QIODevice::WriteOnly | QIODevice::Truncate)) return;
    QTextStream outputstream(&output);
    // Construct the DOM from the table
    QDomDocument docBindings = buildDomElement();
    // Save the DOM to the XML file
    docBindings.save(outputstream, 4);
    output.close();
}

/* applyPreset()
 * Load the current bindings set into the MIDI handler, and the outputs info into
 * the LED handler.
 */
void MidiMapping::applyPreset() {
    qDebug() << "MidiMapping::applyPreset()";
    QMutexLocker locker(&m_mappingLock);

#ifdef __MIDISCRIPT__
    // Since this can be called after re-enabling a device without reloading the XML preset,
    // the script engine must have its code loaded here as well
    QStringList scriptFunctions;
    if (m_pScriptEngine != NULL) {
        scriptFunctions = m_pScriptEngine->getScriptFunctions();
    }
    if (scriptFunctions.isEmpty()) loadScriptCode();
    
    initializeScripts();
#endif

    if (m_pOutputMidiDevice != NULL) {
        //^^^ Only execute this code if we have an output device hooked up
        //    to this MidiMapping...

        QDomElement controller = m_Bindings.firstChildElement("controller");
        // For each device
        while (!controller.isNull()) {
            // Device Outputs - LEDs
            QString deviceId = controller.attribute("id","");

            qDebug() << "MidiMapping: Processing MIDI Output Bindings for" << deviceId;
            MidiLedHandler::createHandlers(controller.namedItem("outputs").firstChild(),
                                           *m_pOutputMidiDevice);

            // Next device
            controller = controller.nextSiblingElement("controller");
        }
        MidiLedHandler::updateAll();
    }
}

/* clearPreset()
 * Creates a blank bindings preset.
 */
void MidiMapping::clearPreset() {
    // Assumes the lock is held.
    // Create a new blank DomNode
    QString blank = "<MixxxMIDIPreset schemaVersion=\"" + QString(XML_SCHEMA_VERSION) + "\" mixxxVersion=\"" + QString(VERSION) + "+\">\n"
    "</MixxxMIDIPreset>\n";
    QDomDocument doc("Bindings");
    doc.setContent(blank);
    m_Bindings = doc.documentElement();
}

/* buildDomElement()
 * Updates the DOM with what is currently in the table
 */
 QDomDocument MidiMapping::buildDomElement() {
     // We should hold the mapping lock. The lock is recursive so if we already
     // hold it it will relock.
     QMutexLocker locker(&m_mappingLock);

    clearPreset(); // Create blank document

    QDomDocument doc("Bindings");
    QString blank = "<MixxxMIDIPreset schemaVersion=\"" + QString(XML_SCHEMA_VERSION) + "\" mixxxVersion=\"" + QString(VERSION) + "+\">\n"
                    "</MixxxMIDIPreset>\n";

    doc.setContent(blank);

    QDomElement rootNode = doc.documentElement();
    QDomElement controller = doc.createElement("controller");
    controller.setAttribute("id", m_deviceName.right(m_deviceName.size()-m_deviceName.indexOf(" ")-1));
    rootNode.appendChild(controller);

#ifdef __MIDISCRIPT__
    //This sucks, put this code inside MidiScriptEngine instead of here,
    // and just ask MidiScriptEngine to spit it out for us.
//     qDebug() << "MidiMapping: Writing script block!";

    QDomElement scriptFiles = doc.createElement("scriptfiles");
    controller.appendChild(scriptFiles);


    for (int i = 0; i < m_scriptFileNames.count(); i++) {
//         qDebug() << "MidiMapping: writing script block for" << m_scriptFileNames[i];
        QString filename = m_scriptFileNames[i];


        //Don't need to write anything for the required mapping file.
        if (filename != REQUIRED_SCRIPT_FILE) {
            qDebug() << "MidiMapping: writing script block for" << filename;
            QString functionPrefix = m_scriptFunctionPrefixes[i];
            QDomElement scriptFile = doc.createElement("file");


            scriptFile.setAttribute("filename", filename);
            scriptFile.setAttribute("functionprefix", functionPrefix);

            scriptFiles.appendChild(scriptFile);
        }
    }
#endif

    QDomElement controls = doc.createElement("controls");
    controller.appendChild(controls);


    //Iterate over all of the command/control pairs in the input mapping
    QHashIterator<MidiMessage, MixxxControl> it(m_inputMapping);
    while (it.hasNext()) {
        it.next();

        QDomElement controlNode = doc.createElement("control");

        //Save the MidiMessage and MixxxControl objects as XML
        it.key().serializeToXML(controlNode);
        it.value().serializeToXML(controlNode);

        //Add the control node we just created to the XML document in the proper spot
        controls.appendChild(controlNode);
    }


    QDomElement outputs = doc.createElement("outputs");
    controller.appendChild(outputs);

    //Iterate over all of the control/command pairs in the OUTPUT mapping
    QHashIterator<MixxxControl, MidiMessage> outIt(m_outputMapping);
    while (outIt.hasNext()) {
        outIt.next();

        QDomElement outputNode = doc.createElement("output");


        //Save the MidiMessage and MixxxControl objects as XML
        outIt.key().serializeToXML(outputNode, true);
        outIt.value().serializeToXML(outputNode, true);

        //Add the control node we just created to the XML document in the proper spot
        outputs.appendChild(outputNode);
    }

    m_Bindings = doc.documentElement();
    return doc;
}

/* -------- ------------------------------------------------------
   Purpose: Adds an input MIDI mapping block to the XML.
   Input:   QDomElement control, QString device
   Output:  -
   -------- ------------------------------------------------------ */
void MidiMapping::addControl(QDomElement &control, QString device) {
    QDomDocument nodeMaker;
    //Add control to correct device tag - find the correct tag
    QDomElement controller = m_Bindings.firstChildElement("controller");
    while (controller.attribute("id","") != device && !controller.isNull()) {
        controller = controller.nextSiblingElement("controller");
    }
    if (controller.isNull()) {
        // No tag was found - create it
        controller = nodeMaker.createElement("controller");
        controller.setAttribute("id", device);
        m_Bindings.appendChild(controller);
    }
    // Check for controls tag
    QDomElement controls = controller.firstChildElement("controls");
    if (controls.isNull()) {
        controls = nodeMaker.createElement("controls");
        controller.appendChild(controls);
    }
    controls.appendChild(control);
}

/* -------- ------------------------------------------------------
   Purpose: This code sucks, temporary hack
   Input:   QDomElement control, QString device
   Output:  -
   -------- ------------------------------------------------------ */
void MidiMapping::addMidiScriptInfo(QDomElement &scriptFile, QString device) {
    QDomDocument nodeMaker;
    //Add control to correct device tag - find the correct tag
    QDomElement controller = m_Bindings.firstChildElement("controller");
    while (controller.attribute("id","") != device && !controller.isNull()) {
        controller = controller.nextSiblingElement("controller");
    }
    if (controller.isNull()) {
        // No tag was found - create it
        controller = nodeMaker.createElement("controller");
        controller.setAttribute("id", device);
        m_Bindings.appendChild(controller);
    }
    // Check for controls tag
    QDomElement scriptfiles = controller.firstChildElement("scriptfiles");
    if (scriptfiles.isNull()) {
        scriptfiles = nodeMaker.createElement("scriptfiles");
        controller.appendChild(scriptfiles);
    }
    scriptfiles.appendChild(scriptFile);
}

/* -------- ------------------------------------------------------
   Purpose: Adds an output (LED, etc.) MIDI mapping block to the XML.
   Input:   QDomElement output, QString device
   Output:  -
   -------- ------------------------------------------------------ */
void MidiMapping::addOutput(QDomElement &output, QString device) {
    QDomDocument nodeMaker;
    // Find the controller to attach the XML to...
    QDomElement controller = m_Bindings.firstChildElement("controller");
    while (controller.attribute("id","") != device && !controller.isNull()) {
        controller = controller.nextSiblingElement("controller");
    }
    if (controller.isNull()) {
        // No tag was found - create it
        controller = nodeMaker.createElement("controller");
        controller.setAttribute("id", device);
        m_Bindings.appendChild(controller);
    }

    // Find the outputs block
    QDomElement outputs = controller.firstChildElement("outputs");
    if (outputs.isNull()) {
        outputs = nodeMaker.createElement("outputs");
        controller.appendChild(outputs);
    }
    // attach the output to the outputs block
    outputs.appendChild(output);
}

bool MidiMapping::addInputControl(MidiStatusByte midiStatus, int midiNo, int midiChannel,
                                  QString controlObjectGroup, QString controlObjectKey,
                                  QString controlObjectDescription, MidiOption midiOption)
{
    return addInputControl(MidiMessage(midiStatus, midiNo, midiChannel),
                           MixxxControl(controlObjectGroup, controlObjectKey,
                                        controlObjectDescription, midiOption));
}

bool MidiMapping::addInputControl(MidiMessage message, MixxxControl control)
{
    //TODO: Check if mapping already exists for this MidiMessage.

    //Add to the input mapping.
    m_inputMapping.insert(message, control);
    return true; //XXX is this right? should this be returning whether the add happened successfully?

}

void MidiMapping::removeInputMapping(MidiStatusByte midiStatus, int midiNo, int midiChannel)
{
    m_inputMapping.remove(MidiMessage(midiStatus, midiNo, midiChannel));
}

MidiInputMappingTableModel* MidiMapping::getMidiInputMappingTableModel()
{
    return m_pMidiInputMappingTableModel;
}

MidiOutputMappingTableModel* MidiMapping::getMidiOutputMappingTableModel()
{
    return m_pMidiOutputMappingTableModel;
}

//Used by MidiObject to query what control matches a given MIDI command.
/*MixxxControl* MidiMapping::getInputMixxxControl(MidiMessage command)
{
    if (!m_inputMapping.contains(command)) {
        qWarning() << "Unbound MIDI command";
        qDebug() << "Midi Status:" << command.getMidiStatusByte();
        qDebug() << "Midi No:" << command.getMidiNo();
        qDebug() << "Midi Channel:" << command.getMidiChannel();
        return NULL;
    }

    MixxxControl* control = &(m_inputMapping[command]);
    return control;
    }*/

// BJW: Note: _prevmidivalue is not the previous MIDI value. It's the
// current controller value, scaled to 0-127 but only in the case of pots.
// (See Control*::GetMidiValue())

 // static
double MidiMapping::ComputeValue(MidiOption midioption, double _prevmidivalue, double _newmidivalue)
{
    double tempval = 0.;
    double diff = 0.;

    // qDebug() << "ComputeValue: option " << midioption << ", MIDI value " << _newmidivalue << ", current control value " << _prevmidivalue;
    if (midioption == MIDI_OPT_NORMAL) {
        return _newmidivalue;
    }
    else if (midioption == MIDI_OPT_INVERT)
    {
        return 127. - _newmidivalue;
    }
    else if (midioption == MIDI_OPT_ROT64 || midioption == MIDI_OPT_ROT64_INV)
    {
        tempval = _prevmidivalue;
        diff = _newmidivalue - 64.;
        if (diff == -1 || diff == 1)
            diff /= 16;
        else
            diff += (diff > 0 ? -1 : +1);
        if (midioption == MIDI_OPT_ROT64)
            tempval += diff;
        else
            tempval -= diff;
        return (tempval < 0. ? 0. : (tempval > 127. ? 127.0 : tempval));
    }
    else if (midioption == MIDI_OPT_ROT64_FAST)
    {
        tempval = _prevmidivalue;
        diff = _newmidivalue - 64.;
        diff *= 1.5;
        tempval += diff;
        return (tempval < 0. ? 0. : (tempval > 127. ? 127.0 : tempval));
    }
    else if (midioption == MIDI_OPT_DIFF)
    {
        //Interpret 7-bit signed value using two's compliment.
        if (_newmidivalue >= 64.)
            _newmidivalue = _newmidivalue - 128.;
        //Apply sensitivity to signed value. FIXME
       // if(sensitivity > 0)
        //    _newmidivalue = _newmidivalue * ((double)sensitivity / 50.);
        //Apply new value to current value.
        _newmidivalue = _prevmidivalue + _newmidivalue;
    }
    else if (midioption == MIDI_OPT_SELECTKNOB)
    {
        //Interpret 7-bit signed value using two's compliment.
        if (_newmidivalue >= 64.)
            _newmidivalue = _newmidivalue - 128.;
        //Apply sensitivity to signed value. FIXME
        //if(sensitivity > 0)
        //    _newmidivalue = _newmidivalue * ((double)sensitivity / 50.);
        //Since this is a selection knob, we do not want to inherit previous values.
    }
    else if (midioption == MIDI_OPT_BUTTON) { _newmidivalue = (_newmidivalue != 0); }
    else if (midioption == MIDI_OPT_SWITCH) { _newmidivalue = 1; }
    else if (midioption == MIDI_OPT_SPREAD64)
    {

        //qDebug() << "MIDI_OPT_SPREAD64";
        // BJW: Spread64: Distance away from centre point (aka "relative CC")
        // Uses a similar non-linear scaling formula as ControlTTRotary::getValueFromWidget()
        // but with added sensitivity adjustment. This formula is still experimental.

        _newmidivalue = _newmidivalue - 64.;
        //FIXME
        //double distance = _newmidivalue - 64.;
        // _newmidivalue = distance * distance * sensitivity / 50000.;
        //if (distance < 0.)
        //    _newmidivalue = -_newmidivalue;

         //qDebug() << "Spread64: in " << distance << "  out " << _newmidivalue;
    }
    else if (midioption == MIDI_OPT_HERC_JOG)
    {
        if (_newmidivalue > 64.) { _newmidivalue -= 128.; }
        _newmidivalue += _prevmidivalue;
        //if (_prevmidivalue != 0.0) { qDebug() << "AAAAAAAAAAAA" << _prevmidivalue; }
    }
    else
    {
        qWarning("Unknown MIDI option %d", midioption);
    }

    return _newmidivalue;
}

void MidiMapping::finishMidiLearn(MidiMessage message)
{
    bool shouldEmit = false;
    m_mappingLock.lock();
    //We've received a MidiMessage that should be mapped onto some control. When
    //beginMidiLearn() was called, we were given the control that should be
    //mapped, so let's connect the message to the saved control and thus
    //"create" the mapping between the two.
    if (!m_controlToLearn.isNull()) { //Ensure we've actually been told to learn
                                      //a control.  Note the ! out front.
        addInputControl(message, m_controlToLearn);

        //If we caught a NOTE_ON message, add a binding for NOTE_OFF as well.
        if (message.getMidiStatusByte() == MIDI_STATUS_NOTE_ON) {
            MidiMessage noteOffMessage(message);
            noteOffMessage.setMidiStatusByte(MIDI_STATUS_NOTE_OFF);
            addInputControl(noteOffMessage, m_controlToLearn);
        }

        //Reset the saved control.
        m_controlToLearn = MixxxControl();

        qDebug() << "MidiMapping: Learning finished!";

        shouldEmit = true;
    }
    m_mappingLock.unlock();

    if (shouldEmit) {
        //Notify the prefs dialog that we've finished doing a MIDI learn.
        emit(midiLearningFinished(message));
        emit(midiLearningFinished()); //Tells MidiObject to stop feeding us messages.
        emit(inputMappingChanged());
    }
}

void MidiMapping::beginMidiLearn(MixxxControl control)
{
    m_mappingLock.lock();
    //Save the internal control we're supposed to map/remap. After this we have
    //to wait for the user to push a button on their controller, which should
    //give us a MidiMessage via finishMidiLearn().
    m_controlToLearn = control;

    qDebug() << "MidiMapping: Learning started!";

    m_mappingLock.unlock();

    //Notify the MIDI device class that it should feed us the next MIDI message...
    emit(midiLearningStarted());
}

void MidiMapping::cancelMidiLearn()
{
    m_mappingLock.lock();
    m_controlToLearn = MixxxControl();
    m_mappingLock.unlock();
}

#ifdef __MIDISCRIPT__
void MidiMapping::restartScriptEngine()
{
    //Note: Locking occurs inside the functions below.
    shutdownScriptEngine();
    startupScriptEngine();
}
#endif

// Reset the MIDI controller
void MidiMapping::reset() {
#ifdef __MIDISCRIPT__   // Can't ifdef slots in the .h file, so we just do the body.
    restartScriptEngine();
#endif
    MidiLedHandler::destroyHandlers(m_pOutputMidiDevice);

    applyPreset();
}

void MidiMapping::slotScriptEngineReady() {
#ifdef __MIDISCRIPT__	// Can't ifdef slots in the .h file, so we just do the body.

    // The lock prevents us from waking before the main thread is waiting on the
    // condition.
    m_scriptEngineInitializedMutex.lock();
    m_scriptEngineInitializedCondition.wakeAll();
    m_scriptEngineInitializedMutex.unlock();

#endif
}
