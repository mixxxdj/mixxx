#include <gtest/gtest.h>

#include <QtConcurrent>

#include "macros/macrorecorder.h"
#include "signalpathtest.h"

namespace {
const QString kConfigGroup = QStringLiteral("[MacroRecording]");
}

TEST(MacrosTest, CreateAndSerializeMacro) {
    Macro macro;
    EXPECT_EQ(macro.getLength(), 0);
    macro.appendJump(0, 1);
    EXPECT_EQ(macro.getLength(), 1);
    EXPECT_EQ(macro.actions[0], MacroAction(0, 1));

    QString filename(QDir::currentPath() % "/src/test/macro_proto");
    ASSERT_TRUE(QFile::exists(filename));
    QFile file(filename);
    file.open(QIODevice::OpenModeFlag::ReadOnly);
    QByteArray content = file.readAll();
    QByteArray serialized = macro.serialize();
    EXPECT_EQ(serialized.length(), content.length());
    EXPECT_EQ(serialized, content);
    Macro deserialized(serialized);
    EXPECT_EQ(deserialized.getLength(), macro.getLength());
    EXPECT_EQ(deserialized, macro);
}

TEST(MacroRecordingTest, StartAndStopRecording) {
    MacroRecorder recorder;
    ASSERT_EQ(ControlProxy(kConfigGroup, "recording_status").get(),
            MacroRecorder::Status::Disabled);
    EXPECT_EQ(recorder.isRecordingActive(), false);
    recorder.startRecording();
    ASSERT_EQ(ControlProxy(kConfigGroup, "recording_status").get(),
            MacroRecorder::Status::Armed);
    EXPECT_EQ(recorder.isRecordingActive(), true);
    recorder.stopRecording();
    ASSERT_EQ(ControlProxy(kConfigGroup, "recording_status").get(),
            MacroRecorder::Status::Disabled);
    EXPECT_EQ(recorder.isRecordingActive(), false);
}

TEST(MacroRecordingTest, RecordCueJump) {
    MacroRecorder recorder;
    auto factory = ChannelHandleFactory();
    ChannelHandle handle = factory.getOrCreateHandle("test-one");
    ASSERT_EQ(recorder.getStatus(), MacroRecorder::Status::Disabled);

    recorder.notifyCueJump(&handle, 0, 1);
    ASSERT_EQ(recorder.getActiveChannel(), nullptr);
    EXPECT_EQ(recorder.getRecordingSize(), 0);

    recorder.startRecording();
    recorder.notifyCueJump(&handle, 0, 1);
    EXPECT_EQ(recorder.getActiveChannel()->handle(), handle.handle());
    ASSERT_EQ(recorder.getRecordingSize(), 1);
    EXPECT_EQ(recorder.getRecordedAction().position, 0);
    EXPECT_EQ(recorder.getRecordedAction().target, 1);

    auto handle2 = factory.getOrCreateHandle("test-two");
    recorder.notifyCueJump(&handle2, 0, 2);
    EXPECT_EQ(recorder.checkOrClaimRecording(&handle2), false);
    EXPECT_EQ(recorder.checkOrClaimRecording(&handle), true);
    ASSERT_EQ(recorder.getRecordingSize(), 1);

    recorder.notifyCueJump(&handle, 3, 5);
    EXPECT_EQ(recorder.getRecordingSize(), 2);

    recorder.pollRecordingStart();
    EXPECT_EQ(ControlProxy(kConfigGroup, "recording_status").get(),
            MacroRecorder::Status::Recording);
}

TEST(MacroRecordingTest, RecordingToggleControl) {
    MacroRecorder recorder;
    ControlObject::toggle(
            ConfigKey(kConfigGroup, "recording_toggle"));
    ASSERT_EQ(recorder.isRecordingActive(), true);
    ControlObject::toggle(
            ConfigKey(kConfigGroup, "recording_toggle"));
    ASSERT_EQ(recorder.isRecordingActive(), false);
}

// Integration Tests

class MacroRecorderTest : public SignalPathTest {
  public:
    MacroRecorderTest()
            : SignalPathTest(new MacroRecorder()) {
    }
};

TEST_F(MacroRecorderTest, RecordSeek) {
    ControlObject::toggle(
            ConfigKey(kConfigGroup, "recording_toggle"));
    ASSERT_EQ(m_pMacroRecorder->isRecordingActive(), true);
    EXPECT_EQ(ControlProxy(kConfigGroup, "recording_status").get(),
            MacroRecorder::Status::Armed);
    m_pChannel1->getEngineBuffer()->slotControlSeekExact(50 * mixxx::kEngineChannelCount);
    ProcessBuffer();
    EXPECT_EQ(m_pMacroRecorder->getRecordingSize(), 0);

    m_pChannel1->getEngineBuffer()->slotControlSeekAbs(10 * mixxx::kEngineChannelCount);
    ProcessBuffer();
    EXPECT_EQ(m_pMacroRecorder->getRecordingSize(), 1);
    EXPECT_EQ(m_pMacroRecorder->getRecordedAction().position, 50);
    EXPECT_EQ(m_pMacroRecorder->getRecordedAction().target, 10);
}

TEST_F(MacroRecorderTest, RecordHotcueActivation) {
    ControlObject::toggle(ConfigKey(kConfigGroup, "recording_toggle"));
    ASSERT_EQ(m_pMacroRecorder->isRecordingActive(), true);
    ControlObject::toggle(ConfigKey("[Channel1]", "hotcue_1_activate"));
    ProcessBuffer();
    m_pChannel1->getEngineBuffer()->slotControlSeekExact(100 * mixxx::kEngineChannelCount);
    ProcessBuffer();
    EXPECT_EQ(m_pMacroRecorder->getRecordingSize(), 0);
    ControlObject::toggle(ConfigKey("[Channel1]", "hotcue_1_activate"));
    ProcessBuffer();
    EXPECT_EQ(m_pMacroRecorder->getRecordingSize(), 1);
    EXPECT_EQ(m_pMacroRecorder->getRecordedAction().position, 100);
    EXPECT_EQ(m_pMacroRecorder->getRecordedAction().target, 0);
}
