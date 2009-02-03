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
#include "midicommand.h"
#include "midiinputmappingtablemodel.h"
#include "midimapping.h"
#include "midiledhandler.h"
#include "configobject.h"

#define REQUIRED_MAPPING_FILE "midi-mappings-scripts.js"

QMutex MidiMapping::m_rowMutex;
QMutex MidiMapping::m_outputRowMutex;

static QString toHex(QString numberStr) {
    return "0x" + QString("0" + QString::number(numberStr.toUShort(), 16).toUpper()).right(2);
}

MidiMapping::MidiMapping(MidiObject& midi_object) : QObject(), m_rMidiObject(midi_object) {
//     m_pMidiObject = midi_object;
    m_pScriptEngine = midi_object.getMidiScriptEngine();

    m_pMidiInputMappingTableModel = new MidiInputMappingTableModel(&m_inputMapping);
}

MidiMapping::~MidiMapping() {
}

#ifdef __MIDISCRIPT__
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

/* loadInitialPreset()
 * Loads a set of MIDI bindings from either the default file or one specified on the command line.
 */
void MidiMapping::loadInitialPreset() {
    // Try to read in the current XML bindings file, one from the command line, or create one if nothing is available
    QStringList commandLineArgs = QApplication::arguments();
    int loadXML = commandLineArgs.indexOf("--loadXMLfile");

//     QMutexLocker lockRows(&m_rowMutex);
//     QMutexLocker lockOutputRows(&m_outputRowMutex);

    if (loadXML!=-1) {
        qDebug() << "MidiMapping: Loading custom MIDI mapping file:" << commandLineArgs.at(loadXML+1);
        loadPreset(commandLineArgs.at(loadXML+1));
    }
    else loadPreset(BINDINGS_PATH);
    applyPreset();
}

/* loadPreset(QString)
 * Overloaded function for convenience
 */
void MidiMapping::loadPreset(QString path) {
    qDebug() << "MidiMapping: Loading MIDI XML from" << path;
    loadPreset(WWidget::openXMLFile(path, "controller"));
}

/* loadPreset(QDomElement)
 * Loads a set of MIDI bindings from a QDomElement structure.
 */
