#pragma once

#include "control/pollingcontrolproxy.h"
#include "engine/controls/enginecontrol.h"
#include "preferences/usersettings.h"
#include "track/track_decl.h"

class ControlObject;
class ControlPushButton;
class ControlProxy;

class VinylControlControl : public EngineControl {
    Q_OBJECT
  public:
    VinylControlControl(const QString& group, UserSettingsPointer pConfig);
    virtual ~VinylControlControl();

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
    ControlObject* m_pControlVinylRate;
    ControlObject* m_pControlVinylSeek;
    ControlObject* m_pControlVinylSpeedType;
    ControlObject* m_pControlVinylStatus;
    ControlPushButton* m_pControlVinylScratching;
    ControlPushButton* m_pControlVinylMode;
    ControlPushButton* m_pControlVinylEnabled;
    ControlPushButton* m_pControlVinylWantEnabled;
    ControlPushButton* m_pControlVinylCueing;
    ControlPushButton* m_pControlVinylSignalEnabled;
    ControlProxy* m_pPlayEnabled;
    PollingControlProxy m_inputConfigured;

    TrackPointer m_pTrack; // is written from an engine worker thread

    bool m_bSeekRequested;
};
