#pragma once

#include <QJSEngine>
#include <QObject>
#include <QQmlEngine>
#include <QUrl>
#include <functional>
#include <memory>

#include "preferences/usersettings.h"

class KeyboardEventFilter;
class VinylControlManager;

namespace mixxx {
namespace qml {

class QmlApplicationProxy : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool keyboardShortcutsEnabled READ keyboardShortcutsEnabled WRITE
                    setKeyboardShortcutsEnabled NOTIFY keyboardShortcutsEnabledChanged)
    Q_PROPERTY(bool textInputFocused READ textInputFocused NOTIFY textInputFocusedChanged)
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
    /// True while the user is typing in a text input.
    ///
    /// The shortcuts of the skins are QML Shortcut objects, which Qt resolves
    /// before the key reaches the focused item and which do not go through
    /// KeyboardEventFilter. They have to be disabled while this is true,
    /// otherwise a key that is also a shortcut is typed into nothing, e.g. the
    /// Space of "Maximize Library".
    bool textInputFocused() const;
    /// Called by the hosts of widgets that are painted outside the window
    /// hierarchy when the focus of their embedded widgets changes.
    static void notifyTextInputFocusChanged();
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
    static void registerVinylControlManager(VinylControlManager* pVinylControlManager) {
        s_pVinylControlManager = pVinylControlManager;
    }
    static VinylControlManager* vinylControlManager() {
        return s_pVinylControlManager;
    }
    static void registerReloadCallback(std::function<void()> reloadCallback) {
        s_reloadCallback = std::move(reloadCallback);
    }
    static void registerSetFullScreenCallback(std::function<void(bool)> callback) {
        s_setFullScreenCallback = std::move(callback);
    }
    static void registerAudioDeviceCheckCallback(std::function<void()> callback) {
        s_audioDeviceCheckCallback = std::move(callback);
    }
    /// Looks for connected audio devices again and offers to use the sound card
    /// of a controller that has just been connected, see AudioDeviceAssistant.
    Q_INVOKABLE void checkAudioDevices() const {
        if (s_audioDeviceCheckCallback) {
            s_audioDeviceCheckCallback();
        }
    }
    /// Applies the fullscreen state to the window owned by the QML application
    /// engine and returns true. Returns false when no such window exists, so the
    /// caller can fall back to the QWidget window used by the legacy skins.
    static bool setWindowFullScreen(bool fullScreen) {
        if (!s_setFullScreenCallback) {
            return false;
        }
        s_setFullScreenCallback(fullScreen);
        return true;
    }

  signals:
    void applicationMenuRequested();
    void keyboardShortcutsEnabledChanged();
    void textInputFocusedChanged();
    void menuShortcutsChanged();
    void statsModeChanged();
    void debuggerEnabledChanged();

  private:
    QUrl documentationUrl(const QString& fileName, const QString& onlineUrl) const;

    static inline UserSettingsPointer s_pConfig;
    static inline std::shared_ptr<KeyboardEventFilter> s_pKeyboardEventFilter;
    static inline VinylControlManager* s_pVinylControlManager{nullptr};
    static inline std::function<void()> s_reloadCallback;
    static inline std::function<void(bool)> s_setFullScreenCallback;
    static inline std::function<void()> s_audioDeviceCheckCallback;
    /// Set in the constructor, called by notifyTextInputFocusChanged().
    static inline std::function<void()> s_textInputFocusChangedCallback;
};

} // namespace qml
} // namespace mixxx
