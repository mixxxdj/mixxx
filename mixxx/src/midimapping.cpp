/***************************************************************************
                             midimapping.cpp - "Wow, I wrote more new code!"
                           MIDI Mapping Class
                           -------------------
    begin                : Sat Jan 17 2009
    copyright            : (C) 2009 Sean M. Pappalardo
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
#include "widget/wwidget.h"    // FIXME: This should be xmlparse.h
#include "midimapping.h"
#include "midiledhandler.h"
#include "configobject.h"
#include <qapplication.h>

QMutex MidiMapping::m_rowMutex;
QMutex MidiMapping::m_outputRowMutex;

static QString toHex(QString numberStr) {
    return "0x" + QString("0" + QString::number(numberStr.toUShort(), 16).toUpper()).right(2);
}

MidiMapping::MidiMapping(MidiObject& midi_object) : QObject(), m_rMidiObject(midi_object) {
//     m_pMidiObject = midi_object;
    m_pScriptEngine = midi_object.getMidiScriptEngine();
    
    // Try to read in the current XML bindings file, one from the command line, or create one if nothing is available
    QStringList commandLineArgs = QApplication::arguments();
    int loadXML = commandLineArgs.indexOf("--loadXMLfile");
    if (loadXML!=-1) {
        qDebug() << "MidiMapping: Loading custom MIDI mapping file:" << commandLineArgs.at(loadXML+1);
        loadPreset(commandLineArgs.at(loadXML+1));
    }
    else loadPreset(BINDINGS_PATH);
    applyPreset();
}

MidiMapping::~MidiMapping() {
}

#ifdef __SCRIPT__
/* -------- ------------------------------------------------------
   Purpose: Adds an entry to the list of script file names
            & associated list of function prefixes
   Input:   QString file name, QString function prefix
   Output:  -
   -------- ------------------------------------------------------ */
void MidiMapping::addScriptFile(QString filename, QString functionprefix) {
   m_pScriptFileNames.append(filename);
   m_pScriptFunctionPrefixes.append(functionprefix);
}
#endif

/* loadPreset(QString)
 * Overloaded function for convenience
 */
void MidiMapping::loadPreset(QString path) {
    loadPreset(WWidget::openXMLFile(path, "controller"));
}

/* loadPreset(QDomElement)
 * Loads a set of MIDI bindings from a QDomElement structure.
 */
