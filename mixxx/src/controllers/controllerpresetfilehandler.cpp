/**
* @file controllerpresetfilehandler.cpp
* @author Sean Pappalardo spappalardo@mixxx.org
* @date Mon 9 Apr 2012
* @brief Handles loading and saving of Controller presets.
*
*/

#include "controllers/controllerpresetfilehandler.h"
#include "controllers/defs_controllers.h"

ControllerPresetPointer ControllerPresetFileHandler::load(const QString path,
                                                          const QString deviceName,
                                                          const bool forceLoad) {
    qDebug() << "Loading controller preset from" << path;
    ControllerPresetPointer pPreset = load(XmlParse::openXMLFile(path, "controller"),
                                           deviceName, forceLoad);
    if (pPreset) {
        pPreset->setFilePath(path);
    }
    return pPreset;
}

void ControllerPresetFileHandler::parsePresetInfo(const QDomElement& root,
                                                  ControllerPreset* preset) const {
    if (root.isNull() || !preset) {
        return;
    }

    QDomElement info = root.firstChildElement("info");
    if (info.isNull()) {
        return;
    }

    QString mixxxVersion = root.attribute("mixxxVersion", "");
    preset->setMixxxVersion(mixxxVersion);
    QString schemaVersion = root.attribute("schemaVersion", "");
    preset->setSchemaVersion(schemaVersion);
    QDomElement name = info.firstChildElement("name");
    preset->setName(name.isNull() ? "" : name.text());
    QDomElement author = info.firstChildElement("author");
    preset->setAuthor(author.isNull() ? "" : author.text());
    QDomElement description = info.firstChildElement("description");
    preset->setDescription(description.isNull() ? "" : description.text());
}

QDomElement ControllerPresetFileHandler::getControllerNode(const QDomElement& root,
                                                           const QString deviceName,
                                                           const bool forceLoad) {
    // All callers of this method as of 4/2012 provide forceLoad true so the
    // deviceId check is not really used.
    if (root.isNull()) {
        return QDomElement();
    }

    QDomElement controller = root.firstChildElement("controller");

    // For each controller in the preset XML... (Only parse the <controller>
    // block if its id matches our device name, otherwise keep looking at the
    // next controller blocks....)
    while (!controller.isNull()) {
        // Get deviceid
        QString device = controller.attribute("id", "");
        if (device != rootDeviceName(deviceName) && !forceLoad) {
            controller = controller.nextSiblingElement("controller");
        } else {
            qDebug() << device << "settings found";
            break;
        }
    }
    return controller;
}

void ControllerPresetFileHandler::addScriptFilesToPreset(
    const QDomElement& controller, ControllerPreset* preset) const {
    if (controller.isNull())
        return;

    QString deviceId = controller.attribute("id", "");
    preset->setDeviceId(deviceId);

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

void addTextTag(QDomDocument& doc, QDomElement& holder,
                QString tagName, QString tagText) {
    QDomElement tag = doc.createElement(tagName);
    QDomText textNode = doc.createTextNode(tagText);
    tag.appendChild(textNode);
    holder.appendChild(tag);
}

QDomDocument ControllerPresetFileHandler::buildRootWithScripts(const ControllerPreset& preset,
                                                               const QString deviceName) const {
    QDomDocument doc("Preset");
    QString blank = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
        "<MixxxControllerPreset>\n"
        "</MixxxControllerPreset>\n";
    doc.setContent(blank);

    QDomElement rootNode = doc.documentElement();
    rootNode.setAttribute("schemaVersion", XML_SCHEMA_VERSION);
    rootNode.setAttribute("mixxxVersion", preset.mixxxVersion());

    QDomElement info = doc.createElement("info");
    rootNode.appendChild(info);
    if (preset.name().length() > 0) {
        addTextTag(doc, info, "name", preset.name());
    }
    if (preset.author().length() > 0) {
        addTextTag(doc, info, "author", preset.author());
    }
    if (preset.description().length() > 0) {
        addTextTag(doc, info, "description", preset.description());
    }

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
