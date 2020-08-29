#include <QDebug>

#include "macro_test.h"
#include "test/signalpathtest.h"

constexpr int kMacro = 2;
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

    void toggleRecording() {
        m_record.set(1);
    }

    MacroPtr getMacro() {
        return m_pEngineBuffer1->getLoadedTrack()->getMacros().value(kMacro);
    }

    void prepRecording(double position = kAction.position) {
        toggleRecording();
        ASSERT_EQ(getStatus(), MacroControl::Status::Armed);

        m_pEngineBuffer1->slotControlSeekAbs(0);
        ProcessBuffer();

        m_pEngineBuffer1->slotControlSeekExact(position * mixxx::kEngineChannelCount);
        ProcessBuffer();
    }

    EngineBuffer* m_pEngineBuffer1;
    ControlProxy m_status;
    ControlProxy m_record;
};

TEST_F(MacroRecordingTest, RecordSeekAndPlay) {
    prepRecording();

    m_pEngineBuffer1->slotControlSeekAbs(kAction.getTargetSamplePos());
    ProcessBuffer();

    toggleRecording();
    checkMacroAction(getMacro());
    // Should activate automatically
    EXPECT_EQ(getStatus(), MacroControl::Status::Playing);
    double startPos = getMacro()->getActions().first().getTargetSamplePos();
    EXPECT_EQ(startPos, 0);
    ProcessBuffer();
    EXPECT_EQ(m_pEngineBuffer1->getExactPlayPos(), startPos);

    MacroAction action2(1'000, 9'000);
    getMacro()->addAction(action2);
    MacroAction action3(action2.target + 100, 14'000);
    getMacro()->addAction(action3);
    EXPECT_EQ(getMacro()->size(), 4);

    // Seek to first action
    m_pEngineBuffer1->slotControlSeekExact(kAction.getSamplePos());
    ProcessBuffer();
    ASSERT_EQ(m_pEngineBuffer1->getExactPlayPos(), kAction.getSamplePos());

    ProcessBuffer();
    ProcessBuffer();
    EXPECT_EQ(m_pEngineBuffer1->getExactPlayPos(), kAction.getTargetSamplePos());
    ASSERT_EQ(getStatus(), MacroControl::Status::Playing);

    // Seek to next action
    m_pEngineBuffer1->slotControlSeekExact(action2.getSamplePos());
    ProcessBuffer();
    ASSERT_EQ(m_pEngineBuffer1->getExactPlayPos(), action2.getSamplePos());

    // Trigger remaining actions
    ProcessBuffer();
    EXPECT_EQ(m_pEngineBuffer1->getExactPlayPos(), action2.getTargetSamplePos());
    ProcessBuffer();
    EXPECT_EQ(m_pEngineBuffer1->getExactPlayPos(), action3.getTargetSamplePos());
    EXPECT_EQ(getStatus(), MacroControl::Status::PlaybackStopped);
}

TEST_F(MacroRecordingTest, RecordHotcueAndPlay) {
    // Place hotcue 1 at position 0
    ControlObject::set(ConfigKey(kChannelGroup, "hotcue_1_set"), 1);

    MacroAction action(10'000, 0);
    prepRecording(action.position);

    ControlObject::set(ConfigKey(kChannelGroup, "hotcue_1_goto"), 1);
    ProcessBuffer();
    EXPECT_EQ(m_pEngineBuffer1->getExactPlayPos(), action.getTargetSamplePos());
    MacroPtr pMacro = getMacro();

    // Check that recording stops gracefully when ejecting
    m_pEngineBuffer1->slotEjectTrack(1);
    EXPECT_EQ(getStatus(), MacroControl::Status::NoTrack);
    checkMacroAction(pMacro, action);
}
