#pragma once

#include <qglobal.h>
#include <qqmlintegration.h>
#include <qstringliteral.h>
#include <qtmetamacros.h>

#include <QDialog>
#include <QObject>
#include <QQmlEngine>
#include <memory>
#include <optional>

#include "controllers/controllermappinginfo.h"
#include "controllers/controllermappinginfoenumerator.h"
#include "controllers/legacycontrollersettingslayout.h"
#include "qmlconfigproxy.h"

class ControllerManager;
class LegacyControllerMapping;
class AbstractLegacyControllerSetting;
class Controller;

namespace mixxx {
namespace qml {

class QmlControllerSettingElement : public QObject {
    Q_OBJECT
    Q_PROPERTY(QList<QmlControllerSettingElement*> children MEMBER m_pChildren CONSTANT)
    Q_PROPERTY(QString type READ getType CONSTANT)
    QML_ANONYMOUS
    QML_UNCREATABLE("Use Mixxx.ControllerSettingElement to get devices")
  public:
    explicit QmlControllerSettingElement(
            const LegacyControllerSettingsLayoutElement* pInternal,
            QObject* parent);
    virtual QString getType() const = 0;

    static QmlControllerSettingElement* build(
            LegacyControllerSettingsLayoutElement* element, QObject* parent);

  protected:
    QList<QmlControllerSettingElement*> m_pChildren;
};

class QmlControllerSettingItem : public QmlControllerSettingElement {
    Q_OBJECT
    Q_PROPERTY(AbstractLegacyControllerSetting* properties READ getSetting CONSTANT)
    Q_PROPERTY(LegacyControllerSettingsLayoutContainer::Disposition
                    preferredOrientation READ preferredOrientation CONSTANT)
    QML_ANONYMOUS
    QML_UNCREATABLE("Use Mixxx.ControllerSettingElement to get devices")
  public:
    explicit QmlControllerSettingItem(
            const LegacyControllerSettingsLayoutItem* pInternal,
            QObject* parent);
    QString getType() const override {
        return QStringLiteral("item");
    }
    AbstractLegacyControllerSetting* getSetting() const;
    LegacyControllerSettingsLayoutContainer::Disposition preferredOrientation() const {
        return m_pInternal->preferredOrientation();
    }

  private:
    const LegacyControllerSettingsLayoutItem* m_pInternal;
};

class QmlControllerSettingContainer : public QmlControllerSettingElement {
    Q_OBJECT
    Q_PROPERTY(LegacyControllerSettingsLayoutContainer::Disposition disposition
                    READ disposition CONSTANT)
    Q_PROPERTY(LegacyControllerSettingsLayoutContainer::Disposition
                    widgetOrientation READ widgetOrientation CONSTANT)
    QML_ANONYMOUS
    QML_UNCREATABLE("Use Mixxx.ControllerSettingElement to get devices")
  public:
    explicit QmlControllerSettingContainer(
            const LegacyControllerSettingsLayoutContainer* pInternal,
            QObject* parent);
    QString getType() const override {
        return QStringLiteral("container");
    }
    LegacyControllerSettingsLayoutContainer::Disposition disposition() const {
        return m_pInternal->disposition();
    }
    LegacyControllerSettingsLayoutContainer::Disposition widgetOrientation() const {
        return m_pInternal->widgetOrientation();
    }

  private:
    const LegacyControllerSettingsLayoutContainer* m_pInternal;
};

class QmlControllerSettingGroup : public QmlControllerSettingElement {
    Q_OBJECT
    Q_PROPERTY(QString label READ label CONSTANT)
    QML_ANONYMOUS
    QML_UNCREATABLE("Use Mixxx.ControllerSettingElement to get devices")
  public:
    explicit QmlControllerSettingGroup(
            const LegacyControllerSettingsGroup* pInternal, QObject* parent);
    QString getType() const override {
        return QStringLiteral("group");
    }
    QString label() const {
        return m_pInternal->label();
    }

  private:
    const LegacyControllerSettingsGroup* m_pInternal;
};

class QmlControllerMappingProxy : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString name READ getName CONSTANT)
    Q_PROPERTY(QString author READ getAuthor CONSTANT)
    Q_PROPERTY(QString description READ getDescription CONSTANT)
    Q_PROPERTY(QString forumLink READ getForumLink CONSTANT)
    Q_PROPERTY(QString wikiLink READ getWikiLink CONSTANT)
    Q_PROPERTY(bool hasSettings READ hasSettings CONSTANT)
    QML_NAMED_ELEMENT(SoundDevice)
    QML_UNCREATABLE("Use Mixxx.ControllerManager to get devices")
  public:
    enum class Type {
        HID,
        MIDI,
    };
    Q_ENUM(Type)
    explicit QmlControllerMappingProxy(const MappingInfo& mapping, QObject* parent);

