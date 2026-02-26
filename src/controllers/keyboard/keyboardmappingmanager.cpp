#include "controllers/keyboard/keyboardmappingmanager.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QLocale>
#include <QStandardPaths>
#include <QTextStream>
#include <QtDebug>

#include "moc_keyboardmappingmanager.cpp"

KeyboardMappingManager::KeyboardMappingManager(UserSettingsPointer pConfig, QObject* parent)
        : QObject(parent),
          m_pConfig(pConfig) {
    // Set up custom mappings directory
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    m_customMappingsDir = QDir(configPath).filePath("keyboard_mappings");
    
    // Create directory if it doesn't exist
    QDir dir;
    if (!dir.exists(m_customMappingsDir)) {
        dir.mkpath(m_customMappingsDir);
    }

    // Scan for available mappings
    scanMappings();
}

KeyboardMappingManager::~KeyboardMappingManager() {
}

void KeyboardMappingManager::scanMappings() {
    m_mappings.clear();

    // Scan default mappings from resources
    QDir resourceDir(":/keyboard");
    QStringList filters;
    filters << "*.kbd.cfg";
    
    QFileInfoList defaultMappings = resourceDir.entryInfoList(filters, QDir::Files);
    for (const QFileInfo& fileInfo : defaultMappings) {
        KeyboardMappingInfo info = parseMetadata(fileInfo.absoluteFilePath());
        info.isDefault = true;
        info.isReadOnly = true;
        m_mappings[fileInfo.absoluteFilePath()] = info;
    }

    // Scan custom mappings from user directory
    QDir customDir(m_customMappingsDir);
    QFileInfoList customMappings = customDir.entryInfoList(filters, QDir::Files);
    for (const QFileInfo& fileInfo : customMappings) {
        KeyboardMappingInfo info = parseMetadata(fileInfo.absoluteFilePath());
        info.isDefault = false;
        info.isReadOnly = false;
        m_mappings[fileInfo.absoluteFilePath()] = info;
    }

    // Add registered external mappings
    for (const QString& filePath : m_externalMappings) {
        if (!m_mappings.contains(filePath) && QFile::exists(filePath)) {
            KeyboardMappingInfo info = parseMetadata(filePath);
            info.isDefault = false;
            info.isReadOnly = false;
            m_mappings[filePath] = info;
        }
    }
}

QList<KeyboardMappingInfo> KeyboardMappingManager::getAvailableMappings() const {
    return m_mappings.values();
}

KeyboardMappingInfo KeyboardMappingManager::getMappingInfo(const QString& filePath) const {
    return m_mappings.value(filePath);
}

KeyboardMappingInfo KeyboardMappingManager::parseMetadata(const QString& filePath) const {
    KeyboardMappingInfo info;
    info.filePath = filePath;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open keyboard mapping file:" << filePath;
        return info;
    }

    QTextStream in(&file);
    QString line;
    bool inMetadata = false;

    // Extract filename as default name
    QFileInfo fileInfo(filePath);
    QString baseName = fileInfo.baseName();
    info.name = baseName.replace('_', ' ');

    while (!in.atEnd()) {
        line = in.readLine().trimmed();

        if (line == "[Metadata]") {
            inMetadata = true;
            continue;
        }

        if (line.startsWith('[') && line != "[Metadata]") {
            inMetadata = false;
            break;
        }

        if (inMetadata && line.contains('=')) {
            QStringList parts = line.split('=', Qt::SkipEmptyParts);
            if (parts.size() >= 2) {
                QString key = parts[0].trimmed();
                QString value = parts.mid(1).join('=').trimmed();

                if (key == "Name") {
                    info.name = value;
                } else if (key == "Author") {
                    info.author = value;
                } else if (key == "Description") {
                    info.description = value;
                } else if (key == "Version") {
                    info.version = value;
                }
            }
        }
    }

    file.close();
    return info;
}

QSharedPointer<ConfigObject<ConfigValueKbd>> KeyboardMappingManager::loadMapping(
        const QString& filePath) {
    auto pMapping = QSharedPointer<ConfigObject<ConfigValueKbd>>::create(filePath);
    return pMapping;
}

