#include "macros_test.h"
#include "test/signalpathtest.h"

class MacroRecordingTest : public SignalPathTest {
  public:
    MacroRecordingTest()
            : SignalPathTest(new MacroRecorder()),
              m_pEngineBuffer1(m_pChannel1->getEngineBuffer()) {
    }

    void checkRecordedAction(MacroAction action = kAction) {
        return ::checkRecordedAction(m_pMacroRecorder, action);
    }

    EngineBuffer* m_pEngineBuffer1;
};

TEST_F(MacroRecordingTest, RecordSeek) {
    ControlObject::set(ConfigKey(kConfigGroup, "record"), 1);
    ASSERT_EQ(m_pMacroRecorder->isRecordingActive(), true);
    EXPECT_EQ(ControlProxy(kConfigGroup, "status").get(),
            MacroRecorder::Status::Armed);

    m_pEngineBuffer1->slotControlSeekExact(
            kAction.position * mixxx::kEngineChannelCount);
    ProcessBuffer();
    EXPECT_EQ(m_pMacroRecorder->getRecordingSize(), 0);

    m_pEngineBuffer1->slotControlSeekAbs(
            kAction.target * mixxx::kEngineChannelCount);
    ProcessBuffer();
    checkRecordedAction();
}

TEST_F(MacroRecordingTest, RecordHotcueActivation) {
    MacroAction action(100, 0);
    ControlObject::set(ConfigKey(kConfigGroup, "record"), 1);
    ASSERT_EQ(m_pMacroRecorder->isRecordingActive(), true);

    // Place hotcue 1 at position 0
    ControlObject::set(ConfigKey("[Channel1]", "hotcue_1_set"), 1);

    m_pEngineBuffer1->slotControlSeekExact(action.position * mixxx::kEngineChannelCount);
    ProcessBuffer();
    EXPECT_EQ(m_pMacroRecorder->getRecordingSize(), 0);

    ControlObject::set(ConfigKey("[Channel1]", "hotcue_1_goto"), 1);
    ProcessBuffer();
    EXPECT_EQ(m_pEngineBuffer1->getExactPlayPos(), action.target * mixxx::kEngineChannelCount);
    checkRecordedAction(action);

    // Check that recording stops when ejecting
    m_pEngineBuffer1->slotEjectTrack(1);
    EXPECT_EQ(m_pMacroRecorder->isRecordingActive(), false);
}
