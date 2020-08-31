#include <QDebug>

#include "macro_test.h"
#include "test/signalpathtest.h"

constexpr int kMacro = 2;
constexpr int kStartPos = 0;
class MacroRecordingTest : public BaseSignalPathTest {
  public:
    MacroRecordingTest()
            : BaseSignalPathTest(),
              m_pEngineBuffer1(m_pChannel1->getEngineBuffer()),
              m_status(kChannelGroup, QString("macro_%1_status").arg(kMacro)),
              m_record(kChannelGroup, QString("macro_%1_record").arg(kMacro)) {
        TrackPointer pTrack = getTestTrack();
        loadTrack(m_pMixerDeck1, pTrack);
    }

    MacroControl::Status getStatus() {
        return MacroControl::Status(m_status.get());
    }

    MacroPointer getMacro() {
        return m_pEngineBuffer1->getLoadedTrack()->getMacros().value(kMacro);
    }

    /// Starts recording and performs the initial jump to samplePos + assertions
    void prepRecording(double samplePos = kAction.getSourcePositionSample()) {
        m_record.set(1);
        ASSERT_EQ(getStatus(), MacroControl::Status::Armed);

        m_pEngineBuffer1->slotControlSeekExact(kStartPos);
        ProcessBuffer();
        ASSERT_EQ(m_pEngineBuffer1->getExactPlayPos(), kStartPos);
        m_pEngineBuffer1->slotControlSeekAbs(kStartPos);
        ProcessBuffer();

        m_pEngineBuffer1->slotControlSeekExact(samplePos);
        ProcessBuffer();
        ASSERT_EQ(m_pEngineBuffer1->getExactPlayPos(), samplePos);
    }

    EngineBuffer* m_pEngineBuffer1;
    ControlProxy m_status;
    ControlProxy m_record;
};

TEST_F(MacroRecordingTest, RecordSeekAndPlay) {
    prepRecording();

    m_pEngineBuffer1->slotControlSeekAbs(kAction.getTargetPositionSample());
    ProcessBuffer();

    m_record.set(0);
    checkMacroAction(getMacro());
    // Should activate automatically
    EXPECT_EQ(getStatus(), MacroControl::Status::Playing);
    EXPECT_EQ(getMacro()->getActions().first().getTargetPositionSample(), kStartPos);
    ProcessBuffer();
    EXPECT_EQ(m_pEngineBuffer1->getExactPlayPos(), kStartPos);

    MacroAction action2(1'000, 9'000);
    getMacro()->addAction(action2);
    MacroAction action3(action2.targetFrame + 100, 14'000);
    getMacro()->addAction(action3);
    EXPECT_EQ(getMacro()->size(), 4);

    // Seek to first action
    m_pEngineBuffer1->slotControlSeekExact(kAction.getSourcePositionSample());
    ProcessBuffer();

    ProcessBuffer();
    ProcessBuffer();
    EXPECT_EQ(m_pEngineBuffer1->getExactPlayPos(), kAction.getTargetPositionSample());
    ASSERT_EQ(getStatus(), MacroControl::Status::Playing);

    // Seek to next action
    m_pEngineBuffer1->slotControlSeekExact(action2.getSourcePositionSample());
    ProcessBuffer();

    // Trigger remaining actions
    ProcessBuffer();
    EXPECT_EQ(m_pEngineBuffer1->getExactPlayPos(), action2.getTargetPositionSample());
    ProcessBuffer();
    EXPECT_EQ(m_pEngineBuffer1->getExactPlayPos(), action3.getTargetPositionSample());
    EXPECT_EQ(getStatus(), MacroControl::Status::Recorded);
}

TEST_F(MacroRecordingTest, RecordHotcueAndPlay) {
    // Place hotcue 1 at position 0
    ControlObject::set(ConfigKey(kChannelGroup, "hotcue_1_set"), 1);

    MacroAction action(10'000, 0);
    prepRecording(action.getSourcePositionSample());

    ControlObject::set(ConfigKey(kChannelGroup, "hotcue_1_goto"), 1);
    ProcessBuffer();
    EXPECT_EQ(m_pEngineBuffer1->getExactPlayPos(), action.getTargetPositionSample());
    MacroPointer pMacro = getMacro();

    // Check that recording stops gracefully when ejecting
    m_pEngineBuffer1->slotEjectTrack(1);
    EXPECT_EQ(getStatus(), MacroControl::Status::NoTrack);
    checkMacroAction(pMacro, action);
}
