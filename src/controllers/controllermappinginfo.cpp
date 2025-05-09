#include "controllers/controllermappinginfo.h"

#include <qglobal.h>
#include <qregularexpression.h>
#include <qstringliteral.h>

#include <QDir>
#include <QFileInfo>

#include "util/xml.h"

namespace {
QVersionNumber sanitizeVersion(QString rawVersion) {
    return !rawVersion.isEmpty()
            ? QVersionNumber::fromString(rawVersion.remove(QChar('+')))
            : QVersionNumber();
}
} // namespace

bool operator==(const ProductInfo& a, const ProductInfo& b) {
    return a.protocol == b.protocol &&
            a.vendor_id == b.vendor_id &&
            a.product_id == b.product_id &&
            a.interface_number == b.interface_number &&
            a.usage_page == b.usage_page &&
            a.usage == b.usage &&
            a.in_epaddr == b.in_epaddr &&
            a.out_epaddr == b.out_epaddr;
}

size_t qHash(const ProductInfo& product) {
    return qHash(product.protocol) +
            qHash(product.vendor_id) +
            qHash(product.product_id) +
            qHash(product.interface_number) +
            qHash(product.usage_page) +
            qHash(product.usage) +
            qHash(product.in_epaddr) +
            qHash(product.out_epaddr);
}

QDebug operator<<(QDebug dbg, const ProductInfo& product) {
    dbg << QStringLiteral(
            "ProductInfo<protocol=%1, friendlyName=%2, vendor_id=%3, "
            "product_id=%4, interface_number=%5>")
                    .arg(product.protocol,
                            product.friendlyName,
                            product.vendor_id,
                            product.product_id,
                            product.interface_number);
    return dbg;
}
MappingInfo::MappingInfo()
        : m_valid(false) {
}

MappingInfo::MappingInfo(const QString& mapping_path)
        : m_valid(false) {
    // Parse <info> header section from a controller description XML file\
    // Contents parsed by xml path:
    // info.name        Mapping name, used for drop down menus in dialogs
    // info.author      Mapping author
    // info.description Mapping description
    // info.forums      Link to mixxx forum discussion for the mapping
    // info.wiki        Link to mixxx wiki for the mapping
    // info.devices.product List of device matches, specific to device type
    QFileInfo fileInfo(mapping_path);
    m_path = fileInfo.absoluteFilePath();
    m_dirPath = fileInfo.dir().absolutePath();
    m_name = "";
    m_author = "";
    m_description = "";
    m_forumlink = "";
    m_wikilink = "";

    QDomElement root = XmlParse::openXMLFile(m_path, "controller");
    if (root.isNull()) {
        qWarning() << "ERROR parsing" << m_path;
        return;
    }
    QDomElement info = root.firstChildElement("info");
    if (info.isNull()) {
        qDebug() << "MISSING <info> ELEMENT: " << m_path;
        return;
    }

    m_valid = true;

    m_hasSettings = !root.firstChildElement("settings").isNull();

    auto dom_mixxx_version = root.attribute("mixxxVersion");
    if (!dom_mixxx_version.isEmpty()) {
        m_mixxxVersion = sanitizeVersion(dom_mixxx_version);
    } else {
        m_mixxxVersion = QVersionNumber();
    }

    QDomElement dom_name = info.firstChildElement("friendly_name");
    if (!dom_name.isNull()) {
        m_name = dom_name.text();
    } else {
        m_name = fileInfo.baseName();
    }

    QDomElement dom_author = info.firstChildElement("author");
    if (!dom_author.isNull()) {
        m_author = dom_author.text();
    }

    QDomElement dom_description = info.firstChildElement("description");
    if (!dom_description.isNull()) {
        m_description = dom_description.text();
    }

    QDomElement dom_forums = info.firstChildElement("forums");
    if (!dom_forums.isNull()) {
        m_forumlink = dom_forums.text();
    }

    QDomElement dom_wiki = info.firstChildElement("wiki");
    if (!dom_wiki.isNull()) {
        m_wikilink = dom_wiki.text();
    }

    QDomElement devices = info.firstChildElement("devices");
    if (!devices.isNull()) {
        QDomElement product = devices.firstChildElement("product");
        while (!product.isNull()) {
            auto device = parseGenericProduct(product);
            if (device.protocol == "hid") {
                parseHIDProduct(product, device);
            } else if (device.protocol == "bulk") {
                parseBulkProduct(product, device);
            } else if (device.protocol == "midi") {
                qDebug("MIDI product info parsing not yet implemented");
                //m_products.append(parseMIDIProduct(product);
            } else if (device.protocol == "osc") {
                qDebug("OSC product info parsing not yet implemented");
                //m_products.append(parseOSCProduct(product);
            } else {
                qDebug("Product specification missing protocol attribute");
                product = product.nextSiblingElement("product");
                continue;
            }
            m_products.append(device);
            product = product.nextSiblingElement("product");
        }
    }
}

ProductInfo MappingInfo::parseGenericProduct(const QDomElement& element) const {
    // <product protocol="..." vendor_id="0x06f8" product_id="0x0b105"
    // interface_number="0x04" ... />
    ProductInfo product;
    product.protocol = element.attribute("protocol");
    product.vendor_id = element.attribute("vendor_id");
    product.product_id = element.attribute("product_id");
    product.interface_number = element.attribute("interface_number");
    product.friendlyName = element.attribute("friendly_name");

    product.visualUrl = QUrl(element.attribute("image"));
    return product;
}

void MappingInfo::parseBulkProduct(const QDomElement& element, ProductInfo& product) const {
    // <product ... in_epaddr="0x82" out_epaddr="0x03" />
    product.in_epaddr = element.attribute("in_epaddr");
    product.out_epaddr = element.attribute("out_epaddr");
}

void MappingInfo::parseHIDProduct(const QDomElement& element, ProductInfo& product) const {
    // HID device <product> element parsing. Example of valid element:
    //   <product ... usage_page="0x3" usage="0x4"/>
    // All numbers must be hex prefixed with 0x
    // Only vendor_id and product_id fields are required to map a device.
    // usage_page and usage are matched on OS/X and windows
    // interface_number is matched on linux, which does support usage_page/usage

    product.usage_page = element.attribute("usage_page");
    product.usage = element.attribute("usage");
}
