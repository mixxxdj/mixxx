#include "controllers/legacycontrollermappingfilehandler.h"

#include "controllers/defs_controllers.h"
#include "controllers/midi/legacymidicontrollermappingfilehandler.h"
#include "util/xml.h"

#if defined(__HID__) || defined(__BULK__)
#include "controllers/hid/legacyhidcontrollermappingfilehandler.h"
#endif

namespace {

/// Find script file in the mapping or system path.
///
/// @param mapping The controller mapping the script belongs to.
/// @param filename The script filename.
/// @param systemMappingsPath The system mappings path to use as fallback.
/// @return Returns a QFileInfo object. If the script was not found in either
/// of the search directories, the QFileInfo object might point to a
/// non-existing file.
QFileInfo findScriptFile(std::shared_ptr<LegacyControllerMapping> mapping,
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
std::shared_ptr<LegacyControllerMapping> LegacyControllerMappingFileHandler::loadMapping(
        const QFileInfo& mappingFile, const QDir& systemMappingsPath) {
    if (mappingFile.path().isEmpty()) {
        return nullptr;
    }
    if (!mappingFile.exists() || !mappingFile.isReadable()) {
        qDebug() << "Mapping" << mappingFile.absoluteFilePath()
                 << "does not exist or is unreadable.";
        return nullptr;
    }

    std::unique_ptr<LegacyControllerMappingFileHandler> pHandler;
    if (mappingFile.fileName().endsWith(
                MIDI_MAPPING_EXTENSION, Qt::CaseInsensitive)) {
        pHandler = std::make_unique<LegacyMidiControllerMappingFileHandler>();
    } else if (mappingFile.fileName().endsWith(
                       HID_MAPPING_EXTENSION, Qt::CaseInsensitive)) {
#ifdef __HID__
        pHandler = std::make_unique<LegacyHidControllerMappingFileHandler>();
#endif
    } else if (mappingFile.fileName().endsWith(
                       BULK_MAPPING_EXTENSION, Qt::CaseInsensitive)) {
#ifdef __BULK__
        pHandler = std::make_unique<LegacyHidControllerMappingFileHandler>();
#endif
    }

    if (!pHandler) {
        qDebug() << "Mapping" << mappingFile.absoluteFilePath()
                 << "has an unrecognized extension.";
        return nullptr;
    }

    std::shared_ptr<LegacyControllerMapping> pMapping = pHandler->load(
            mappingFile.absoluteFilePath(), systemMappingsPath);

    if (pMapping) {
        pMapping->setDirty(false);
    }
    return pMapping;
}

std::shared_ptr<LegacyControllerMapping> LegacyControllerMappingFileHandler::load(
        const QString& path, const QDir& systemMappingsPath) {
    qDebug() << "Loading controller mapping from" << path;
    return load(
            XmlParse::openXMLFile(path, "controller"), path, systemMappingsPath);
}

void LegacyControllerMappingFileHandler::parseMappingInfo(
        const QDomElement& root, std::shared_ptr<LegacyControllerMapping> mapping) const {
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
    mapping->setManualLink(wiki.isNull() ? "" : wiki.text());
}

void LegacyControllerMappingFileHandler::parseMappingSettings(
        const QDomElement& root, LegacyControllerMapping* mapping) const {
    if (root.isNull() || !mapping) {
        return;
    }

    QDomElement settings = root.firstChildElement("settings");
    if (settings.isNull()) {
        return;
    }

    std::unique_ptr<LegacyControllerSettingsLayoutContainer> settingLayout =
            std::make_unique<LegacyControllerSettingsLayoutContainer>(
                    LegacyControllerSettingsLayoutContainer::Disposition::
                            VERTICAL);
    parseMappingSettingsElement(settings, mapping, settingLayout.get());
    mapping->setSettingLayout(std::move(settingLayout));
}

void LegacyControllerMappingFileHandler::parseMappingSettingsElement(
        const QDomElement& current,
        LegacyControllerMapping* pMapping,
        LegacyControllerSettingsLayoutContainer* pLayout)
        const {
    for (QDomElement element = current.firstChildElement();
            !element.isNull();
            element = element.nextSiblingElement()) {
        const QString& tagName = element.tagName().toLower();
        if (tagName == "option") {
            std::shared_ptr<AbstractLegacyControllerSetting> pSetting =
                    LegacyControllerSettingBuilder::build(element);
            if (pSetting.get() == nullptr) {
                qDebug() << "Ignoring unsupported controller setting in file"
                         << pMapping->filePath() << "at line"
                         << element.lineNumber() << ".";
                continue;
            }
            if (!pSetting->valid()) {
                qDebug() << "The parsed setting in file" << pMapping->filePath()
                         << "at line" << element.lineNumber()
                         << "appears to be invalid. It will be ignored.";
                continue;
            }
            if (pMapping->addSetting(pSetting)) {
                pLayout->addItem(pSetting);
            } else {
                qDebug() << "The parsed setting in file" << pMapping->filePath()
                         << "at line" << element.lineNumber()
                         << "couldn't be added. Its layout information will also be ignored.";
                continue;
            }
        } else if (tagName == "row") {
            LegacyControllerSettingsLayoutContainer::Disposition orientation =
                    element.attribute("orientation").trimmed().toLower() ==
                            "vertical"
                    ? LegacyControllerSettingsLayoutContainer::VERTICAL
                    : LegacyControllerSettingsLayoutContainer::HORIZONTAL;
            std::unique_ptr<LegacyControllerSettingsLayoutContainer> row =
                    std::make_unique<LegacyControllerSettingsLayoutContainer>(
                            LegacyControllerSettingsLayoutContainer::HORIZONTAL,
                            orientation);
            parseMappingSettingsElement(element, pMapping, row.get());
            pLayout->addItem(std::move(row));
        } else if (tagName == "group") {
            std::unique_ptr<LegacyControllerSettingsLayoutContainer> group =
                    std::make_unique<LegacyControllerSettingsGroup>(
                            element.attribute("label"));
            parseMappingSettingsElement(element, pMapping, group.get());
            pLayout->addItem(std::move(group));
        } else {
            qDebug() << "Ignoring unsupported tag" << tagName
                     << "in file" << pMapping->filePath()
                     << "on line" << element.lineNumber()
                     << "for controller layout settings. Check the documentation supported tags.";
            continue;
        }
    }
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
        std::shared_ptr<LegacyControllerMapping> mapping,
        const QDir& systemMappingsPath) const {
    if (controller.isNull()) {
        return;
    }

    QString deviceId = controller.attribute("id", "");
    mapping->setDeviceId(deviceId);

    // See TODO in LegacyControllerMapping::DeviceDirection - `direction` should
    // only be used as a workaround till the bulk integration gets refactored
    QString deviceDirection = controller.attribute("direction", "").toLower();
    if (deviceDirection == "in") {
        mapping->setDeviceDirection(LegacyControllerMapping::DeviceDirection::Incoming);
    } else if (deviceDirection == "out") {
        mapping->setDeviceDirection(LegacyControllerMapping::DeviceDirection::Outgoing);
    } else {
        mapping->setDeviceDirection(LegacyControllerMapping::DeviceDirection::Bidirectionnal);
    }

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
            "</MixxxControllerPreset>\n";
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
    if (mapping.manuallink().length() > 0) {
        addTextTag(doc, info, "wiki", mapping.manuallink());
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
