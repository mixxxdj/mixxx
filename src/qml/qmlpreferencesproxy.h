#pragma once

#ifndef QMLPREFERENCESPROXY_H
#define QMLPREFERENCESPROXY_H

#include <QImage>
#include <QJSValue>
#include <QList>
#include <QObject>
#include <QQmlEngine>
#include <QQmlListProperty>
#include <QUrl>
#include <memory>
#include <optional>

#ifndef Q_OS_ANDROID
#include <QVideoFrame>
#endif

#include "controllers/controllermappinginfo.h"
#include "controllers/controllermappinginfoenumerator.h"
#include "controllers/legacycontrollermapping.h"
#include "controllers/legacycontrollersettingslayout.h"
#include "qml/qmltrackproxy.h"
#include "util/time.h"

class Controller;
class ControllerManager;

namespace mixxx {
namespace qml {

class QmlConfigProxy;
class QmlControllerDeviceProxy;
class QmlControllerMappingProxy;

class QmlControllerSettingElement : public QObject {
    Q_OBJECT
  public:
    using QObject::QObject;
    ~QmlControllerSettingElement() override = default;

  signals:
    void dirtyChanged();
};

class QmlControllerScreenElement : public QObject {
    Q_OBJECT
    Q_PROPERTY(int fps READ fps NOTIFY fpsChanged)
  public:
    QmlControllerScreenElement(QObject* parent, const LegacyControllerMapping::ScreenInfo& screen);

    int fps() const;

    void updateFrame(const LegacyControllerMapping::ScreenInfo& screen, const QImage& frame);
    void clear();

  signals:
    void fpsChanged();
#ifndef Q_OS_ANDROID
    void videoFrameAvailable(const QVideoFrame& frame);
#endif

  private:
    LegacyControllerMapping::ScreenInfo m_screenInfo;
    mixxx::Time::time_point m_lastFrameTimestamp;
    double m_averageFrameDuration;
};

class QmlControllerSettingItem : public QmlControllerSettingElement {
    Q_OBJECT
    Q_PROPERTY(QString label READ label CONSTANT)
    Q_PROPERTY(QJSValue value READ value WRITE setValue NOTIFY dirtyChanged)
    Q_PROPERTY(QJSValue savedValue READ savedValue CONSTANT)
    Q_PROPERTY(QJSValue defaultValue READ defaultValue CONSTANT)
    Q_PROPERTY(QString type READ type CONSTANT)
  public:
    QmlControllerSettingItem(LegacyControllerSettingsLayoutItem* pInternal, QObject* parent);

    QString label() const;
    QJSValue value() const;
    void setValue(const QJSValue& value);
    QJSValue savedValue() const;
    QJSValue defaultValue() const;
    QString type() const;

  private:
    LegacyControllerSettingsLayoutItem* m_pInternal;
};

class QmlControllerSettingContainer : public QmlControllerSettingElement {
    Q_OBJECT
    Q_PROPERTY(QQmlListProperty<mixxx::qml::QmlControllerSettingElement> children READ children CONSTANT)
    Q_PROPERTY(LegacyControllerSettingsLayoutContainer::Disposition disposition READ disposition CONSTANT)
  public:
    QmlControllerSettingContainer(const LegacyControllerSettingsLayoutContainer* pInternal, QObject* parent);

    QQmlListProperty<mixxx::qml::QmlControllerSettingElement> children();
    LegacyControllerSettingsLayoutContainer::Disposition disposition() const;

  private:
    const LegacyControllerSettingsLayoutContainer* m_pInternal;
    QList<mixxx::qml::QmlControllerSettingElement*> m_children;
};

class QmlControllerSettingGroup : public QmlControllerSettingElement {
    Q_OBJECT
    Q_PROPERTY(QString label READ label CONSTANT)
    Q_PROPERTY(QQmlListProperty<mixxx::qml::QmlControllerSettingElement> children READ children CONSTANT)
    Q_PROPERTY(LegacyControllerSettingsLayoutContainer::Disposition disposition READ disposition CONSTANT)
  public:
    QmlControllerSettingGroup(const LegacyControllerSettingsGroup* pInternal, QObject* parent);

