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

TEST(MacroRecordingTest, ClaimRecording) {
    MacroRecorder recorder;
    EXPECT_EQ(recorder.isRecordingActive(), false);
    recorder.claimRecording();
    EXPECT_EQ(recorder.isRecordingActive(), false);
    EXPECT_EQ(
            ControlProxy(kConfigGroup, "recording_status").get(),
            MacroRecorder::Status::Disabled);
    recorder.startRecording();
    EXPECT_EQ(
            ControlProxy(kConfigGroup, "recording_status").get(),
            MacroRecorder::Status::Armed);
    recorder.claimRecording();
    EXPECT_EQ(recorder.isRecordingActive(), true);
    recorder.setState(MacroRecorder::State::Armed);
    recorder.stopRecording();
    EXPECT_EQ(
            ControlProxy(kConfigGroup, "recording_status").get(),
            MacroRecorder::Status::Disabled);
}

TEST(MacroRecordingTest, RecordCueJump) {
    MacroRecorder recorder;
    auto factory = ChannelHandleFactory();
    ChannelHandle handle = factory.getOrCreateHandle("test-one");
    EXPECT_EQ(recorder.getState(), MacroRecorder::State::Disabled);

    recorder.notifyCueJump(&handle, 0, 1);
    EXPECT_EQ(recorder.getActiveChannel(), nullptr);
    EXPECT_EQ(recorder.getMacro().getLength(), 0);

    recorder.startRecording();
    recorder.notifyCueJump(&handle, 0, 1);
    EXPECT_EQ(recorder.getActiveChannel()->handle(), handle.handle());
    EXPECT_EQ(recorder.getMacro().actions[0].position, 0);
    EXPECT_EQ(recorder.getMacro().actions[0].target, 1);

    auto handle2 = factory.getOrCreateHandle("test-two");
    EXPECT_EQ(recorder.checkOrClaimRecording(&handle2), false);
    EXPECT_EQ(recorder.checkOrClaimRecording(&handle), true);

    recorder.pollRecordingStart();
    EXPECT_EQ(
            ControlProxy(kConfigGroup, "recording_status").get(),
            MacroRecorder::Status::Recording);
}

TEST(MacroRecordingTest, StopRecordingAsync) {
    MacroRecorder recorder;
    recorder.setState(MacroRecorder::State::Recording);
    std::atomic<MacroRecorder::State> state{};
    QtConcurrent::run([&recorder, &state] {
        QThread::msleep(100);
        state.store(recorder.getState());
        recorder.setState(MacroRecorder::State::Armed);
    });
    recorder.stopRecording();
    EXPECT_EQ(state.load(), MacroRecorder::State::Recording);
    EXPECT_EQ(recorder.getState(), MacroRecorder::State::Disabled);
}

TEST(MacroRecordingTest, RecordingToggleControl) {
    MacroRecorder recorder;
    ControlObject::set(
            ConfigKey(kConfigGroup, "recording_toggle"), 1);
    EXPECT_EQ(recorder.isRecordingActive(), true);
    ControlObject::set(
            ConfigKey(kConfigGroup, "recording_toggle"), 0);
    EXPECT_EQ(recorder.isRecordingActive(), true);
    ControlObject::set(
            ConfigKey(kConfigGroup, "recording_toggle"), 1);
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
    ControlObject::toggle(
            ConfigKey(kConfigGroup, "recording_toggle"));
    ASSERT_EQ(m_pMacroRecorder->isRecordingActive(), true);
    EXPECT_EQ(
            ControlProxy(kConfigGroup, "recording_status").get(),
            MacroRecorder::Status::Armed);
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
    ControlObject::toggle(ConfigKey(kConfigGroup, "recording_toggle"));
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
