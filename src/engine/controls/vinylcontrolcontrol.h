#pragma once

#include "control/pollingcontrolproxy.h"
#include "engine/controls/enginecontrol.h"
#include "preferences/usersettings.h"
#include "track/track_decl.h"

class ControlObject;
class ControlPushButton;

class VinylControlControl : public EngineControl {
    Q_OBJECT
  public:
    VinylControlControl(const QString& group, UserSettingsPointer pConfig);

    // If the engine asks for a seek, we may need to disable absolute mode.
    void notifySeekQueued();
    bool isEnabled();
    bool isScratching();
    void trackLoaded(TrackPointer pNewTrack) override;

  signals:
    void noVinylControlInputConfigured();

  private slots:
    void slotControlEnabledChangeRequest(double v);
    void slotControlVinylSeek(double fractionalPos);

  private:
    std::unique_ptr<ControlObject> m_pControlVinylRate;
    std::unique_ptr<ControlObject> m_pControlVinylSeek;
    std::unique_ptr<ControlObject> m_pControlVinylSpeedType;
    std::unique_ptr<ControlObject> m_pControlVinylStatus;
    std::unique_ptr<ControlPushButton> m_pControlVinylScratching;
    std::unique_ptr<ControlPushButton> m_pControlVinylMode;
    std::unique_ptr<ControlPushButton> m_pControlVinylEnabled;
    std::unique_ptr<ControlPushButton> m_pControlVinylWantEnabled;
    std::unique_ptr<ControlPushButton> m_pControlVinylCueing;
    std::unique_ptr<ControlPushButton> m_pControlVinylSignalEnabled;
    PollingControlProxy m_playEnabled;
    PollingControlProxy m_inputConfigured;

    TrackPointer m_pTrack; // is written from an engine worker thread

    bool m_bSeekRequested;
};
