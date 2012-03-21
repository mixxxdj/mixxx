/***************************************************************************
                             controller.cpp
                           Controller Class
                           ----------------
    begin                : Sat Apr 30 2011
    copyright            : (C) 2011 Sean M. Pappalardo
    email                : spappalardo@mixxx.org

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
// #include "widget/wwidget.h"    // FIXME: This should be xmlparse.h
#include "xmlparse.h"
#include "controller.h"
#include "defs_controllers.h"

Controller::Controller() : QObject() {
    m_bIsOutputDevice = false;
    m_bIsInputDevice = false;
    m_bIsOpen = false;
    m_pEngine = NULL;
    m_bPolling = false;

    // Get --controllerDebug command line option
    QStringList commandLineArgs = QApplication::arguments();
    m_bDebug = commandLineArgs.contains("--controllerDebug", Qt::CaseInsensitive);
}

Controller::~Controller() {
//     close(); // I wish I could put this here to enforce it automatically
}

QString Controller::presetExtension() {
    return CONTROLLER_PRESET_EXTENSION;
}

void Controller::startEngine()
{
    if (debugging()) qDebug() << "  Starting engine";
    if (m_pEngine != NULL) {
        qWarning() << "Controller: Engine already exists! Restarting:";
        stopEngine();
    }
    m_pEngine = new ControllerEngine(this);
}

void Controller::stopEngine()
{
    if (debugging()) qDebug() << "  Shutting down engine";
    if (m_pEngine == NULL) {
        qWarning() << "Controller::stopEngine(): No engine exists!";
        return;
    }
    m_pEngine->gracefulShutdown();
    delete m_pEngine;
    m_pEngine = NULL;
}

/** loadPreset(bool)
* Overloaded function for convenience, uses the default device path
* @param forceLoad Forces the preset to be loaded, regardless of whether or not the controller id
*        specified within matches the name of this Controller.
*/
void Controller::loadPreset(bool forceLoad) {
    loadPreset(DEFAULT_DEVICE_PRESET, forceLoad);
}

/** loadPreset(QString,bool)
* Overloaded function for convenience
* @param path The path to a controller preset XML file.
* @param forceLoad Forces the preset to be loaded, regardless of whether or not the controller id
*        specified within matches the name of this Controller.
*/
void Controller::loadPreset(QString path, bool forceLoad) {
    qDebug() << "Loading controller preset from" << path;
    loadPreset(XmlParse::openXMLFile(path, "controller"), forceLoad);
}

/** loadPreset(QDomElement,bool)
* Loads a controller preset from a QDomElement structure.
* @param root The root node of the XML document for the preset.
* @param forceLoad Forces the preset to be loaded, regardless of whether or not the controller id
*        specified within matches the name of this Controller.
*/
QDomElement Controller::loadPreset(QDomElement root, bool forceLoad) {

    if (root.isNull()) return root;

    m_scriptFileNames.clear();
    m_scriptFunctionPrefixes.clear();
    
    // For each controller in the DOM
    QDomElement controller = root.firstChildElement("controller");
    
    // For each controller in the preset XML...
    //(Only parse the <controller> block if its id matches our device name, otherwise
    //keep looking at the next controller blocks....)
    QString device;
    while (!controller.isNull()) {
        // Get deviceid
        device = controller.attribute("id","");
        if (device != m_sDeviceName.left(m_sDeviceName.size()-m_sDeviceName.indexOf(" ")-1) && !forceLoad) {
            controller = controller.nextSiblingElement("controller");
        }
        else
            break;
    }
    
    if (!controller.isNull()) {
        
        qDebug() << device << "settings found";
        // Build a list of script files to load
        
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

    }
    return controller;
}   // END loadPreset(QDomElement)

void Controller::applyPreset() {
    qDebug() << "Applying controller preset...";

    // Load the script code into the engine
    if (m_pEngine != NULL) {
        m_pEngine->loadScriptFiles(m_scriptFileNames);
        m_pEngine->initializeScripts(m_scriptFunctionPrefixes);
    }
    else qWarning() << "Controller::applyPreset(): No engine exists!";

    bindScriptFunctions();
}

/** addScriptFile(QString,QString)
* Adds an entry to the list of script file names & associated list of function prefixes
* @param filename Name of the XML file to add
* @param functionprefix Function prefix to add
*/
void Controller::addScriptFile(QString filename, QString functionprefix) {
    m_scriptFileNames.append(filename);
    m_scriptFunctionPrefixes.append(functionprefix);
}

void Controller::savePreset() {
    savePreset(DEFAULT_DEVICE_PRESET);
}