    QString label() const;
    QQmlListProperty<mixxx::qml::QmlControllerSettingElement> children();
    LegacyControllerSettingsLayoutContainer::Disposition disposition() const;

  private:
    const LegacyControllerSettingsGroup* m_pInternal;
    QList<mixxx::qml::QmlControllerSettingElement*> m_children;
};

class QmlControllerMappingProxy : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString name READ getName CONSTANT)
    Q_PROPERTY(QString author READ getAuthor CONSTANT)
    Q_PROPERTY(QString description READ getDescription CONSTANT)
    Q_PROPERTY(QUrl forumLink READ getForumLink CONSTANT)
    Q_PROPERTY(QUrl wikiLink READ getWikiLink CONSTANT)
    Q_PROPERTY(bool hasSettings READ hasSettings CONSTANT)
    Q_PROPERTY(bool hasScreens READ hasScreens CONSTANT)
  public:
    QmlControllerMappingProxy(const MappingInfo& mapping, QObject* parent);

    Q_INVOKABLE mixxx::qml::QmlControllerSettingElement* loadSettings(
            const mixxx::qml::QmlConfigProxy* pConfig,
            mixxx::qml::QmlControllerDeviceProxy* pController);
    Q_INVOKABLE QList<mixxx::qml::QmlControllerScreenElement*> loadScreens(
            const mixxx::qml::QmlConfigProxy* pConfig,
            mixxx::qml::QmlControllerDeviceProxy* pController);
    Q_INVOKABLE void resetSettings(mixxx::qml::QmlControllerDeviceProxy* pController);

    QString getName() const;
    QString getAuthor() const;
    QString getDescription() const;
    QUrl getForumLink() const;
    QUrl getWikiLink() const;
    bool hasSettings() const;
    bool hasScreens() const;

    Q_INVOKABLE bool isUserMapping(const mixxx::qml::QmlConfigProxy* pConfig) const;

    const MappingInfo& definition() const {
        return m_mappingDefinition;
    }

  signals:
    void mappingErrored();

  private:
    void fetchMappingDetails() const;

    mutable MappingInfo m_mappingDefinition;
    mutable std::optional<bool> m_hasSettings;
    mutable std::optional<bool> m_hasScreens;
};

