#include "recording/macromanager.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QDebug>

#include "mixxxtest.h"

class MacroManagerTest : protected MixxxTest {
};

TEST(MacrosTest, CreateMacro) {
    auto macro = new Macro();
    ASSERT_EQ(macro->m_length, 0);
}

TEST(MacroManagerTest, ClaimRecording) {
    auto mgr = MacroManager();
    EXPECT_EQ(mgr.isRecordingActive(), false);
    mgr.claimRecording();
    EXPECT_EQ(mgr.isRecordingActive(), false);
    mgr.m_macroRecordingState.store(MacroState::Armed);
    mgr.claimRecording();
    EXPECT_EQ(mgr.isRecordingActive(), true);
}

TEST(MacroManagerTest, RecordCueJump) {
    auto mgr = MacroManager();
    ChannelHandle handle = ChannelHandleFactory().getOrCreateHandle("");
    EXPECT_EQ(mgr.m_macroRecordingState.load(), MacroState::Disabled);
    mgr.notifyCueJump(handle, 0, 1);
    EXPECT_EQ(mgr.m_activeChannel, nullptr);
    EXPECT_EQ(mgr.getMacro().m_length, 0);
    mgr.m_macroRecordingState.store(MacroState::Armed);
    mgr.notifyCueJump(handle, 0, 1);
    EXPECT_EQ(mgr.m_activeChannel->handle(), handle.handle());
    EXPECT_EQ(mgr.getMacro().actions[0].position, 0);
    EXPECT_EQ(mgr.getMacro().actions[0].target, 1);
    EXPECT_EQ(mgr.checkOrClaimRecording(handle), true);
}

TEST(MacroManagerTest, RecordingToggleControl) {
    auto mgr = MacroManager();
    ControlObject::set(ConfigKey(kMacroRecordingKey, "recording_toggle"), 1);
    EXPECT_EQ(mgr.isRecordingActive(), true);
    ControlObject::set(ConfigKey(kMacroRecordingKey, "recording_toggle"), 0);
    EXPECT_EQ(mgr.isRecordingActive(), true);
    ControlObject::set(ConfigKey(kMacroRecordingKey, "recording_toggle"), 1);
    EXPECT_EQ(mgr.isRecordingActive(), false);
}