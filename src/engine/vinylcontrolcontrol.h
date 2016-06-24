#ifndef VINYLCONTROLCONTROL_H
#define VINYLCONTROLCONTROL_H

#include "engine/enginecontrol.h"
#include "track/track.h"
#include "preferences/usersettings.h"
#include "control/controlobject.h"
#include "control/controlproxy.h"
#include "control/controlpushbutton.h"

class VinylControlControl : public EngineControl {
    Q_OBJECT
  public:
    VinylControlControl(QString group, UserSettingsPointer pConfig);
    virtual ~VinylControlControl();

    // If the engine asks for a seek, we may need to disable absolute mode.
    void notifySeekQueued();
    bool isEnabled();
    bool isScratching();

  private slots:
    void slotControlVinylSeek(double fractionalPos);
    void trackLoaded(TrackPointer pNewTrack, TrackPointer pOldTrack) override;

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
    TrackPointer m_pCurrentTrack;
    bool m_bSeekRequested;
};

#endif /* VINYLCONTROLCONTROL_H */
