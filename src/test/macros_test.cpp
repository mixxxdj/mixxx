#include <gtest/gtest.h>

#include <QtConcurrent>

#include "macros/macrorecorder.h"
#include "signalpathtest.h"

namespace {
const QString kConfigGroup = QStringLiteral("[MacroRecording]");

const MacroAction s_action(500, 15);
void checkRecordedAction(MacroRecorder* recorder, MacroAction action = s_action) {
    EXPECT_EQ(recorder->getRecordingSize(), 1);
    auto recordedAction = recorder->fetchRecordedActions().first();
    EXPECT_EQ(recordedAction.position, action.position);
    EXPECT_EQ(recordedAction.target, action.target);
}
}

TEST(MacrosTest, SerializeMacroActions) {
    QVector<MacroAction> actions;
    actions.append(MacroAction(0, 1));
    ASSERT_EQ(actions.length(), 1);

    QString filename(QDir::currentPath() % "/src/test/macro_proto");
    ASSERT_TRUE(QFile::exists(filename));
    QFile file(filename);
    file.open(QIODevice::OpenModeFlag::ReadOnly);
    QByteArray content = file.readAll();
    QByteArray serialized = Macro::serialize(actions);
    EXPECT_EQ(serialized.length(), content.length());
    EXPECT_EQ(serialized, content);
    QVector<MacroAction> deserialized = Macro::deserialize(serialized);
    EXPECT_EQ(deserialized.size(), 1);
    EXPECT_EQ(deserialized, actions);
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

    recorder.notifyCueJump(&handle, s_action.position, s_action.target);
    ASSERT_EQ(recorder.getActiveChannel(), nullptr);
    EXPECT_EQ(recorder.getRecordingSize(), 0);

    recorder.startRecording();
    recorder.notifyCueJump(&handle, s_action.position, s_action.target);
    EXPECT_EQ(recorder.getActiveChannel()->handle(), handle.handle());
    ::checkRecordedAction(&recorder);

    auto handle2 = factory.getOrCreateHandle("test-two");
    recorder.notifyCueJump(&handle2, 0, 2);
    EXPECT_EQ(recorder.checkOrClaimRecording(&handle2), false);
    EXPECT_EQ(recorder.checkOrClaimRecording(&handle), true);
    ASSERT_EQ(recorder.getRecordingSize(), 0);

    MacroAction otherAction(3, 5);
    recorder.notifyCueJump(&handle, otherAction.position, otherAction.target);
    ::checkRecordedAction(&recorder, otherAction);

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

    void checkRecordedAction(MacroAction action = s_action) {
        return ::checkRecordedAction(m_pMacroRecorder, action);
    }
};

TEST_F(MacroRecorderTest, RecordSeek) {
    ControlObject::toggle(
            ConfigKey(kConfigGroup, "recording_toggle"));
    ASSERT_EQ(m_pMacroRecorder->isRecordingActive(), true);
    EXPECT_EQ(ControlProxy(kConfigGroup, "recording_status").get(),
            MacroRecorder::Status::Armed);

    m_pChannel1->getEngineBuffer()->slotControlSeekExact(
            s_action.position * mixxx::kEngineChannelCount);
    ProcessBuffer();
    EXPECT_EQ(m_pMacroRecorder->getRecordingSize(), 0);

    m_pChannel1->getEngineBuffer()->slotControlSeekAbs(
            s_action.target * mixxx::kEngineChannelCount);
    ProcessBuffer();
    checkRecordedAction();
}

TEST_F(MacroRecorderTest, RecordHotcueActivation) {
    MacroAction action(100, 0);
    ControlObject::toggle(ConfigKey(kConfigGroup, "recording_toggle"));
    ASSERT_EQ(m_pMacroRecorder->isRecordingActive(), true);

    // Place hotcue 1 at position 0
    ControlObject::toggle(ConfigKey("[Channel1]", "hotcue_1_activate"));

    ProcessBuffer();
    m_pChannel1->getEngineBuffer()->slotControlSeekExact(
            action.position * mixxx::kEngineChannelCount);
    ProcessBuffer();
    EXPECT_EQ(m_pMacroRecorder->getRecordingSize(), 0);

    ControlObject::toggle(ConfigKey("[Channel1]", "hotcue_1_activate"));
    ProcessBuffer();
    checkRecordedAction(action);
}
