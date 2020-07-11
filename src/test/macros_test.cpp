#include <gtest/gtest.h>

#include "macros/macrorecorder.h"
#include "signalpathtest.h"

TEST(MacrosTest, CreateMacro) {
    Macro macro;
    ASSERT_EQ(macro.getLength(), 0);
}

TEST(MacroRecordingTest, ClaimRecording) {
    MacroRecorder recorder;
    EXPECT_EQ(recorder.isRecordingActive(), false);
    recorder.claimRecording();
    EXPECT_EQ(recorder.isRecordingActive(), false);
    recorder.setState(MacroState::Armed);
    recorder.claimRecording();
    EXPECT_EQ(recorder.isRecordingActive(), true);
}

TEST(MacroRecordingTest, RecordCueJump) {
    MacroRecorder recorder;
    ChannelHandle handle = ChannelHandleFactory().getOrCreateHandle("");
    EXPECT_EQ(recorder.getState(), MacroState::Disabled);
    recorder.notifyCueJump(handle, 0, 1);
    EXPECT_EQ(recorder.getActiveChannel(), nullptr);
    EXPECT_EQ(recorder.getMacro().getLength(), 0);
    recorder.setState(MacroState::Armed);
    recorder.notifyCueJump(handle, 0, 1);
    EXPECT_EQ(recorder.getActiveChannel()->handle(), handle.handle());
    EXPECT_EQ(recorder.getMacro().actions[0].position, 0);
    EXPECT_EQ(recorder.getMacro().actions[0].target, 1);
    EXPECT_EQ(recorder.checkOrClaimRecording(handle), true);
}

TEST(MacroRecordingTest, RecordingToggleControl) {
    MacroRecorder recorder;
    ControlObject::set(ConfigKey(kMacroRecordingKey, "recording_toggle"), 1);
    EXPECT_EQ(recorder.isRecordingActive(), true);
    ControlObject::set(ConfigKey(kMacroRecordingKey, "recording_toggle"), 0);
    EXPECT_EQ(recorder.isRecordingActive(), true);
    ControlObject::set(ConfigKey(kMacroRecordingKey, "recording_toggle"), 1);
    EXPECT_EQ(recorder.isRecordingActive(), false);
}

class MacroRecorderTest : public SignalPathTest {
  public:
    MacroRecorderTest()
            : SignalPathTest(new MacroRecorder()) {
    }
};

TEST_F(MacroRecorderTest, RecordSeek) {
    ControlObject::toggle(ConfigKey(kMacroRecordingKey, "recording_toggle"));
    ASSERT_EQ(m_pMacroRecorder->isRecordingActive(), true);
    m_pChannel1->getEngineBuffer()->slotControlSeekExact(50 * mixxx::kEngineChannelCount);
    ProcessBuffer();
    EXPECT_EQ(m_pMacroRecorder->getMacro().getLength(), 0);
    m_pChannel1->getEngineBuffer()->slotControlSeekAbs(10 * mixxx::kEngineChannelCount);
    ProcessBuffer();
    EXPECT_EQ(m_pMacroRecorder->getMacro().getLength(), 1);
    EXPECT_EQ(m_pMacroRecorder->getMacro().actions[0].position, 50);
    EXPECT_EQ(m_pMacroRecorder->getMacro().actions[0].target, 10);
}

TEST_F(MacroRecorderTest, RecordHotcueActivation) {
    ControlObject::toggle(ConfigKey(kMacroRecordingKey, "recording_toggle"));
    ASSERT_EQ(m_pMacroRecorder->isRecordingActive(), true);
    ControlObject::toggle(ConfigKey("[Channel1]", "hotcue_1_activate"));
    ProcessBuffer();
    m_pChannel1->getEngineBuffer()->slotControlSeekExact(100 * mixxx::kEngineChannelCount);
    ProcessBuffer();
    EXPECT_EQ(m_pMacroRecorder->getMacro().getLength(), 0);
    ControlObject::toggle(ConfigKey("[Channel1]", "hotcue_1_activate"));
    ProcessBuffer();
    EXPECT_EQ(m_pMacroRecorder->getMacro().getLength(), 1);
    EXPECT_EQ(m_pMacroRecorder->getMacro().actions[0].position, 100);
    EXPECT_EQ(m_pMacroRecorder->getMacro().actions[0].target, 0);
}
