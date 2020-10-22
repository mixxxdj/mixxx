/// @file controllerpresetfilehandler.cpp
/// @author Sean Pappalardo spappalardo@mixxx.org
/// @date Mon 9 Apr 2012
/// @brief Handles loading and saving of Controller presets.

#include "controllers/controllerpresetfilehandler.h"
#include "controllers/controllermanager.h"
#include "controllers/defs_controllers.h"
#include "controllers/midi/midicontrollerpresetfilehandler.h"
#include "controllers/hid/hidcontrollerpresetfilehandler.h"

namespace {

/// Find script file in the preset or system path.
///
/// @param preset The controller preset the script belongs to.
/// @param filename The script filename.
/// @param systemPresetsPath The system presets path to use as fallback.
/// @return Returns a QFileInfo object. If the script was not found in either
/// of the search directories, the QFileInfo object might point to a
/// non-existing file.
QFileInfo findScriptFile(ControllerPreset* preset,
        const QString& filename,
        const QDir& systemPresetsPath) {
    // Always try to load script from the mapping's directory first
    QFileInfo file = QFileInfo(preset->dirPath().absoluteFilePath(filename));

    // If the script does not exist, try to find it in the fallback dir
    if (!file.exists()) {
        file = QFileInfo(systemPresetsPath.absoluteFilePath(filename));
    }
    return file;
}

} // namespace

// static
ControllerPresetPointer ControllerPresetFileHandler::loadPreset(
        const QFileInfo& presetFile, const QDir& systemPresetsPath) {
    if (!presetFile.exists() || !presetFile.isReadable()) {
        qDebug() << "Preset" << presetFile.absoluteFilePath()
                 << "does not exist or is unreadable.";
        return ControllerPresetPointer();
    }

    ControllerPresetFileHandler* pHandler = nullptr;
    if (presetFile.fileName().endsWith(
                MIDI_PRESET_EXTENSION, Qt::CaseInsensitive)) {
        pHandler = new MidiControllerPresetFileHandler();
    } else if (presetFile.fileName().endsWith(
                       HID_PRESET_EXTENSION, Qt::CaseInsensitive) ||
            presetFile.fileName().endsWith(
                    BULK_PRESET_EXTENSION, Qt::CaseInsensitive)) {
        pHandler = new HidControllerPresetFileHandler();
    }

    if (pHandler == nullptr) {
        qDebug() << "Preset" << presetFile.absoluteFilePath()
                 << "has an unrecognized extension.";
        return ControllerPresetPointer();
    }

    ControllerPresetPointer pPreset = pHandler->load(
            presetFile.absoluteFilePath(), systemPresetsPath);
    if (pPreset) {
        pPreset->setDirty(false);
    }
    return pPreset;
}

ControllerPresetPointer ControllerPresetFileHandler::load(
        const QString& path, const QDir& systemPresetsPath) {
    qDebug() << "Loading controller preset from" << path;
    ControllerPresetPointer pPreset = load(
            XmlParse::openXMLFile(path, "controller"), path, systemPresetsPath);
    return pPreset;
}

void ControllerPresetFileHandler::parsePresetInfo(
        const QDomElement& root, ControllerPreset* preset) const {
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
    QDomElement manualPage = info.firstChildElement("manual");
    preset->setManualPage(manualPage.isNull() ? "" : manualPage.text());
    QDomElement wiki = info.firstChildElement("wiki");
    preset->setWikiLink(wiki.isNull() ? "" : wiki.text());
}

QDomElement ControllerPresetFileHandler::getControllerNode(
        const QDomElement& root) {
    if (root.isNull()) {
        return QDomElement();
    }

    // TODO(XXX): Controllers can have multiple <controller> blocks. We should
    // expose this to the user and let them pick them as alternate "versions" of
    // a preset.
    return root.firstChildElement("controller");
}

void ControllerPresetFileHandler::addScriptFilesToPreset(
        const QDomElement& controller,
        ControllerPreset* preset,
        const QDir& systemPresetsPath) const {
    if (controller.isNull())
        return;

    QString deviceId = controller.attribute("id", "");
    preset->setDeviceId(deviceId);

    // Build a list of script files to load
    QDomElement scriptFile = controller.firstChildElement("scriptfiles")
                                     .firstChildElement("file");

    // Default currently required file
    preset->addScriptFile(REQUIRED_SCRIPT_FILE,
            "",
            findScriptFile(preset, REQUIRED_SCRIPT_FILE, systemPresetsPath),
            true);

    // Look for additional ones
    while (!scriptFile.isNull()) {
        QString functionPrefix = scriptFile.attribute("functionprefix", "");
        QString filename = scriptFile.attribute("filename", "");
        QFileInfo file = findScriptFile(preset, filename, systemPresetsPath);

        preset->addScriptFile(filename, functionPrefix, file);
        scriptFile = scriptFile.nextSiblingElement("file");
    }

    QString moduleFileName = controller.firstChildElement("module").text();

    if (moduleFileName.isEmpty()) {
        return;
    }

    QFileInfo moduleFileInfo(preset->dirPath().absoluteFilePath(moduleFileName));
    if (!moduleFileInfo.isFile()) {
        qWarning() << "Controller Module is not a file:" << moduleFileInfo.absoluteFilePath();
        return;
    }

    preset->setModuleFileInfo(moduleFileInfo);
}

bool ControllerPresetFileHandler::writeDocument(
        QDomDocument root, const QString fileName) const {
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

void addTextTag(QDomDocument& doc,
        QDomElement& holder,
        QString tagName,
        QString tagText) {
    QDomElement tag = doc.createElement(tagName);
    QDomText textNode = doc.createTextNode(tagText);
    tag.appendChild(textNode);
    holder.appendChild(tag);
}

QDomDocument ControllerPresetFileHandler::buildRootWithScripts(
        const ControllerPreset& preset) const {
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
    controller.setAttribute("id", rootDeviceName(preset.deviceId()));
    rootNode.appendChild(controller);

    QDomElement scriptFiles = doc.createElement("scriptfiles");
    controller.appendChild(scriptFiles);

    for (const ControllerPreset::ScriptFileInfo& script : preset.getScriptFiles()) {
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