class QmlControllerDeviceProxy : public QObject {
    Q_OBJECT
    Q_PROPERTY(mixxx::qml::QmlControllerDeviceProxy::Type type READ getType CONSTANT)
    Q_PROPERTY(QString name READ getName CONSTANT)
    Q_PROPERTY(mixxx::qml::QmlControllerMappingProxy* mapping READ getMapping WRITE setMapping NOTIFY mappingChanged)
    Q_PROPERTY(bool enabled READ getEnabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(QString vendor READ vendor CONSTANT)
    Q_PROPERTY(QString product READ product CONSTANT)
    Q_PROPERTY(QString serialNumber READ serialNumber CONSTANT)
    Q_PROPERTY(QList<mixxx::qml::QmlControllerMappingProxy*> mappings READ getMappings CONSTANT)
  public:
    enum class Type {
        MIDI,
        HID,
        BULK,
    };
    Q_ENUM(Type);

    QmlControllerDeviceProxy(Controller* pInternal,
            const std::optional<ProductInfo>& productInfo,
            const QList<mixxx::qml::QmlControllerMappingProxy*>& mappings,
            QObject* parent,
            ControllerManager* pControllerManager);

    Type getType() const;
    QString getName() const;
    mixxx::qml::QmlControllerMappingProxy* getMapping() const;
    void setMapping(mixxx::qml::QmlControllerMappingProxy* mapping);
    bool getEnabled() const;
    void setEnabled(bool state);
    QString vendor() const;
    QString product() const;
    QString serialNumber() const;
    QList<mixxx::qml::QmlControllerMappingProxy*> getMappings() const {
        return m_mappings;
    }

    Q_INVOKABLE bool isEdited() const {
        return m_edited;
    }
    Q_INVOKABLE bool save(const mixxx::qml::QmlConfigProxy* pConfig);
    Q_INVOKABLE void clear();

    std::shared_ptr<LegacyControllerMapping> instanceFor(const QString& mappingPath) const;
    void setInstanceFor(const QString& mappingPath, std::shared_ptr<LegacyControllerMapping> pMapping);

  signals:
    void mappingChanged();
    void enabledChanged();
    void mappingAssigned(Controller* pInternal, LegacyControllerMapping* pMapping, bool enabled);
    void mappingCreated(mixxx::qml::QmlControllerDeviceProxy::Type type, const MappingInfo& mapping);
    void mappingUpdated(mixxx::qml::QmlControllerMappingProxy* pMapping, const MappingInfo& mapping);

  private:
    void setEdited() {
        if (!m_edited) {
            m_edited = true;
        }
    }
    void clearEdited() {
        m_edited = false;
    }

    bool m_edited;
    Controller* m_pInternal;
    std::optional<ProductInfo> m_productInfo;
    QList<mixxx::qml::QmlControllerMappingProxy*> m_mappings;
    mixxx::qml::QmlControllerMappingProxy* m_pMapping;
    std::optional<bool> m_enabled;
    ControllerManager* m_pControllerManager;
    mutable QHash<QString, std::shared_ptr<LegacyControllerMapping>> m_mappingInstance;
};

using QmlControllerMappingProxyList = QList<QmlControllerMappingProxy*>;

class QmlControllerManagerProxy : public QObject {
    Q_OBJECT
    Q_PROPERTY(QQmlListProperty<mixxx::qml::QmlControllerDeviceProxy> knownDevices READ knownDevices NOTIFY deviceListChanged)
    Q_PROPERTY(QQmlListProperty<mixxx::qml::QmlControllerDeviceProxy> unknownDevices READ unknownDevices NOTIFY deviceListChanged)
    Q_PROPERTY(bool controllerScreenDebug READ isControllerScreenDebug CONSTANT)
  public:
    QmlControllerManagerProxy(std::shared_ptr<ControllerManager> pControllerManager, QObject* parent = nullptr);

    QQmlListProperty<mixxx::qml::QmlControllerDeviceProxy> knownDevices();
    QQmlListProperty<mixxx::qml::QmlControllerDeviceProxy> unknownDevices();
    bool isControllerScreenDebug() const;

    std::shared_ptr<ControllerManager> internal() const;

    static QmlControllerManagerProxy* create(QQmlEngine* pQmlEngine, QJSEngine* pJsEngine);
    static inline void registerManager(std::shared_ptr<ControllerManager> pControllerManager, bool controllerPreviewScreens = false) {
        s_pControllerManager = std::move(pControllerManager);
        s_controllerPreviewScreens = controllerPreviewScreens;
    }

  signals:
    void deviceListChanged();

  public slots:
    void refreshKnownDevices();
    Q_INVOKABLE void rescan();
    void refreshMappings();
    void loadNewMapping(mixxx::qml::QmlControllerDeviceProxy::Type type, const MappingInfo& mapping);
    void updateExistingMapping(mixxx::qml::QmlControllerMappingProxy* pMapping, const MappingInfo& mapping);

  private:
    void loadMappingFromEnumerator(QSharedPointer<MappingInfoEnumerator> enumerator);

    std::shared_ptr<ControllerManager> m_pControllerManager;
    QList<Controller*> m_knownControllers;
    QHash<mixxx::qml::QmlControllerDeviceProxy::Type, QList<mixxx::qml::QmlControllerMappingProxy*>> m_knownMappings;

    QList<mixxx::qml::QmlControllerDeviceProxy*> m_knownDevicesFound;
    QList<mixxx::qml::QmlControllerDeviceProxy*> m_unknownDevicesFound;

    static inline std::shared_ptr<ControllerManager> s_pControllerManager = nullptr;
    static inline bool s_controllerPreviewScreens = false;
};

} // namespace qml
} // namespace mixxx

Q_DECLARE_METATYPE(mixxx::qml::QmlControllerDeviceProxy::Type)

#endif // QMLPREFERENCESPROXY_H
