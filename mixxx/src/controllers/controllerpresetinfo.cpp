/**
* @file controllerpresetinfo.cpp
* @author Ilkka Tuohela hile@iki.fi
* @date Wed May 15 2012
* @brief Implement handling enumeration and parsing of preset info headers
*
* This class handles enumeration and parsing of controller XML description file
* <info> header tags. It can be used to match controllers automatically or to
* show details for a mapping.
*/

#include "controllers/controllerpresetinfo.h"

#include "controllers/defs_controllers.h"
#include "xmlparse.h"

PresetInfo::PresetInfo()
        : m_valid(false) {
}

PresetInfo::PresetInfo(const QString preset_path) {
    // Parse <info> header section from a controller description XML file
    // Contents parsed by xml path:
    // info.name        Preset name, used for drop down menus in dialogs
    // info.author      Preset author
    // info.description Preset description
    // info.forums      Link to mixxx forum discussion for the preset
    // info.wiki        Link to mixxx wiki for the preset
    // info.devices.product List of device matches, specific to device type
    path = QFileInfo(preset_path).absoluteFilePath();
    name = "";
    author = "";
    description = "";
    forumlink = "";
    wikilink = "";

    QDomElement root = XmlParse::openXMLFile(path, "controller");
    if (root.isNull()) {
        qDebug() << "ERROR parsing" << path;
        return;
    }
    QDomElement info = root.firstChildElement("info");
    if (info.isNull()) {
        qDebug() << "MISSING <info> ELEMENT: " << path;
        return;
    }

    m_valid = true;

    QDomElement dom_name = info.firstChildElement("name");
    if (!dom_name.isNull()) name = dom_name.text();

    QDomElement dom_author = info.firstChildElement("author");
    if (!dom_author.isNull()) author = dom_author.text();

    QDomElement dom_description = info.firstChildElement("description");
    if (!dom_description.isNull()) description = dom_description.text();

    QDomElement dom_forums = info.firstChildElement("forums");
    if (!dom_forums.isNull()) forumlink = dom_forums.text();

    QDomElement dom_wiki = info.firstChildElement("wiki");
    if (!dom_wiki.isNull()) wikilink = dom_wiki.text();

    QDomElement devices = info.firstChildElement("devices");
    if (!devices.isNull()) {
        QDomElement product = devices.firstChildElement("product");
        while (!product.isNull()) {
            QString protocol = product.attribute("protocol","");
            if (protocol=="hid") {
                products.append(parseHIDProduct(product));
            } else if (protocol=="bulk") {
                products.append(parseBulkProduct(product));
            } else if (protocol=="midi") {
                qDebug("MIDI product info parsing not yet implemented");
                //products.append(parseMIDIProduct(product);
            } else if (protocol=="osc") {
                qDebug("OSC product info parsing not yet implemented");
                //products.append(parseOSCProduct(product);
            } else {
                qDebug("Product specification missing protocol attribute");
            }
            product = product.nextSiblingElement("product");
        }
    }
}

QHash<QString,QString> PresetInfo::parseBulkProduct(const QDomElement& element) const {
    // <product protocol="bulk" vendor_id="0x06f8" product_id="0x0b105" in_epaddr="0x82" out_epaddr="0x03">

    QHash<QString,QString> product;
    product.insert("protocol", element.attribute("protocol",""));
    product.insert("vendor_id", element.attribute("vendor_id",""));
    product.insert("product_id", element.attribute("product_id",""));
    product.insert("in_epaddr", element.attribute("in_epaddr",""));
    product.insert("out_epaddr", element.attribute("out_epaddr",""));
    return product;
}

QHash<QString,QString> PresetInfo::parseHIDProduct(const QDomElement& element) const {
    // HID device <product> element parsing. Example of valid element:
    //   <product protocol="hid" vendor_id="0x1" product_id="0x2" usage_page="0x3" usage="0x4" interface_number="0x3" />
    // All numbers must be hex prefixed with 0x
    // Only vendor_id and product_id fields are required to map a device.
    // usage_page and usage are matched on OS/X and windows
    // interface_number is matched on linux, which does support usage_page/usage

    QHash<QString,QString> product;
    product.insert("procotol", element.attribute("protocol",""));
    product.insert("vendor_id", element.attribute("vendor_id",""));
    product.insert("product_id", element.attribute("product_id",""));
    product.insert("usage_page", element.attribute("usage_page",""));
    product.insert("usage", element.attribute("usage",""));
    product.insert("interface_number", element.attribute("interface_number",""));
    return product;
}

QHash<QString,QString> PresetInfo::parseMIDIProduct(const QDomElement& element) const {
    // TODO - implement parsing of MIDI attributes
    // When done, remember to fix switch() above to call this
    QHash<QString,QString> product;
    product.insert("procotol",element.attribute("protocol",""));
    return product;
}

