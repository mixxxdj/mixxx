#include "engine/controls/macrocontrol.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "engine/enginebuffer.h"
#include "engine/enginemixer.h"
#include "macro_test.h"
#include "track/track.h"

class MacroControlTest : public MacroControl, public testing::Test {
  public:
    MacroControlTest()
            : MacroControl(kChannelGroup, nullptr, 2) {
        EXPECT_EQ(getStatus(), MacroControl::Status::NoTrack);
        trackLoaded(pLoadedTrack);
        EXPECT_EQ(getStatus(), MacroControl::Status::Empty);
        EXPECT_FALSE(getMacro()->isDirty());
        EXPECT_FALSE(pLoadedTrack->isDirty());
        EXPECT_FALSE(isRecording());
    }

    MOCK_METHOD1(seekExact, void(mixxx::audio::FramePos));

    const TrackPointer pLoadedTrack = Track::newTemporary();
};

TEST_F(MacroControlTest, Dirty) {
    getMacro()->setLabel("hello");
    EXPECT_TRUE(getMacro()->isDirty());
    EXPECT_TRUE(pLoadedTrack->isDirty());
    // TODO test interrupt label
    // TODO test automatic reassignment
}

TEST_F(MacroControlTest, RecordSeek) {
    // Start recording
    slotRecord(1);
    EXPECT_TRUE(isRecording());
    EXPECT_EQ(getMacro()->getLabel(), " [Recording]");
    // Prepare recording
    mixxx::audio::SampleRate frameRate(1'000);
    auto seek = [this, frameRate](mixxx::audio::FramePos position) {
        notifySeek(position);
        setFrameInfo(position, mixxx::audio::FramePos(99'000), frameRate);
        process(frameRate, position, 1);
    };
    seek(mixxx::audio::FramePos(0));
    ASSERT_EQ(getStatus(), MacroControl::Status::RecordingArmed);

    // Initial jump
    mixxx::audio::FramePos startPos(504);
    slotJumpQueued(startPos);
    seek(startPos);
    // Jump with quantization adjustment
    TestMacro testMacro;
    mixxx::audio::FrameDiff_t diff(10);
    seek(testMacro.action.getSourcePosition() - diff);
    slotJumpQueued(testMacro.action.getTargetPosition());
    seek(testMacro.action.getTargetPosition() - diff);

    // Stop recording
    seek(testMacro.action.getTargetPosition());
    EXPECT_CALL(*this, seekExact(startPos));
    slotRecord(0);
    EXPECT_EQ(getStatus(), MacroControl::Status::Playing);
    // Check recording result
    testMacro.checkMacroAction(getMacro());
    EXPECT_EQ(getMacro()->getActions().first().getTargetPosition(), startPos);
    EXPECT_TRUE(pLoadedTrack->isDirty());
    // Check generated label
    EXPECT_EQ((startPos / frameRate).value(), 0.504);
    EXPECT_EQ(getMacro()->getLabel().toStdString(), "0.50");
    // Activate
    EXPECT_CALL(*this, seekExact(startPos));
    slotGotoPlay();
    EXPECT_EQ(getStatus(), MacroControl::Status::Playing);
    // Check status on eject
    trackLoaded(nullptr);
    EXPECT_EQ(getStatus(), MacroControl::Status::NoTrack);
}

TEST_F(MacroControlTest, ControlObjects) {
    ControlProxy status(kChannelGroup, "macro_2_status");
#define ASSERT_STATUS(expected) ASSERT_EQ(MacroControl::Status(status.get()), expected);
    QString label("Intro");
    getMacro()->setLabel(label);

    ControlProxy record(kChannelGroup, "macro_2_record");
    record.set(1);
    ASSERT_STATUS(MacroControl::Status::RecordingArmed);
    EXPECT_EQ(getMacro()->getLabel(), "Intro [Recording]");
    record.set(0);
    ASSERT_STATUS(MacroControl::Status::Empty);
    EXPECT_EQ(getMacro()->getLabel(), label);

    ControlProxy activate(kChannelGroup, "macro_2_activate");
    activate.set(1);
    ASSERT_STATUS(MacroControl::Status::RecordingArmed);
    activate.set(1);
    ASSERT_STATUS(MacroControl::Status::Empty);
    activate.set(1);
    ASSERT_STATUS(MacroControl::Status::RecordingArmed);
    EXPECT_EQ(getMacro()->getLabel(), "Intro [Recording]");

    // Record
    ControlProxy(kChannelGroup, "macro_2_enable").set(0);
    ControlProxy(kChannelGroup, "macro_2_loop").set(1);
    EXPECT_TRUE(getMacro()->isLooped());
    slotJumpQueued(mixxx::audio::FramePos(0));
    notifySeek(mixxx::audio::FramePos(0));
    activate.set(1);
    ASSERT_STATUS(MacroControl::Status::Recorded);
    EXPECT_EQ(getMacro()->getLabel(), label);
    activate.set(1);
    ASSERT_STATUS(MacroControl::Status::Playing);

    // Restart
    EXPECT_CALL(*this, seekExact(mixxx::audio::FramePos(0)));
    activate.set(1);
    ASSERT_STATUS(MacroControl::Status::Playing);

    ControlProxy play(kChannelGroup, "macro_2_play");
    play.set(0);
    ASSERT_STATUS(MacroControl::Status::Recorded);

    ControlProxy(kChannelGroup, "macro_2_clear").set(1);
    EXPECT_TRUE(getMacro()->isEmpty());
    EXPECT_EQ(getMacro()->getLabel(), label);
    ASSERT_STATUS(MacroControl::Status::Empty);
}

TEST_F(MacroControlTest, LoadTrackAndPlayAndClear) {
    MacroAction jumpAction(mixxx::audio::FramePos(40'000), mixxx::audio::FramePos(0));
    TestMacro testMacro;

    QString label = QStringLiteral("test");
    pLoadedTrack->setMacros({{2,
            std::make_shared<Macro>(
                    QList{testMacro.action, jumpAction},
                    label,
                    Macro::State())}});
    trackLoaded(pLoadedTrack);
    EXPECT_EQ(getStatus(), MacroControl::Status::Recorded);

    slotActivate();
    EXPECT_EQ(getStatus(), MacroControl::Status::Playing);
    EXPECT_CALL(*this, seekExact(jumpAction.getTargetPosition()));
    process(0, jumpAction.getSourcePosition(), 2);
    EXPECT_EQ(getStatus(), MacroControl::Status::Recorded);

    // LOOP
    slotLoop(1);
    EXPECT_CALL(*this, seekExact(testMacro.action.getTargetPosition()));
    slotActivate();
    slotActivate();
    // Jump
    EXPECT_CALL(*this, seekExact(jumpAction.getTargetPosition()));
    process(0, jumpAction.getSourcePosition(), 2);
    // Loop back
    EXPECT_CALL(*this, seekExact(testMacro.action.getTargetPosition()));
    process(0, testMacro.action.getSourcePosition(), 2);
    // Jump again
    EXPECT_CALL(*this, seekExact(jumpAction.getTargetPosition()));
    process(0, jumpAction.getSourcePosition(), 2);
    EXPECT_EQ(getStatus(), MacroControl::Status::Playing);

    // Stop playing
    slotPlay(0);
    EXPECT_EQ(getStatus(), MacroControl::Status::Recorded);
    // Clear
    slotClear();
    EXPECT_TRUE(getMacro()->isEmpty());
    EXPECT_EQ(getMacro()->getLabel(), label);
}