void MidiMapping::loadPreset(QDomElement root) {
    qDebug() << QString("MidiMapping: loadPreset() called in thread ID=%1").arg(this->thread()->currentThreadId(),0,16);
    
//     m_rowMutex.lock();
//     m_outputRowMutex.lock();
    QMutexLocker lockRows(&m_rowMutex);
    QMutexLocker lockOutputRows(&m_outputRowMutex);

    if (root.isNull()) return;
    // For each controller in the DOM
    m_pBindings = root;
    QDomElement controller = m_pBindings.firstChildElement("controller");
    while (!controller.isNull()) {
        // For each controller
        // Get deviceid
        QString device = controller.attribute("id","");
        qDebug() << device << " settings found" << endl;
        
#ifdef __SCRIPT__
        // Get a list of MIDI script files to load
        QDomElement scriptFile = controller.firstChildElement("scriptfiles").firstChildElement("file");
        
        // Default currently required file
        addScriptFile("midi-mappings-scripts.js","");
        
        // Look for additional ones
        while (!scriptFile.isNull()) {
        
            QString functionPrefix = scriptFile.attribute("functionprefix","");
            QString filename = WWidget::selectNodeQString(scriptFile, "filename");
            addScriptFile(filename, functionPrefix);
        
            scriptFile = scriptFile.nextSiblingElement("file");
        }

        // Load Script files
        ConfigObject<ConfigValue> *m_pConfig = new ConfigObject<ConfigValue>(QDir::homePath().append("/").append(SETTINGS_FILE));
        
        // Individually load & evaluate script files in the QList to check for errors
        // so line numbers will be accurate per file (for syntax errors at least) to make troubleshooting much easier
        bool scriptError = false;
        
        for (int i=0; i<m_pScriptFileNames.size(); i++) {
            m_pScriptEngine->clearCode();   // So line numbers will be correct
            
            QString filename = m_pScriptFileNames.at(i);
            qDebug() << "MidiMapping: Loading & testing MIDI script" << filename;
            m_pScriptEngine->loadScript(m_pConfig->getConfigPath().append("midi/").append(filename));
            
            m_pScriptEngine->evaluateScript();
            if (!m_pScriptEngine->checkException() && m_pScriptEngine->isGood()) qDebug() << "MidiMapping: Success";
            else {
                // This is only included for completeness since checkException should pop a qCritical() itself if there's a problem
                qCritical() << "MidiMapping: Failure evaluating MIDI script" << filename;
                scriptError = true;
            }
        }
        
        qDebug() << "MidiMapping: Loading & evaluating all MIDI script code";
        m_pScriptEngine->clearCode();   // Start from scratch
        
        if (!scriptError) {
            while (!m_pScriptFileNames.isEmpty()) {
                m_pScriptEngine->loadScript(m_pConfig->getConfigPath().append("midi/").append(m_pScriptFileNames.takeFirst()));
            }
            
            m_pScriptEngine->evaluateScript();
            if (!m_pScriptEngine->checkException() && m_pScriptEngine->isGood()) qDebug() << "MidiMapping: Script code evaluated successfully";
        
            // Call each script's init function if it exists
            while (!m_pScriptFunctionPrefixes.isEmpty()) {
                QString initName = m_pScriptFunctionPrefixes.takeFirst();
                if (initName!="") {
                    initName.append(".init");
                    qDebug() << "MidiMapping: Executing" << initName;
    //                 QScriptValue scriptFunction = m_pScriptEngine->execute(initName);
                    if (!m_pScriptEngine->execute(initName)) qWarning() << "MidiMapping: No" << initName << "function in script";
    //                 if (!scriptFunction.isFunction()) qWarning() << "MidiMapping: No" << initName << "function in script";
    //                 else {
    //                     scriptFunction.call(QScriptValue());
    //                     m_pScriptEngine->checkException();
    //                 }
                }
            }
        }

        bool scriptGood = m_pScriptEngine->isGood();
        QStringList scriptFunctions;
        if (scriptGood) scriptFunctions = m_pScriptEngine->getFunctionList();
#endif

        QDomElement control = controller.firstChildElement("controls").firstChildElement("control");
        
        while (!control.isNull()) {
            // For each control
            QString group = WWidget::selectNodeQString(control, "group");
            QString key = WWidget::selectNodeQString(control, "key");
            QString controltype = WWidget::selectNodeQString(control, "controltype");
            QString miditype = WWidget::selectNodeQString(control, "miditype");
            // We convert to midino and midichan to base 10 because that his how they will be matched to midi keys internally.
            bool ok = false;
            QString midino = QString::number(WWidget::selectNodeQString(control, "midino").toUShort(&ok, 0), 10);
            QString midichan = !ok ? "" : QString::number(WWidget::selectNodeQString(control, "midichan").toUShort(&ok, 0), 10);
            if (miditype.trimmed().length() == 0 || !ok) { // Blank all values, if they one is invalid
                qDebug() << "MidiMapping: One or more of miditype, midino, or midichan elements were omitted. The MIDI control has been cleared, you'll have to reteach it.";
                miditype = "";
                midino = "";
                midichan = "";
            }
            QDomElement optionsNode = control.firstChildElement("options");
            // At the moment, use one element, in future iterate through options
            QString option;
            if (optionsNode.hasChildNodes()) {
                option = optionsNode.firstChild().nodeName();
            } else {
                option = "Normal";
            }
            
            // Assemble a QList of QHashes here for dlgprefmidibindings to pick up when it's ready
            QHash<QString, QString> parameters;
            
            parameters["device"]=device;
            parameters["group"]=group;
            parameters["key"]=key;
            parameters["controltype"]=controltype;
            parameters["option"]=option;
            
#ifdef __SCRIPT__
            // Verify script functions are loaded
            if (scriptGood && option.toLower()=="script-binding" && scriptFunctions.indexOf(key)==-1) {
                // Drop the midi binding bit...
                parameters["miditype"]="";
                parameters["midino"]="";
                parameters["midichan"]="";
            } else {
#endif
            parameters["miditype"]=miditype;
            parameters["midino"]=midino;
            parameters["midichan"]=midichan;
#ifdef __SCRIPT__
            }
#endif
            m_addRowParams << parameters;  // Add to the master list
            control = control.nextSiblingElement("control");
        }
        qDebug() << "MidiMapping: Rows ready!";
//         m_rowsReady.wakeAll();
        m_rowMutex.unlock();

        QDomNode output = controller.namedItem("outputs").toElement().firstChild();
        while (!output.isNull()) {
            QString outputType = output.nodeName();
            QString group = WWidget::selectNodeQString(output, "group");
            QString key = WWidget::selectNodeQString(output, "key");

            QString status = QString::number(WWidget::selectNodeInt(output, "status"));
            QString midino = QString::number(WWidget::selectNodeInt(output, "midino"));

            QString on = "0x7F";    // Compatible with Hercules and others
            QString off = "0x00";
            QString min = "";
            QString max = "";

            if(outputType == "light") {
                if (!output.firstChildElement("on").isNull()) {
                    on = WWidget::selectNodeQString(output, "on");
                }
                if (!output.firstChildElement("off").isNull()) {
                    off = WWidget::selectNodeQString(output, "off");
                }
                if (!output.firstChildElement("threshold").isNull()) {
                    min = WWidget::selectNodeQString(output, "threshold");
                }
                if (!output.firstChildElement("minimum").isNull()) {
                    min = WWidget::selectNodeQString(output, "minimum");
                }
                if (!output.firstChildElement("maximum").isNull()) {
                    max = WWidget::selectNodeQString(output, "maximum");
                }
            }
            if (outputType!="#comment") qDebug() << "Loaded Output type:" << outputType << " -> " << group << key << "between"<< min << "and" << max << "to midi out:" << status << midino << "on" << device << "on/off:" << on << off;

            // Assemble a QList of QHashes here for dlgprefmidibindings to pick up when it's ready
            QHash<QString, QString> parameters;
            
            parameters["device"]=device;
            parameters["group"]=group;
            parameters["key"]=key;
            parameters["outputType"]=outputType;
            parameters["status"]=toHex(status);
            parameters["midino"]=toHex(midino);
            parameters["min"]=min;
            parameters["max"]=max;
            parameters["on"]=on;
            parameters["off"]=off;
            
            if (parameters["outputType"]!="#comment") m_addOutputRowParams << parameters;
//             addOutputRow(outputType, group, key, min, max, toHex(status), toHex(midino), device, on, off);

            output = output.nextSibling();
        }
        qDebug() << "MidiMapping: Output rows ready!";
//         m_outputRowsReady.wakeAll();
//         m_outputRowMutex.unlock();

        controller = controller.nextSiblingElement("controller");
    }
}   // END loadPreset(QDomElement)