bool KeyboardMappingManager::saveMapping(
        const QString& filePath,
        const KeyboardMappingInfo& info,
        ConfigObject<ConfigValueKbd>* pMapping) {
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Failed to open file for writing:" << filePath;
        return false;
    }

    QTextStream out(&file);

    // Write metadata section
    out << "[Metadata]\n";
    if (!info.name.isEmpty()) {
        out << "Name=" << info.name << "\n";
    }
    if (!info.author.isEmpty()) {
        out << "Author=" << info.author << "\n";
    }
    if (!info.description.isEmpty()) {
        out << "Description=" << info.description << "\n";
    }
    if (!info.version.isEmpty()) {
        out << "Version=" << info.version << "\n";
    }
    out << "\n";

    // Write mapping data
    // Get all groups from the config object
    QList<QString> groups = pMapping->getGroups().values();
    std::sort(groups.begin(), groups.end());
    for (const QString& group : std::as_const(groups)) {
        if (group == "Metadata") {
            continue; // Skip metadata group
        }

        out << "[" << group << "]\n";

        // Get all keys in this group
        QList<ConfigKey> keys = pMapping->getKeysWithGroup(group);
        for (const ConfigKey& configKey : keys) {
            QString value = pMapping->getValueString(configKey);
            if (!value.isEmpty()) {
                out << configKey.item << " " << value << "\n";
            }
        }
        out << "\n";
    }

    file.close();

    // Rescan to update our internal list
    scanMappings();
    emit mappingsChanged();

    return true;
}

QString KeyboardMappingManager::createCustomMapping(
        const QString& name,
        const QString& author,
        const QString& description) {
    // Generate filename from name
    QString filename = name.toLower().replace(' ', '_') + ".kbd.cfg";
    QString filePath = QDir(m_customMappingsDir).filePath(filename);

    // Check if file already exists
    int counter = 1;
    while (QFile::exists(filePath)) {
        filename = name.toLower().replace(' ', '_') + QString("_%1.kbd.cfg").arg(counter);
        filePath = QDir(m_customMappingsDir).filePath(filename);
        counter++;
    }

    // Create empty mapping with metadata
    KeyboardMappingInfo info;
    info.name = name;
    info.author = author;
    info.description = description;
    info.version = "1.0";

    auto pEmptyMapping = QSharedPointer<ConfigObject<ConfigValueKbd>>::create(QString());
    
    if (saveMapping(filePath, info, pEmptyMapping.get())) {
        return filePath;
    }

    return QString();
}

bool KeyboardMappingManager::deleteMapping(const QString& filePath) {
    KeyboardMappingInfo info = getMappingInfo(filePath);
    
    // Don't allow deleting default mappings
    if (info.isReadOnly || info.isDefault) {
        qWarning() << "Cannot delete read-only or default mapping:" << filePath;
        return false;
    }

    QFile file(filePath);
    if (file.remove()) {
        scanMappings();
        emit mappingsChanged();
        return true;
    }

    return false;
}

bool KeyboardMappingManager::exportToJson(const QString& filePath,
                                          const KeyboardMappingInfo& info,
                                          ConfigObject<ConfigValueKbd>* pMapping) {
    QJsonObject root;
    
    // Metadata
    QJsonObject metadata;
    metadata["Name"] = info.name;
    metadata["Author"] = info.author;
    metadata["Description"] = info.description;
    metadata["Version"] = info.version;
    root["Metadata"] = metadata;
    
    // Mappings
    QJsonObject mappings;
    QList<QString> groups = pMapping->getGroups().values();
    for (const QString& group : groups) {
        if (group == "Metadata") continue;
        
        QJsonObject groupObj;
        QList<ConfigKey> keys = pMapping->getKeysWithGroup(group);
        for (const ConfigKey& key : keys) {
            groupObj[key.item] = pMapping->getValueString(key);
        }
        mappings[group] = groupObj;
    }
    root["Mappings"] = mappings;
    
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) return false;
    
    file.write(QJsonDocument(root).toJson());
    file.close();
    return true;
}

bool KeyboardMappingManager::importFromJson(const QString& filePath,
                                            KeyboardMappingInfo* pInfo,
                                            QSharedPointer<ConfigObject<ConfigValueKbd>> pMapping) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) return false;
    
    QByteArray data = file.readAll();
    file.close();
    
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) return false;
    
    QJsonObject root = doc.object();
    
    // Metadata
    if (root.contains("Metadata") && root["Metadata"].isObject()) {
        QJsonObject metadata = root["Metadata"].toObject();
        if (pInfo) {
            if (metadata.contains("Name")) pInfo->name = metadata["Name"].toString();
            if (metadata.contains("Author")) pInfo->author = metadata["Author"].toString();
            if (metadata.contains("Description")) pInfo->description = metadata["Description"].toString();
            if (metadata.contains("Version")) pInfo->version = metadata["Version"].toString();
        }
    }
    
    // Mappings
    if (root.contains("Mappings") && root["Mappings"].isObject()) {
        QJsonObject mappings = root["Mappings"].toObject();
        for (auto it = mappings.begin(); it != mappings.end(); ++it) {
            QString group = it.key();
            if (it.value().isObject()) {
                QJsonObject groupObj = it.value().toObject();
                for (auto keyIt = groupObj.begin(); keyIt != groupObj.end(); ++keyIt) {
                    pMapping->set(ConfigKey(group, keyIt.key()), ConfigValueKbd(keyIt.value().toString()));
                }
            }
        }
    }
    
    return true;
}

