/**
* @file controllerpresetfilehandler.cpp
* @author Sean Pappalardo spappalardo@mixxx.org
* @date Mon 9 Apr 2012
* @brief Handles loading and saving of Controller presets.
*
*/

#include "controllers/controllerpresetfilehandler.h"
#include "controllers/defs_controllers.h"

ControllerPreset* ControllerPresetFileHandler::load(const QString path,
                                                    const QString deviceName,
                                                    const bool forceLoad) {
    qDebug() << "Loading controller preset from" << path;
    return load(XmlParse::openXMLFile(path, "controller"), deviceName, forceLoad);
}

void ControllerPresetFileHandler::addScriptFilesToPreset(
    const QDomElement root,
    const QString deviceName,
    const bool forceLoad,
    ControllerPreset* preset) const {
    if (root.isNull())
        return;

    // For each controller in the DOM
    QDomElement controller = root.firstChildElement("controller");

    // For each controller in the preset XML... (Only parse the <controller>
    // block if its id matches our device name, otherwise keep looking at the
    // next controller blocks....)
    QString device;
    while (!controller.isNull()) {
        // Get deviceid
        device = controller.attribute("id", "");
        if (device != rootDeviceName(deviceName) && !forceLoad) {
            controller = controller.nextSiblingElement("controller");
        } else {
            break;
        }
    }

    if (!controller.isNull()) {
        qDebug() << device << "settings found";
        // Build a list of script files to load
        QDomElement scriptFile = controller.firstChildElement("scriptfiles")
                .firstChildElement("file");

        // Default currently required file
        preset->addScriptFile(REQUIRED_SCRIPT_FILE, "");

        // Look for additional ones
        while (!scriptFile.isNull()) {
            QString functionPrefix = scriptFile.attribute("functionprefix","");
            QString filename = scriptFile.attribute("filename","");
            preset->addScriptFile(filename, functionPrefix);
            scriptFile = scriptFile.nextSiblingElement("file");
        }
    }
}

bool ControllerPresetFileHandler::writeDocument(QDomDocument root,
                                                const QString fileName) const {
    // Need to do this on Windows
    QDir directory;
    if (!directory.mkpath(fileName.left(fileName.lastIndexOf("/")))) {
        return false;
    }

    // FIXME(XXX): This is not the right way to replace a file (O_TRUNCATE). The
    // right thing to do is to create a temporary file, write the output to
    // it. Delete the existing file, and then move the temporary file to the
    // existing file's name.
    QFile output(fileName);
    if (!output.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return false;
    }

    // Save the DOM to the XML file
    QTextStream outputstream(&output);
    root.save(outputstream, 4);
    output.close();

    return true;
}

QDomDocument ControllerPresetFileHandler::buildRootWithScripts(const ControllerPreset& preset,
                                                               const QString deviceName) const {
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

        // Don't need to write anything for the required mapping file.
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
