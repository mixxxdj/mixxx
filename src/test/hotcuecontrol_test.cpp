#include "engine/controls/cuecontrol.h"
#include "test/signalpathtest.h"

namespace {
double getBeatLengthFrames(TrackPointer pTrack) {
    double beatLengthSecs = 60.0 / pTrack->getBpm();
    return beatLengthSecs * (pTrack->getSampleRate() * mixxx::kEngineChannelCount);
}
} // anonymous namespace

class HotcueControlTest : public BaseSignalPathTest {
  protected:
    void SetUp() override {
        BaseSignalPathTest::SetUp();

        m_pBeatloopActivate = std::make_unique<ControlProxy>(m_sGroup1, "beatloop_activate");
        m_pBeatloopSize = std::make_unique<ControlProxy>(m_sGroup1, "beatloop_size");
        m_pLoopEnabled = std::make_unique<ControlProxy>(m_sGroup1, "loop_enabled");
        m_pLoopDouble = std::make_unique<ControlProxy>(m_sGroup1, "loop_double");
        m_pLoopHalve = std::make_unique<ControlProxy>(m_sGroup1, "loop_halve");
        m_pLoopMove = std::make_unique<ControlProxy>(m_sGroup1, "loop_move");
        m_pLoopToggle = std::make_unique<ControlProxy>(m_sGroup1, "loop_toggle");
        m_pHotcue1Activate = std::make_unique<ControlProxy>(m_sGroup1, "hotcue_1_activate");
        m_pHotcue1ActivateCue = std::make_unique<ControlProxy>(m_sGroup1, "hotcue_1_activatecue");
        m_pHotcue1ActivateLoop = std::make_unique<ControlProxy>(m_sGroup1, "hotcue_1_activateloop");
        m_pHotcue1Set = std::make_unique<ControlProxy>(m_sGroup1, "hotcue_1_set");
        m_pHotcue1SetCue = std::make_unique<ControlProxy>(m_sGroup1, "hotcue_1_setcue");
        m_pHotcue1SetLoop = std::make_unique<ControlProxy>(m_sGroup1, "hotcue_1_setloop");
        m_pHotcue1Position = std::make_unique<ControlProxy>(m_sGroup1, "hotcue_1_position");
        m_pHotcue1EndPosition = std::make_unique<ControlProxy>(m_sGroup1, "hotcue_1_endposition");
        m_pHotcue1Enabled = std::make_unique<ControlProxy>(m_sGroup1, "hotcue_1_enabled");
        m_pHotcue1Clear = std::make_unique<ControlProxy>(m_sGroup1, "hotcue_1_clear");
        m_pQuantizeEnabled = std::make_unique<ControlProxy>(m_sGroup1, "quantize");
    }

    TrackPointer createTestTrack() const {
        const QString kTrackLocationTest = QDir::currentPath() + "/src/test/sine-30.wav";
        const auto pTrack = Track::newTemporary(kTrackLocationTest, SecurityTokenPointer());
        pTrack->setAudioProperties(
                mixxx::audio::ChannelCount(2),
                mixxx::audio::SampleRate(44100),
                mixxx::audio::Bitrate(),
                mixxx::Duration::fromSeconds(180));
        return pTrack;
    }

    void loadTrack(TrackPointer pTrack) {
        BaseSignalPathTest::loadTrack(m_pMixerDeck1, pTrack);
        ProcessBuffer();
    }

    TrackPointer createAndLoadFakeTrack() {
        return m_pMixerDeck1->loadFakeTrack(false, 0.0);
    }

    void unloadTrack() {
        m_pMixerDeck1->slotLoadTrack(TrackPointer(), false);
    }

    double getCurrentSample() {
        return m_pChannel1->getEngineBuffer()->m_pCueControl->getSampleOfTrack().current;
    }

    void setCurrentSample(double sample) {
        m_pChannel1->getEngineBuffer()->queueNewPlaypos(sample, EngineBuffer::SEEK_STANDARD);
        ProcessBuffer();
    }