void Controller::savePreset(QString path) {
    qDebug() << "Writing controller preset file" << path;
    
    // Need to do this on Windows
    QDir directory;
    directory.mkpath(path.left(path.lastIndexOf("/")));
    
    QFile output(path);
    if (!output.open(QIODevice::WriteOnly | QIODevice::Truncate)) return;
    QTextStream outputstream(&output);
    // Construct the DOM from the table
    QDomDocument docPreset = buildDomElement();
    // Save the DOM to the XML file
    docPreset.save(outputstream, 4);
    output.close();
}

QDomDocument Controller::buildDomElement() {

    QDomDocument doc("Preset");
    QString blank = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
        "<MixxxControllerPreset schemaVersion=\"" + QString(XML_SCHEMA_VERSION) + "\">\n"
        "</MixxxControllerPreset>\n";

    doc.setContent(blank);

    QDomElement rootNode = doc.documentElement();
    QDomElement controller = doc.createElement("controller");
    // Strip off the serial number
    controller.setAttribute("id", m_sDeviceName.left(m_sDeviceName.size()-m_sDeviceName.indexOf(" ")-1));
    rootNode.appendChild(controller);

    QDomElement scriptFiles = doc.createElement("scriptfiles");
    controller.appendChild(scriptFiles);

    for (int i = 0; i < m_scriptFileNames.count(); i++) {
        QString filename = m_scriptFileNames[i];

        //Don't need to write anything for the required mapping file.
        if (filename != REQUIRED_SCRIPT_FILE) {
            qDebug() << "  writing script block for" << filename;
            QString functionPrefix = m_scriptFunctionPrefixes[i];
            QDomElement scriptFile = doc.createElement("file");

            scriptFile.setAttribute("filename", filename);
            scriptFile.setAttribute("functionprefix", functionPrefix);

            scriptFiles.appendChild(scriptFile);
        }
    }
    return doc;
}

void Controller::send(QList<int> data, unsigned int length) {

    // If you change this implementation, also change it in HidController
    //  (That function is required due to HID devices having report IDs)
    
    unsigned char * msg;
    msg = new unsigned char [length];

    for (unsigned int i=0; i<length; i++) {
        msg[i] = data.at(i);
    }

    send(msg,length);
    delete[] msg;
}

void Controller::sendBa(QByteArray data, unsigned int length) {
    
    // If you change this implementation, also change it in HidController
    //  (That function is required due to HID devices having report IDs)
    
    unsigned char* msg = reinterpret_cast<unsigned char*>(data.data());
    send(msg,length);
}

void Controller::send(unsigned char data[], unsigned int length) {
    Q_UNUSED(data);
    Q_UNUSED(length);
    qWarning() << "Error: data sending not yet implemented for this API or platform!";
}

void Controller::receivePointer(unsigned char* data, unsigned int length) {
    receive(data,length);
    // Deleted here even though created in controllers' read threads
    delete[] data;
}

void Controller::receive(const unsigned char data[], unsigned int length) {

    if (m_pEngine == NULL) {
//         qWarning() << "Controller::receive called with no active engine!";
        // Don't complain, since this will always show after closing a device as
        //  queued signals flush out
        return;
    }

    if (debugging()) {
        // Formatted packet display
        QString message = QString("%1: %2 bytes:\n").arg(m_sDeviceName).arg(length);
        for(uint i=0; i<length; i++) {
            QString spacer=" ";
            if ((i+1) % 4 == 0) spacer="  ";
            if ((i+1) % 16 == 0) spacer="\n";
            message += QString("%1%2")
                        .arg(data[i], 2, 16, QChar('0')).toUpper()
                        .arg(spacer);
        }
        qDebug()<< message;
    }

    QListIterator<QString> prefixIt(m_pEngine->getScriptFunctionPrefixes());
    while (prefixIt.hasNext()) {
        QString function = prefixIt.next();
        if (function!="") {
            function.append(".incomingData");

            QScriptValue incomingData = m_scriptBindings.value(function);
            if (!m_pEngine->execute(incomingData, data, length)) {
                qWarning() << "Controller: Invalid script function" << function;
            }
        }
    }
    return;
}

void Controller::bindScriptFunctions() {
    if (m_pEngine == NULL) {
	// qWarning() << "Controller::receive called with no active engine!";
        // Don't complain, since this will always show after closing a device as
        //  queued signals flush out
        return;
    }

    QListIterator<QString> prefixIt(m_pEngine->getScriptFunctionPrefixes());
    while (prefixIt.hasNext()) {
        QString function = prefixIt.next();
        if (function!="") {
            function.append(".incomingData");

            QScriptValue incomingData = m_pEngine->resolveFunction(function);
            if (!incomingData.isValid() || !incomingData.isFunction()) {
            	qWarning() << "Controller: unable to resolve function:" << function;
            	continue;
            }
            m_scriptBindings.insert(function, incomingData);
        }
    }
}

void Controller::timerEvent(QTimerEvent *event, bool poll) {
    if (poll) return; // Sub-classes use the poll flag

    // Pass it on to the engine
    m_pEngine->timerEvent(event);
}
