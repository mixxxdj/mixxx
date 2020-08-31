#include "engine/controls/macrocontrol.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "engine/enginebuffer.h"
#include "engine/enginemaster.h"
#include "macro_test.h"

TEST(MacroControl, Create) {
    MacroControl macroControl(kChannelGroup, nullptr, 2);
    EXPECT_EQ(macroControl.getStatus(), MacroControl::Status::NoTrack);
    // Ensure that randomly invoking COs doesn't throw
    macroControl.slotRecord();
    macroControl.slotActivate();
    macroControl.slotToggle();
    macroControl.slotClear();
    EXPECT_EQ(macroControl.getStatus(), MacroControl::Status::NoTrack);
}

TEST(MacroControl, LoadTrack) {
    MacroControl macroControl(kChannelGroup, nullptr, 2);
    TrackPointer pTrack = Track::newTemporary();
    macroControl.trackLoaded(pTrack);
    EXPECT_EQ(macroControl.getStatus(), MacroControl::Status::Empty);

    macroControl.getMacro()->setLabel("hello");
    EXPECT_TRUE(pTrack->isDirty());
}

TEST(MacroControlTest, RecordSeek) {
    MacroControl macroControl(kChannelGroup, nullptr, 2);
    EXPECT_FALSE(macroControl.isRecording());

    // Load track
    TrackPointer pTrack = Track::newTemporary();
    macroControl.trackLoaded(pTrack);
    ASSERT_EQ(macroControl.getStatus(), MacroControl::Status::Empty);
    // Start recording
    macroControl.slotRecord();
    EXPECT_TRUE(macroControl.isRecording());
    EXPECT_EQ(macroControl.getMacro()->getLabel(), "[Recording]");
    // Prepare recording
    int frameRate = 1'000;
    auto seek = [&macroControl, frameRate](double position) {
        macroControl.notifySeek(position);
        macroControl.setCurrentSample(position, 99'000, frameRate);
        macroControl.process(0, position, 2);
    };
    seek(0);
    ASSERT_EQ(macroControl.getStatus(), MacroControl::Status::Armed);
    // Disable auto-playback after recording
    // TODO(xerus) create & test control for this
    macroControl.getMacro()->setState(Macro::StateFlag::Enabled, false);

    // Initial jump
    double startFramePos = 1'160;
    macroControl.slotJumpQueued();
    seek(startFramePos * mixxx::kEngineChannelCount);
    // Jump kAction
    seek(kAction.getSamplePos());
    macroControl.slotJumpQueued();
    seek(kAction.getTargetSamplePos());

    // Stop recording
    macroControl.slotRecord();
    EXPECT_EQ(macroControl.getStatus(), MacroControl::Status::Recorded);
    // Check recording result
    checkMacroAction(macroControl.getMacro());
    EXPECT_EQ(macroControl.getMacro()->getActions().first().target, startFramePos);
    EXPECT_TRUE(pTrack->isDirty());
    EXPECT_EQ(startFramePos / frameRate, 1.16);
    EXPECT_EQ(macroControl.getMacro()->getLabel().toStdString(), "1.2");
    // Activate
    macroControl.slotGotoPlay();
    EXPECT_EQ(macroControl.getStatus(), MacroControl::Status::Playing);
    // Check status on eject
    macroControl.trackLoaded(nullptr);
    EXPECT_EQ(macroControl.getStatus(), MacroControl::Status::NoTrack);
}

TEST(MacroControlTest, ControlObjects) {
    MacroControl macroControl(kChannelGroup, nullptr, 2);
    ControlProxy status(kChannelGroup, "macro_2_status");
    const auto ASSERT_STATUS = [&status](MacroControl::Status expectedStatus) {
        ASSERT_EQ(MacroControl::Status(status.get()), expectedStatus);
    };
    ASSERT_STATUS(MacroControl::Status::NoTrack);

    macroControl.trackLoaded(Track::newTemporary());
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
    macroControl.slotJumpQueued();
    macroControl.notifySeek(0);
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

class MacroControlMock : public MacroControl {
  public:
    MacroControlMock()
            : MacroControl(kChannelGroup, nullptr, 2) {
    }
    MOCK_METHOD1(seekExact, void(double));
};

TEST(MacroControlTest, LoadTrackAndPlay) {
    MacroControlMock macroControl;
    MacroAction jumpAction(40'000, 0);

    auto pTrack = Track::newTemporary();
    pTrack->setMacros({{2,
            std::make_shared<Macro>(
                    QList{kAction, jumpAction},
                    "test",
                    Macro::State())}});
    macroControl.trackLoaded(pTrack);
    EXPECT_EQ(macroControl.getStatus(), MacroControl::Status::Recorded);

    macroControl.slotActivate();
    EXPECT_EQ(macroControl.getStatus(), MacroControl::Status::Playing);
    EXPECT_CALL(macroControl, seekExact(jumpAction.target)).Times(1);
    macroControl.process(0, jumpAction.getSamplePos(), 2);
    EXPECT_EQ(macroControl.getStatus(), MacroControl::Status::Recorded);

    // LOOP
    macroControl.getMacro()->setState(Macro::StateFlag::Looped);
    EXPECT_CALL(macroControl, seekExact(kAction.getTargetSamplePos())).Times(1);
    macroControl.slotActivate();
    macroControl.slotActivate();
    // Jump
    EXPECT_CALL(macroControl, seekExact(jumpAction.getTargetSamplePos())).Times(1);
    macroControl.process(0, jumpAction.getSamplePos(), 2);
    // Loop back
    EXPECT_CALL(macroControl, seekExact(kAction.getTargetSamplePos())).Times(1);
    macroControl.process(0, kAction.getSamplePos(), 2);
    // Jump again
    EXPECT_CALL(macroControl, seekExact(jumpAction.getTargetSamplePos())).Times(1);
    macroControl.process(0, jumpAction.getSamplePos(), 2);
    EXPECT_EQ(macroControl.getStatus(), MacroControl::Status::Playing);
}
