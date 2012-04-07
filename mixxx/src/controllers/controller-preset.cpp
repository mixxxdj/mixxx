/**
* @file controller-preset.cpp
* @author Sean Pappalardo spappalardo@mixxx.org
* @date Sat 7 Apr 2012
* @brief Controller class' XML preset handling functions, separated for clarity
*
*/

#include "xmlparse.h"

QString Controller::defaultPreset() {
    return PRESETS_PATH.append(m_sDeviceName.replace(" ", "_") + CONTROLLER_PRESET_EXTENSION);
}

QString Controller::presetExtension() {
    return CONTROLLER_PRESET_EXTENSION;
}

/** loadPreset(bool)
* Overloaded function for convenience, uses the default device path
* @param forceLoad Forces the preset to be loaded, regardless of whether or not the controller id
*        specified within matches the name of this Controller.
*/
void Controller::loadPreset(bool forceLoad) {
    loadPreset(defaultPreset(), forceLoad);
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
        if (device != rootDeviceName(m_sDeviceName) && !forceLoad) {
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


void Controller::savePreset() {
    savePreset(defaultPreset());
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
    QDomDocument docPreset = buildXML();
    // Save the DOM to the XML file
    docPreset.save(outputstream, 4);
    output.close();
}

QDomDocument Controller::buildXML() {
    QDomDocument doc("Preset");
    QString blank = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
        "<MixxxControllerPreset schemaVersion=\"" + QString(XML_SCHEMA_VERSION) + "\">\n"
        "</MixxxControllerPreset>\n";

    doc.setContent(blank);

    QDomElement rootNode = doc.documentElement();
    QDomElement controller = doc.createElement("controller");
    // Strip off the serial number
    controller.setAttribute("id", rootDeviceName(m_sDeviceName));
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