    Q_INVOKABLE mixxx::qml::QmlControllerSettingElement* getSettingTree(
            const mixxx::qml::QmlConfigProxy* config);
    QString getName() const;
    QString getAuthor() const;
    QString getDescription() const;
    QString getForumLink() const;
    QString getWikiLink() const;
    bool hasSettings() const;

    const MappingInfo& definition() const {
        return m_mappingDefinition;
    }

  private:
    MappingInfo m_mappingDefinition;
    std::shared_ptr<LegacyControllerMapping> m_pInternal;
    QmlControllerSettingElement* m_settingRoot;
};

class QmlControllerDeviceProxy : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString name READ getName CONSTANT)
    Q_PROPERTY(QString sinceVersion READ getSinceVersion CONSTANT)
    Q_PROPERTY(QString vendor READ vendor CONSTANT)
    Q_PROPERTY(QString product READ product CONSTANT)
    Q_PROPERTY(QString serialNumber READ serialNumber CONSTANT)
    Q_PROPERTY(QUrl visualUrl READ getVisualUrl WRITE setVisualUrl NOTIFY visualUrlChanged)
    Q_PROPERTY(bool enabled READ isEnabled CONSTANT)
    Q_PROPERTY(mixxx::qml::QmlControllerDeviceProxy::Type type READ getType CONSTANT)
    Q_PROPERTY(QUrl image READ getImage CONSTANT)
    Q_PROPERTY(QList<QmlControllerMappingProxy*> availableMappings MEMBER m_mappings CONSTANT)
    QML_NAMED_ELEMENT(ControllerDevice)
    QML_UNCREATABLE("Use Mixxx.ControllerManager to get devices")
  public:
    enum class Type {
        MIDI,
        HID,
        BULK,
    };
    Q_ENUM(Type)
    explicit QmlControllerDeviceProxy(Controller* pInternal,
            const std::optional<ProductInfo>& productInfo,
            const QList<MappingInfo>& mappings,
            QObject* parent);
    mixxx::qml::QmlControllerDeviceProxy::Type getType() const;
    QString getName() const;
    QUrl getImage() const;
    QString getSinceVersion() const;
    QUrl getVisualUrl() const {
        return m_productInfo.value_or(ProductInfo{}).visualUrl;
    }
    void setVisualUrl(const QUrl& url) {
        if (!m_productInfo.has_value()) {
            m_productInfo = ProductInfo{
                    .visualUrl = url};
        } else {
            m_productInfo.value().visualUrl = url;
        }
        emit visualUrlChanged();
    }
    bool isEnabled() const;
    QString vendor() const;
    QString product() const;
    QString serialNumber() const;
  signals:
    void visualUrlChanged();

  private:
    Controller* m_pInternal;
    std::optional<ProductInfo> m_productInfo;
    QList<QmlControllerMappingProxy*> m_mappings;
};

class QmlControllerManagerProxy : public QObject {
    Q_OBJECT
    QML_NAMED_ELEMENT(ControllerManager)
    Q_PROPERTY(QList<QmlControllerDeviceProxy*> knownDevices MEMBER
                    m_knownDevicesFound NOTIFY deviceListChanged)
    Q_PROPERTY(QList<QmlControllerDeviceProxy*> unknownDevices MEMBER
                    m_unknownDevicesFound NOTIFY deviceListChanged)
    QML_SINGLETON
  public:
    explicit QmlControllerManagerProxy(
            std::shared_ptr<ControllerManager> pControllerManager,
            QObject* parent = nullptr);

    QQmlListProperty<QmlControllerDeviceProxy> knownDevices();
    QQmlListProperty<QmlControllerDeviceProxy> unknownDevices();

    std::shared_ptr<ControllerManager> internal() const;

    Q_INVOKABLE QList<mixxx::qml::QmlControllerMappingProxy*> mappings(
            mixxx::qml::QmlControllerDeviceProxy::Type type) const {
        return m_knownMappings.value(type);
    }

    static QmlControllerManagerProxy* create(QQmlEngine* pQmlEngine, QJSEngine* pJsEngine);
    static void registerManager(std::shared_ptr<ControllerManager> pManager) {
        s_pControllerManager = std::move(pManager);
    }

  private slots:
    void refreshKnownDevices();
    void refreshMappings();

  signals:
    void deviceListChanged();

  private:
    static inline std::shared_ptr<ControllerManager> s_pControllerManager;

    void loadMappingFromEnumerator(QSharedPointer<MappingInfoEnumerator> enumerator);

    QList<Controller*> m_knownControllers;
    QHash<ProductInfo, QList<MappingInfo>> m_knownDevices;
    QHash<QmlControllerDeviceProxy::Type, QList<QmlControllerMappingProxy*>> m_knownMappings;
    QList<QmlControllerDeviceProxy*> m_knownDevicesFound;
    QList<QmlControllerDeviceProxy*> m_unknownDevicesFound;

    std::shared_ptr<ControllerManager> m_pControllerManager;
};

} // namespace qml
} // namespace mixxx
