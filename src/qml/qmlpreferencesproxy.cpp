#include "qml/qmlpreferencesproxy.h"

#ifndef Q_OS_ANDROID
#include <QVideoFrame>
#include <QVideoFrameFormat>
#endif
#include <QDir>
#include <algorithm>
#include <cstring>

#include "controllers/controller.h"
#include "controllers/controllermanager.h"
#include "controllers/defs_controllers.h"
#include "controllers/legacycontrollermappingfilehandler.h"
#include "controllers/legacycontrollersettings.h"
#include "qml/qmlconfigproxy.h"
#include "qml/qmllibraryproxy.h"
#include "util/assert.h"

namespace mixxx {
namespace qml {

QmlControllerScreenElement::QmlControllerScreenElement(QObject* parent, const LegacyControllerMapping::ScreenInfo& screen)
        : QObject(parent),
          m_screenInfo(screen),
          m_averageFrameDuration(0.0) {
}

int QmlControllerScreenElement::fps() const {
    if (m_averageFrameDuration <= 0.0) {
        return 0;
    }
    return static_cast<int>(1000.0 / m_averageFrameDuration);
}

void QmlControllerScreenElement::updateFrame(const LegacyControllerMapping::ScreenInfo& screen, const QImage& frame) {
    Q_UNUSED(screen);
    auto now = mixxx::Time::now();
    if (m_lastFrameTimestamp != mixxx::Time::time_point()) {
        double duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_lastFrameTimestamp).count();
        if (m_averageFrameDuration <= 0.0) {
            m_averageFrameDuration = duration;
        } else {
            m_averageFrameDuration = m_averageFrameDuration * 0.9 + duration * 0.1;
        }
        Q_EMIT fpsChanged();
    }
    m_lastFrameTimestamp = now;

#ifndef Q_OS_ANDROID
    QVideoFrameFormat format(frame.size(), QVideoFrameFormat::Format_BGRA8888);
    QVideoFrame vframe(format);
    if (vframe.map(QVideoFrame::WriteOnly)) {
        for (int y = 0; y < frame.height(); ++y) {
            std::memcpy(vframe.bits(0) + y * vframe.bytesPerLine(0),
                    frame.scanLine(y),
                    static_cast<size_t>(std::min(vframe.bytesPerLine(0),
                            static_cast<int>(frame.bytesPerLine()))));
        }
        vframe.unmap();
    }
    Q_EMIT videoFrameAvailable(vframe);
#endif
}

void QmlControllerScreenElement::clear() {
    m_lastFrameTimestamp = mixxx::Time::time_point();
    m_averageFrameDuration = 0.0;
    Q_EMIT fpsChanged();
}

QmlControllerSettingItem::QmlControllerSettingItem(LegacyControllerSettingsLayoutItem* pInternal, QObject* parent)
        : QmlControllerSettingElement(parent),
          m_pInternal(pInternal) {
    if (m_pInternal->setting()) {
        connect(m_pInternal->setting(),
                &AbstractLegacyControllerSetting::changed,
                this,
                &QmlControllerSettingElement::dirtyChanged);
    }
}

QString QmlControllerSettingItem::label() const {
    return m_pInternal->setting() ? m_pInternal->setting()->label() : QString();
}

QJSValue QmlControllerSettingItem::value() const {
    return m_pInternal->setting() ? m_pInternal->setting()->value() : QJSValue();
}

void QmlControllerSettingItem::setValue(const QJSValue& value) {
    if (m_pInternal->setting()) {
        m_pInternal->setting()->setValue(value);
    }
}

QJSValue QmlControllerSettingItem::savedValue() const {
    return m_pInternal->setting() ? m_pInternal->setting()->savedValue() : QJSValue();
}

QJSValue QmlControllerSettingItem::defaultValue() const {
    return m_pInternal->setting() ? m_pInternal->setting()->defaultValue() : QJSValue();
}

QString QmlControllerSettingItem::type() const {
    return m_pInternal->setting() ? m_pInternal->setting()->getType() : QString();
}

QmlControllerSettingContainer::QmlControllerSettingContainer(const LegacyControllerSettingsLayoutContainer* pInternal, QObject* parent)
        : QmlControllerSettingElement(parent),
          m_pInternal(pInternal) {
    const auto children = pInternal->children();
    for (auto* pChild : children) {
        if (auto* pItem = dynamic_cast<LegacyControllerSettingsLayoutItem*>(pChild)) {
            m_children.append(new QmlControllerSettingItem(pItem, this));
        } else if (auto* pContainer = dynamic_cast<LegacyControllerSettingsLayoutContainer*>(pChild)) {
            m_children.append(new QmlControllerSettingContainer(pContainer, this));
        }
    }
}

QQmlListProperty<QmlControllerSettingElement> QmlControllerSettingContainer::children() {
    return {this, &m_children};
}

LegacyControllerSettingsLayoutContainer::Disposition QmlControllerSettingContainer::disposition() const {
    return m_pInternal->disposition();
}

QmlControllerSettingGroup::QmlControllerSettingGroup(const LegacyControllerSettingsGroup* pInternal, QObject* parent)
        : QmlControllerSettingElement(parent),
          m_pInternal(pInternal) {
    const auto children = pInternal->children();
    for (auto* pChild : children) {
        if (auto* pItem = dynamic_cast<LegacyControllerSettingsLayoutItem*>(pChild)) {
            m_children.append(new QmlControllerSettingItem(pItem, this));
        } else if (auto* pContainer = dynamic_cast<LegacyControllerSettingsLayoutContainer*>(pChild)) {
            m_children.append(new QmlControllerSettingContainer(pContainer, this));
        }
    }
}

QString QmlControllerSettingGroup::label() const {
    return m_pInternal->label();
}

QQmlListProperty<QmlControllerSettingElement> QmlControllerSettingGroup::children() {
    return {this, &m_children};
}

LegacyControllerSettingsLayoutContainer::Disposition QmlControllerSettingGroup::disposition() const {
    return m_pInternal->disposition();
}

QmlControllerMappingProxy::QmlControllerMappingProxy(const MappingInfo& mapping, QObject* parent)
        : QObject(parent),
          m_mappingDefinition(mapping) {
}

QmlControllerSettingElement* QmlControllerMappingProxy::loadSettings(
        const QmlConfigProxy* pConfig, QmlControllerDeviceProxy* pController) {
    Q_UNUSED(pConfig);
    auto pMapping = pController->instanceFor(m_mappingDefinition.getPath());
    if (!pMapping) {
        return nullptr;
    }
    auto* pLayout = pMapping->getSettingsLayout();
    if (!pLayout) {
        return nullptr;
    }
    if (auto* pGroup = dynamic_cast<LegacyControllerSettingsGroup*>(pLayout)) {
        return new QmlControllerSettingGroup(pGroup, this);
    }
    if (auto* pContainer = dynamic_cast<LegacyControllerSettingsLayoutContainer*>(pLayout)) {
        return new QmlControllerSettingContainer(pContainer, this);
    }
    return nullptr;
}

QList<QmlControllerScreenElement*> QmlControllerMappingProxy::loadScreens(
        const QmlConfigProxy* pConfig, QmlControllerDeviceProxy* pController) {
    Q_UNUSED(pConfig);
    auto pMapping = pController->instanceFor(m_mappingDefinition.getPath());
    if (!pMapping) {
        return {};
    }
    QList<QmlControllerScreenElement*> screens;
    for (const auto& screen : pMapping->getInfoScreens()) {
        screens.append(new QmlControllerScreenElement(this, screen));
    }
    return screens;
}

void QmlControllerMappingProxy::resetSettings(QmlControllerDeviceProxy* pController) {
    auto pMapping = pController->instanceFor(m_mappingDefinition.getPath());
    if (pMapping) {
        pMapping->resetSettings();
    }
}

QString QmlControllerMappingProxy::getName() const {
    return m_mappingDefinition.getName();
}

QString QmlControllerMappingProxy::getAuthor() const {
    fetchMappingDetails();
    return m_mappingDefinition.getAuthor();
}

QString QmlControllerMappingProxy::getDescription() const {
    fetchMappingDetails();
    return m_mappingDefinition.getDescription();
}

QUrl QmlControllerMappingProxy::getForumLink() const {
    fetchMappingDetails();
    return QUrl(m_mappingDefinition.getForumLink());
}

QUrl QmlControllerMappingProxy::getWikiLink() const {
    fetchMappingDetails();
    return QUrl(m_mappingDefinition.getWikiLink());
}

bool QmlControllerMappingProxy::hasSettings() const {
    if (!m_hasSettings.has_value()) {
        fetchMappingDetails();
    }
    return m_hasSettings.value_or(false);
}

bool QmlControllerMappingProxy::hasScreens() const {
    if (!m_hasScreens.has_value()) {
        fetchMappingDetails();
    }
    return m_hasScreens.value_or(false);
}

bool QmlControllerMappingProxy::isUserMapping(const QmlConfigProxy* pConfig) const {
    Q_UNUSED(pConfig);
    return m_mappingDefinition.getPath().startsWith(userMappingsPath(QmlConfigProxy::get()));
}

void QmlControllerMappingProxy::fetchMappingDetails() const {
    auto pMapping = LegacyControllerMappingFileHandler::loadMapping(
            QFileInfo(m_mappingDefinition.getPath()),
            QDir(resourceMappingsPath(QmlConfigProxy::get())));
    if (pMapping) {
        m_mappingDefinition.setAuthor(pMapping->author());
        m_mappingDefinition.setDescription(pMapping->description());
        m_mappingDefinition.setForumLink(pMapping->forumlink());
        m_mappingDefinition.setWikiLink(pMapping->wikilink());
        m_hasSettings = pMapping->hasSettings();
        m_hasScreens = pMapping->hasScreens();
    } else {
        Q_EMIT const_cast<QmlControllerMappingProxy*>(this)->mappingErrored();
    }
}

QmlControllerDeviceProxy::QmlControllerDeviceProxy(Controller* pInternal,
        const std::optional<ProductInfo>& productInfo,
        const QList<QmlControllerMappingProxy*>& mappings,
        QObject* parent,
        ControllerManager* pControllerManager)
        : QObject(parent),
          m_edited(false),
          m_pInternal(pInternal),
          m_productInfo(productInfo),
          m_mappings(mappings),
          m_pMapping(nullptr),
          m_enabled(std::nullopt),
          m_pControllerManager(pControllerManager) {
    QString currentMappingPath = m_pControllerManager->getConfiguredMappingFileForDevice(pInternal->getName());
    for (auto* pMapping : mappings) {
        if (pMapping->definition().getPath() == currentMappingPath) {
            m_pMapping = pMapping;
            break;
        }
    }
}

QmlControllerDeviceProxy::Type QmlControllerDeviceProxy::getType() const {
    switch (m_pInternal->getDataRepresentationProtocol()) {
    case ::DataRepresentationProtocol::MIDI:
        return Type::MIDI;
    case ::DataRepresentationProtocol::HID:
        return Type::HID;
    default:
        return Type::BULK;
    }
}

QString QmlControllerDeviceProxy::getName() const {
    return m_pInternal->getName();
}

QmlControllerMappingProxy* QmlControllerDeviceProxy::getMapping() const {
    return m_pMapping;
}

void QmlControllerDeviceProxy::setMapping(QmlControllerMappingProxy* mapping) {
    if (m_pMapping != mapping) {
        m_pMapping = mapping;
        setEdited();
        Q_EMIT mappingChanged();
    }
}

bool QmlControllerDeviceProxy::getEnabled() const {
    return m_enabled.value_or(m_pInternal->isOpen());
}

void QmlControllerDeviceProxy::setEnabled(bool state) {
    if (getEnabled() != state) {
        m_enabled = state;
        setEdited();
        Q_EMIT enabledChanged();
    }
}

QString QmlControllerDeviceProxy::vendor() const {
    return m_pInternal->getVendorString();
}

QString QmlControllerDeviceProxy::product() const {
    return m_pInternal->getProductString();
}

QString QmlControllerDeviceProxy::serialNumber() const {
    return m_pInternal->getSerialNumber();
}

bool QmlControllerDeviceProxy::save(const QmlConfigProxy* pConfig) {
    Q_UNUSED(pConfig);
    if (!m_edited) {
        return true;
    }
    bool enabled = getEnabled();
    auto pMapping = m_pMapping ? instanceFor(m_pMapping->definition().getPath()) : nullptr;
    Q_EMIT mappingAssigned(m_pInternal, pMapping.get(), enabled);
    clearEdited();
    return true;
}

void QmlControllerDeviceProxy::clear() {
    m_pMapping = nullptr;
    QString currentMappingPath = m_pControllerManager->getConfiguredMappingFileForDevice(m_pInternal->getName());
    for (auto* pMapping : std::as_const(m_mappings)) {
        if (pMapping->definition().getPath() == currentMappingPath) {
            m_pMapping = pMapping;
            break;
        }
    }
    m_enabled = std::nullopt;
    clearEdited();
    Q_EMIT mappingChanged();
    Q_EMIT enabledChanged();
}

std::shared_ptr<LegacyControllerMapping> QmlControllerDeviceProxy::instanceFor(const QString& mappingPath) const {
    if (m_mappingInstance.contains(mappingPath)) {
        return m_mappingInstance.value(mappingPath);
    }
    auto pMapping = LegacyControllerMappingFileHandler::loadMapping(
            QFileInfo(mappingPath),
            QDir(resourceMappingsPath(QmlConfigProxy::get())));
    if (pMapping) {
        const_cast<QmlControllerDeviceProxy*>(this)->m_mappingInstance.insert(mappingPath, pMapping);
        return pMapping;
    }
    return nullptr;
}

void QmlControllerDeviceProxy::setInstanceFor(const QString& mappingPath, std::shared_ptr<LegacyControllerMapping> pMapping) {
    m_mappingInstance.insert(mappingPath, pMapping);
}

QmlControllerManagerProxy::QmlControllerManagerProxy(std::shared_ptr<ControllerManager> pControllerManager, QObject* parent)
        : QObject(parent),
          m_pControllerManager(std::move(pControllerManager)) {
}

QQmlListProperty<QmlControllerDeviceProxy> QmlControllerManagerProxy::knownDevices() {
    return {this, &m_knownDevicesFound};
}

QQmlListProperty<QmlControllerDeviceProxy> QmlControllerManagerProxy::unknownDevices() {
    return {this, &m_unknownDevicesFound};
}

bool QmlControllerManagerProxy::isControllerScreenDebug() const {
    return s_controllerPreviewScreens;
}

std::shared_ptr<ControllerManager> QmlControllerManagerProxy::internal() const {
    return m_pControllerManager;
}

// static
QmlControllerManagerProxy* QmlControllerManagerProxy::create(QQmlEngine* pQmlEngine, QJSEngine* pJsEngine) {
    Q_UNUSED(pJsEngine);
    if (!s_pControllerManager) {
        return nullptr;
    }
    auto* pProxy = new QmlControllerManagerProxy(s_pControllerManager, pQmlEngine);
    pProxy->refreshMappings();
    pProxy->refreshKnownDevices();
    return pProxy;
}

void QmlControllerManagerProxy::refreshKnownDevices() {
    qDeleteAll(m_knownDevicesFound);
    m_knownDevicesFound.clear();
    qDeleteAll(m_unknownDevicesFound);
    m_unknownDevicesFound.clear();

    m_knownControllers = m_pControllerManager->getControllers();
    for (auto* pInternal : std::as_const(m_knownControllers)) {
        auto type = QmlControllerDeviceProxy::Type::BULK;
        switch (pInternal->getDataRepresentationProtocol()) {
        case ::DataRepresentationProtocol::MIDI:
            type = QmlControllerDeviceProxy::Type::MIDI;
            break;
        case ::DataRepresentationProtocol::HID:
            type = QmlControllerDeviceProxy::Type::HID;
            break;
        default:
            break;
        }

        auto* pProxy = new QmlControllerDeviceProxy(pInternal,
                std::nullopt,
                m_knownMappings.value(type),
                this,
                m_pControllerManager.get());
        if (pProxy->getMapping()) {
            m_knownDevicesFound.append(pProxy);
        } else {
            m_unknownDevicesFound.append(pProxy);
        }
    }
    Q_EMIT deviceListChanged();
}

// Re-enumerates attached controllers (USB hot-plug) and refreshes the
// known/unknown device lists. Bound to the Rescan button in the
// Controller settings page (idea from JaviLabarumDJ/mixxx-android).
void QmlControllerManagerProxy::rescan() {
    if (m_pControllerManager) {
        m_pControllerManager->setUpDevices();
    }
    refreshKnownDevices();
}

void QmlControllerManagerProxy::refreshMappings() {
    m_knownMappings.clear();
    loadMappingFromEnumerator(m_pControllerManager->getMainThreadSystemMappingEnumerator());
    loadMappingFromEnumerator(m_pControllerManager->getMainThreadUserMappingEnumerator());
}

void QmlControllerManagerProxy::loadNewMapping(mixxx::qml::QmlControllerDeviceProxy::Type type, const MappingInfo& mapping) {
    auto* pMappingProxy = new QmlControllerMappingProxy(mapping, this);
    m_knownMappings[type].append(pMappingProxy);
    refreshKnownDevices();
}

void QmlControllerManagerProxy::updateExistingMapping(mixxx::qml::QmlControllerMappingProxy* pMapping, const MappingInfo& mapping) {
    Q_UNUSED(pMapping);
    Q_UNUSED(mapping);
    refreshMappings();
    refreshKnownDevices();
}

void QmlControllerManagerProxy::loadMappingFromEnumerator(QSharedPointer<MappingInfoEnumerator> enumerator) {
    if (!enumerator) {
        return;
    }

    const auto midiMappings = enumerator->getMappingsByExtension(MIDI_MAPPING_EXTENSION);
    for (const auto& mapping : midiMappings) {
        m_knownMappings[QmlControllerDeviceProxy::Type::MIDI].append(new QmlControllerMappingProxy(mapping, this));
    }

    const auto hidMappings = enumerator->getMappingsByExtension(HID_MAPPING_EXTENSION);
    for (const auto& mapping : hidMappings) {
        m_knownMappings[QmlControllerDeviceProxy::Type::HID].append(new QmlControllerMappingProxy(mapping, this));
    }

    const auto bulkMappings = enumerator->getMappingsByExtension(BULK_MAPPING_EXTENSION);
    for (const auto& mapping : bulkMappings) {
        m_knownMappings[QmlControllerDeviceProxy::Type::BULK].append(new QmlControllerMappingProxy(mapping, this));
    }
}

} // namespace qml
} // namespace mixxx

#include "moc_qmlpreferencesproxy.cpp"
