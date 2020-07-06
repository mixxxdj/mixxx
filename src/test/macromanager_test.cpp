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

TEST(MacroManagerTest, claimRecording) {
    auto handle = ChannelHandleFactory().getOrCreateHandle("");
    auto mgr = MacroManager();
    qDebug() << "Handle" << handle.handle();
    ASSERT_EQ(mgr.m_macroRecordingState.load(), MacroState::Disabled);
    mgr.notifyCueJump(handle, 0, 1);
    ASSERT_EQ(mgr.m_macroRecordingState.load(), MacroState::Disabled);
    ASSERT_EQ(mgr.m_activeChannel, nullptr);
    mgr.m_macroRecordingState.store(MacroState::Armed);
    mgr.notifyCueJump(handle, 0, 1);
    ASSERT_EQ(mgr.m_activeChannel->handle(), handle.handle());
}