bool KeyboardMappingManager::exportToXml(const QString& filePath,
                                         const KeyboardMappingInfo& info,
                                         ConfigObject<ConfigValueKbd>* pMapping) {
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return false;
    
    QXmlStreamWriter xml(&file);
    xml.setAutoFormatting(true);
    xml.writeStartDocument();
    xml.writeStartElement("KeyboardMapping");
    
    // Metadata
    xml.writeStartElement("Metadata");
    xml.writeTextElement("Name", info.name);
    xml.writeTextElement("Author", info.author);
    xml.writeTextElement("Description", info.description);
    xml.writeTextElement("Version", info.version);
    xml.writeEndElement();
    
    // Mappings
    xml.writeStartElement("Mappings");
    QList<QString> groups = pMapping->getGroups().values();
    for (const QString& group : groups) {
        if (group == "Metadata") continue;
        
        xml.writeStartElement("Group");
        xml.writeAttribute("name", group);
        
        QList<ConfigKey> keys = pMapping->getKeysWithGroup(group);
        for (const ConfigKey& key : keys) {
            xml.writeStartElement("Map");
            xml.writeAttribute("item", key.item);
            xml.writeCharacters(pMapping->getValueString(key));
            xml.writeEndElement();
        }
        xml.writeEndElement();
    }
    xml.writeEndElement(); // Mappings
    
    xml.writeEndElement(); // KeyboardMapping
    xml.writeEndDocument();
    
    file.close();
    return true;
}

bool KeyboardMappingManager::importFromXml(const QString& filePath,
                                           KeyboardMappingInfo* pInfo,
                                           QSharedPointer<ConfigObject<ConfigValueKbd>> pMapping) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return false;
    
    QXmlStreamReader xml(&file);
    QString currentGroup;
    
    while (!xml.atEnd() && !xml.hasError()) {
        QXmlStreamReader::TokenType token = xml.readNext();
        if (token == QXmlStreamReader::StartElement) {
            if (xml.name() == QLatin1String("Name") && pInfo) pInfo->name = xml.readElementText();
            else if (xml.name() == QLatin1String("Author") && pInfo) pInfo->author = xml.readElementText();
            else if (xml.name() == QLatin1String("Description") && pInfo) pInfo->description = xml.readElementText();
            else if (xml.name() == QLatin1String("Version") && pInfo) pInfo->version = xml.readElementText();
            else if (xml.name() == QLatin1String("Group")) {
                currentGroup = xml.attributes().value("name").toString();
            } else if (xml.name() == QLatin1String("Map")) {
                QString item = xml.attributes().value("item").toString();
                QString value = xml.readElementText();
                if (!currentGroup.isEmpty()) {
                    pMapping->set(ConfigKey(currentGroup, item), ConfigValueKbd(value));
                }
            }
        }
    }
    
    file.close();
    return !xml.hasError();
}

QString KeyboardMappingManager::getDefaultMappingPath() const {
    return getLocaleDefaultMapping();
}

QString KeyboardMappingManager::getCustomMappingsDirectory() const {
    return m_customMappingsDir;
}

QString KeyboardMappingManager::getLocaleDefaultMapping() const {
    // Get system locale
    QString locale = QLocale::system().name(); // e.g., "en_US"
    
    // Try exact locale match
    QString exactPath = QString(":/keyboard/%1.kbd.cfg").arg(locale);
    if (QFile::exists(exactPath)) {
        return exactPath;
    }

    // Try language-only match (e.g., "en" from "en_US")
    QString language = locale.split('_').first();
    QDir resourceDir(":/keyboard");
    QStringList filters;
    filters << QString("%1_*.kbd.cfg").arg(language);
    
    QFileInfoList matches = resourceDir.entryInfoList(filters, QDir::Files);
    if (!matches.isEmpty()) {
        return matches.first().absoluteFilePath();
    }

    // Fall back to en_US
    return ":/keyboard/en_US.kbd.cfg";
}

void KeyboardMappingManager::addExternalMapping(const QString& filePath) {
    if (!m_externalMappings.contains(filePath)) {
        m_externalMappings.append(filePath);
        scanMappings();
        emit mappingsChanged();
    }
}