    std::unique_ptr<ControlProxy> m_pBeatloopActivate;
    std::unique_ptr<ControlProxy> m_pBeatloopSize;
    std::unique_ptr<ControlProxy> m_pLoopEnabled;
    std::unique_ptr<ControlProxy> m_pLoopDouble;
    std::unique_ptr<ControlProxy> m_pLoopHalve;
    std::unique_ptr<ControlProxy> m_pLoopMove;
    std::unique_ptr<ControlProxy> m_pLoopToggle;
    std::unique_ptr<ControlProxy> m_pHotcue1Activate;
    std::unique_ptr<ControlProxy> m_pHotcue1ActivateCue;
    std::unique_ptr<ControlProxy> m_pHotcue1ActivateLoop;
    std::unique_ptr<ControlProxy> m_pHotcue1Set;
    std::unique_ptr<ControlProxy> m_pHotcue1SetCue;
    std::unique_ptr<ControlProxy> m_pHotcue1SetLoop;
    std::unique_ptr<ControlProxy> m_pHotcue1Position;
    std::unique_ptr<ControlProxy> m_pHotcue1EndPosition;
    std::unique_ptr<ControlProxy> m_pHotcue1Enabled;
    std::unique_ptr<ControlProxy> m_pHotcue1Clear;
    std::unique_ptr<ControlProxy> m_pQuantizeEnabled;
};

TEST_F(HotcueControlTest, DefautltControlValues) {
    TrackPointer pTrack = createTestTrack();

    EXPECT_DOUBLE_EQ(static_cast<double>(HotcueControl::Status::Invalid), m_pHotcue1Enabled->get());
    EXPECT_DOUBLE_EQ(Cue::kNoPosition, m_pHotcue1Position->get());
    EXPECT_DOUBLE_EQ(Cue::kNoPosition, m_pHotcue1EndPosition->get());

    loadTrack(pTrack);

    EXPECT_DOUBLE_EQ(static_cast<double>(HotcueControl::Status::Invalid), m_pHotcue1Enabled->get());
    EXPECT_DOUBLE_EQ(Cue::kNoPosition, m_pHotcue1Position->get());
    EXPECT_DOUBLE_EQ(Cue::kNoPosition, m_pHotcue1EndPosition->get());
}

TEST_F(HotcueControlTest, NoTrackLoaded) {
    TrackPointer pTrack = createTestTrack();

    m_pHotcue1Set->slotSet(1);
    m_pHotcue1Set->slotSet(0);
    EXPECT_DOUBLE_EQ(static_cast<double>(HotcueControl::Status::Invalid), m_pHotcue1Enabled->get());
    EXPECT_DOUBLE_EQ(Cue::kNoPosition, m_pHotcue1Position->get());
    EXPECT_DOUBLE_EQ(Cue::kNoPosition, m_pHotcue1EndPosition->get());

    m_pHotcue1SetCue->slotSet(1);
    m_pHotcue1SetCue->slotSet(0);
    EXPECT_DOUBLE_EQ(static_cast<double>(HotcueControl::Status::Invalid), m_pHotcue1Enabled->get());
    EXPECT_DOUBLE_EQ(Cue::kNoPosition, m_pHotcue1Position->get());
    EXPECT_DOUBLE_EQ(Cue::kNoPosition, m_pHotcue1EndPosition->get());

    m_pHotcue1SetLoop->slotSet(1);
    m_pHotcue1SetLoop->slotSet(0);
    EXPECT_DOUBLE_EQ(static_cast<double>(HotcueControl::Status::Invalid), m_pHotcue1Enabled->get());
    EXPECT_DOUBLE_EQ(Cue::kNoPosition, m_pHotcue1Position->get());
    EXPECT_DOUBLE_EQ(Cue::kNoPosition, m_pHotcue1EndPosition->get());

    m_pHotcue1Activate->slotSet(1);
    m_pHotcue1Activate->slotSet(0);
    EXPECT_DOUBLE_EQ(static_cast<double>(HotcueControl::Status::Invalid), m_pHotcue1Enabled->get());
    EXPECT_DOUBLE_EQ(Cue::kNoPosition, m_pHotcue1Position->get());
    EXPECT_DOUBLE_EQ(Cue::kNoPosition, m_pHotcue1EndPosition->get());

    m_pHotcue1ActivateCue->slotSet(1);
    m_pHotcue1ActivateCue->slotSet(0);
    EXPECT_DOUBLE_EQ(static_cast<double>(HotcueControl::Status::Invalid), m_pHotcue1Enabled->get());
    EXPECT_DOUBLE_EQ(Cue::kNoPosition, m_pHotcue1Position->get());
    EXPECT_DOUBLE_EQ(Cue::kNoPosition, m_pHotcue1EndPosition->get());

    m_pHotcue1ActivateLoop->slotSet(1);
    m_pHotcue1ActivateLoop->slotSet(0);
    EXPECT_DOUBLE_EQ(static_cast<double>(HotcueControl::Status::Invalid), m_pHotcue1Enabled->get());
    EXPECT_DOUBLE_EQ(Cue::kNoPosition, m_pHotcue1Position->get());
    EXPECT_DOUBLE_EQ(Cue::kNoPosition, m_pHotcue1EndPosition->get());
}

