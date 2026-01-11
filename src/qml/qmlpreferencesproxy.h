#pragma once

#include <QDialog>
#include <QObject>
#include <QQmlEngine>
#include <QVideoSink>
#include <memory>
#include <optional>

#include "controllers/controllermappinginfo.h"
#include "controllers/controllermappinginfoenumerator.h"
#include "controllers/legacycontrollermapping.h"
#include "controllers/legacycontrollersettingslayout.h"
#include "qmlconfigproxy.h"

class ControllerManager;
class LegacyControllerMapping;
class AbstractLegacyControllerSetting;
class Controller;

namespace mixxx {
namespace qml {

class QmlControllerScreenElement : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString identifier READ identifier CONSTANT)
    Q_PROPERTY(QSize size READ size CONSTANT)
    Q_PROPERTY(int targetFps READ targetFps CONSTANT)
    Q_PROPERTY(int currentFps READ fps NOTIFY fpsChanged)
    QML_ANONYMOUS
  public:
    explicit QmlControllerScreenElement(
            QObject* parent, const LegacyControllerMapping::ScreenInfo& screen);
    const QString& identifier() const {
        return m_screenInfo.identifier;
    }
    const QSize& size() const {
        return m_screenInfo.size;
    }
    int targetFps() const {
        return m_screenInfo.target_fps;
    }
    int fps() const {
        return static_cast<int>(
                1000000 / m_averageFrameDuration);
    }
    Q_INVOKABLE void connectVideoSink(QVideoSink* videoSink) {
        disconnect(videoSink);
        connect(this,
                &QmlControllerScreenElement::videoFrameAvailable,
                videoSink,
                &QVideoSink::setVideoFrame);
    }
  signals:
    void fpsChanged();
    void videoSinkChanged();
    void videoFrameAvailable(const QVideoFrame& videoFrame);
  public slots:
    void updateFrame(const LegacyControllerMapping::ScreenInfo& screen, const QImage& frame);
    void clear() {
        m_averageFrameDuration = std::numeric_limits<double>::max();
    }

  private:
    LegacyControllerMapping::ScreenInfo m_screenInfo;

    double m_averageFrameDuration;
    using Clock = std::chrono::steady_clock;
    Clock::time_point m_lastFrameTimestamp;
};

class QmlControllerSettingElement : public QObject {
    Q_OBJECT
    Q_PROPERTY(QList<QmlControllerSettingElement*> children MEMBER m_pChildren CONSTANT)
    Q_PROPERTY(QString type READ getType CONSTANT)
    Q_PROPERTY(bool dirty READ isDirty NOTIFY dirtyChanged)
    QML_ANONYMOUS
  public:
    explicit QmlControllerSettingElement(
            const LegacyControllerSettingsLayoutElement* pInternal,
            QObject* parent);
    virtual QString getType() const = 0;
    virtual bool isDirty() const;

    static QmlControllerSettingElement* build(
            LegacyControllerSettingsLayoutElement* element, QObject* parent);

  signals:
    void dirtyChanged();

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
    bool isDirty() const override;
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

class QmlControllerDeviceProxy;
class QmlControllerMappingProxy : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString name READ getName CONSTANT)
    Q_PROPERTY(QString author READ getAuthor CONSTANT)
    Q_PROPERTY(QString description READ getDescription CONSTANT)
    Q_PROPERTY(QUrl forumLink READ getForumLink CONSTANT)
    Q_PROPERTY(QUrl wikiLink READ getWikiLink CONSTANT)
    Q_PROPERTY(bool hasSettings READ hasSettings CONSTANT)
    Q_PROPERTY(bool hasScreens READ hasScreens CONSTANT)
    QML_NAMED_ELEMENT(SoundDevice)
    QML_UNCREATABLE("Use Mixxx.ControllerManager to get devices")
  public:
    enum class Type {
        HID,
        MIDI,
    };
    Q_ENUM(Type)
    explicit QmlControllerMappingProxy(const MappingInfo& mapping, QObject* parent);

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
    // The following accessors cannot be const as the information will be lazy
    // loaded and cached into the object.
    bool hasSettings();
    bool hasScreens();

    const MappingInfo& definition() const {
        return m_mappingDefinition;
    }
    Q_INVOKABLE bool isUserMapping(const mixxx::qml::QmlConfigProxy* pConfig) const;

  signals:
    // This signal gets emitted when the mapping cannot be read anymore. This
    // allows QML to handle the case where this file is not available anymore,
    // but information about this mapping may still be known.
    void mappingErrored();

  private:
    void fetchMappingDetails();

    // The following information may require to parse the XML file entirely. To
    // avoid unnecessarily doing so, we only load these fields if needed.
    // Accessors make sure to fetch this information just in time.
    std::optional<bool> m_hasSettings;
    std::optional<bool> m_hasScreens;
    MappingInfo m_mappingDefinition;
};

