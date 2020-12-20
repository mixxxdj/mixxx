#include "controllers/legacycontrollermappingfilehandler.h"

#include "controllers/controllermanager.h"
#include "controllers/defs_controllers.h"
#include "controllers/hid/legacyhidcontrollermappingfilehandler.h"
#include "controllers/midi/legacymidicontrollermappingfilehandler.h"

namespace {

/// Find script file in the mapping or system path.
///
/// @param mapping The controller mapping the script belongs to.
/// @param filename The script filename.
/// @param systemMappingsPath The system mappings path to use as fallback.
/// @return Returns a QFileInfo object. If the script was not found in either
/// of the search directories, the QFileInfo object might point to a
/// non-existing file.
QFileInfo findScriptFile(LegacyControllerMapping* mapping,
        const QString& filename,
        const QDir& systemMappingsPath) {
    // Always try to load script from the mapping's directory first
    QFileInfo file = QFileInfo(mapping->dirPath().absoluteFilePath(filename));

    // If the script does not exist, try to find it in the fallback dir
    if (!file.exists()) {
        file = QFileInfo(systemMappingsPath.absoluteFilePath(filename));
    }
    return file;
}

} // namespace

// static
LegacyControllerMappingPointer LegacyControllerMappingFileHandler::loadMapping(
        const QFileInfo& mappingFile, const QDir& systemMappingsPath) {
    if (!mappingFile.exists() || !mappingFile.isReadable()) {
        qDebug() << "Mapping" << mappingFile.absoluteFilePath()
                 << "does not exist or is unreadable.";
        return LegacyControllerMappingPointer();
    }

    LegacyControllerMappingFileHandler* pHandler = nullptr;
    if (mappingFile.fileName().endsWith(
                MIDI_MAPPING_EXTENSION, Qt::CaseInsensitive)) {
        pHandler = new LegacyMidiControllerMappingFileHandler();
    } else if (mappingFile.fileName().endsWith(
                       HID_MAPPING_EXTENSION, Qt::CaseInsensitive) ||
            mappingFile.fileName().endsWith(
                    BULK_MAPPING_EXTENSION, Qt::CaseInsensitive)) {
        pHandler = new LegacyHidControllerMappingFileHandler();
    }

    if (pHandler == nullptr) {
        qDebug() << "Mapping" << mappingFile.absoluteFilePath()
                 << "has an unrecognized extension.";
        return LegacyControllerMappingPointer();
    }

    LegacyControllerMappingPointer pMapping = pHandler->load(
            mappingFile.absoluteFilePath(), systemMappingsPath);
    if (pMapping) {
        pMapping->setDirty(false);
    }
    return pMapping;
}

LegacyControllerMappingPointer LegacyControllerMappingFileHandler::load(
        const QString& path, const QDir& systemMappingsPath) {
    qDebug() << "Loading controller mapping from" << path;
    LegacyControllerMappingPointer pMapping = load(
            XmlParse::openXMLFile(path, "controller"), path, systemMappingsPath);
    return pMapping;
}

void LegacyControllerMappingFileHandler::parseMappingInfo(
        const QDomElement& root, LegacyControllerMapping* mapping) const {
    if (root.isNull() || !mapping) {
        return;
    }

    QDomElement info = root.firstChildElement("info");
    if (info.isNull()) {
        return;
    }

    QString mixxxVersion = root.attribute("mixxxVersion", "");
    mapping->setMixxxVersion(mixxxVersion);
    QString schemaVersion = root.attribute("schemaVersion", "");
    mapping->setSchemaVersion(schemaVersion);
    QDomElement name = info.firstChildElement("name");
    mapping->setName(name.isNull() ? "" : name.text());
    QDomElement author = info.firstChildElement("author");
    mapping->setAuthor(author.isNull() ? "" : author.text());
    QDomElement description = info.firstChildElement("description");
    mapping->setDescription(description.isNull() ? "" : description.text());
    QDomElement forums = info.firstChildElement("forums");
    mapping->setForumLink(forums.isNull() ? "" : forums.text());
    QDomElement manualPage = info.firstChildElement("manual");
    mapping->setManualPage(manualPage.isNull() ? "" : manualPage.text());
    QDomElement wiki = info.firstChildElement("wiki");
    mapping->setWikiLink(wiki.isNull() ? "" : wiki.text());
}

