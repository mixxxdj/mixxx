#pragma once

#include <rigtorp/SPSCQueue.h>

#include <QString>

#include "control/controlpushbutton.h"
#include "engine/controls/enginecontrol.h"
#include "preferences/usersettings.h"

class MacroControl : public EngineControl {
    Q_OBJECT
  public:
    MacroControl(QString group, UserSettingsPointer pConfig, int number);

    void trackLoaded(TrackPointer pNewTrack) override;
    void process(const double dRate, const double dCurrentSample, const int iBufferSize) override;

    bool isRecording() const;
    bool isPlaying() const;

    enum Status {
        NoTrack = -1,
        Empty = 0,
        Armed = 1,
        Recording = 2,
        Recorded = 3,
        Playing = 4,
        PlaybackStopped = 5,
    };

  private slots:
    void controlToggle();
    void controlClear();
    void controlActivate();

  private:
    void play();
    void stop();

    MacroPtr m_pMacro;
    int m_iNextAction;

    int m_number;
    QString m_controlPattern;
    ConfigKey getConfigKey(QString name);

    ControlObject m_COStatus;
    ControlObject m_COActive;

    ControlPushButton m_toggle;
    ControlPushButton m_clear;
    ControlPushButton m_activate;
};