class QmlControllerDeviceProxy : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString sinceVersion READ getSinceVersion CONSTANT)
    Q_PROPERTY(QString vendor READ vendor CONSTANT)
    Q_PROPERTY(QString product READ product CONSTANT)
    Q_PROPERTY(QString serialNumber READ serialNumber CONSTANT)
    Q_PROPERTY(mixxx::qml::QmlControllerDeviceProxy::Type type READ getType CONSTANT)

    Q_PROPERTY(QString name READ getName WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QUrl visualUrl READ getVisualUrl WRITE setVisualUrl NOTIFY visualUrlChanged)
    Q_PROPERTY(bool enabled READ getEnabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(bool edited MEMBER m_edited NOTIFY editedChanged)
    Q_PROPERTY(QSet<QmlControllerMappingProxy*> availableMappings MEMBER
                    m_mappings NOTIFY mappingsChanged)
    Q_PROPERTY(QmlControllerMappingProxy* mapping READ getMapping WRITE
                    setMapping NOTIFY mappingChanged)
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
            const QSet<QmlControllerMappingProxy*>& mappings,
            QObject* parent);
    mixxx::qml::QmlControllerDeviceProxy::Type getType() const;
    QString getName() const;
    void setName(const QString& name);
    QString getSinceVersion() const;
    QUrl getVisualUrl() const;
    void setVisualUrl(const QUrl& url);
    QmlControllerMappingProxy* getMapping() const;
    void setMapping(QmlControllerMappingProxy* url);
    bool getEnabled() const;
    void setEnabled(bool state);
    void setEdited() {
        bool changed = !m_edited;
        m_edited = true;
        if (changed) {
            emit editedChanged();
        }
    }
    void setMappings(const QSet<QmlControllerMappingProxy*>& mappings) {
        m_mappings = mappings;
        emit mappingChanged();
    }
    void clearEdited() {
        bool changed = m_edited;
        m_edited = false;
        if (changed) {
            emit editedChanged();
        }
    }
    QString vendor() const;
    QString product() const;
    QString serialNumber() const;
    Q_INVOKABLE bool save(const mixxx::qml::QmlConfigProxy* pConfig);
    Q_INVOKABLE void clear();

    Controller* internal() const {
        return m_pInternal;
    }

    std::shared_ptr<LegacyControllerMapping> instanceFor(const QString& mappingPath) const;
    void setInstanceFor(const QString& mappingPath, std::shared_ptr<LegacyControllerMapping>);
  signals:
    void visualUrlChanged();
    void nameChanged();
    void mappingChanged();
    void enabledChanged();
    void editedChanged();
    void mappingsChanged();

    void mappingAssigned(Controller* pController,
            std::shared_ptr<LegacyControllerMapping> pMapping,
            bool bEnabled);

    void mappingCreated(QmlControllerDeviceProxy::Type type, const MappingInfo& mapping);
#if QT_VERSION >= QT_VERSION_CHECK(6, 9, 0)
    void mappingUpdated(QmlControllerMappingProxy* pMapping, const MappingInfo& mapping);
#else
    void mappingUpdated(mixxx::qml::QmlControllerMappingProxy* pMapping,
            const MappingInfo& mapping);
#endif
    void deviceLearned();

  private:
    bool m_edited;
    QString m_editedFriendlyName;
    QUrl m_editedVisualUrl;
    std::optional<bool> m_enabled;
    Controller* m_pInternal;
    std::optional<ProductInfo> m_productInfo;
    QSet<QmlControllerMappingProxy*> m_mappings;
    QmlControllerMappingProxy* m_pMapping;

    QHash<QString, std::shared_ptr<LegacyControllerMapping>> m_mappingInstance;
};

class QmlControllerManagerProxy : public QObject {
    Q_OBJECT
    QML_NAMED_ELEMENT(ControllerManager)
    Q_PROPERTY(QList<QmlControllerDeviceProxy*> knownDevices MEMBER
                    m_knownDevicesFound NOTIFY deviceListChanged)
    Q_PROPERTY(QList<QmlControllerDeviceProxy*> unknownDevices MEMBER
                    m_unknownDevicesFound NOTIFY deviceListChanged)
    Q_PROPERTY(bool showControllerScreen MEMBER s_controllerPreviewScreens CONSTANT)
    QML_SINGLETON
  public:
    explicit QmlControllerManagerProxy(
            std::shared_ptr<ControllerManager> pControllerManager,
            QObject* parent = nullptr);

    QQmlListProperty<QmlControllerDeviceProxy> knownDevices();
    QQmlListProperty<QmlControllerDeviceProxy> unknownDevices();

    bool isControllerScreenDebug() const;

    std::shared_ptr<ControllerManager> internal() const;

    Q_INVOKABLE QList<mixxx::qml::QmlControllerMappingProxy*> mappings(
            mixxx::qml::QmlControllerDeviceProxy::Type type) const {
        return m_knownMappings.value(type);
    }

    static QmlControllerManagerProxy* create(QQmlEngine* pQmlEngine, QJSEngine* pJsEngine);
    static void registerManager(std::shared_ptr<ControllerManager> pManager,
            bool controllerPreviewScreens = false) {
        s_pControllerManager = std::move(pManager);
        s_controllerPreviewScreens = controllerPreviewScreens;
    }

  private slots:
    void refreshKnownDevices();
    void refreshMappings();
    void loadNewMapping(QmlControllerDeviceProxy::Type type, const MappingInfo& mapping);
#if QT_VERSION >= QT_VERSION_CHECK(6, 9, 0)
    void updateExistingMapping(QmlControllerMappingProxy* pMapping, const MappingInfo& mapping);
#else
    void updateExistingMapping(mixxx::qml::QmlControllerMappingProxy* pMapping,
            const MappingInfo& mapping);
#endif

  signals:
    void deviceListChanged();

  private:
    static inline std::shared_ptr<ControllerManager> s_pControllerManager;
    static inline bool s_controllerPreviewScreens;

    void loadMappingFromEnumerator(QSharedPointer<MappingInfoEnumerator> enumerator);

    QList<Controller*> m_knownControllers;
    QHash<ProductInfo, QSet<QmlControllerMappingProxy*>> m_knownDevices;
    QHash<QmlControllerDeviceProxy::Type, QList<QmlControllerMappingProxy*>> m_knownMappings;
    QList<QmlControllerDeviceProxy*> m_knownDevicesFound;
    QList<QmlControllerDeviceProxy*> m_unknownDevicesFound;

    std::shared_ptr<ControllerManager> m_pControllerManager;
};

} // namespace qml
} // namespace mixxx