void MidiMapping::loadPreset(QDomElement root) {
    //qDebug() << QString("MidiMapping: loadPreset() called in thread ID=%1").arg(this->thread()->currentThreadId(),0,16);

    QMutexLocker lockRows(&m_rowMutex);
    QMutexLocker lockOutputRows(&m_outputRowMutex);

    if (root.isNull()) return;

    // For each controller in the DOM
    m_Bindings = root;
    QDomElement controller = m_Bindings.firstChildElement("controller");
    while (!controller.isNull()) {
        // For each controller
        // Get deviceid
        QString device = controller.attribute("id","");
        qDebug() << device << " settings found" << endl;

#ifdef __MIDISCRIPT__

        // Get a list of MIDI script files to load
        QDomElement scriptFile = controller.firstChildElement("scriptfiles").firstChildElement("file");

        // Default currently required file
        addScriptFile(REQUIRED_MAPPING_FILE,"");

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
            QListIterator<QString> it(m_pScriptFileNames);
            while (it.hasNext()) {
                QString curScriptFileName = it.next();
                m_pScriptEngine->loadScript(m_pConfig->getConfigPath().append("midi/").append(curScriptFileName));
            }

            m_pScriptEngine->evaluateScript();
            if (!m_pScriptEngine->checkException() && m_pScriptEngine->isGood()) qDebug() << "MidiMapping: Script code evaluated successfully";

            // Call each script's init function if it exists
            QListIterator<QString> prefixIt(m_pScriptFunctionPrefixes);
            while (prefixIt.hasNext()) {
                QString initName = prefixIt.next();
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

        //Itearate through each <control> block in the XML
        while (!control.isNull()) {

            //Unserialize these objects from the XML
            MidiCommand midiCommand(control);
            MidiControl midiControl(control);

            //Add to the input mapping.
            m_inputMapping.insert(midiCommand, midiControl);
            control = control.nextSiblingElement("control");
        }

           qDebug() << "MidiMapping: Input parsed!";
//         m_rowsReady.wakeAll();
        m_rowMutex.unlock();

		//
		//
		//
		//   Everything below this needs to get rewritten.
		//
		//
		//

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
MidiInputMapping* MidiMapping::getInputMapping() {

//     qDebug() << QString("MidiMapping: getRowParams() called in thread ID=%1").arg(this->thread()->currentThreadId(),0,16);
    m_rowMutex.lock();  // Wait until we're done building the QList
//     m_rowsReady.wait(&m_rowMutex);
    m_rowMutex.unlock();
    qDebug() << "MidiMapping: Getting rowParams";

    return &m_inputMapping;
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
   buildDomElement();
    // Save the DOM to the XML file
    m_Bindings.save(outputstream, 4);
    output.close();
}

/* applyPreset()
 * Load the current bindings set into the MIDI handler, and the outputs info into
 * the LED handler.
 */
void MidiMapping::applyPreset() {
    QMutexLocker lockRows(&m_rowMutex);
    QMutexLocker lockOutputRows(&m_outputRowMutex);

    MidiLedHandler::destroyHandlers();

    QDomElement controller = m_Bindings.firstChildElement("controller");
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
    m_Bindings = doc.documentElement();
}

/* buildDomElement()
 * Updates the DOM with what is currently in the table
 */
 void MidiMapping::buildDomElement() {
     clearPreset(); // Create blank document

     const QString wtfbbqdevicename = "foobar";
#ifdef __MIDISCRIPT__
      //This sucks, put this code inside MidiScriptEngine instead of here,
      // and just ask MidiScriptEngine to spit it out for us.
     qDebug() << "Writing script block!";
     for (int i = 0; i < m_pScriptFileNames.count(); i++) {
         qDebug() << "writing script block for" << m_pScriptFileNames[i];
          QString filename = m_pScriptFileNames[i];
          if (filename != REQUIRED_MAPPING_FILE) { //Don't need to write anything for the required mapping file.
              QString functionPrefix = m_pScriptFunctionPrefixes[i];
              //and now for the worst XML code since... WWidget...
              QDomDocument sucksBalls;
              QDomElement scriptFile = sucksBalls.createElement("file");
              scriptFile.setAttribute("functionprefix", functionPrefix);
              QDomElement scriptFileName = sucksBalls.createElement("filename");
              QDomText scriptFileNameText = sucksBalls.createTextNode(filename);
              scriptFileName.appendChild(scriptFileNameText);
              scriptFile.appendChild(scriptFileName);

              //Add the XML dom element to the right spot in the XML document.
              addMidiScriptInfo(scriptFile, wtfbbqdevicename);
          }
     }
#endif

    //Iterate over all of the command/control pairs in the input mapping
     QMapIterator<MidiCommand, MidiControl> it(m_inputMapping);
     while (it.hasNext()) {
         it.next();
         QDomElement controlNode;
         QDomDocument nodeMaker;

         //Create <control> block
         controlNode = nodeMaker.createElement("control");

         //Save the MidiCommand and MidiControl objects as XML
         it.key().serializeToXML(controlNode);
         it.value().serializeToXML(controlNode);

          //Add the control node we just created to the XML document in the proper spot
         addControl(controlNode, wtfbbqdevicename); //FIXME: Remove this device shit until we have multiple device support.
     }
/*
    //TODO: Rewrite this code when we reimplement output mapping stuff.

     for (int y = 0; y < m_pTblOutputBindings.rowCount(); y++) {
         // For each row
         QString outputType = ((QComboBox*)m_pTblOutputBindings.cellWidget(y,0))->currentText().trimmed();
         QString device = m_pTblOutputBindings.item(y,3)->text().trimmed();

         QHash<QString, QString> outputMapping;
         QString controlKey = ((QComboBox*)m_pTblOutputBindings.cellWidget(y,1))->currentText().trimmed();
         if (controlKey.isEmpty()
             || controlKey.trimmed().split(' ').count() < 2
             || controlKey.indexOf("[") == -1
             || controlKey.indexOf("]") == -1
             || m_pTblOutputBindings.item(y,2)->text().trimmed().split(' ').count() < 2) {
             qDebug() << "MIDI Output Row"<<y+1<<"was dropped during save because it contains invalid values.";
             continue; // Invalid mapping, skip it.
         }

         outputMapping["group"] = controlKey.trimmed().split(' ').at(0).trimmed();
         outputMapping["key"] = controlKey.trimmed().split(' ').at(1).trimmed();
         outputMapping["status"] = m_pTblOutputBindings.item(y,2)->text().trimmed().split(' ').at(0).trimmed();
         outputMapping["midino"] = m_pTblOutputBindings.item(y,2)->text().trimmed().split(' ').at(1).trimmed();
         //        outputMapping["device"] = m_pTblOutputBindings.item(y,3)->text().trimmed();
         outputMapping["minimum"] = m_pTblOutputBindings.item(y,4)->text().trimmed();
         outputMapping["maximum"] = m_pTblOutputBindings.item(y,5)->text().trimmed();
         outputMapping["on"] = m_pTblOutputBindings.item(y,6)->text().trimmed();
         outputMapping["off"] = m_pTblOutputBindings.item(y,7)->text().trimmed();

         // Clean up any optional values
         if (outputMapping["maximum"].isEmpty()) {
             if (!outputMapping["minimum"].isEmpty()) {
                 outputMapping["threshold"] = outputMapping["minimum"];
             }
             outputMapping.remove("minimum");
             outputMapping.remove("maximum");
         }
         if (outputMapping["on"].isEmpty() || outputMapping["off"].isEmpty()) {
             outputMapping.remove("on");
             outputMapping.remove("off");
         }

         // Generate output XML
         QDomText text;
         QDomDocument nodeMaker;
         QDomElement output = nodeMaker.createElement(outputType);

         // TODO: make these output in a more human friendly order
         foreach (QString tagName, outputMapping.keys()) {
             QDomElement tagNode = nodeMaker.createElement(tagName);
             text = nodeMaker.createTextNode(outputMapping.value(tagName));
             tagNode.appendChild(text);
             output.appendChild(tagNode);
         }

         addOutput(output, device);
     }
 */

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

bool MidiMapping::addInputControl(MidiType midiType, int midiNo, int midiChannel,
                                  QString controlObjectGroup, QString controlObjectKey,
                                  MidiOption midiOption)
{
    //TODO: Check if mapping already exists for this MidiCommand.

    //Add to the input mapping.
    m_inputMapping.insert(MidiCommand(midiType, midiNo, midiChannel),
                          MidiControl(controlObjectGroup, controlObjectKey,
                                       midiOption));
}

void MidiMapping::removeInputMapping(MidiType midiType, int midiNo, int midiChannel)
{
    m_inputMapping.remove(MidiCommand(midiType, midiNo, midiChannel));
}

MidiInputMappingTableModel* MidiMapping::getMidiInputMappingTableModel()
{
    return m_pMidiInputMappingTableModel;
}

//Used by MidiObject to query what control matches a given MIDI command.
MidiControl* MidiMapping::getInputMidiControl(MidiCommand command)
{
    if (!m_inputMapping.contains(command)) {
        qDebug() << "Warning: unbound MIDI command";
        qDebug() << "Midi Type:" << command.getMidiType();
        qDebug() << "Midi No:" << command.getMidiNo();
        qDebug() << "Midi Channel:" << command.getMidiChannel();
        return NULL;
    }

    MidiControl* control = &(m_inputMapping[command]);
    return control;
}

// BJW: Note: _prevmidivalue is not the previous MIDI value. It's the
// current controller value, scaled to 0-127 but only in the case of pots.
// (See Control*::GetMidiValue())
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
        return _newmidivalue;
    }
    else if (midioption == MIDI_OPT_BUTTON)
    {
        if (_newmidivalue != 0.) {
            _newmidivalue = !_prevmidivalue;
        } else {
            _newmidivalue = _prevmidivalue;
        }
    }
    else if (midioption == MIDI_OPT_SWITCH)
    {
        _newmidivalue = (_newmidivalue != 0);
    }
    else if (midioption == MIDI_OPT_SPREAD64)
    {
        // BJW: Spread64: Distance away from centre point (aka "relative CC")
        // Uses a similar non-linear scaling formula as ControlTTRotary::getValueFromWidget()
        // but with added sensitivity adjustment. This formula is still experimental.
        /*
         //FIXME
        double distance = _newmidivalue - 64.;
        _newmidivalue = distance * distance * sensitivity / 50000.;
        if (distance < 0.)
            _newmidivalue = -_newmidivalue;
        */

        // qDebug() << "Spread64: in " << distance << "  out " << _newmidivalue;
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