TEST_F(HotcueControlTest, SetCueAuto) {
    createAndLoadFakeTrack();

    EXPECT_DOUBLE_EQ(static_cast<double>(HotcueControl::Status::Invalid), m_pHotcue1Enabled->get());
    EXPECT_DOUBLE_EQ(Cue::kNoPosition, m_pHotcue1Position->get());
    EXPECT_DOUBLE_EQ(Cue::kNoPosition, m_pHotcue1EndPosition->get());

    m_pQuantizeEnabled->slotSet(0);
    setCurrentSample(100);
    ProcessBuffer();

    m_pHotcue1Set->slotSet(1);
    m_pHotcue1Set->slotSet(0);
    EXPECT_DOUBLE_EQ(static_cast<double>(HotcueControl::Status::Valid), m_pHotcue1Enabled->get());
    EXPECT_DOUBLE_EQ(100, m_pHotcue1Position->get());
    EXPECT_DOUBLE_EQ(Cue::kNoPosition, m_pHotcue1EndPosition->get());
}

TEST_F(HotcueControlTest, SetCueManual) {
    createAndLoadFakeTrack();

    EXPECT_DOUBLE_EQ(static_cast<double>(HotcueControl::Status::Invalid), m_pHotcue1Enabled->get());
    EXPECT_DOUBLE_EQ(Cue::kNoPosition, m_pHotcue1Position->get());
    EXPECT_DOUBLE_EQ(Cue::kNoPosition, m_pHotcue1EndPosition->get());

    m_pQuantizeEnabled->slotSet(0);
    setCurrentSample(100);

    m_pHotcue1SetCue->slotSet(1);
    m_pHotcue1SetCue->slotSet(0);
    EXPECT_DOUBLE_EQ(static_cast<double>(HotcueControl::Status::Valid), m_pHotcue1Enabled->get());
    EXPECT_DOUBLE_EQ(100, m_pHotcue1Position->get());
    EXPECT_DOUBLE_EQ(Cue::kNoPosition, m_pHotcue1EndPosition->get());
}

TEST_F(HotcueControlTest, SetLoopAuto) {
    createAndLoadFakeTrack();

    EXPECT_DOUBLE_EQ(static_cast<double>(HotcueControl::Status::Invalid), m_pHotcue1Enabled->get());
    EXPECT_DOUBLE_EQ(Cue::kNoPosition, m_pHotcue1Position->get());
    EXPECT_DOUBLE_EQ(Cue::kNoPosition, m_pHotcue1EndPosition->get());

    m_pChannel1->getEngineBuffer()->setLoop(100, 200, true);

    m_pHotcue1Set->slotSet(1);
    m_pHotcue1Set->slotSet(0);
    EXPECT_DOUBLE_EQ(static_cast<double>(HotcueControl::Status::Active), m_pHotcue1Enabled->get());
    EXPECT_DOUBLE_EQ(100, m_pHotcue1Position->get());
    EXPECT_DOUBLE_EQ(200, m_pHotcue1EndPosition->get());
}

TEST_F(HotcueControlTest, SetLoopManual) {
    createAndLoadFakeTrack();

    EXPECT_DOUBLE_EQ(static_cast<double>(HotcueControl::Status::Invalid), m_pHotcue1Enabled->get());
    EXPECT_DOUBLE_EQ(Cue::kNoPosition, m_pHotcue1Position->get());
    EXPECT_DOUBLE_EQ(Cue::kNoPosition, m_pHotcue1EndPosition->get());

    m_pChannel1->getEngineBuffer()->setLoop(100, 200, true);

    m_pHotcue1SetLoop->slotSet(1);
    m_pHotcue1SetLoop->slotSet(0);
    EXPECT_DOUBLE_EQ(static_cast<double>(HotcueControl::Status::Active), m_pHotcue1Enabled->get());
    EXPECT_DOUBLE_EQ(100, m_pHotcue1Position->get());
    EXPECT_DOUBLE_EQ(200, m_pHotcue1EndPosition->get());
}

