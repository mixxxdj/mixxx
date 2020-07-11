#include "macros/macromanager.h"

#include <gtest/gtest.h>

#include <QDebug>

#include "mixxxtest.h"
#include "signalpathtest.h"

TEST(MacrosTest, CreateMacro) {
    Macro macro;
    ASSERT_EQ(macro.getLength(), 0);
}

TEST(MacroManagerTest, ClaimRecording) {
    MacroManager macroManager;
    EXPECT_EQ(macroManager.isRecordingActive(), false);
    macroManager.claimRecording();
    EXPECT_EQ(macroManager.isRecordingActive(), false);
    macroManager.setState(MacroState::Armed);
    macroManager.claimRecording();
    EXPECT_EQ(macroManager.isRecordingActive(), true);
}

TEST(MacroManagerTest, RecordCueJump) {
    MacroManager macroManager;
    ChannelHandle handle = ChannelHandleFactory().getOrCreateHandle("");
    EXPECT_EQ(macroManager.getState(), MacroState::Disabled);
    macroManager.notifyCueJump(handle, 0, 1);
    EXPECT_EQ(macroManager.getActiveChannel(), nullptr);
    EXPECT_EQ(macroManager.getMacro().getLength(), 0);
    macroManager.setState(MacroState::Armed);
    macroManager.notifyCueJump(handle, 0, 1);
    EXPECT_EQ(macroManager.getActiveChannel()->handle(), handle.handle());
    EXPECT_EQ(macroManager.getMacro().actions[0].position, 0);
    EXPECT_EQ(macroManager.getMacro().actions[0].target, 1);
    EXPECT_EQ(macroManager.checkOrClaimRecording(handle), true);
}

TEST(MacroManagerTest, RecordingToggleControl) {
    MacroManager macroManager;
    ControlObject::set(ConfigKey(kMacroRecordingKey, "recording_toggle"), 1);
    EXPECT_EQ(macroManager.isRecordingActive(), true);
    ControlObject::set(ConfigKey(kMacroRecordingKey, "recording_toggle"), 0);
    EXPECT_EQ(macroManager.isRecordingActive(), true);
    ControlObject::set(ConfigKey(kMacroRecordingKey, "recording_toggle"), 1);
    EXPECT_EQ(macroManager.isRecordingActive(), false);
}

class MacroManagerE2ETest : public SignalPathTest {
  public:
    MacroManagerE2ETest()
            : SignalPathTest(new MacroManager()) {
    }
};

TEST_F(MacroManagerE2ETest, RecordSeek) {
    auto mgr = m_pMacroManager;
    ControlObject::toggle(ConfigKey(kMacroRecordingKey, "recording_toggle"));
    ASSERT_EQ(mgr->isRecordingActive(), true);
    m_pChannel1->getEngineBuffer()->slotControlSeekExact(50 * mixxx::kEngineChannelCount);
    ProcessBuffer();
    EXPECT_EQ(mgr->getMacro().getLength(), 0);
    m_pChannel1->getEngineBuffer()->slotControlSeekAbs(10 * mixxx::kEngineChannelCount);
    ProcessBuffer();
    EXPECT_EQ(mgr->getMacro().getLength(), 1);
    EXPECT_EQ(mgr->getMacro().actions[0].position, 50);
    EXPECT_EQ(mgr->getMacro().actions[0].target, 10);
}

TEST_F(MacroManagerE2ETest, RecordHotcueActivation) {
    auto mgr = m_pMacroManager;
    ControlObject::toggle(ConfigKey(kMacroRecordingKey, "recording_toggle"));
    ASSERT_EQ(mgr->isRecordingActive(), true);
    ControlObject::toggle(ConfigKey("[Channel1]", "hotcue_1_activate"));
    ProcessBuffer();
    m_pChannel1->getEngineBuffer()->slotControlSeekExact(100 * mixxx::kEngineChannelCount);
    ProcessBuffer();
    EXPECT_EQ(mgr->getMacro().getLength(), 0);
    ControlObject::toggle(ConfigKey("[Channel1]", "hotcue_1_activate"));
    ProcessBuffer();
    EXPECT_EQ(mgr->getMacro().getLength(), 1);
    EXPECT_EQ(mgr->getMacro().actions[0].position, 100);
    EXPECT_EQ(mgr->getMacro().actions[0].target, 0);
}
