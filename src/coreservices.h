#pragma once

#include "control/controlpushbutton.h"
#include "preferences/configobject.h"
#include "preferences/constants.h"
#include "preferences/settingsmanager.h"
#include "soundio/sounddeviceerror.h"
#include "util/cmdlineargs.h"
#include "util/timer.h"

class QApplication;
class CmdlineArgs;
class KeyboardEventFilter;
class EffectsManager;
class EngineMaster;
class SoundManager;
class PlayerManager;
class RecordingManager;
#ifdef __BROADCAST__
class BroadcastManager;
#endif
class ControllerManager;
class VinylControlManager;
class TrackCollectionManager;
class Library;
class LV2Backend;

namespace mixxx {

class DbConnectionPool;
class ScreensaverManager;

class CoreServices : public QObject {
    Q_OBJECT

  public:
    CoreServices(const CmdlineArgs& args);
    ~CoreServices() = default;

    void initializeSettings();
    // FIXME: should be private, but WMainMenuBar needs it initialized early
    void initializeKeyboard();
    void initializeScreensaverManager();
    void initialize(QApplication* pApp);
    void shutdown();

    std::shared_ptr<KeyboardEventFilter> getKeyboardEventFilter() const {
        return m_pKeyboardEventFilter;
    }

    std::shared_ptr<ConfigObject<ConfigValueKbd>> getKeyboardConfig() const {
        return m_pKbdConfig;
    }

    std::shared_ptr<SoundManager> getSoundManager() const {
        return m_pSoundManager;
    }

    std::shared_ptr<PlayerManager> getPlayerManager() const {
        return m_pPlayerManager;
    }

    std::shared_ptr<RecordingManager> getRecordingManager() const {
        return m_pRecordingManager;
    }

#ifdef __BROADCAST__
    std::shared_ptr<BroadcastManager> getBroadcastManager() const {
        return m_pBroadcastManager;
    }
#endif

    std::shared_ptr<ControllerManager> getControllerManager() const {
        return m_pControllerManager;
    }

    std::shared_ptr<VinylControlManager> getVinylControlManager() const {
        return m_pVCManager;
    }

    LV2Backend* getLV2Backend() const {
        return m_pLV2Backend;
    }

    std::shared_ptr<EffectsManager> getEffectsManager() const {
        return m_pEffectsManager;
    }

    std::shared_ptr<Library> getLibrary() const {
        return m_pLibrary;
    }

    std::shared_ptr<TrackCollectionManager> getTrackCollectionManager() const {
        return m_pTrackCollectionManager;
    }

    std::shared_ptr<SettingsManager> getSettingsManager() const {
        return m_pSettingsManager;
    }

    UserSettingsPointer getSettings() const {
        return m_pSettingsManager->settings();
    }

    std::shared_ptr<ScreensaverManager> getScreensaverManager() const {
        return m_pScreensaverManager;
    }

  signals:
    void initializationProgressUpdate(int progress, const QString& serviceName);

  public slots:
    void slotOptionsKeyboard(bool toggle);

  private:
    bool initializeDatabase();

    std::shared_ptr<SettingsManager> m_pSettingsManager;
    std::shared_ptr<EffectsManager> m_pEffectsManager;
    // owned by EffectsManager
    LV2Backend* m_pLV2Backend;
    std::shared_ptr<EngineMaster> m_pEngine;
    std::shared_ptr<SoundManager> m_pSoundManager;
    std::shared_ptr<PlayerManager> m_pPlayerManager;
    std::shared_ptr<RecordingManager> m_pRecordingManager;
#ifdef __BROADCAST__
    std::shared_ptr<BroadcastManager> m_pBroadcastManager;
#endif
    std::shared_ptr<ControllerManager> m_pControllerManager;

    std::shared_ptr<VinylControlManager> m_pVCManager;

    std::shared_ptr<DbConnectionPool> m_pDbConnectionPool;
    std::shared_ptr<TrackCollectionManager> m_pTrackCollectionManager;
    std::shared_ptr<Library> m_pLibrary;

    std::shared_ptr<KeyboardEventFilter> m_pKeyboardEventFilter;
    std::shared_ptr<ConfigObject<ConfigValueKbd>> m_pKbdConfig;
    std::shared_ptr<ConfigObject<ConfigValueKbd>> m_pKbdConfigEmpty;

    std::shared_ptr<mixxx::ScreensaverManager> m_pScreensaverManager;

    std::unique_ptr<ControlPushButton> m_pTouchShift;

    Timer m_runtime_timer;
    const CmdlineArgs& m_cmdlineArgs;
};

} // namespace mixxx
