#include <QDebug>

#include "macro_test.h"
#include "test/signalpathtest.h"

constexpr int kMacro = 2;
const mixxx::audio::FramePos kStartPos(0);
class MacroRecordingTest : public BaseSignalPathTest {
  public:
    MacroRecordingTest()
            : BaseSignalPathTest(),
              m_pEngineBuffer1(m_pChannel1->getEngineBuffer()),
              m_status(kChannelGroup, QString("macro_%1_status").arg(kMacro)),
              m_record(kChannelGroup, QString("macro_%1_record").arg(kMacro)) {
        TrackPointer pTrack = getTestTrack();
        loadTrack(m_pMixerDeck1, pTrack);
        ProcessBuffer();
    }

    MacroControl::Status getStatus() {
        return MacroControl::Status(m_status.get());
    }

    MacroPointer getMacro() {
        return m_pEngineBuffer1->getLoadedTrack()->getMacro(kMacro);
    }

    /// Starts recording and performs the initial jump to samplePos with assertions
    void prepRecording(mixxx::audio::FramePos samplePos) {
        m_record.set(1);
        ASSERT_EQ(getStatus(), MacroControl::Status::Armed);

        m_pEngineBuffer1->seekExact(kStartPos);
        ProcessBuffer();
        ASSERT_EQ(m_pEngineBuffer1->getExactPlayPos(), kStartPos);
        m_pEngineBuffer1->seekAbs(kStartPos);
        ProcessBuffer();

        m_pEngineBuffer1->seekExact(samplePos);
        ProcessBuffer();
        ASSERT_EQ(m_pEngineBuffer1->getExactPlayPos(), samplePos);
    }

    EngineBuffer* m_pEngineBuffer1;
    ControlProxy m_status;
    ControlProxy m_record;
};

TEST_F(MacroRecordingTest, RecordSeekAndPlay) {
    TestMacro testMacro;
    prepRecording(testMacro.action.getSourcePosition());

    m_pEngineBuffer1->seekAbs(testMacro.action.getTargetPosition());
    ProcessBuffer();

    m_record.set(0);
    testMacro.checkMacroAction(getMacro());
    // Should activate automatically
    EXPECT_EQ(getStatus(), MacroControl::Status::Playing);
    EXPECT_EQ(getMacro()->getActions().first().getTargetPosition(), kStartPos);
    ProcessBuffer();
    EXPECT_EQ(m_pEngineBuffer1->getExactPlayPos(), kStartPos);

    MacroAction action2(mixxx::audio::FramePos(1'000), mixxx::audio::FramePos(9'000));
    getMacro()->addAction(action2);
    MacroAction action3(
            mixxx::audio::FramePos(action2.getTargetPosition() + 100),
            mixxx::audio::FramePos(14'000));
    getMacro()->addAction(action3);
    EXPECT_EQ(getMacro()->size(), 4);

    // Seek to first action
    m_pEngineBuffer1->seekExact(testMacro.action.getSourcePosition());
    ProcessBuffer();

    ProcessBuffer();
    ProcessBuffer();
    EXPECT_EQ(m_pEngineBuffer1->getExactPlayPos(), testMacro.action.getTargetPosition());
    ASSERT_EQ(getStatus(), MacroControl::Status::Playing);

    // Seek to next action
    m_pEngineBuffer1->seekExact(action2.getSourcePosition());
    ProcessBuffer();

    // Trigger remaining actions
    ProcessBuffer();
    EXPECT_EQ(m_pEngineBuffer1->getExactPlayPos(), action2.getTargetPosition());
    ProcessBuffer();
    EXPECT_EQ(m_pEngineBuffer1->getExactPlayPos(), action3.getTargetPosition());
    EXPECT_EQ(getStatus(), MacroControl::Status::Recorded);
}

TEST_F(MacroRecordingTest, RecordHotcueAndPlay) {
    // Place hotcue 1 at position 0
    ControlObject::set(ConfigKey(kChannelGroup, "hotcue_1_set"), 1.0);
    EXPECT_EQ(ControlObject::get(ConfigKey(kChannelGroup, "hotcue_1_position")), 0);

    TestMacro testMacro(10'000, 0);
    EXPECT_EQ(m_pEngineBuffer1->getExactPlayPos(), testMacro.action.getTargetPosition());
    prepRecording(testMacro.action.getSourcePosition());

    ControlObject::set(ConfigKey(kChannelGroup, "hotcue_1_goto"), 1.0);
    ProcessBuffer();
    EXPECT_EQ(m_pEngineBuffer1->getExactPlayPos().value(),
            testMacro.action.getTargetPosition().value());
    MacroPointer pMacro = getMacro();

    // Check that recording stops gracefully when ejecting
    m_pEngineBuffer1->ejectTrack();
    EXPECT_EQ(getStatus(), MacroControl::Status::NoTrack);
    testMacro.checkMacroAction(pMacro);
}