TEST_F(HotcueControlTest, SavedLoopStatus) {
    createAndLoadFakeTrack();

    EXPECT_DOUBLE_EQ(static_cast<double>(HotcueControl::Status::Invalid), m_pHotcue1Enabled->get());
    EXPECT_DOUBLE_EQ(Cue::kNoPosition, m_pHotcue1Position->get());
    EXPECT_DOUBLE_EQ(Cue::kNoPosition, m_pHotcue1EndPosition->get());

    m_pChannel1->getEngineBuffer()->setLoop(100, 200, true);

    m_pHotcue1SetLoop->slotSet(1);
    m_pHotcue1SetLoop->slotSet(0);
    EXPECT_DOUBLE_EQ(1.0, m_pLoopEnabled->get());
    EXPECT_DOUBLE_EQ(static_cast<double>(HotcueControl::Status::Active), m_pHotcue1Enabled->get());
    EXPECT_DOUBLE_EQ(100, m_pHotcue1Position->get());
    EXPECT_DOUBLE_EQ(200, m_pHotcue1EndPosition->get());

    // Disable Loop
    m_pLoopToggle->slotSet(1);
    m_pLoopToggle->slotSet(0);

    EXPECT_DOUBLE_EQ(0.0, m_pLoopEnabled->get());
    EXPECT_DOUBLE_EQ(static_cast<double>(HotcueControl::Status::Valid), m_pHotcue1Enabled->get());
    EXPECT_DOUBLE_EQ(100, m_pHotcue1Position->get());
    EXPECT_DOUBLE_EQ(200, m_pHotcue1EndPosition->get());

    // Re-Enable Loop
    m_pLoopToggle->slotSet(1);
    m_pLoopToggle->slotSet(0);

    EXPECT_DOUBLE_EQ(1.0, m_pLoopEnabled->get());
    EXPECT_DOUBLE_EQ(static_cast<double>(HotcueControl::Status::Active), m_pHotcue1Enabled->get());
    EXPECT_DOUBLE_EQ(100, m_pHotcue1Position->get());
    EXPECT_DOUBLE_EQ(200, m_pHotcue1EndPosition->get());

    m_pHotcue1Clear->slotSet(1);
    m_pHotcue1Clear->slotSet(0);

    EXPECT_DOUBLE_EQ(1.0, m_pLoopEnabled->get());
    EXPECT_DOUBLE_EQ(static_cast<double>(HotcueControl::Status::Invalid), m_pHotcue1Enabled->get());
    EXPECT_DOUBLE_EQ(Cue::kNoPosition, m_pHotcue1Position->get());
    EXPECT_DOUBLE_EQ(Cue::kNoPosition, m_pHotcue1EndPosition->get());
}

TEST_F(HotcueControlTest, SavedLoopScale) {
    TrackPointer pTrack = createTestTrack();
    pTrack->setBpm(120.0);

    const double beatLength = getBeatLengthFrames(pTrack);
    const double loopSize = 4;
    const double loopLength = loopSize * beatLength;

    loadTrack(pTrack);
    ProcessBuffer();

    EXPECT_DOUBLE_EQ(static_cast<double>(HotcueControl::Status::Invalid), m_pHotcue1Enabled->get());
    EXPECT_DOUBLE_EQ(Cue::kNoPosition, m_pHotcue1Position->get());
    EXPECT_DOUBLE_EQ(Cue::kNoPosition, m_pHotcue1EndPosition->get());

    // Set a beatloop
    m_pBeatloopSize->slotSet(loopSize);
    m_pBeatloopActivate->slotSet(1);
    m_pBeatloopActivate->slotSet(0);
    ProcessBuffer();

    m_pHotcue1SetLoop->slotSet(1);
    m_pHotcue1SetLoop->slotSet(0);
    EXPECT_DOUBLE_EQ(0, m_pHotcue1Position->get());
    EXPECT_DOUBLE_EQ(loopLength, m_pHotcue1EndPosition->get());

    m_pLoopDouble->slotSet(1);
    m_pLoopDouble->slotSet(0);
    EXPECT_DOUBLE_EQ(0, m_pHotcue1Position->get());
    EXPECT_DOUBLE_EQ(2 * loopLength, m_pHotcue1EndPosition->get());

    m_pLoopHalve->slotSet(1);
    m_pLoopHalve->slotSet(0);
    EXPECT_DOUBLE_EQ(0, m_pHotcue1Position->get());
    EXPECT_DOUBLE_EQ(loopLength, m_pHotcue1EndPosition->get());

    m_pLoopHalve->slotSet(1);
    m_pLoopHalve->slotSet(0);
    EXPECT_DOUBLE_EQ(0, m_pHotcue1Position->get());
    EXPECT_DOUBLE_EQ(loopLength / 2, m_pHotcue1EndPosition->get());
}

