#include "controllers/controllermappinginfo.h"

#include "controllers/defs_controllers.h"
#include "util/xml.h"

MappingInfo::MappingInfo()
        : m_valid(false) {
}

MappingInfo::MappingInfo(const QString& mapping_path)
        : m_valid(false) {
    // Parse <info> header section from a controller description XML file
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

    QDomElement dom_name = info.firstChildElement("name");
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
            QString protocol = product.attribute("protocol", "");
            if (protocol == "hid") {
                m_products.append(parseHIDProduct(product));
            } else if (protocol == "bulk") {
                m_products.append(parseBulkProduct(product));
            } else if (protocol == "midi") {
                qDebug("MIDI product info parsing not yet implemented");
                //m_products.append(parseMIDIProduct(product);
            } else if (protocol == "osc") {
                qDebug("OSC product info parsing not yet implemented");
                //m_products.append(parseOSCProduct(product);
            } else {
                qDebug("Product specification missing protocol attribute");
            }
            product = product.nextSiblingElement("product");
        }
    }
}

ProductInfo MappingInfo::parseBulkProduct(const QDomElement& element) const {
    // <product protocol="bulk" vendor_id="0x06f8" product_id="0x0b105" in_epaddr="0x82" out_epaddr="0x03">
    ProductInfo product;
    product.protocol = element.attribute("protocol");
    product.vendor_id = element.attribute("vendor_id");
    product.product_id = element.attribute("product_id");
    product.in_epaddr = element.attribute("in_epaddr");
    product.out_epaddr = element.attribute("out_epaddr");
    return product;
}

ProductInfo MappingInfo::parseHIDProduct(const QDomElement& element) const {
    // HID device <product> element parsing. Example of valid element:
    //   <product protocol="hid" vendor_id="0x1" product_id="0x2" usage_page="0x3" usage="0x4" interface_number="0x3" />
    // All numbers must be hex prefixed with 0x
    // Only vendor_id and product_id fields are required to map a device.
    // usage_page and usage are matched on OS/X and windows
    // interface_number is matched on linux, which does support usage_page/usage

    ProductInfo product;
    product.protocol = element.attribute("protocol");
    product.vendor_id = element.attribute("vendor_id");
    product.product_id = element.attribute("product_id");
    product.usage_page = element.attribute("usage_page");
    product.usage = element.attribute("usage");
    product.interface_number = element.attribute("interface_number");
    return product;
}
