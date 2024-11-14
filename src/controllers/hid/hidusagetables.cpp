#include "controllers/hid/hidusagetables.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonValue>

namespace {
// The HID Usage Tables 1.5 PDF specifies that the vendor-defined Usage-Page
// range is 0xFF00 to 0xFFFF.
constexpr uint16_t kStartOfVendorDefinedUsagePageRange = 0xFF00;

} // namespace

namespace mixxx {

namespace hid {

HidUsageTables::HidUsageTables(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open HID usage tables file:" << filePath;
        m_hidUsageTables = QJsonObject();
        return;
    }
    QByteArray fileData = file.readAll();
    QJsonDocument jsonDoc = QJsonDocument::fromJson(fileData);
    m_hidUsageTables = jsonDoc.object();
}

QString HidUsageTables::getUsagePageDescription(unsigned short usagePage) const {
    if (usagePage >= kStartOfVendorDefinedUsagePageRange) {
        return QStringLiteral("Vendor-defined");
    }

    const QJsonArray usagePages = m_hidUsageTables.value("UsagePages").toArray();
    for (const QJsonValue& pageValue : usagePages) {
        QJsonObject pageObject = pageValue.toObject();
        if (pageObject.value("Id").toInt() == usagePage) {
            return pageObject.value("Name").toString();
        }
    }
    return QStringLiteral("Reserved");
}

QString HidUsageTables::getUsageDescription(unsigned short usagePage, unsigned short usage) const {
    if (usagePage >= kStartOfVendorDefinedUsagePageRange) {
        return QStringLiteral("Vendor-defined");
    }

    const QJsonArray usagePages = m_hidUsageTables.value("UsagePages").toArray();
    for (const QJsonValue& pageValue : usagePages) {
        QJsonObject pageObject = pageValue.toObject();
        if (pageObject.value("Id").toInt() == usagePage) {
            const QJsonArray usageIds = pageObject.value("UsageIds").toArray();
            for (const QJsonValue& usageValue : usageIds) {
                QJsonObject usageObject = usageValue.toObject();
                if (usageObject.value("Id").toInt() == usage) {
                    return usageObject.value("Name").toString();
                }
            }
            break; // No need to continue if the usage page is found
        }
    }
    return QStringLiteral("Reserved");
}

} // namespace hid

} // namespace mixxx