TEST_F(HotcueControlTest, SavedLoopMove) {
    TrackPointer pTrack = createTestTrack();
    pTrack->setBpm(120.0);

    const double beatLength = getBeatLengthFrames(pTrack);
    const double loopSize = 4;
    const double loopLength = loopSize * beatLength;

    loadTrack(pTrack);
    ProcessBuffer();

    EXPECT_DOUBLE_EQ(static_cast<double>(HotcueControl::Status::Invalid), m_pHotcue1Enabled->get());
    EXPECT_DOUBLE_EQ(Cue::kNoPosition, m_pHotcue1Position->get());
    EXPECT_DOUBLE_EQ(Cue::kNoPosition, m_pHotcue1EndPosition->get());

    // Set a beatloop
    m_pBeatloopSize->slotSet(loopSize);
    m_pBeatloopActivate->slotSet(1);
    m_pBeatloopActivate->slotSet(0);
    ProcessBuffer();

    m_pHotcue1SetLoop->slotSet(1);
    m_pHotcue1SetLoop->slotSet(0);
    EXPECT_DOUBLE_EQ(0, m_pHotcue1Position->get());
    EXPECT_DOUBLE_EQ(loopLength, m_pHotcue1EndPosition->get());

    m_pLoopMove->slotSet(loopSize);
    EXPECT_DOUBLE_EQ(loopLength, m_pHotcue1Position->get());
    EXPECT_DOUBLE_EQ(2 * loopLength, m_pHotcue1EndPosition->get());

    m_pLoopMove->slotSet(-loopSize);
    EXPECT_DOUBLE_EQ(0, m_pHotcue1Position->get());
    EXPECT_DOUBLE_EQ(loopLength, m_pHotcue1EndPosition->get());

    m_pLoopMove->slotSet(-loopSize);
    EXPECT_DOUBLE_EQ(-loopLength, m_pHotcue1Position->get());
    EXPECT_DOUBLE_EQ(0, m_pHotcue1EndPosition->get());
}

TEST_F(HotcueControlTest, SavedLoopReset) {
    TrackPointer pTrack = createTestTrack();
    pTrack->setBpm(120.0);

    const double beatLength = getBeatLengthFrames(pTrack);
    const double loopSize = 4;
    const double loopLength = loopSize * beatLength;

    loadTrack(pTrack);
    ProcessBuffer();

    EXPECT_DOUBLE_EQ(static_cast<double>(HotcueControl::Status::Invalid), m_pHotcue1Enabled->get());
    EXPECT_DOUBLE_EQ(Cue::kNoPosition, m_pHotcue1Position->get());
    EXPECT_DOUBLE_EQ(Cue::kNoPosition, m_pHotcue1EndPosition->get());

    // Set a beatloop
    m_pBeatloopSize->slotSet(loopSize);
    m_pBeatloopActivate->slotSet(1);
    m_pBeatloopActivate->slotSet(0);
    ProcessBuffer();

    m_pHotcue1SetLoop->slotSet(1);
    m_pHotcue1SetLoop->slotSet(0);
    EXPECT_DOUBLE_EQ(static_cast<double>(HotcueControl::Status::Active), m_pHotcue1Enabled->get());
    EXPECT_DOUBLE_EQ(0, m_pHotcue1Position->get());
    EXPECT_DOUBLE_EQ(loopLength, m_pHotcue1EndPosition->get());

    // Set a new beatloop (should disable saved loop)
    setCurrentSample(loopLength);
    m_pBeatloopActivate->slotSet(1);
    m_pBeatloopActivate->slotSet(0);
    ProcessBuffer();

    EXPECT_DOUBLE_EQ(static_cast<double>(HotcueControl::Status::Valid), m_pHotcue1Enabled->get());
    EXPECT_DOUBLE_EQ(0, m_pHotcue1Position->get());
    EXPECT_DOUBLE_EQ(loopLength, m_pHotcue1EndPosition->get());
}