/* -------- ------------------------------------------------------
   Purpose: Returns a reference to the QList of parameters for
            dlgprefmidibinding's addRow function.
   Input:   -
   Output:  Reference to QList of QHashes, each hash containing
            a parameter by name
   -------- ------------------------------------------------------ */
QList<QHash<QString,QString> > * MidiMapping::getRowParams() {

//     qDebug() << QString("MidiMapping: getRowParams() called in thread ID=%1").arg(this->thread()->currentThreadId(),0,16);
    m_rowMutex.lock();  // Wait until we're done building the QList
//     m_rowsReady.wait(&m_rowMutex);
    m_rowMutex.unlock();
    qDebug() << "MidiMapping: Getting rowParams";

    return &m_addRowParams;
}

/* -------- ------------------------------------------------------
   Purpose: Returns a reference to the QList of parameters for
            dlgprefmidibinding's addOutputRow function.
   Input:   -
   Output:  Reference to QList of QHashes, each hash containing
            a parameter by name
   -------- ------------------------------------------------------ */
QList<QHash<QString,QString> > * MidiMapping::getOutputRowParams() {
//     qDebug() << QString("MidiMapping: getOutputRowParams() called in thread ID=%1").arg(this->thread()->currentThreadId(),0,16);
    m_outputRowMutex.lock();  // Wait until we're done building the QList
//     m_outputRowsReady.wait(&m_outputRowMutex);
    m_outputRowMutex.unlock();
    qDebug() << "MidiMapping: Getting outputRowParams";

    return &m_addOutputRowParams;
}

