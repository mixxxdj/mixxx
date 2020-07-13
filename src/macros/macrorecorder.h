#pragma once

#include <gtest/gtest_prod.h>

#include <QDebug>
#include <QObject>
#include <QtCore>

#include "control/controlpushbutton.h"
#include "engine/channelhandle.h"
#include "macros/macro.h"
#include "recording/recordingmanagerbase.h"

enum MacroRecordingState : uint8_t {
    /// Nothing is going on
    Disabled,
    /// Recording is active, but nothing currently going on
    Armed,
    /// An Action is currently being recorded
    Recording,
};

/// The MacroRecorder handles the recording of Macros and the [MacroRecording] controls.
class MacroRecorder : public RecordingManagerBase {
    Q_OBJECT
  public:
    static const QString kControlsGroup;

    MacroRecorder();

    void startRecording() override;
    void stopRecording() override;
    bool isRecordingActive() const override;

    /// Tries to append this cue jump to the currently recorded Macro.
    /// Returns true if the currently recorded Macro was changed - so only if
    /// recording is active and the channel handle matches.
    /// Only called from RT.
    void notifyCueJump(ChannelHandle& channel, double origin, double target);

    Macro getMacro() const;
    MacroRecordingState getState() const;
    ChannelHandle* getActiveChannel() const;

  signals:
    void saveMacro(ChannelHandle channel, Macro macro);

  private slots:
    void pollRecordingStart();

  private:
    FRIEND_TEST(MacroRecordingTest, ClaimRecording);
    FRIEND_TEST(MacroRecordingTest, RecordCueJump);

    /// Checks if ths channel is recording, otherwise tries to claim it.
    /// Returns true if this channel is recording.
    /// Called from RT.
    bool checkOrClaimRecording(ChannelHandle& channel);

    /// Claims the recording if it is Armed.
    /// Called from RT.
    bool claimRecording();

    void setState(MacroRecordingState state);

    ControlPushButton m_COToggleRecording;
    ControlObject m_CORecStatus;

    ChannelHandle* m_activeChannel;
    std::atomic<MacroRecordingState> m_macroRecordingState;
    QTimer m_pStartRecordingTimer;

    Macro m_recordedMacro;
};
