#pragma once

#include <QJSEngine>
#include <QObject>
#include <QQmlEngine>
#include <QUrl>
#include <functional>
#include <memory>

#include "preferences/usersettings.h"

class KeyboardEventFilter;

namespace mixxx {
namespace qml {

class QmlApplicationProxy : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool keyboardShortcutsEnabled READ keyboardShortcutsEnabled WRITE
                    setKeyboardShortcutsEnabled NOTIFY keyboardShortcutsEnabledChanged)
    Q_PROPERTY(bool developerMode READ developerMode CONSTANT)
    Q_PROPERTY(bool experimentStatsEnabled READ experimentStatsEnabled NOTIFY statsModeChanged)
    Q_PROPERTY(bool baseStatsEnabled READ baseStatsEnabled NOTIFY statsModeChanged)
    Q_PROPERTY(bool debuggerEnabled READ debuggerEnabled WRITE setDebuggerEnabled NOTIFY
                    debuggerEnabledChanged)
    Q_PROPERTY(QUrl settingsDirectoryUrl READ settingsDirectoryUrl CONSTANT)
    Q_PROPERTY(QString applicationName READ applicationName CONSTANT)
    Q_PROPERTY(QString version READ version CONSTANT)
    Q_PROPERTY(QString platform READ platform CONSTANT)
    Q_PROPERTY(bool vinylControlAvailable READ vinylControlAvailable CONSTANT)
    Q_PROPERTY(bool liveBroadcastingAvailable READ liveBroadcastingAvailable CONSTANT)
    Q_PROPERTY(QUrl userManualUrl READ userManualUrl CONSTANT)
    Q_PROPERTY(bool userManualExternal READ userManualExternal CONSTANT)
    Q_PROPERTY(QUrl keyboardShortcutsUrl READ keyboardShortcutsUrl CONSTANT)
    Q_PROPERTY(bool keyboardShortcutsExternal READ keyboardShortcutsExternal CONSTANT)
    Q_PROPERTY(bool supportsGlobalMenuBar READ supportsGlobalMenuBar CONSTANT)
    QML_NAMED_ELEMENT(Application)
    QML_SINGLETON

  public:
    explicit QmlApplicationProxy(QObject* pParent = nullptr);

    bool keyboardShortcutsEnabled() const;
    void setKeyboardShortcutsEnabled(bool enabled);
    bool developerMode() const;
    bool experimentStatsEnabled() const;
    bool baseStatsEnabled() const;
    bool debuggerEnabled() const;
    void setDebuggerEnabled(bool enabled);
    QUrl settingsDirectoryUrl() const;
    QString applicationName() const;
    QString version() const;
    QString platform() const;
    bool vinylControlAvailable() const;
    bool liveBroadcastingAvailable() const;
    QUrl userManualUrl() const;
    bool userManualExternal() const;
    QUrl keyboardShortcutsUrl() const;
    bool keyboardShortcutsExternal() const;
    bool supportsGlobalMenuBar() const;

    Q_INVOKABLE void setExperimentStatsEnabled(bool enabled);
    Q_INVOKABLE void setBaseStatsEnabled(bool enabled);
    Q_INVOKABLE QString menuShortcut(
            const QString& command, const QString& defaultShortcut) const;
    Q_INVOKABLE void reloadSkin();

    static QmlApplicationProxy* create(QQmlEngine* pQmlEngine, QJSEngine* pJsEngine);
    static void registerUserSettings(UserSettingsPointer pConfig) {
        s_pConfig = std::move(pConfig);
    }
    static void registerKeyboardEventFilter(
            std::shared_ptr<KeyboardEventFilter> pKeyboardEventFilter) {
        s_pKeyboardEventFilter = std::move(pKeyboardEventFilter);
    }
    static void registerReloadCallback(std::function<void()> reloadCallback) {
        s_reloadCallback = std::move(reloadCallback);
    }

  signals:
    void applicationMenuRequested();
    void keyboardShortcutsEnabledChanged();
    void menuShortcutsChanged();
    void statsModeChanged();
    void debuggerEnabledChanged();

  private:
    QUrl documentationUrl(const QString& fileName, const QString& onlineUrl) const;

    static inline UserSettingsPointer s_pConfig;
    static inline std::shared_ptr<KeyboardEventFilter> s_pKeyboardEventFilter;
    static inline std::function<void()> s_reloadCallback;
};

} // namespace qml
} // namespace mixxx
