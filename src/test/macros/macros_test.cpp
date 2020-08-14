#include "macros_test.h"

#include <QtConcurrent>

TEST(MacrosTest, SerializeMacroActions) {
    QVector<MacroAction> actions{MacroAction(0, 1)};
    ASSERT_EQ(actions.length(), 1);

    QString filename(QDir::currentPath() % "/src/test/macros/macro_proto");
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

TEST(MacroRecordingTest, StartAndStopRecordingCOs) {
    MacroRecorder recorder;
    ASSERT_EQ(ControlProxy(kConfigGroup, "status").get(),
            MacroRecorder::Status::Disabled);
    EXPECT_EQ(recorder.isRecordingActive(), false);

    ControlObject::set(ConfigKey(kConfigGroup, "record"), 1);
    ASSERT_EQ(ControlProxy(kConfigGroup, "status").get(),
            MacroRecorder::Status::Armed);
    EXPECT_EQ(recorder.isRecordingActive(), true);

    ControlObject::set(ConfigKey(kConfigGroup, "record"), 1);
    ASSERT_EQ(ControlProxy(kConfigGroup, "status").get(),
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
    checkRecordedAction(&recorder);

    auto handle2 = factory.getOrCreateHandle("test-two");
    recorder.notifyCueJump(&handle2, 0, 2);
    EXPECT_EQ(recorder.checkOrClaimRecording(&handle2), false);
    EXPECT_EQ(recorder.checkOrClaimRecording(&handle), true);
    ASSERT_EQ(recorder.getRecordingSize(), 0);

    MacroAction otherAction(3, 5);
    recorder.notifyCueJump(&handle, otherAction.position, otherAction.target);
    checkRecordedAction(&recorder, otherAction);

    recorder.pollRecordingStart();
    EXPECT_EQ(ControlProxy(kConfigGroup, "status").get(),
            MacroRecorder::Status::Recording);
}
