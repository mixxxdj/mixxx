#include "qmlpreferencesproxy.h"

#include <qglobal.h>
#include <qhash.h>
#include <qqmlengine.h>
#include <qstringliteral.h>
#include <qvideosink.h>

#include <QVideoFrame>
#include <QVideoFrameFormat>
#include <algorithm>
#include <cstdint>
#include <limits>
#include <memory>
#include <optional>
#include <utility>

#include "controllers/bulk/bulkcontroller.h"
#include "controllers/controller.h"
#include "controllers/controllermanager.h"
#include "controllers/controllermappinginfo.h"
#include "controllers/controllermappinginfoenumerator.h"
#include "controllers/defs_controllers.h"
#include "controllers/hid/hidcontroller.h"
#include "controllers/legacycontrollermappingfilehandler.h"
#include "controllers/legacycontrollersettings.h"
#include "controllers/legacycontrollersettingslayout.h"
#include "controllers/midi/midicontroller.h"
#include "controllers/scripting/legacy/controllerscriptenginelegacy.h"
#include "moc_qmlpreferencesproxy.cpp"
#include "qmlconfigproxy.h"
#include "util/assert.h"

namespace {
/// Number of sample frame timestamp sample to perform a smooth average FPS label.
constexpr double kFrameSmoothAverageFactor = 20;
} // namespace

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
    const auto children = pInternal->children();
    for (auto* pSetting : std::as_const(children)) {
        m_pChildren.append(QmlControllerSettingElement::build(pSetting, this));
        connect(m_pChildren.last(),
                &QmlControllerSettingElement::dirtyChanged,
                this,
                &QmlControllerSettingElement::dirtyChanged);
    }
}
bool QmlControllerSettingElement::isDirty() const {
    for (const auto& child : std::as_const(m_pChildren)) {
        if (child->isDirty()) {
            return true;
        }
    }
    return false;
}

AbstractLegacyControllerSetting* QmlControllerSettingItem::getSetting() const {
    return m_pInternal->setting();
}
bool QmlControllerSettingItem::isDirty() const {
    return m_pInternal->setting()->isDirty();
}

