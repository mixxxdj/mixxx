#include "qmlpreferencesproxy.h"

#include <qglobal.h>
#include <qhash.h>
#include <qqmlengine.h>
#include <qstringliteral.h>

#include <algorithm>
#include <cstdint>
#include <memory>
#include <optional>

#include "controllers/controller.h"
#include "controllers/controllermanager.h"
#include "controllers/controllermappinginfo.h"
#include "controllers/controllermappinginfoenumerator.h"
#include "controllers/defs_controllers.h"
#include "controllers/legacycontrollermappingfilehandler.h"
#include "controllers/legacycontrollersettingslayout.h"
#include "moc_qmlpreferencesproxy.cpp"
#include "qmlconfigproxy.h"
#include "util/assert.h"

namespace mixxx {
namespace qml {

QmlControllerSettingItem::QmlControllerSettingItem(
        const LegacyControllerSettingsLayoutItem* pInternal, QObject* parent)
        : QmlControllerSettingElement(pInternal, parent),
          m_pInternal(pInternal) {
}
QmlControllerSettingContainer::QmlControllerSettingContainer(
        const LegacyControllerSettingsLayoutContainer* pInternal,
        QObject* parent)
        : QmlControllerSettingElement(pInternal, parent),
          m_pInternal(pInternal) {
}
QmlControllerSettingGroup::QmlControllerSettingGroup(
        const LegacyControllerSettingsGroup* pInternal, QObject* parent)
        : QmlControllerSettingElement(pInternal, parent),
          m_pInternal(pInternal) {
}
QmlControllerSettingElement::QmlControllerSettingElement(
        const LegacyControllerSettingsLayoutElement* pInternal, QObject* parent)
        : QObject(parent) {
    for (const auto& setting : pInternal->children()) {
        m_pChildren.append(QmlControllerSettingElement::build(setting, this));
    }
}

AbstractLegacyControllerSetting* QmlControllerSettingItem::getSetting() const {
    return m_pInternal->setting();
}

// Static
QmlControllerSettingElement* QmlControllerSettingElement::build(
        LegacyControllerSettingsLayoutElement* element, QObject* parent) {
    auto* pItem = dynamic_cast<LegacyControllerSettingsLayoutItem*>(element);
    if (pItem) {
        return new QmlControllerSettingItem(pItem, parent);
    }
    auto* pGroup = dynamic_cast<LegacyControllerSettingsGroup*>(element);
    if (pGroup) {
        return new QmlControllerSettingGroup(pGroup, parent);
    }
    auto* pContainer = dynamic_cast<LegacyControllerSettingsLayoutContainer*>(element);
    if (pContainer) {
        return new QmlControllerSettingContainer(pContainer, parent);
    }
    DEBUG_ASSERT(!"Unreachable");
    return nullptr;
}

QmlControllerSettingElement* QmlControllerMappingProxy::getSettingTree(
        const QmlConfigProxy* config) {
    if (!hasSettings()) {
        return nullptr;
    }
    // Need actual instance
    if (!m_pInternal) {
        m_pInternal = LegacyControllerMappingFileHandler::loadMapping(
                QFileInfo(m_mappingDefinition.getPath()),
                QDir(resourceMappingsPath(config->internal())));
    }

    if (!m_settingRoot) {
        m_settingRoot = QmlControllerSettingElement::build(m_pInternal->getSettingsLayout(), this);
    }
    // qDebug() << instance->getSettings() << instance->getSettingsLayout();
    return m_settingRoot;
}

QString QmlControllerMappingProxy::getName() const {
    return m_mappingDefinition.getName();
}

QString QmlControllerMappingProxy::getAuthor() const {
    return m_mappingDefinition.getAuthor();
}

QString QmlControllerMappingProxy::getDescription() const {
    return m_mappingDefinition.getDescription();
}

QString QmlControllerMappingProxy::getForumLink() const {
    return m_mappingDefinition.getForumLink();
}

QString QmlControllerMappingProxy::getWikiLink() const {
    return m_mappingDefinition.getWikiLink();
}

bool QmlControllerMappingProxy::hasSettings() const {
    return m_mappingDefinition.hasSettings();
}

QmlControllerMappingProxy::QmlControllerMappingProxy(
        const MappingInfo& mapping, QObject* parent)
        : QObject(parent),
          m_settingRoot(nullptr),
          m_pInternal(nullptr),
          m_mappingDefinition(mapping) {
}

QmlControllerDeviceProxy::QmlControllerDeviceProxy(Controller* pInternal,
        const std::optional<ProductInfo>& productInfo,
        const QList<MappingInfo>& mappings,
        QObject* parent)
        : QObject(parent),
          m_pInternal(pInternal),
          m_productInfo(productInfo),
          m_mappings() {
    for (const auto& mapping : mappings) {
        m_mappings.append(new QmlControllerMappingProxy(mapping, this));
    }
}

QmlControllerDeviceProxy::Type QmlControllerDeviceProxy::getType() const {
    return static_cast<QmlControllerDeviceProxy::Type>(
            m_pInternal->getDataRepresentationProtocol());
}
QString QmlControllerDeviceProxy::getName() const {
    if (m_productInfo.has_value() && !m_productInfo.value().friendlyName.trimmed().isEmpty()) {
        return m_productInfo.value().friendlyName;
    }
    return m_pInternal->getName();
}
QUrl QmlControllerDeviceProxy::getImage() const {
    // if (m_productInfo.has_value() &&
    // !m_productInfo.value().friendlyName.trimmed().isEmpty()){
    //     return AsyncImageProvider::mappingLocationToControllerUrl(const
    //     QString& mappingLocation);
    // }
    return {};
}
bool QmlControllerDeviceProxy::isEnabled() const {
    return m_pInternal->isOpen();
}
QString QmlControllerDeviceProxy::vendor() const {
    uint16_t id = m_pInternal->getVendorId().value_or(0);
    return QStringLiteral("%1 (%2)").arg(m_pInternal->getVendorString(), id ? QString::number(id, 16).leftJustified(4, '0').toUpper() : "N/A");
}
QString QmlControllerDeviceProxy::product() const {
    uint16_t id = m_pInternal->getProductId().value_or(0);
    return QStringLiteral("%1 (%2)").arg(m_pInternal->getProductString(), id ? QString::number(id, 16).leftJustified(4, '0').toUpper() : "N/A");
}
QString QmlControllerDeviceProxy::serialNumber() const {
    auto sn = m_pInternal->getSerialNumber().trimmed();
    if (!sn.isEmpty()){
        return sn;
    }
    return QStringLiteral("N/A");
}
QString QmlControllerDeviceProxy::getSinceVersion() const {
    QVersionNumber version;
    for (const auto& mapping : m_mappings) {
        const auto& currentVersion = mapping->definition().getMixxxVersion();
        if (currentVersion.isNull()) {
            continue;
        }
        if (version.isNull() || version > currentVersion) {
            version = currentVersion;
        }
    }
    return QStringLiteral("%1.%2").arg(QString::number(version.majorVersion()),
            QString::number(version.minorVersion()));
}

QmlControllerManagerProxy::QmlControllerManagerProxy(
        std::shared_ptr<ControllerManager> pControllerManager,
        QObject* parent)
        : QObject(parent),
          m_pControllerManager(pControllerManager),
          m_knownDevicesFound(),
          m_unknownDevicesFound() {
    refreshMappings();

    connect(m_pControllerManager.get(),
            &ControllerManager::initialized,
            this,
            &QmlControllerManagerProxy::refreshMappings);

    refreshKnownDevices();

    connect(m_pControllerManager.get(),
            &ControllerManager::devicesChanged,
            this,
            &QmlControllerManagerProxy::refreshKnownDevices);
}

void QmlControllerManagerProxy::loadMappingFromEnumerator(
        QSharedPointer<MappingInfoEnumerator> enumerator) {
    if (!enumerator) {
        return;
    }

    enumerator->loadSupportedMappings();

    for (const auto& mapping : enumerator->getMappingsByExtension(HID_MAPPING_EXTENSION)) {
        for (const auto& product : mapping.getProducts()) {
            auto devices = m_knownDevices.value(product, QList<MappingInfo>());
            devices.append(mapping);
            qDebug() << "ADD DEVICE" << product;
            if (m_knownDevices.contains(product)) {
                qDebug() << "contains" << product;
            }
            m_knownDevices.emplace(product, devices);
        }
        m_knownMappings[QmlControllerDeviceProxy::Type::HID].push_back(
                new QmlControllerMappingProxy(mapping, this));
    }

    for (const auto& mapping : enumerator->getMappingsByExtension(MIDI_MAPPING_EXTENSION)) {
        for (const auto& product : mapping.getProducts()) {
            auto devices = m_knownDevices.value(product, QList<MappingInfo>());
            devices.append(mapping);
            if (m_knownDevices.contains(product)) {
                qDebug() << "contains" << product;
            }
            m_knownDevices.emplace(product, devices);
        }
        m_knownMappings[QmlControllerDeviceProxy::Type::MIDI].push_back(
                new QmlControllerMappingProxy(mapping, this));
    }

    for (const auto& mapping : enumerator->getMappingsByExtension(BULK_MAPPING_EXTENSION)) {
        for (const auto& product : mapping.getProducts()) {
            auto devices = m_knownDevices.value(product, QList<MappingInfo>());
            devices.append(mapping);
            if (m_knownDevices.contains(product)) {
                qDebug() << "contains" << product;
            }
            m_knownDevices.emplace(product, devices);
        }
        m_knownMappings[QmlControllerDeviceProxy::Type::BULK].push_back(
                new QmlControllerMappingProxy(mapping, this));
    }
}
void QmlControllerManagerProxy::refreshMappings() {
    m_knownMappings.insert(QmlControllerDeviceProxy::Type::HID, {});
    m_knownMappings.insert(QmlControllerDeviceProxy::Type::MIDI, {});
    m_knownMappings.insert(QmlControllerDeviceProxy::Type::BULK, {});

    loadMappingFromEnumerator(m_pControllerManager->getMainThreadSystemMappingEnumerator());
    loadMappingFromEnumerator(m_pControllerManager->getMainThreadUserMappingEnumerator());

    qDebug() << "Total known devices:" << m_knownDevices.count();
    qDebug() << "Total HID mappings:"
             << m_knownMappings[QmlControllerDeviceProxy::Type::HID].count();
    qDebug() << "Total MIDI mappings:"
             << m_knownMappings[QmlControllerDeviceProxy::Type::MIDI].count();
    qDebug() << "Total BULK mappings:"
             << m_knownMappings[QmlControllerDeviceProxy::Type::BULK].count();
}
void QmlControllerManagerProxy::refreshKnownDevices() {
    for (const auto& controller : m_pControllerManager->getControllers()) {
        if (m_knownControllers.contains(controller)) {
            continue;
        }
        const auto vendor_id = QStringLiteral("0x%1").arg(
                controller->getVendorId().value_or(0), 4, 16, QChar('0'));
        const auto product_id = QStringLiteral("0x%1").arg(
                controller->getProductId().value_or(0), 4, 16, QChar('0'));
        const auto interface_number = QStringLiteral("0x%1").arg(
                controller->getUsbInterfaceNumber().value_or(0),
                1,
                16,
                QChar('0'));

        auto foundDevice = std::find_if(m_knownDevices.constKeyValueBegin(),
                m_knownDevices.constKeyValueEnd(),
                [vendor_id, product_id, interface_number](const auto& it) {
                    return it.first.vendor_id == vendor_id &&
                            it.first.product_id == product_id &&
                            it.first.interface_number == interface_number;
                });
        m_knownControllers.append(controller);
        if (foundDevice != m_knownDevices.constKeyValueEnd()) {
            qDebug() << "FOUND DEVICE!" << controller << foundDevice->first
                     << "with" << foundDevice->second.count() << "mappings";
            m_knownDevicesFound.append(new QmlControllerDeviceProxy(
                    controller, foundDevice->first, foundDevice->second, this));
        } else {
            // m_unknownDevicesFound
            m_unknownDevicesFound.append(new QmlControllerDeviceProxy(
                    controller, std::nullopt, {}, this));
            qDebug()
                    << "NOT FOUND DEVICE!"
                    << QStringLiteral(
                               "Controller<protocol=%1, name=%2, "
                               "vendor_id=0x%3, product_id=0x%4, "
                               "interface_number=0x%5>")
                               .arg(static_cast<int>(controller
                                               ->getDataRepresentationProtocol()))
                               .arg(controller->getName(),
                                       vendor_id,
                                       product_id,
                                       interface_number);
        }
    }
    emit deviceListChanged();
}

QQmlListProperty<QmlControllerDeviceProxy> QmlControllerManagerProxy::knownDevices() {
    return {this, &m_knownDevicesFound};
}

QQmlListProperty<QmlControllerDeviceProxy> QmlControllerManagerProxy::unknownDevices() {
    return {this, &m_unknownDevicesFound};
}

// static
QmlControllerManagerProxy* QmlControllerManagerProxy::create(
        QQmlEngine* pQmlEngine,
        QJSEngine*) {
    // The instance has to exist before it is used. We cannot replace it.
    VERIFY_OR_DEBUG_ASSERT(s_pControllerManager) {
        qWarning() << "ControllerManager hasn't been registered yet";
        return nullptr;
    }
    return new QmlControllerManagerProxy(s_pControllerManager, pQmlEngine);
}

std::shared_ptr<ControllerManager> QmlControllerManagerProxy::internal() const {
    return m_pControllerManager;
}

} // namespace qml
} // namespace mixxx