/* -------- ------------------------------------------------------
   Purpose: Frees the memory used by the row parameters QLists
            when dlgprefmidibindings is done with them.
   Input:   -
   Output:  -
   -------- ------------------------------------------------------ */
void MidiMapping::deleteRowParams() {
// TODO: need to delete lists elements
//    delete m_addRowParams; 
//    delete m_addOutputRowParams;
}

/* savePreset(QString)
 * Given a path, saves the current table of bindings to an XML file.
 */
void MidiMapping::savePreset(QString path) {
    QFile output(path);
    if (!output.open(QIODevice::WriteOnly | QIODevice::Truncate)) return;
    QTextStream outputstream(&output);
    // Construct the DOM from the table
//    buildDomElement();    // TODO: Needs to be migrated from dlgprefmidibindings too.
    // Save the DOM to the XML file
    m_pBindings.save(outputstream, 4);
    output.close();
}

/* applyPreset()
 * Load the current bindings set into the MIDI handler, and the outputs info into
 * the LED handler.
 */
void MidiMapping::applyPreset() {
    MidiLedHandler::destroyHandlers();

    QDomElement controller = m_pBindings.firstChildElement("controller");
    // For each device
    ConfigObject<ConfigValueMidi> * MidiConfig;
    while (!controller.isNull()) {
        // Device Outputs - LEDs
        QString deviceId = controller.attribute("id","");

        qDebug() << "MidiMapping: Processing MIDI Control Bindings for" << deviceId;
        MidiConfig = new ConfigObject<ConfigValueMidi>(controller.namedItem("controls"));

        qDebug() << "MidiMapping: Processing MIDI Output Bindings for" << deviceId;
        MidiLedHandler::createHandlers(controller.namedItem("outputs").firstChild(), m_rMidiObject, deviceId);

        // Next device
        controller = controller.nextSiblingElement("controller");
    }
    // TODO: replace with multiple controller supported call and move into above loop
    m_rMidiObject.setMidiConfig(MidiConfig);
}

/* clearPreset()
 * Creates a blank bindings preset.
 */
void MidiMapping::clearPreset() {
    // Create a new blank DomNode
    QString blank = "<MixxxMIDIPreset version=\"" + QString(VERSION) + "\">\n"
    "</MixxxMIDIPreset>\n";
    QDomDocument doc("Bindings");
    doc.setContent(blank);
    m_pBindings = doc.documentElement();
}

/* -------- ------------------------------------------------------
   Purpose: Adds an input MIDI mapping block to the XML.
   Input:   QDomElement control, QString device
   Output:  -
   -------- ------------------------------------------------------ */
void MidiMapping::addControl(QDomElement &control, QString device) {
    QDomDocument nodeMaker;
    //Add control to correct device tag - find the correct tag
    QDomElement controller = m_pBindings.firstChildElement("controller");
    while (controller.attribute("id","") != device && !controller.isNull()) {
        controller = controller.nextSiblingElement("controller");
    }
    if (controller.isNull()) {
        // No tag was found - create it
        controller = nodeMaker.createElement("controller");
        controller.setAttribute("id", device);
        m_pBindings.appendChild(controller);
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
   Purpose: Adds an output (LED, etc.) MIDI mapping block to the XML.
   Input:   QDomElement output, QString device
   Output:  -
   -------- ------------------------------------------------------ */
void MidiMapping::addOutput(QDomElement &output, QString device) {
    QDomDocument nodeMaker;
    // Find the controller to attach the XML to...
    QDomElement controller = m_pBindings.firstChildElement("controller");
    while (controller.attribute("id","") != device && !controller.isNull()) {
        controller = controller.nextSiblingElement("controller");
    }
    if (controller.isNull()) {
        // No tag was found - create it
        controller = nodeMaker.createElement("controller");
        controller.setAttribute("id", device);
        m_pBindings.appendChild(controller);
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
