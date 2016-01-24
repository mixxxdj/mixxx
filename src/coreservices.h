#ifndef MIXXX_CORESERVICES_H
#define MIXXX_CORESERVICES_H

#include <memory>

#include <QCoreApplication>
#include <QObject>

#ifdef __BROADCAST__
#include "broadcast/broadcastmanager.h"
#endif
#include "controllers/controllermanager.h"
#include "effects/effectsmanager.h"
#include "engine/enginemaster.h"
#include "library/library.h"
#include "mixer/playermanager.h"
#include "preferences/configobject.h"
#include "preferences/settingsmanager.h"
#include "recording/recordingmanager.h"
#include "soundio/soundmanager.h"
#include "util/db/dbconnectionpool.h"
#include "util/memory.h"
#include "util/timer.h"
#ifdef __VINYLCONTROL__
#include "vinylcontrol/vinylcontrolmanager.h"
#else
class VinylControlManager;
#endif
#include "waveform/guitick.h"

namespace mixxx {

class CoreServices : public QObject {
    Q_OBJECT
  public:
    CoreServices(QObject* pParent, QCoreApplication* pApp, const QString& profilePath);
    ~CoreServices() override;

    static void initializeStaticServices();
    static void shutdownStaticServices();

    // TODO(rryan): return Status.
    void initialize();
    void finalize();

    std::shared_ptr<SettingsManager> settingsManager() const {
        return m_pSettingsManager;
    }

    std::shared_ptr<EffectsManager> effectsManager() const {
        return m_pEffectsManager;
    }

    std::shared_ptr<SoundManager> soundManager() const {
        return m_pSoundManager;
    }

    std::shared_ptr<RecordingManager> recordingManager() const {
        return m_pRecordingManager;
    }

    std::shared_ptr<PlayerManager> playerManager() const {
        return m_pPlayerManager;
    }

    std::shared_ptr<Library> libraryManager() const {
        return m_pLibrary;
    }

    std::shared_ptr<ControllerManager> controllerManager() const {
        return m_pControllerManager;
    }

#ifdef __BROADCAST__
    std::shared_ptr<BroadcastManager> broadcastManager() const {
        return m_pBroadcastManager;
    }
#endif

    std::shared_ptr<VinylControlManager> vinylControlManager() const {
        return m_pVinylControlManager;
    }


    std::shared_ptr<EngineMaster> engine() const {
        return m_pEngine;
    }

    std::shared_ptr<GuiTick> getGuiTick() const {
        return m_pGuiTick;
    }

  private:
    void controlLeakCheck() const;
    bool initializeDatabase();

    std::shared_ptr<GuiTick> m_pGuiTick;

    std::shared_ptr<SettingsManager> m_pSettingsManager;
    std::shared_ptr<ChannelHandleFactory> m_pChannelHandleFactory;
    std::shared_ptr<EffectsManager> m_pEffectsManager;
    std::shared_ptr<SoundManager> m_pSoundManager;
    std::shared_ptr<RecordingManager> m_pRecordingManager;
    std::shared_ptr<PlayerManager> m_pPlayerManager;
#ifdef __BROADCAST__
    std::shared_ptr<BroadcastManager> m_pBroadcastManager;
#endif
    std::shared_ptr<VinylControlManager> m_pVinylControlManager;
    std::shared_ptr<EngineMaster> m_pEngine;
    DbConnectionPoolPtr m_pDbConnectionPool;
    std::shared_ptr<Library> m_pLibrary;
    std::shared_ptr<ControllerManager> m_pControllerManager;

    // Timer that tracks how long Mixxx has been running.
    ScopedTimer m_runtime_timer;
};

}  // namespace mixxx

#endif /* MIXXX_CORESERVICES_H */
