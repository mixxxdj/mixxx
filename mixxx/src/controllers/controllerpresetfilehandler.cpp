/**
* @file controllerpresetfilehandler.cpp
* @author Sean Pappalardo spappalardo@mixxx.org
* @date Mon 9 Apr 2012
* @brief Handles loading and saving of Controller presets.
*
*/

#include "controllerpresetfilehandler.h"
#include "defs_controllers.h"

/** load(QString,QString,bool)
* Overloaded function for convenience
* @param path The path to a controller preset XML file.
* @param deviceName The name/id of the controller
* @param forceLoad Forces the preset to be loaded, regardless of whether or not the controller id
*        specified within matches the name of this Controller.
*/
ControllerPreset ControllerPresetFileHandler::load(const QString path,
                                                   const QString deviceName,
                                                   const bool forceLoad) {
    qDebug() << "Loading controller preset from" << path;
    return load(XmlParse::openXMLFile(path, "controller"), deviceName, forceLoad);
}

/** load(QDomElement,QString,bool)
* Loads a controller preset from a QDomElement structure.
* @param root The root node of the XML document for the preset.
* @param deviceName The name/id of the controller
* @param forceLoad Forces the preset to be loaded, regardless of whether or not the controller id
*        specified within matches the name of this Controller.
*/
ControllerPreset ControllerPresetFileHandler::load(const QDomElement root,
                                                   const QString deviceName,
                                                   const bool forceLoad)
{
    ControllerPreset preset;

    if (root.isNull()) return preset;

    // For each controller in the DOM
    QDomElement controller = root.firstChildElement("controller");

    // For each controller in the preset XML...
    //(Only parse the <controller> block if its id matches our device name, otherwise
    //keep looking at the next controller blocks....)
    QString device;
    while (!controller.isNull()) {
        // Get deviceid
        device = controller.attribute("id","");
        if (device != rootDeviceName(deviceName) && !forceLoad) {
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
        preset.addScriptFile(REQUIRED_SCRIPT_FILE,"");

        // Look for additional ones
        while (!scriptFile.isNull()) {
            QString functionPrefix = scriptFile.attribute("functionprefix","");
            QString filename = scriptFile.attribute("filename","");
            preset.addScriptFile(filename, functionPrefix);

            scriptFile = scriptFile.nextSiblingElement("file");
        }

    }
    return preset;
}

bool ControllerPresetFileHandler::save(const ControllerPreset preset,
                                       const QString deviceName,
                                       const QString path) {
    qDebug() << "Writing preset for" << deviceName << "to file" << path;

    // Need to do this on Windows
    QDir directory;
    directory.mkpath(path.left(path.lastIndexOf("/")));

    QFile output(path);
    if (!output.open(QIODevice::WriteOnly | QIODevice::Truncate)) return false;
    QTextStream outputstream(&output);
    // Construct the DOM from the table
    QDomDocument docPreset = buildXML(preset,deviceName);
    // Save the DOM to the XML file
    docPreset.save(outputstream, 4);
    output.close();

    return true;
}

QDomDocument ControllerPresetFileHandler::buildXML(const ControllerPreset preset,
                                                   const QString deviceName) {
    QDomDocument doc("Preset");
    QString blank = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
        "<MixxxControllerPreset schemaVersion=\"" + QString(XML_SCHEMA_VERSION) + "\">\n"
        "</MixxxControllerPreset>\n";

    doc.setContent(blank);

    QDomElement rootNode = doc.documentElement();
    QDomElement controller = doc.createElement("controller");
    // Strip off the serial number
    controller.setAttribute("id", rootDeviceName(deviceName));
    rootNode.appendChild(controller);

    QDomElement scriptFiles = doc.createElement("scriptfiles");
    controller.appendChild(scriptFiles);

    for (int i = 0; i < preset.scriptFileNames.count(); i++) {
        QString filename = preset.scriptFileNames.at(i);

        //Don't need to write anything for the required mapping file.
        if (filename != REQUIRED_SCRIPT_FILE) {
            qDebug() << "  writing script block for" << filename;
            QString functionPrefix = preset.scriptFunctionPrefixes.at(i);
            QDomElement scriptFile = doc.createElement("file");

            scriptFile.setAttribute("filename", filename);
            scriptFile.setAttribute("functionprefix", functionPrefix);

            scriptFiles.appendChild(scriptFile);
        }
    }
    return doc;
}
