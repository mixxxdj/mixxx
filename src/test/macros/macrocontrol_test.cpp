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
    macroControl.controlRecord();
    macroControl.controlActivate();
    macroControl.controlToggle();
    macroControl.controlClear();
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

    TrackPointer pTrack = Track::newTemporary();
    macroControl.trackLoaded(pTrack);
    ASSERT_EQ(macroControl.getStatus(), MacroControl::Status::Empty);
    macroControl.controlRecord();
    EXPECT_TRUE(macroControl.isRecording());

    macroControl.notifySeek(kAction.getSamplePos());
    macroControl.setCurrentSample(kAction.getSamplePos(), 99000, 44100);
    macroControl.process(0, kAction.getSamplePos(), 1);
    ASSERT_EQ(macroControl.getStatus(), MacroControl::Status::Armed);

    macroControl.slotJumpQueued();
    macroControl.notifySeek(kAction.getSamplePos());
    macroControl.slotJumpQueued();
    macroControl.notifySeek(kAction.getTargetSamplePos());
    macroControl.setCurrentSample(kAction.getTargetSamplePos(), 99000, 44100);
    macroControl.process(0, kAction.getTargetSamplePos(), 1);

    macroControl.controlRecord();
    EXPECT_EQ(macroControl.getStatus(), MacroControl::Status::Playing);
    checkMacroAction(macroControl.getMacro());
    EXPECT_TRUE(pTrack->isDirty());

    macroControl.trackLoaded(nullptr);
    EXPECT_EQ(macroControl.getStatus(), MacroControl::Status::NoTrack);
}

TEST(MacroControlTest, Controls) {
    MacroControl macroControl(kChannelGroup, nullptr, 2);
    ControlProxy status(kChannelGroup, "macro_2_status");
    ASSERT_EQ(status.get(), MacroControl::Status::NoTrack);

    macroControl.trackLoaded(Track::newTemporary());
    ASSERT_EQ(status.get(), MacroControl::Status::Empty);

    ControlProxy record(kChannelGroup, "macro_2_record");
    record.set(1);
    ASSERT_EQ(status.get(), MacroControl::Status::Armed);
    record.set(1);
    ASSERT_EQ(status.get(), MacroControl::Status::Empty);

    ControlProxy activate(kChannelGroup, "macro_2_activate");
    activate.set(1);
    ASSERT_EQ(status.get(), MacroControl::Status::Armed);
    activate.set(1);
    ASSERT_EQ(status.get(), MacroControl::Status::Empty);
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
    int position = 0;

    auto pTrack = Track::newTemporary();
    pTrack->setMacros({{2,
            std::make_shared<Macro>(
                    QList{MacroAction(0, 0), MacroAction(position, kAction.position)},
                    "test",
                    Macro::State())}});
    macroControl.trackLoaded(pTrack);
    EXPECT_EQ(macroControl.getStatus(), MacroControl::Status::Recorded);

    macroControl.controlActivate();
    EXPECT_EQ(macroControl.getStatus(), MacroControl::Status::Playing);
    EXPECT_CALL(macroControl, seekExact(kAction.getSamplePos())).Times(1);
    macroControl.process(0, position, 2);
}