QDomElement LegacyControllerMappingFileHandler::getControllerNode(
        const QDomElement& root) {
    if (root.isNull()) {
        return QDomElement();
    }

    // TODO(XXX): Controllers can have multiple <controller> blocks. We should
    // expose this to the user and let them pick them as alternate "versions" of
    // a mapping.
    return root.firstChildElement("controller");
}

void LegacyControllerMappingFileHandler::addScriptFilesToMapping(
        const QDomElement& controller,
        LegacyControllerMapping* mapping,
        const QDir& systemMappingsPath) const {
    if (controller.isNull()) {
        return;
    }

    QString deviceId = controller.attribute("id", "");
    mapping->setDeviceId(deviceId);

    // Build a list of script files to load
    QDomElement scriptFile = controller.firstChildElement("scriptfiles")
                                     .firstChildElement("file");

    // Default currently required file
    mapping->addScriptFile(REQUIRED_SCRIPT_FILE,
            "",
            findScriptFile(mapping, REQUIRED_SCRIPT_FILE, systemMappingsPath),
            true);

    // Look for additional ones
    while (!scriptFile.isNull()) {
        QString functionPrefix = scriptFile.attribute("functionprefix", "");
        QString filename = scriptFile.attribute("filename", "");
        QFileInfo file = findScriptFile(mapping, filename, systemMappingsPath);

        mapping->addScriptFile(filename, functionPrefix, file);
        scriptFile = scriptFile.nextSiblingElement("file");
    }
}

bool LegacyControllerMappingFileHandler::writeDocument(
        const QDomDocument& root, const QString& fileName) const {
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
        const QString& tagName,
        const QString& tagText) {
    QDomElement tag = doc.createElement(tagName);
    QDomText textNode = doc.createTextNode(tagText);
    tag.appendChild(textNode);
    holder.appendChild(tag);
}

QDomDocument LegacyControllerMappingFileHandler::buildRootWithScripts(
        const LegacyControllerMapping& mapping) const {
    QDomDocument doc("Mapping");
    QString blank =
            "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
            "<MixxxControllerPreset>\n"
            "</<MixxxControllerPreset>\n";
    doc.setContent(blank);

    QDomElement rootNode = doc.documentElement();
    rootNode.setAttribute("schemaVersion", XML_SCHEMA_VERSION);
    rootNode.setAttribute("mixxxVersion", mapping.mixxxVersion());

    QDomElement info = doc.createElement("info");
    rootNode.appendChild(info);
    if (mapping.name().length() > 0) {
        addTextTag(doc, info, "name", mapping.name());
    }
    if (mapping.author().length() > 0) {
        addTextTag(doc, info, "author", mapping.author());
    }
    if (mapping.description().length() > 0) {
        addTextTag(doc, info, "description", mapping.description());
    }
    if (mapping.forumlink().length() > 0) {
        addTextTag(doc, info, "forums", mapping.forumlink());
    }
    if (mapping.wikilink().length() > 0) {
        addTextTag(doc, info, "wiki", mapping.wikilink());
    }

    QDomElement controller = doc.createElement("controller");
    // Strip off the serial number
    controller.setAttribute("id", rootDeviceName(mapping.deviceId()));
    rootNode.appendChild(controller);

    QDomElement scriptFiles = doc.createElement("scriptfiles");
    controller.appendChild(scriptFiles);

    for (const LegacyControllerMapping::ScriptFileInfo& script : mapping.getScriptFiles()) {
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
