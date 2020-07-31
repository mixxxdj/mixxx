#pragma once

#include <gtest/gtest_prod.h>
#include <rigtorp/SPSCQueue.h>

#include <QDebug>
#include <QObject>
#include <QtCore>

#include "control/controlpushbutton.h"
#include "engine/channelhandle.h"
#include "macros/macro.h"
#include "recording/recordingmanagerbase.h"

/// The MacroRecorder handles the recording of Macros and the [MacroRecording] controls.
class MacroRecorder : public RecordingManagerBase {
    Q_OBJECT
  public:
    MacroRecorder();

    void startRecording() override;
    void stopRecording() override;

    enum Status : uint8_t {
        /// Nothing is going on
        Disabled = 0,
        /// Recording is awaiting assignment to a channel
        Armed = 1,
        /// Recording is active
        Recording = 2,
    };

    bool isRecordingActive() const override;
    Status getStatus() const;

    /// Tries to append this cue jump to the currently recorded Macro.
    /// Returns true if the currently recorded Macro was changed - so only if
    /// recording is active and the channel handle matches.
    /// Only called in realtime code.
    void notifyCueJump(ChannelHandle* channel, double sourceFramePos, double destFramePos);

    size_t getRecordingSize() const;
    const MacroAction getRecordedAction();
    const ChannelHandle* getActiveChannel() const;

  signals:
    void saveMacro(ChannelHandle channel, QVector<MacroAction> actions);

  private slots:
    void pollRecordingStart();

  private:
    FRIEND_TEST(MacroRecordingTest, RecordCueJump);

    /// Checks if this channel is recording, otherwise tries to claim it.
    /// Returns true if this channel is recording.
    /// Called in realtime code.
    bool checkOrClaimRecording(ChannelHandle* channel);

    ControlPushButton m_COToggleRecording;
    ControlObject m_CORecStatus;

    std::atomic<ChannelHandle*> m_activeChannel;
    QTimer m_pStartRecordingTimer;

    rigtorp::SPSCQueue<MacroAction> m_pRecordedActions;
};
