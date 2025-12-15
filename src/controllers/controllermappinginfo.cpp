#include "controllers/controllermappinginfo.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QLoggingCategory>
#include <QXmlStreamReader>

Q_LOGGING_CATEGORY(kLogger, "controllers.mappinginfo")

MappingInfo::MappingInfo(const QFileInfo& fileInfo) {
    // Parse only the <info> header section from a controller description XML file
    // using a streaming parser to avoid loading/parsing the entire (potentially
    // very large) XML file.
    // Contents parsed by xml path:
    // info.name        Mapping name, used for drop down menus in dialogs
    // info.author      Mapping author
    // info.description Mapping description
    // info.forums      Link to mixxx forum discussion for the mapping
    // info.wiki        Link to mixxx wiki for the mapping
    // info.devices.product List of device matches, specific to device type
    m_path = fileInfo.absoluteFilePath();
    m_dirPath = fileInfo.dir().absolutePath();

    QFile file(m_path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qCWarning(kLogger) << "ERROR opening" << m_path;
        return;
    }

    QXmlStreamReader xml(&file);
    bool inInfo = false;
    bool inInfoDevices = false;
    int xmlHierachyDepth = 0;

    while (!xml.atEnd()) {
        QXmlStreamReader::TokenType token = xml.readNext();
        if (token == QXmlStreamReader::StartElement) {
            ++xmlHierachyDepth;
            const QStringView xmlElementName = xml.name();

            // Accept info block only as a top-level child of the document root
            // root element will have depth == 1, so its direct children have depth == 2
            if (!inInfo && xmlElementName == QStringLiteral("info") && xmlHierachyDepth == 2) {
                inInfo = true;
                // We've found the info block; set valid now and start parsing children
                m_valid = true;
                continue;
            }

            // Parse simple info children when inside the info block
            if (inInfo) {
                if (!inInfo && xmlElementName == QStringLiteral("info") && xmlHierachyDepth == 2) {
                    m_name = xml.readElementText();
                    continue;
                } else if (xmlElementName == QStringLiteral("author")) {
                    m_author = xml.readElementText();
                    continue;
                } else if (xmlElementName == QStringLiteral("description")) {
                    m_description = xml.readElementText();
                    continue;
                } else if (xmlElementName == QStringLiteral("forums")) {
                    m_forumlink = xml.readElementText();
                    continue;
                } else if (xmlElementName == QStringLiteral("wiki")) {
                    m_wikilink = xml.readElementText();
                    continue;
                } else if (xmlElementName == QStringLiteral("devices")) {
                    // Enter devices block
                    inInfoDevices = true;
                    continue;
                } else if (inInfoDevices && xmlElementName == QStringLiteral("product")) {
                    QXmlStreamAttributes xmlElementAttributes = xml.attributes();
                    ProductInfo product;
                    QStringView protocol = xmlElementAttributes
                                                   .value(QStringLiteral("protocol"));

                    if (protocol == QStringLiteral("hid")) {
                        product = parseHIDProduct(xml.attributes());
                    } else if (protocol == QStringLiteral("bulk")) {
                        product = parseBulkProduct(xml.attributes());
                    } else if (protocol == QStringLiteral("midi")) {
                        qCInfo(kLogger) << "MIDI product info parsing not yet implemented";
                    } else {
                        qCCritical(kLogger)
                                << "Product info element contains missing or "
                                   "invalid protocol attribute";
                    }

                    if (!product.protocol.isEmpty()) {
                        m_products.append(std::move(product));
                    }
                    continue;
                }
            }

        } else if (token == QXmlStreamReader::EndElement) {
            const QString name = xml.name().toString();

            if (inInfoDevices && name == QStringLiteral("devices")) {
                inInfoDevices = false;
                --xmlHierachyDepth;
                continue;
            }
            if (inInfo && name == QStringLiteral("info")) {
                // End of info block; we stop file-reading/parsing entirely
                // Stopping here saves several hundreds milliseconds of startup time
                break;
            }

            --xmlHierachyDepth;
        }
    }

    if (xml.hasError()) {
        qCWarning(kLogger) << "ERROR parsing" << m_path << ":" << xml.errorString();
    }

    file.close();

    if (m_name.isEmpty()) {
        m_name = fileInfo.baseName();
    }
}

ProductInfo MappingInfo::parseBulkProduct(const QXmlStreamAttributes& xmlElementAttributes) {
    // <product protocol="bulk" vendor_id="0x06f8" product_id="0x0b105"
    // in_epaddr="0x82" out_epaddr="0x03" interface_number="0x04" />
    // All number strings must be hex prefixed with 0x

    ProductInfo productInfo;
    productInfo.protocol = xmlElementAttributes.value(QStringLiteral("protocol")).toString();
    productInfo.vendor_id = xmlElementAttributes.value(QStringLiteral("vendor_id")).toString();
    productInfo.product_id = xmlElementAttributes.value(QStringLiteral("product_id")).toString();
    // Check for HID-specific attributes which are not allowed for bulk protocol
    if (xmlElementAttributes.hasAttribute(QStringLiteral("usage_page"))) {
        qCCritical(kLogger) << "Product info element for bulk contains "
                               "unallowed UsagePage attribute";
    }
    if (xmlElementAttributes.hasAttribute(QStringLiteral("usage"))) {
        qCCritical(kLogger) << "Product info element for bulk contains unallowed Usage attribute";
    }
    productInfo.in_epaddr = xmlElementAttributes.value(QStringLiteral("in_epaddr")).toString();
    productInfo.out_epaddr = xmlElementAttributes.value(QStringLiteral("out_epaddr")).toString();
    productInfo.interface_number =
            xmlElementAttributes.value(QStringLiteral("interface_number"))
                    .toString();
    return productInfo;
}

ProductInfo MappingInfo::parseHIDProduct(const QXmlStreamAttributes& xmlElementAttributes) {
    // HID device <product> element parsing. Example of valid element:
    //   <product protocol="hid" vendor_id="0x1" product_id="0x2" usage_page="0x3" usage="0x4" interface_number="0x3" />
    // All number strings must be hex prefixed with 0x

    ProductInfo productInfo;
    productInfo.protocol = xmlElementAttributes.value(QStringLiteral("protocol")).toString();
    productInfo.vendor_id = xmlElementAttributes.value(QStringLiteral("vendor_id")).toString();
    productInfo.product_id = xmlElementAttributes.value(QStringLiteral("product_id")).toString();
    productInfo.usage_page = xmlElementAttributes.value(QStringLiteral("usage_page")).toString();
    productInfo.usage = xmlElementAttributes.value(QStringLiteral("usage")).toString();
    // Check for bulk-specific attributes which are not allowed for hid protocol
    if (xmlElementAttributes.hasAttribute(QStringLiteral("in_epaddr"))) {
        qCCritical(kLogger) << "Product info element for hid contains "
                               "unallowed in_epaddr attribute";
    }
    if (xmlElementAttributes.hasAttribute(QStringLiteral("out_epaddr"))) {
        qCCritical(kLogger) << "Product info element for hid contains "
                               "unallowed out_epaddr attribute";
    }
    productInfo.interface_number =
            xmlElementAttributes.value(QStringLiteral("interface_number"))
                    .toString();
    return productInfo;
}
