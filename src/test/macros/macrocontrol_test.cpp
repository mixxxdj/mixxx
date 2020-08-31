#include "engine/controls/macrocontrol.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "engine/enginebuffer.h"
#include "engine/enginemaster.h"
#include "macro_test.h"
#include "track/track.h"

class MacroControlTest : public MacroControl, public testing::Test {
  public:
    MacroControlTest()
            : MacroControl(kChannelGroup, nullptr, 2) {
    }
    MOCK_METHOD1(seekExact, void(double));
};

TEST_F(MacroControlTest, Create) {
    EXPECT_EQ(getStatus(), MacroControl::Status::NoTrack);
    // Ensure that randomly invoking COs doesn't throw
    slotRecord();
    slotActivate();
    slotToggle();
    slotClear();
    EXPECT_EQ(getStatus(), MacroControl::Status::NoTrack);
}

TEST_F(MacroControlTest, LoadTrack) {
    TrackPointer pTrack = Track::newTemporary();
    trackLoaded(pTrack);
    EXPECT_EQ(getStatus(), MacroControl::Status::Empty);

    getMacro()->setLabel("hello");
    EXPECT_TRUE(pTrack->isDirty());
}

TEST_F(MacroControlTest, RecordSeek) {
    EXPECT_FALSE(isRecording());

    // Load track
    TrackPointer pTrack = Track::newTemporary();
    trackLoaded(pTrack);
    ASSERT_EQ(getStatus(), MacroControl::Status::Empty);
    // Start recording
    slotRecord();
    EXPECT_TRUE(isRecording());
    EXPECT_EQ(getMacro()->getLabel(), "[Recording]");
    // Prepare recording
    int frameRate = 1'000;
    auto seek = [this, frameRate](double position) {
        notifySeek(position);
        setCurrentSample(position, 99'000, frameRate);
        process(0, position, 2);
    };
    seek(0);
    ASSERT_EQ(getStatus(), MacroControl::Status::Armed);
    // Disable auto-playback after recording
    // TODO(xerus) create & test control for this
    getMacro()->setState(Macro::StateFlag::Enabled, false);

    // Initial jump
    double startFramePos = 1'160;
    slotJumpQueued();
    seek(startFramePos * mixxx::kEngineChannelCount);
    // Jump kAction
    seek(kAction.getSamplePos());
    slotJumpQueued();
    seek(kAction.getTargetSamplePos());

    // Stop recording
    slotRecord();
    EXPECT_EQ(getStatus(), MacroControl::Status::Recorded);
    // Check recording result
    checkMacroAction(getMacro());
    EXPECT_EQ(getMacro()->getActions().first().target, startFramePos);
    EXPECT_TRUE(pTrack->isDirty());
    // Check generated label
    EXPECT_EQ(startFramePos / frameRate, 1.16);
    EXPECT_EQ(getMacro()->getLabel().toStdString(), "1.2");
    // Activate
    slotGotoPlay();
    EXPECT_EQ(getStatus(), MacroControl::Status::Playing);
    // Check status on eject
    trackLoaded(nullptr);
    EXPECT_EQ(getStatus(), MacroControl::Status::NoTrack);
}

TEST_F(MacroControlTest, ControlObjects) {
    ControlProxy status(kChannelGroup, "macro_2_status");
    const auto ASSERT_STATUS = [&status](MacroControl::Status expectedStatus) {
        ASSERT_EQ(MacroControl::Status(status.get()), expectedStatus);
    };
    ASSERT_STATUS(MacroControl::Status::NoTrack);

    trackLoaded(Track::newTemporary());
    ASSERT_STATUS(MacroControl::Status::Empty);

    ControlProxy record(kChannelGroup, "macro_2_record");
    record.set(1);
    ASSERT_STATUS(MacroControl::Status::Armed);
    record.set(1);
    ASSERT_STATUS(MacroControl::Status::Empty);

    ControlProxy activate(kChannelGroup, "macro_2_activate");
    activate.set(1);
    ASSERT_STATUS(MacroControl::Status::Armed);

    // Record
    slotJumpQueued();
    notifySeek(0);
    activate.set(1);
    ASSERT_STATUS(MacroControl::Status::Playing);

    // Restart
    activate.set(1);
    ASSERT_STATUS(MacroControl::Status::Playing);

    ControlProxy toggle(kChannelGroup, "macro_2_toggle");
    toggle.set(1);
    ASSERT_STATUS(MacroControl::Status::Recorded);
    toggle.set(1);
    ASSERT_STATUS(MacroControl::Status::Playing);
}

TEST_F(MacroControlTest, LoadTrackAndPlay) {
    MacroAction jumpAction(40'000, 0);

    auto pTrack = Track::newTemporary();
    pTrack->setMacros({{2,
            std::make_shared<Macro>(
                    QList{kAction, jumpAction},
                    "test",
                    Macro::State())}});
    trackLoaded(pTrack);
    EXPECT_EQ(getStatus(), MacroControl::Status::Recorded);

    slotActivate();
    EXPECT_EQ(getStatus(), MacroControl::Status::Playing);
    EXPECT_CALL(*this, seekExact(jumpAction.target)).Times(1);
    process(0, jumpAction.getSamplePos(), 2);
    EXPECT_EQ(getStatus(), MacroControl::Status::Recorded);

    // LOOP
    getMacro()->setState(Macro::StateFlag::Looped);
    EXPECT_CALL(*this, seekExact(kAction.getTargetSamplePos())).Times(1);
    slotActivate();
    slotActivate();
    // Jump
    EXPECT_CALL(*this, seekExact(jumpAction.getTargetSamplePos())).Times(1);
    process(0, jumpAction.getSamplePos(), 2);
    // Loop back
    EXPECT_CALL(*this, seekExact(kAction.getTargetSamplePos())).Times(1);
    process(0, kAction.getSamplePos(), 2);
    // Jump again
    EXPECT_CALL(*this, seekExact(jumpAction.getTargetSamplePos())).Times(1);
    process(0, jumpAction.getSamplePos(), 2);
    EXPECT_EQ(getStatus(), MacroControl::Status::Playing);
}
