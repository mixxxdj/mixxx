/**
* @file controllerpresetfilehandler.cpp
* @author Sean Pappalardo spappalardo@mixxx.org
* @date Mon 9 Apr 2012
* @brief Handles loading and saving of Controller presets.
*
*/

#include "controllers/controllerpresetfilehandler.h"
#include "controllers/controllermanager.h"
#include "controllers/defs_controllers.h"
#include "controllers/midi/midicontrollerpresetfilehandler.h"
#include "controllers/hid/hidcontrollerpresetfilehandler.h"

// static
ControllerPresetPointer ControllerPresetFileHandler::loadPreset(const QString& pathOrFilename,
                                                                const QStringList& presetPaths) {
    qDebug() << "Searching for controller preset" << pathOrFilename
             << "in paths:" << presetPaths.join(",");
    QString scriptPath = ControllerManager::getAbsolutePath(pathOrFilename,
                                                            presetPaths);

    if (scriptPath.isEmpty()) {
        qDebug() << "Could not find" << pathOrFilename
                 << "in any preset path.";
        return ControllerPresetPointer();
    }

    QFileInfo scriptPathInfo(scriptPath);
    if (!scriptPathInfo.exists() || !scriptPathInfo.isReadable()) {
        qDebug() << "Preset" << scriptPath << "does not exist or is unreadable.";
        return ControllerPresetPointer();
    }

    // TODO(XXX): This means filenames can't have .foo.midi.xml filenames. We
    // should regex match against the end.
    // NOTE(rryan): We prepend a dot because all the XXX_PRESET_EXTENSION
    // defines include the dot.
    QString extension = "." + scriptPathInfo.completeSuffix();

    ControllerPresetFileHandler* pHandler = NULL;
    if (scriptPath.endsWith(MIDI_PRESET_EXTENSION, Qt::CaseInsensitive)) {
        pHandler = new MidiControllerPresetFileHandler();
    } else if (scriptPath.endsWith(HID_PRESET_EXTENSION, Qt::CaseInsensitive) ||
               scriptPath.endsWith(BULK_PRESET_EXTENSION, Qt::CaseInsensitive)) {
        pHandler = new HidControllerPresetFileHandler();
    }

    if (pHandler == NULL) {
        qDebug() << "Preset" << scriptPath << "has an unrecognized extension.";
        return ControllerPresetPointer();
    }

    // NOTE(rryan): We don't provide a device name. It's unused currently.
    // TODO(rryan): Delete pHandler.
    return pHandler->load(scriptPath, QString());
}

ControllerPresetPointer ControllerPresetFileHandler::load(const QString path,
                                                          const QString deviceName) {
    qDebug() << "Loading controller preset from" << path;
    ControllerPresetPointer pPreset = load(XmlParse::openXMLFile(path, "controller"),
                                           deviceName);
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
    QDomElement forums = info.firstChildElement("forums");
    preset->setForumLink(forums.isNull() ? "" : forums.text());
    QDomElement wiki = info.firstChildElement("wiki");
    preset->setWikiLink(wiki.isNull() ? "" : wiki.text());
}

QDomElement ControllerPresetFileHandler::getControllerNode(const QDomElement& root,
                                                           const QString deviceName) {
    Q_UNUSED(deviceName);
    if (root.isNull()) {
        return QDomElement();
    }

    // TODO(XXX): Controllers can have multiple <controller> blocks. We should
    // expose this to the user and let them pick them as alternate "versions" of
    // a preset.
    return root.firstChildElement("controller");
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
    preset->addScriptFile(REQUIRED_SCRIPT_FILE, "", true);

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
    if (preset.forumlink().length() > 0) {
        addTextTag(doc, info, "forums", preset.forumlink());
    }
    if (preset.wikilink().length() > 0) {
        addTextTag(doc, info, "wiki", preset.wikilink());
    }

    QDomElement controller = doc.createElement("controller");
    // Strip off the serial number
    controller.setAttribute("id", rootDeviceName(deviceName));
    rootNode.appendChild(controller);

    QDomElement scriptFiles = doc.createElement("scriptfiles");
    controller.appendChild(scriptFiles);

    foreach (const ControllerPreset::ScriptFileInfo& script, preset.scripts) {
        QString filename = script.name;
        // Don't need to write anything for built-in files.
        if (script.builtin) {
            continue;
        }
        qDebug() << "writing script block for" << filename;
        QString functionPrefix = script.functionPrefix;
        QDomElement scriptFile = doc.createElement("file");

        scriptFile.setAttribute("filename", filename);
        scriptFile.setAttribute("functionprefix", functionPrefix);
        scriptFiles.appendChild(scriptFile);
    }
    return doc;
}
