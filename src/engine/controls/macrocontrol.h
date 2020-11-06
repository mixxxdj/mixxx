#pragma once

#include <rigtorp/SPSCQueue.h>

#include <QString>

#include "control/controlpushbutton.h"
#include "engine/controls/enginecontrol.h"
#include "track/macro.h"

class MacroControl : public EngineControl {
    Q_OBJECT
  public:
    MacroControl(QString group, UserSettingsPointer pConfig, int slot);

    void trackLoaded(TrackPointer pNewTrack) override;
    void process(const double dRate, const double dCurrentSample, const int iBufferSize) override;
    void notifySeek(double dNewPlaypos) override;

    bool isRecording() const;

    enum class Status : int {
        NoTrack = -1,
        Empty = 0,
        Armed = 1,
        Recording = 2,
        Recorded = 3,
        Playing = 4,
    };

    Status getStatus() const;
    MacroPointer getMacro() const;

  public slots:
    void slotRecord(double value);
    void slotPlay(double value);
    void slotEnable(double value);
    void slotLoop(double value);
    void slotActivate(double value = 1);
    void slotGotoPlay(double value = 1);
    void slotClear(double value = 1);

    void slotJumpQueued();

  private:
    void updateRecording();
    void stopRecording();

    void play();
    void stop();

    void setStatus(Status);

    ConfigKey getConfigKey(QString name);
    int m_slot;
    QString m_controlPattern;

    bool m_bJumpPending;
    rigtorp::SPSCQueue<MacroAction> m_recordedActions;
    QTimer m_updateRecordingTimer;

    MacroPointer m_pMacro;
    unsigned int m_iNextAction;

    ControlObject m_COStatus;

    ControlPushButton m_CORecord;
    ControlPushButton m_COPlay;
    ControlPushButton m_COEnable;
    ControlPushButton m_COLoop;

    ControlPushButton m_activate;
    ControlPushButton m_clear;
};
