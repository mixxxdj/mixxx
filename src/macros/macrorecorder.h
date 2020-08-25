#pragma once

#include <gtest/gtest_prod.h>
#include <rigtorp/SPSCQueue.h>

#include <QDebug>
#include <QObject>
#include <QtCore>

#include "control/controlpushbutton.h"
#include "engine/channelhandle.h"
#include "recording/recordingmanagerbase.h"
#include "track/macro.h"
#include "track/track.h"

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

    void notifyTrackChange(ChannelHandle* channel, TrackPointer pTrack);

    /// Fetches all Actions recorded so far - note that this also clears them!
    QList<MacroAction> fetchRecordedActions();

    size_t getRecordingSize() const;
    const ChannelHandle* getActiveChannel() const;

  signals:
    void saveMacroFromChannel(QList<MacroAction>, ChannelHandle);
    void saveMacro(QList<MacroAction>, TrackPointer);

  private slots:
    void pollRecordingStart();

  private:
    FRIEND_TEST(MacroRecorderTest, RecordCueJump);

    /// Checks if this channel is recording, otherwise tries to claim it.
    /// Returns true if this channel is recording.
    /// Called in realtime code.
    bool checkOrClaimRecording(ChannelHandle* channel);

    ControlPushButton m_COToggleRecording;
    ControlObject m_CORecStatus;

    std::atomic<ChannelHandle*> m_activeChannel;
    QTimer m_pStartRecordingTimer;

    rigtorp::SPSCQueue<MacroAction> m_recordedActions;
};