QHash<QString,QString> PresetInfo::parseOSCProduct(const QDomElement& element) const {
    // TODO - implement parsing of OSC product attributes
    // When done, remember to fix switch() above to call this
    QHash<QString,QString> product;
    product.insert("procotol",element.attribute("protocol",""));
    return product;
}

PresetInfoEnumerator::PresetInfoEnumerator(ConfigObject<ConfigValue> *pConfig)
    : m_pConfig(pConfig) {

    QString configPath = m_pConfig->getResourcePath();
    controllerDirPaths.append(configPath.append("controllers/"));
    controllerDirPaths.append(LOCAL_PRESETS_PATH);

    // Static list of supported default extensions, sorted by popularity
    fileExtensions.append(QString(".midi.xml"));
    fileExtensions.append(QString(".cntrlr.xml"));
    fileExtensions.append(QString(".hid.xml"));
    fileExtensions.append(QString(".bulk.xml"));
    fileExtensions.append(QString(".osc.xml"));

    loadSupportedPresets();
}

bool PresetInfoEnumerator::isValidExtension(const QString extension) {
    if (presetsByExtension.contains(extension))
        return true;
    return false;
}

bool PresetInfoEnumerator::hasPresetInfo(const QString extension, const QString name) {
    // Check if preset info matching extension and preset name can be found
    if (!isValidExtension(extension))
        return false;
    foreach (QString extension, presetsByExtension.keys()) {
        QMap <QString,PresetInfo> presets = presetsByExtension[extension];
        foreach (PresetInfo preset, presets.values())
            if (name == preset.getName())
                return true;
    }
    return false;
}

bool PresetInfoEnumerator::hasPresetInfo(const QString path) {
    foreach (QString extension, presetsByExtension.keys()) {
        QMap <QString,PresetInfo> presets = presetsByExtension[extension];
        if (presets.contains(path))
            return true;
    }
    return false;
}

PresetInfo PresetInfoEnumerator::getPresetInfo(const QString extension, const QString name) {
    QList<PresetInfo> extension_presets;
    if (!isValidExtension(extension))
        return PresetInfo();

    foreach (QString extension, presetsByExtension.keys()) {
        QMap <QString,PresetInfo> presets = presetsByExtension[extension];
        foreach (PresetInfo preset, presets.values())
            if (name == preset.getName())
                return preset;
    }
    return PresetInfo();
}

PresetInfo PresetInfoEnumerator::getPresetInfo(const QString path) {
    // Lookup and return controller script preset info by script path
    // Return NULL if path is not found.
    foreach (QString extension, presetsByExtension.keys()) {
        QMap <QString,PresetInfo> presets = presetsByExtension[extension];
        if (presets.contains(path))
            return presets[path];
    }
    return PresetInfo();
}

QList<PresetInfo> PresetInfoEnumerator::getPresets(const QString extension) {
    // Return list of PresetInfo items matching extension
    // Returns empty list if no matching extension presets can be found
    QList <PresetInfo> presets;
    if (presetsByExtension.contains(extension)) {
        presets = presetsByExtension[extension].values();
        return presetsByExtension[extension].values();
    }
    qDebug() << "Extension not registered to presetinfo" << extension;
    return presets;
}

void PresetInfoEnumerator::addExtension(const QString extension) {
    if (presetsByExtension.contains(extension))
        return;
    QMap <QString,PresetInfo> presets;
    presetsByExtension[extension] = presets;
}

void PresetInfoEnumerator::loadSupportedPresets() {

    foreach (QString dirPath, controllerDirPaths) {
        QDirIterator it(dirPath);
        while (it.hasNext()) {
            it.next();
            const QString path = it.filePath();
            foreach (QString extension, fileExtensions) {
                if (!path.endsWith(extension))
                    continue;
                if (!presetsByExtension.contains(extension)) {
                    addExtension(extension);
                }
                presetsByExtension[extension][path] = PresetInfo(path);
            }
        }
    }

    foreach (QString extension, presetsByExtension.keys()) {
        QMap <QString,PresetInfo> presets = presetsByExtension[extension];
        qDebug() << "Extension" << extension << "total" << presets.keys().length() << "presets";
    }
}

void PresetInfoEnumerator::updatePresets(const QString extension) {
    QMap <QString,PresetInfo> presets;

    if (presetsByExtension.contains(extension))
        presetsByExtension.remove(extension);

    foreach (QString dirPath, controllerDirPaths) {
        QDirIterator it(dirPath);
        while (it.hasNext()) {
            it.next();
            const QString path = it.filePath();
            if (!path.endsWith(extension))
                continue;
            presets[path] = PresetInfo(path);
        }
    }

    presetsByExtension[extension] = presets;
}