// Static
QmlControllerSettingElement* QmlControllerSettingElement::build(
        LegacyControllerSettingsLayoutElement* element, QObject* parent) {
    auto* pItem = dynamic_cast<LegacyControllerSettingsLayoutItem*>(element);
    if (pItem) {
        auto* pElement = new QmlControllerSettingItem(pItem, parent);
        connect(pItem->setting(),
                &AbstractLegacyControllerSetting::changed,
                pElement,
                &QmlControllerSettingElement::dirtyChanged);
        return pElement;
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

QmlControllerScreenElement::QmlControllerScreenElement(
        QObject* parent, const LegacyControllerMapping::ScreenInfo& screen)
        : QObject(parent),
          m_screenInfo(screen),
          m_averageFrameDuration(0) {
    clear();
}

void QmlControllerScreenElement::updateFrame(
        const LegacyControllerMapping::ScreenInfo& screen, const QImage& frame) {
    if (m_screenInfo.identifier != screen.identifier) {
        return;
    }
    auto currentTimestamp = Clock::now();
    if (m_lastFrameTimestamp == Clock::time_point()) {
        m_lastFrameTimestamp = currentTimestamp;
        return;
    }

    double averageFrameDuration;
    if (m_averageFrameDuration == std::numeric_limits<double>::max()) {
        averageFrameDuration =
                std::chrono::duration_cast<std::chrono::microseconds>(
                        currentTimestamp - m_lastFrameTimestamp)
                        .count();
    } else {
        averageFrameDuration = std::lerp(m_averageFrameDuration,
                std::chrono::duration_cast<std::chrono::microseconds>(
                        currentTimestamp - m_lastFrameTimestamp)
                        .count(),
                1.0 / kFrameSmoothAverageFactor);
    }
    m_lastFrameTimestamp = currentTimestamp;

    if (fps() != static_cast<int>(1000000 / averageFrameDuration)) {
        m_averageFrameDuration = averageFrameDuration;
        emit fpsChanged();
    } else {
        m_averageFrameDuration = averageFrameDuration;
    }
#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
    emit videoFrameAvailable(QVideoFrame(frame));
#else
    auto videoFrame = QVideoFrame(QVideoFrameFormat(frame.size(),
            QVideoFrameFormat::pixelFormatFromImageFormat(frame.format())));
    VERIFY_OR_DEBUG_ASSERT(videoFrame.isValid() &&
            videoFrame.map(QVideoFrame::MapMode::WriteOnly)) {
        return;
    }
    VERIFY_OR_DEBUG_ASSERT(videoFrame.mappedBytes(0) == frame.sizeInBytes()) {
        return;
    }
    std::memcpy(videoFrame.bits(0), frame.bits(), frame.sizeInBytes());
    videoFrame.unmap();
    emit videoFrameAvailable(videoFrame);
#endif
}

QList<QmlControllerScreenElement*> QmlControllerMappingProxy::loadScreens(
        const QmlConfigProxy* pConfig, mixxx::qml::QmlControllerDeviceProxy* pController) {
    VERIFY_OR_DEBUG_ASSERT(pController->internal()) {
        return {};
    }
    auto mapping = pController->instanceFor(m_mappingDefinition.getPath());
    if (!mapping) {
        mapping = LegacyControllerMappingFileHandler::loadMapping(
                QFileInfo(m_mappingDefinition.getPath()),
                QDir(resourceMappingsPath(pConfig->get())));
        mapping->loadSettings(pConfig->get(), pController->internal()->getName());
        pController->setInstanceFor(m_mappingDefinition.getPath(), mapping);
    }
    auto screens = mapping->getInfoScreens();
    auto* pScriptEngine = pController->internal()->getScriptEngine().get();

    QList<QmlControllerScreenElement*> screenElements;
    auto connectToEngine = [](const ControllerScriptEngineLegacy* engine,
                                   QmlControllerScreenElement* pElement) {
        connect(engine,
                &ControllerScriptEngineLegacy::previewRenderedScreen,
                pElement,
                &QmlControllerScreenElement::updateFrame);
    };
    for (const LegacyControllerMapping::ScreenInfo& screen : std::as_const(screens)) {
        auto* pElement = new QmlControllerScreenElement(this, screen);
        connect(pController->internal(),
                &Controller::engineStarted,
                pElement,
                [connectToEngine, pElement](const ControllerScriptEngineLegacy* engine) {
                    pElement->clear();
                    connectToEngine(engine, pElement);
                });
        if (pController->internal()->getScriptEngine()) {
            connectToEngine(pController->internal()->getScriptEngine().get(), pElement);
        }
        screenElements.push_back(pElement);
    }
    return screenElements;
}

QmlControllerSettingElement* QmlControllerMappingProxy::loadSettings(
        const QmlConfigProxy* pConfig, mixxx::qml::QmlControllerDeviceProxy* pController) {
    if (!hasSettings()) {
        return nullptr;
    }
    // Mappings settings need an underlying independent mapping instance per controller
    auto mapping = pController->instanceFor(m_mappingDefinition.getPath());
    if (!mapping) {
        mapping = LegacyControllerMappingFileHandler::loadMapping(
                QFileInfo(m_mappingDefinition.getPath()),
                QDir(resourceMappingsPath(pConfig->get())));
        mapping->loadSettings(pConfig->get(), pController->internal()->getName());
        pController->setInstanceFor(m_mappingDefinition.getPath(), mapping);
    }

    auto* pSettings = QmlControllerSettingElement::build(mapping->getSettingsLayout(), this);
    connect(pSettings,
            &QmlControllerSettingElement::dirtyChanged,
            pController,
            &QmlControllerDeviceProxy::setEdited,
            Qt::UniqueConnection);
    return pSettings;
}

void QmlControllerMappingProxy::resetSettings(mixxx::qml::QmlControllerDeviceProxy* pController) {
    if (!pController) {
        qmlEngine(this)->throwError(QStringLiteral("must pass a valid controller!"));
        return;
    }
    if (!hasSettings()) {
        return;
    }
    // Mappings settings need an underlying independent mapping instance per controller
    auto mapping = pController->instanceFor(m_mappingDefinition.getPath());
    if (mapping) {
        mapping->resetSettings();
    }
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

QUrl QmlControllerMappingProxy::getForumLink() const {
    return QUrl(m_mappingDefinition.getForumLink());
}

QUrl QmlControllerMappingProxy::getWikiLink() const {
    return QUrl(m_mappingDefinition.getWikiLink());
}

bool QmlControllerMappingProxy::hasSettings() const {
    return m_mappingDefinition.hasSettings();
}

bool QmlControllerMappingProxy::hasScreens() const {
    return m_mappingDefinition.hasScreens();
}

bool QmlControllerMappingProxy::isUserMapping(const QmlConfigProxy* pConfig) const {
    return m_mappingDefinition.getPath().startsWith(userMappingsPath(pConfig->get()));
}

QmlControllerMappingProxy::QmlControllerMappingProxy(
        const MappingInfo& mapping, QObject* parent)
        : QObject(parent),
          m_mappingDefinition(mapping) {
}

QmlControllerDeviceProxy::QmlControllerDeviceProxy(Controller* pInternal,
        const std::optional<ProductInfo>& productInfo,
        const QSet<QmlControllerMappingProxy*>& mappings,
        QObject* parent)
        : QObject(parent),
          m_pInternal(pInternal),
          m_productInfo(productInfo),
          m_mappings(mappings),
          m_edited(false),
          m_enabled(pInternal->isOpen()),
          m_pMapping(nullptr) {
    clear();
    connect(m_pInternal, &Controller::openChanged, this, &QmlControllerDeviceProxy::enabledChanged);
}

QmlControllerDeviceProxy::Type QmlControllerDeviceProxy::getType() const {
    return static_cast<QmlControllerDeviceProxy::Type>(
            m_pInternal->getDataRepresentationProtocol());
}
QString QmlControllerDeviceProxy::getName() const {
    if (!m_editedFriendlyName.trimmed().isEmpty()) {
        return m_editedFriendlyName;
    }
    if (m_productInfo.has_value() && !m_productInfo.value().friendlyName.trimmed().isEmpty()) {
        return m_productInfo.value().friendlyName;
    }
    return m_pInternal->getName();
}
void QmlControllerDeviceProxy::setName(const QString& name) {
    if (getName() == name) {
        return;
    }
    m_editedFriendlyName = name;
    setEdited();
    emit nameChanged();
}
QUrl QmlControllerDeviceProxy::getVisualUrl() const {
    if (m_editedVisualUrl.isValid()) {
        return m_editedVisualUrl;
    }
    return m_productInfo.value_or(ProductInfo{}).visualUrl;
}
void QmlControllerDeviceProxy::setVisualUrl(const QUrl& url) {
    if (getVisualUrl() == url) {
        return;
    }
    m_editedVisualUrl = url;
    setEdited();
    emit visualUrlChanged();
}
bool QmlControllerDeviceProxy::getEnabled() const {
    return m_enabled.value_or(m_pInternal->isOpen());
}
void QmlControllerDeviceProxy::setEnabled(bool state) {
    if (getEnabled() == state) {
        return;
    }
    m_enabled = state;
    setEdited();
    emit enabledChanged();
}
QmlControllerMappingProxy* QmlControllerDeviceProxy::getMapping() const {
    return m_pMapping;
}
void QmlControllerDeviceProxy::setMapping(QmlControllerMappingProxy* pMapping) {
    if (m_pMapping == pMapping) {
        return;
    }
    m_pMapping = pMapping;
    emit mappingChanged();
    setEdited();
}
QString QmlControllerDeviceProxy::vendor() const {
    uint16_t id = m_pInternal->getVendorId().value_or(0);
    return QStringLiteral("%1 (0x%2)")
            .arg(m_pInternal->getVendorString(),
                    id ? QString::number(id, 16).leftJustified(4, '0').toUpper()
                       : "N/A");
}
QString QmlControllerDeviceProxy::product() const {
    uint16_t id = m_pInternal->getProductId().value_or(0);
    return QStringLiteral("%1 (0x%2)")
            .arg(m_pInternal->getProductString(),
                    id ? QString::number(id, 16).leftJustified(4, '0').toUpper()
                       : "N/A");
}
QString QmlControllerDeviceProxy::serialNumber() const {
    auto sn = m_pInternal->getSerialNumber().trimmed();
    if (!sn.isEmpty()) {
        return sn;
    }
    return QStringLiteral("N/A");
}
void QmlControllerDeviceProxy::clear() {
    m_enabled = std::nullopt;
    emit enabledChanged();
    m_editedFriendlyName.clear();
    emit nameChanged();
    m_editedVisualUrl.clear();
    emit visualUrlChanged();

    std::shared_ptr<LegacyControllerMapping> pSelectedMapping = m_pInternal->getMapping();
    for (const auto& pMapping : std::as_const(m_mappings)) {
        if (pSelectedMapping && pSelectedMapping->filePath() == pMapping->definition().getPath()) {
            m_pMapping = pMapping;
        }
    }
    if (!m_mappings.isEmpty()) {
        m_pMapping = *m_mappings.cbegin();
    }
    emit mappingChanged();
    // FIXME if using a mapping that didn't match the product, we need to fetch
    // it from COntrollerManager if (pMapping && !m_pMapping){
    //     m_pMapping = new
    //     QmlControllerMappingProxy(pMapping->m_productMatches, this);
    // }
    clearEdited();
}
bool QmlControllerDeviceProxy::save(const QmlConfigProxy* pConfig) {
    if (!m_edited) {
        return true;
    }

    std::shared_ptr<LegacyControllerMapping> mapping;
    if (m_pMapping) {
        VERIFY_OR_DEBUG_ASSERT(m_pMapping) {
            return false;
        }
        mapping = instanceFor(m_pMapping->definition().getPath());
        if (!mapping) {
            mapping = LegacyControllerMappingFileHandler::loadMapping(
                    QFileInfo(m_pMapping->definition().getPath()),
                    QDir(resourceMappingsPath(pConfig->get())));
            setInstanceFor(m_pMapping->definition().getPath(), mapping);
        }
        VERIFY_OR_DEBUG_ASSERT(mapping) {
            return false;
        }
        if (m_pMapping->hasSettings()) {
            mapping->saveSettings(pConfig->get(), m_pInternal->getName());
        }

        bool mappingChanged = false;
        ProductInfo productInfo;
        if (!m_productInfo.has_value()) {
            mappingChanged = true;
            productInfo.vendor_id = QStringLiteral("0x%1").arg(
                    m_pInternal->getVendorId().value_or(0), 4, 16, QChar('0'));
            productInfo.product_id = QStringLiteral("0x%1").arg(
                    m_pInternal->getProductId().value_or(0), 4, 16, QChar('0'));
            productInfo.interface_number = QStringLiteral("0x%1").arg(
                    m_pInternal->getUsbInterfaceNumber().value_or(0),
                    1,
                    16,
                    QChar('0'));

            switch (m_pInternal->getDataRepresentationProtocol()) {
            case DataRepresentationProtocol::MIDI: {
                auto* pMidiController = dynamic_cast<MidiController*>(m_pInternal);
                VERIFY_OR_DEBUG_ASSERT(pMidiController) {
                    return false;
                }
                productInfo.protocol = QStringLiteral("midi");
                break;
            }
            case DataRepresentationProtocol::HID: {
                auto* pHidController = dynamic_cast<HidController*>(m_pInternal);
                VERIFY_OR_DEBUG_ASSERT(pHidController) {
                    return false;
                }
                productInfo.protocol = QStringLiteral("hid");
                productInfo.usage_page = QStringLiteral("0x%1").arg(
                        pHidController->getUsagePage(), 4, 16, QChar('0'));
                productInfo.usage = QStringLiteral("0x%1").arg(
                        pHidController->getUsage(), 2, 16, QChar('0'));
                break;
            }
            case DataRepresentationProtocol::USB_BULK_TRANSFER: {
                auto* pBulkController = dynamic_cast<BulkController*>(m_pInternal);
                VERIFY_OR_DEBUG_ASSERT(pBulkController) {
                    return false;
                }
                productInfo.protocol = QStringLiteral("bulk");
                productInfo.in_epaddr = QStringLiteral("0x%1").arg(
                        pBulkController->getInEndpointAddr(), 2, 16, QChar('0'));
                productInfo.out_epaddr = QStringLiteral("0x%1").arg(
                        pBulkController->getOutEndpointAddr(), 2, 16, QChar('0'));
                break;
            }
            default: {
                DEBUG_ASSERT(!"Unreachable");
                return false;
            }
            }
        } else {
            productInfo = m_productInfo.value();
        }

        // if visual, friendlyName or productMatching update
        if (m_editedVisualUrl.isValid()) {
            mappingChanged = true;
            productInfo.visualUrl = m_editedVisualUrl;
            m_editedVisualUrl.clear();
        }
        if (!m_editedFriendlyName.isEmpty()) {
            mappingChanged = true;
            productInfo.friendlyName = m_editedFriendlyName;
            m_editedFriendlyName.clear();
        }
        if (mappingChanged) {
            mapping->addProductMatch(productInfo);
            // write file
            if (m_pMapping->isUserMapping(pConfig)) {
                mapping->saveMapping(m_pMapping->definition().getPath());
                emit mappingUpdated(m_pMapping, MappingInfo(QFileInfo(m_pMapping->definition().getPath())));
            } else {
                auto newMappingFileName =
                        QDir(userMappingsPath(pConfig->get()))
                                .filePath(QFileInfo(
                                        m_pMapping->definition().getPath())
                                                .fileName());
                // FIXME check if file exist and don't overwrite/rename if edited
                mapping->saveMapping(newMappingFileName);
                emit mappingCreated(getType(), MappingInfo(QFileInfo(newMappingFileName)));
            }
            // If the device was "known" till now
            if (!m_productInfo.has_value()) {
                DEBUG_ASSERT(m_mappings.isEmpty());
                m_productInfo = productInfo;
                m_mappings.insert(m_pMapping);
                emit deviceLearned();
            }
            mapping->setDirty(false);
        }
    }

    emit mappingAssigned(m_pInternal, mapping, getEnabled());

    clearEdited();
    return true;
}

std::shared_ptr<LegacyControllerMapping> QmlControllerDeviceProxy::instanceFor(
        const QString& mappingPath) const {
    return m_mappingInstance.value(mappingPath, std::shared_ptr<LegacyControllerMapping>());
}
void QmlControllerDeviceProxy::setInstanceFor(const QString& mappingPath,
        std::shared_ptr<LegacyControllerMapping> mapping) {
    VERIFY_OR_DEBUG_ASSERT(mapping) {
        return;
    }
    m_mappingInstance.emplace(mappingPath, mapping);
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

    const auto mappingDeviceLoader =
            [enumerator, this](const QString& extension,
                    QmlControllerDeviceProxy::Type type) {
                const auto mappings = enumerator->getMappingsByExtension(extension);
                for (const auto& mapping :
                        std::as_const(mappings)) {
                    loadNewMapping(type, mapping);
                }
            };

    mappingDeviceLoader(HID_MAPPING_EXTENSION, QmlControllerDeviceProxy::Type::HID);
    mappingDeviceLoader(MIDI_MAPPING_EXTENSION, QmlControllerDeviceProxy::Type::MIDI);
    mappingDeviceLoader(BULK_MAPPING_EXTENSION, QmlControllerDeviceProxy::Type::BULK);
}
void QmlControllerManagerProxy::loadNewMapping(
        QmlControllerDeviceProxy::Type type, const MappingInfo& mapping) {
    auto* pMapping =
            new QmlControllerMappingProxy(mapping, this);
    updateExistingMapping(pMapping, mapping);
    m_knownMappings[type].push_back(pMapping);
}
void QmlControllerManagerProxy::updateExistingMapping(
        QmlControllerMappingProxy* pMapping, const MappingInfo& mapping) {
    for (const auto& product : mapping.getProducts()) {
        auto devices = m_knownDevices.value(
                product, QSet<QmlControllerMappingProxy*>());
        devices.insert(pMapping);
        if (m_knownDevices.contains(product)) {
        }
        m_knownDevices.emplace(product, devices);
    }
}
void QmlControllerManagerProxy::refreshMappings() {
    m_knownMappings.insert(QmlControllerDeviceProxy::Type::HID, {});
    m_knownMappings.insert(QmlControllerDeviceProxy::Type::MIDI, {});
    m_knownMappings.insert(QmlControllerDeviceProxy::Type::BULK, {});

    loadMappingFromEnumerator(m_pControllerManager->getMainThreadSystemMappingEnumerator());
    loadMappingFromEnumerator(m_pControllerManager->getMainThreadUserMappingEnumerator());
}
void QmlControllerManagerProxy::refreshKnownDevices() {
    const auto controllers = m_pControllerManager->getControllers();
    for (auto* pController : std::as_const(controllers)) {
        if (m_knownControllers.contains(pController)) {
            continue;
        }
        const auto vendor_id = QStringLiteral("0x%1").arg(
                pController->getVendorId().value_or(0), 4, 16, QChar('0'));
        const auto product_id = QStringLiteral("0x%1").arg(
                pController->getProductId().value_or(0), 4, 16, QChar('0'));
        const auto interface_number = QStringLiteral("0x%1").arg(
                pController->getUsbInterfaceNumber().value_or(0),
                1,
                16,
                QChar('0'));

        auto foundDevice = std::find_if(m_knownDevices.constKeyValueBegin(),
                m_knownDevices.constKeyValueEnd(),
                [vendor_id, product_id, interface_number](const auto& it) {
                    return it.first.vendor_id == vendor_id &&
                            it.first.product_id == product_id &&
                            it.first.interface_number == interface_number;
                    // TODO add protocol
                });
        m_knownControllers.append(pController);
        QmlControllerDeviceProxy* device;
        if (foundDevice != m_knownDevices.constKeyValueEnd()) {
            device = new QmlControllerDeviceProxy(
                    pController, foundDevice->first, foundDevice->second, this);
            m_knownDevicesFound.append(device);
        } else {
            device = new QmlControllerDeviceProxy(
                    pController, std::nullopt, {}, this);

            connect(device,
                    &QmlControllerDeviceProxy::deviceLearned,
                    this,
                    [this, device]() {
                        VERIFY_OR_DEBUG_ASSERT(!m_knownDevicesFound.contains(device)) {
                            return;
                        }
                        const auto vendor_id = QStringLiteral("0x%1").arg(
                                device->internal()->getVendorId().value_or(0), 4, 16, QChar('0'));
                        const auto product_id = QStringLiteral("0x%1").arg(
                                device->internal()->getProductId().value_or(0), 4, 16, QChar('0'));
                        const auto interface_number = QStringLiteral("0x%1").arg(
                                device->internal()->getUsbInterfaceNumber().value_or(0),
                                1,
                                16,
                                QChar('0'));
                        auto foundDevice = std::find_if(m_knownDevices.constKeyValueBegin(),
                                m_knownDevices.constKeyValueEnd(),
                                [vendor_id, product_id, interface_number](const auto& it) {
                                    return it.first.vendor_id == vendor_id &&
                                            it.first.product_id == product_id &&
                                            it.first.interface_number == interface_number;
                                    // TODO add protocol
                                });
                        VERIFY_OR_DEBUG_ASSERT(foundDevice != m_knownDevices.constKeyValueEnd()) {
                            return;
                        }
                        m_knownDevicesFound.append(device);
                        device->setMappings(foundDevice->second);
                        device->clear();
                        DEBUG_ASSERT(m_unknownDevicesFound.contains(device));
                        m_unknownDevicesFound.removeAll(device);
                        emit deviceListChanged();
                    });
            m_unknownDevicesFound.append(device);
        }
        connect(device,
                &QmlControllerDeviceProxy::mappingAssigned,
                m_pControllerManager.get(),
                &ControllerManager::slotApplyMapping);
        connect(device,
                &QmlControllerDeviceProxy::mappingCreated,
                this,
                &QmlControllerManagerProxy::loadNewMapping);
        connect(device,
                &QmlControllerDeviceProxy::mappingUpdated,
                this,
                &QmlControllerManagerProxy::updateExistingMapping);
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
