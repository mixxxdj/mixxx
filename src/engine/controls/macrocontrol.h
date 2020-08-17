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

  private slots:
    void controlSet();
    void controlClear();
    void controlActivate();

  private:
    Macro m_macro;
    int m_actionPosition;

    int m_number;
    QString m_controlPattern;
    ConfigKey getConfigKey(QString name);

    ControlObject m_COStatus;
    ControlObject m_COActive;

    ControlPushButton m_set;
    ControlPushButton m_clear;
    ControlPushButton m_activate;
};
