#include "controllers/legacycontrollermappingfilehandler.h"

#include <QStringBuilder>
#include "controllers/defs_controllers.h"
#include "controllers/hid/legacyhidcontrollermappingfilehandler.h"
#include "controllers/midi/legacymidicontrollermappingfilehandler.h"
#include "util/xml.h"

#ifdef MIXXX_USE_QML
QMap<QString, QImage::Format> LegacyControllerMappingFileHandler::kSupportedPixelFormat = {
        {"RBG", QImage::Format_RGB888},
        {"RBGA", QImage::Format_RGBA8888},
        {"RGB565", QImage::Format_RGB16},
};

QMap<QString, std::endian> LegacyControllerMappingFileHandler::kEndianFormat = {
        {"big", std::endian::big},
        {"little", std::endian::little},
};

// static
QFileInfo LegacyControllerMappingFileHandler::findLibraryPath(
        std::shared_ptr<LegacyControllerMapping> mapping,
        const QString& dirname,
        const QDir& systemMappingsPath) {
    // Always try to load module directory from the mapping's directory first
    QFileInfo dir = QFileInfo(mapping->dirPath().absoluteFilePath(dirname));

    // If the module directory does not exist, try to find it in the fallback dir
    if (!dir.isDir()) {
        dir = QFileInfo(systemMappingsPath.absoluteFilePath(dirname));
    }
    return dir;
}
#endif

// Static
QFileInfo LegacyControllerMappingFileHandler::findScriptFile(
        std::shared_ptr<LegacyControllerMapping> mapping,
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
        std::shared_ptr<LegacyControllerMapping> mapping,
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
            LegacyControllerMapping::ScriptFileInfo::Type::JAVASCRIPT,
            true);

    // Look for additional ones
    while (!scriptFile.isNull()) {
        QString filename = scriptFile.attribute("filename", "");
        QFileInfo file = findScriptFile(mapping, filename, systemMappingsPath);
        if (file.suffix() == "qml") {
#ifdef MIXXX_USE_QML
            QString identifier = scriptFile.attribute("identifier", "");
            mapping->addScriptFile(filename,
                    identifier,
                    file,
                    LegacyControllerMapping::ScriptFileInfo::Type::QML);
#else
            qWarning() << "Unsupported render scene. Mixxx isn't built with QML support";
            return;
#endif
        } else {
            QString functionPrefix = scriptFile.attribute("functionprefix", "");
            mapping->addScriptFile(filename,
                    functionPrefix,
                    file,
                    LegacyControllerMapping::ScriptFileInfo::Type::JAVASCRIPT);
        }
        scriptFile = scriptFile.nextSiblingElement("file");
    }

#ifdef MIXXX_USE_QML
    // Build a list of QML files to load
    QDomElement screen = controller.firstChildElement("screens")
                                 .firstChildElement("screen");

    // Look for additional ones
    while (!screen.isNull()) {
        QString identifier = screen.attribute("identifier", "");
        uint targetFps = screen.attribute("targetFps", "30").toUInt();
        QString pixelFormatName = screen.attribute("pixelType", "RBG");
        QString endianName = screen.attribute("endian", "little");
        QString reversedColor = screen.attribute("reversed", "false").toLower();
        QString rawData = screen.attribute("raw", "false").toLower();
        uint splashoff = screen.attribute("splashoff", "0").toUInt();

        if (!targetFps || targetFps > MAX_TARGET_FPS) {
            qWarning() << "Invalid target FPS. Target FPS must be between 1 and" << MAX_TARGET_FPS;
            return;
        }

        if (splashoff > MAX_SPLASHOFF_DURATION) {
            qWarning() << QString(
                    "Invalid splashoff duration. Splashoff duration must be "
                    "between 0 and %1. Clamping to %2")
                                  .arg(MAX_SPLASHOFF_DURATION)
                                  .arg(MAX_SPLASHOFF_DURATION);
            splashoff = MAX_SPLASHOFF_DURATION;
        }

        if (!kSupportedPixelFormat.contains(pixelFormatName)) {
            qWarning() << "Unsupported pixel format" << pixelFormatName;
            return;
        }

        if (!kEndianFormat.contains(endianName)) {
            qWarning() << "Unknown endiant format" << endianName;
            return;
        }

        QImage::Format pixelFormat = kSupportedPixelFormat.value(pixelFormatName);
        std::endian endian = kEndianFormat.value(endianName);

        uint width = screen.attribute("width", "0").toUInt();
        uint height = screen.attribute("height", "0").toUInt();

        if (!width || !height) {
            qWarning() << "Invalid screen size. Screen size must have a width "
                          "and height above 1 pixel";
            return;
        }

        qDebug() << "Adding screen " << identifier;
        mapping->addScreenInfo(identifier,
                QSize(width, height),
                targetFps,
                splashoff,
                pixelFormat,
                endian,
                reversedColor == "yes" || reversedColor == "true" || reversedColor == "1",
                rawData == "yes" || rawData == "true" || rawData == "1");
        screen = screen.nextSiblingElement("screen");
    }
    // Build a list of QML files to load
    QDomElement qmlLibrary = controller.firstChildElement("qmllibraries")
                                     .firstChildElement("library");

    // Look for additional ones
    while (!qmlLibrary.isNull()) {
        QString libFilename = qmlLibrary.attribute("path", "");
        QFileInfo path = findLibraryPath(mapping, libFilename, systemMappingsPath);
        // TODO (ac) asserting file type would fail test - need to chose between one or another
        // if (path.isDir()) {
        qDebug() << "Adding QML directory " << libFilename;
        mapping->addLibraryDirectory(path);
        // } else {
        //     qWarning() << "Unable to add controller QML library path."
        //                << path.absolutePath()
        //                << "is not a directory or is missing";
        // }
        qmlLibrary = qmlLibrary.nextSiblingElement("library");
    }
#endif
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
        QString functionPrefix = script.identifier;
        QDomElement scriptFile = doc.createElement("file");

        scriptFile.setAttribute("filename", filename);
        scriptFile.setAttribute("functionprefix", functionPrefix);
        scriptFiles.appendChild(scriptFile);
    }
    return doc;
}
