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
        pTrack->setMacros({{kMacro, std::make_shared<Macro>()}});
        loadTrack(m_pMixerDeck1, pTrack);
    }

    MacroControl::Status getStatus() {
        return MacroControl::Status(m_status.get());
    }

    void setRecording() {
        m_record.set(1);
    }

    MacroPtr getMacro() {
        return m_pEngineBuffer1->getLoadedTrack()->getMacros().value(kMacro);
    }

    EngineBuffer* m_pEngineBuffer1;
    ControlProxy m_status;
    ControlProxy m_record;
};

TEST_F(MacroRecordingTest, RecordSeek) {
    setRecording();
    ASSERT_EQ(getStatus(), MacroControl::Status::Armed);

    m_pEngineBuffer1->slotControlSeekExact(
            kAction.position * mixxx::kEngineChannelCount);
    ProcessBuffer();
    EXPECT_EQ(getMacro()->size(), 0);

    m_pEngineBuffer1->slotControlSeekAbs(
            kAction.target * mixxx::kEngineChannelCount);
    ProcessBuffer();
    setRecording();
    EXPECT_EQ(getStatus(), MacroControl::Status::Playing);
    checkMacroAction(getMacro());
}

TEST_F(MacroRecordingTest, RecordHotcueActivation) {
    MacroAction action(100, 0);
    setRecording();
    ASSERT_EQ(getStatus(), MacroControl::Status::Armed);

    // Place hotcue 1 at position 0
    ControlObject::set(ConfigKey(kChannelGroup, "hotcue_1_set"), 1);

    m_pEngineBuffer1->slotControlSeekExact(action.position * mixxx::kEngineChannelCount);
    ProcessBuffer();
    EXPECT_EQ(getMacro()->size(), 0);

    ControlObject::set(ConfigKey(kChannelGroup, "hotcue_1_goto"), 1);
    ProcessBuffer();
    EXPECT_EQ(m_pEngineBuffer1->getExactPlayPos(), action.target * mixxx::kEngineChannelCount);
    MacroPtr pMacro = getMacro();

    // Check that recording stops gracefully when ejecting
    m_pEngineBuffer1->slotEjectTrack(1);
    EXPECT_EQ(getStatus(), MacroControl::Status::NoTrack);
    checkMacroAction(pMacro, action);
}
