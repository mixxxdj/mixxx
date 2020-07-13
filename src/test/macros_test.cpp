#include <gtest/gtest.h>

#include "macros/macrorecorder.h"
#include "signalpathtest.h"

TEST(MacrosTest, CreateMacro) {
    Macro macro;
    EXPECT_EQ(macro.getLength(), 0);
    macro.appendJump(0, 1);
    EXPECT_EQ(macro.getLength(), 1);
    QString filename(QDir::currentPath() % "/src/test/macro-proto-test");
    ASSERT_TRUE(QFile::exists(filename));
    QFile file(filename);
    file.open(QIODevice::OpenModeFlag::ReadOnly);
    QByteArray content = file.readAll();
    QByteArray serialized = macro.serialize();
    EXPECT_EQ(serialized.length(), content.length());
    EXPECT_EQ(serialized, content);
}

TEST(MacroRecordingTest, ClaimRecording) {
    MacroRecorder recorder;
    EXPECT_EQ(recorder.isRecordingActive(), false);
    recorder.claimRecording();
    EXPECT_EQ(recorder.isRecordingActive(), false);
    recorder.startRecording();
    EXPECT_EQ(ControlProxy(MacroRecorder::kControlsGroup, "recording_status").get(), 1);
    recorder.claimRecording();
    EXPECT_EQ(recorder.isRecordingActive(), true);
}

TEST(MacroRecordingTest, RecordCueJump) {
    MacroRecorder recorder;
    auto factory = ChannelHandleFactory();
    ChannelHandle handle = factory.getOrCreateHandle("test-one");
    EXPECT_EQ(recorder.getState(), MacroRecordingState::Disabled);

    recorder.notifyCueJump(handle, 0, 1);
    EXPECT_EQ(recorder.getActiveChannel(), nullptr);
    EXPECT_EQ(recorder.getMacro().getLength(), 0);

    recorder.startRecording();
    recorder.notifyCueJump(handle, 0, 1);
    EXPECT_EQ(recorder.getActiveChannel()->handle(), handle.handle());
    EXPECT_EQ(recorder.getMacro().actions[0].position, 0);
    EXPECT_EQ(recorder.getMacro().actions[0].target, 1);

    auto handle2 = factory.getOrCreateHandle("test-two");
    EXPECT_EQ(recorder.checkOrClaimRecording(handle2), false);
    EXPECT_EQ(recorder.checkOrClaimRecording(handle), true);

    recorder.pollRecordingStart();
    EXPECT_EQ(ControlProxy(MacroRecorder::kControlsGroup, "recording_status").get(), 2);
}

TEST(MacroRecordingTest, RecordingToggleControl) {
    MacroRecorder recorder;
    ControlObject::set(ConfigKey(MacroRecorder::kControlsGroup, "recording_toggle"), 1);
    EXPECT_EQ(recorder.isRecordingActive(), true);
    ControlObject::set(ConfigKey(MacroRecorder::kControlsGroup, "recording_toggle"), 0);
    EXPECT_EQ(recorder.isRecordingActive(), true);
    ControlObject::set(ConfigKey(MacroRecorder::kControlsGroup, "recording_toggle"), 1);
    EXPECT_EQ(recorder.isRecordingActive(), false);
}

// Integration Tests

class MacroRecorderTest : public SignalPathTest {
  public:
    MacroRecorderTest()
            : SignalPathTest(new MacroRecorder()) {
    }
};

TEST_F(MacroRecorderTest, RecordSeek) {
    ControlObject::toggle(ConfigKey(MacroRecorder::kControlsGroup, "recording_toggle"));
    ASSERT_EQ(m_pMacroRecorder->isRecordingActive(), true);
    EXPECT_EQ(ControlProxy(MacroRecorder::kControlsGroup, "recording_status").get(), 1);
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
    ControlObject::toggle(ConfigKey(MacroRecorder::kControlsGroup, "recording_toggle"));
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
