#pragma once

#include <rigtorp/SPSCQueue.h>

#include <QString>

#include "control/controlpushbutton.h"
#include "engine/controls/enginecontrol.h"
#include "track/macro.h"

class MacroControl : public EngineControl {
    Q_OBJECT
  public:
    MacroControl(const QString& group, UserSettingsPointer pConfig, int slot);

    void trackLoaded(TrackPointer pNewTrack) override;
    void process(const double dRate,
            mixxx::audio::FramePos currentPosition,
            const int iBufferSize) override;
    void notifySeek(mixxx::audio::FramePos position) override;

    bool isRecording() const;

    enum class Status : int {
        /// No track is loaded.
        NoTrack = -1,
        /// There is no recoded macro present.
        Empty = 0,
        /// Recording is armed. The next supported action (e.g., a hotcue jump
        /// will start the recording).
        RecordingArmed = 1,
        /// Recording has been started and supported actions will be part of the
        /// resulting macro.
        Recording = 2,
        /// There is a recorded macro present, but it's not enabled for
        /// playback.
        Recorded = 3,
        /// The macro is currently enabled for playback.
        Playing = 4,
    };

    Status getStatus() const;
    MacroPointer getMacro() const;

  public slots:
    void slotRecord(double value);
    void slotPlay(double value);
    void slotEnable(double value);
    void slotLoop(double value);

    void slotGotoPlay(double value = 1);
    void slotActivate(double value = 1);
    void slotToggle(double value = 1);
    void slotClear(double value = 1);

    void slotJumpQueued(mixxx::audio::FramePos samplePos);

  private:
    /// Returns whether a new action was recorded
    bool updateRecording();
    /// Returns whether the macro was recorded
    bool stopRecording();

    void play();
    void stop();

    void setStatus(Status);

    ConfigKey getConfigKey(const QString& name);
    int m_slot;

    /// The unquantized FramePos of a jump that is yet to be processed
    mixxx::audio::FramePos m_queuedJumpTarget;

    rigtorp::SPSCQueue<MacroAction> m_recordedActions;
    QTimer m_updateRecordingTimer;

    MacroPointer m_pMacro;
    std::optional<unsigned int> m_nextActionIndex;

    ControlObject m_COStatus;

    ControlPushButton m_CORecord;
    ControlPushButton m_COPlay;
    ControlPushButton m_COEnable;
    ControlPushButton m_COLoop;

    ControlPushButton m_activate;
    ControlPushButton m_toggle;
    ControlPushButton m_clear;
};
