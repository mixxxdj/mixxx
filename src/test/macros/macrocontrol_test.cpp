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

    macroControl.trackLoaded(Track::newTemporary());
    EXPECT_EQ(macroControl.getStatus(), MacroControl::Status::Empty);
}

TEST(MacroControlTest, RecordSeek) {
    MacroControl macroControl(kChannelGroup, nullptr, 2);
    EXPECT_EQ(macroControl.isRecording(), false);

    macroControl.trackLoaded(Track::newTemporary());
    ASSERT_EQ(macroControl.getStatus(), MacroControl::Status::Empty);
    macroControl.controlRecord();
    EXPECT_EQ(macroControl.isRecording(), true);

    macroControl.notifySeek(kAction.position * mixxx::kEngineChannelCount);
    macroControl.setCurrentSample(kAction.position * mixxx::kEngineChannelCount, 99000, 44100);
    macroControl.process(0, kAction.position * mixxx::kEngineChannelCount, 1);
    ASSERT_EQ(macroControl.getStatus(), MacroControl::Status::Armed);

    macroControl.slotJumpQueued();
    macroControl.notifySeek(kAction.position * mixxx::kEngineChannelCount);
    macroControl.slotJumpQueued();
    macroControl.notifySeek(kAction.target * mixxx::kEngineChannelCount);
    macroControl.setCurrentSample(kAction.target * mixxx::kEngineChannelCount, 99000, 44100);
    macroControl.process(0, kAction.target * mixxx::kEngineChannelCount, 1);

    macroControl.controlRecord();
    EXPECT_EQ(macroControl.getStatus(), MacroControl::Status::Playing);
    checkMacroAction(macroControl.getMacro());

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
                    QList{MacroAction(), MacroAction(position, kAction.position)},
                    "test",
                    Macro::State())}});
    macroControl.trackLoaded(pTrack);
    EXPECT_EQ(macroControl.getStatus(), MacroControl::Status::Recorded);

    macroControl.controlActivate();
    ASSERT_EQ(macroControl.isPlaying(), true);
    EXPECT_CALL(macroControl, seekExact(kAction.position * mixxx::kEngineChannelCount)).Times(1);
    macroControl.process(0, position, 2);
}
