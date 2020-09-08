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

        m_pPlay = std::make_unique<ControlProxy>(m_sGroup1, "play");
        m_pBeatloopActivate = std::make_unique<ControlProxy>(m_sGroup1, "beatloop_activate");
        m_pBeatloopSize = std::make_unique<ControlProxy>(m_sGroup1, "beatloop_size");
        m_pLoopStartPosition = std::make_unique<ControlProxy>(m_sGroup1, "loop_start_position");
        m_pLoopEndPosition = std::make_unique<ControlProxy>(m_sGroup1, "loop_end_position");
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
        m_pHotcue1Goto = std::make_unique<ControlProxy>(m_sGroup1, "hotcue_1_goto");
        m_pHotcue1GotoAndPlay = std::make_unique<ControlProxy>(m_sGroup1, "hotcue_1_gotoandplay");
        m_pHotcue1GotoAndLoop = std::make_unique<ControlProxy>(m_sGroup1, "hotcue_1_gotoandloop");
        m_pHotcue1LoopToggle = std::make_unique<ControlProxy>(m_sGroup1, "hotcue_1_loop_toggle");
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

    std::unique_ptr<ControlProxy> m_pPlay;
    std::unique_ptr<ControlProxy> m_pBeatloopActivate;
    std::unique_ptr<ControlProxy> m_pBeatloopSize;
    std::unique_ptr<ControlProxy> m_pLoopStartPosition;
    std::unique_ptr<ControlProxy> m_pLoopEndPosition;
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
    std::unique_ptr<ControlProxy> m_pHotcue1Goto;
    std::unique_ptr<ControlProxy> m_pHotcue1GotoAndPlay;
    std::unique_ptr<ControlProxy> m_pHotcue1GotoAndLoop;
    std::unique_ptr<ControlProxy> m_pHotcue1LoopToggle;
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

TEST_F(HotcueControlTest, CueGoto) {
    // Setup fake track with 120 bpm can calculate loop size
    TrackPointer pTrack = createTestTrack();
    pTrack->setBpm(120.0);

    loadTrack(pTrack);
    ProcessBuffer();

    EXPECT_DOUBLE_EQ(static_cast<double>(HotcueControl::Status::Invalid), m_pHotcue1Enabled->get());
    EXPECT_DOUBLE_EQ(Cue::kNoPosition, m_pHotcue1Position->get());
    EXPECT_DOUBLE_EQ(Cue::kNoPosition, m_pHotcue1EndPosition->get());

    const double cuePosition = 8 * getBeatLengthFrames(pTrack);

    // Seek to cue Position (8th beat)
    setCurrentSample(cuePosition);
    ProcessBuffer();
    EXPECT_DOUBLE_EQ(cuePosition, getCurrentSample());

    m_pHotcue1SetCue->slotSet(1);
    m_pHotcue1SetCue->slotSet(0);
    EXPECT_DOUBLE_EQ(static_cast<double>(HotcueControl::Status::Valid), m_pHotcue1Enabled->get());
    EXPECT_DOUBLE_EQ(cuePosition, m_pHotcue1Position->get());
    EXPECT_DOUBLE_EQ(Cue::kNoPosition, m_pHotcue1EndPosition->get());

    // Seek to start of track
    setCurrentSample(0);
    ProcessBuffer();
    EXPECT_DOUBLE_EQ(0, getCurrentSample());

    m_pHotcue1Goto->slotSet(1);
    m_pHotcue1Goto->slotSet(0);
    ProcessBuffer();
    EXPECT_DOUBLE_EQ(cuePosition, getCurrentSample());

    EXPECT_DOUBLE_EQ(static_cast<double>(HotcueControl::Status::Valid), m_pHotcue1Enabled->get());
    EXPECT_DOUBLE_EQ(cuePosition, m_pHotcue1Position->get());
    EXPECT_DOUBLE_EQ(Cue::kNoPosition, m_pHotcue1EndPosition->get());
    EXPECT_DOUBLE_EQ(0.0, m_pLoopEnabled->get());
}

TEST_F(HotcueControlTest, CueGotoAndPlay) {
    // Setup fake track with 120 bpm can calculate loop size
    TrackPointer pTrack = createTestTrack();
    pTrack->setBpm(120.0);

    loadTrack(pTrack);
    ProcessBuffer();

    EXPECT_DOUBLE_EQ(static_cast<double>(HotcueControl::Status::Invalid), m_pHotcue1Enabled->get());
    EXPECT_DOUBLE_EQ(Cue::kNoPosition, m_pHotcue1Position->get());
    EXPECT_DOUBLE_EQ(Cue::kNoPosition, m_pHotcue1EndPosition->get());

    const double cuePosition = 8 * getBeatLengthFrames(pTrack);

    // Seek to cue Position (8th beat)
    setCurrentSample(cuePosition);
    ProcessBuffer();
    EXPECT_DOUBLE_EQ(cuePosition, getCurrentSample());

    m_pHotcue1SetCue->slotSet(1);
    m_pHotcue1SetCue->slotSet(0);
    EXPECT_DOUBLE_EQ(static_cast<double>(HotcueControl::Status::Valid), m_pHotcue1Enabled->get());
    EXPECT_DOUBLE_EQ(cuePosition, m_pHotcue1Position->get());
    EXPECT_DOUBLE_EQ(Cue::kNoPosition, m_pHotcue1EndPosition->get());

    // Seek to start of track
    setCurrentSample(0);
    ProcessBuffer();
    EXPECT_DOUBLE_EQ(0, getCurrentSample());

    m_pHotcue1GotoAndPlay->slotSet(1);
    m_pHotcue1GotoAndPlay->slotSet(0);
    ProcessBuffer();
    EXPECT_LE(cuePosition, getCurrentSample());

    EXPECT_DOUBLE_EQ(static_cast<double>(HotcueControl::Status::Valid), m_pHotcue1Enabled->get());
    EXPECT_DOUBLE_EQ(cuePosition, m_pHotcue1Position->get());
    EXPECT_DOUBLE_EQ(Cue::kNoPosition, m_pHotcue1EndPosition->get());
    EXPECT_DOUBLE_EQ(0.0, m_pLoopEnabled->get());
}

TEST_F(HotcueControlTest, CueGotoAndLoop) {
    // Setup fake track with 120 bpm can calculate loop size
    TrackPointer pTrack = createTestTrack();
    pTrack->setBpm(120.0);

    loadTrack(pTrack);
    ProcessBuffer();

    EXPECT_DOUBLE_EQ(static_cast<double>(HotcueControl::Status::Invalid), m_pHotcue1Enabled->get());
    EXPECT_DOUBLE_EQ(Cue::kNoPosition, m_pHotcue1Position->get());
    EXPECT_DOUBLE_EQ(Cue::kNoPosition, m_pHotcue1EndPosition->get());

    const double beatLength = getBeatLengthFrames(pTrack);
    const double cuePosition = 8 * beatLength;
    m_pBeatloopSize->slotSet(4);
    const double beatloopLength = m_pBeatloopSize->get() * getBeatLengthFrames(pTrack);

    // Seek to cue Position (8th beat)
    setCurrentSample(cuePosition);
    ProcessBuffer();
    EXPECT_DOUBLE_EQ(cuePosition, getCurrentSample());

    m_pHotcue1SetCue->slotSet(1);
    m_pHotcue1SetCue->slotSet(0);
    EXPECT_DOUBLE_EQ(static_cast<double>(HotcueControl::Status::Valid), m_pHotcue1Enabled->get());
    EXPECT_DOUBLE_EQ(cuePosition, m_pHotcue1Position->get());
    EXPECT_DOUBLE_EQ(Cue::kNoPosition, m_pHotcue1EndPosition->get());

    // Seek to start of track
    setCurrentSample(0);
    ProcessBuffer();
    EXPECT_DOUBLE_EQ(0, getCurrentSample());

    m_pHotcue1GotoAndLoop->slotSet(1);
    m_pHotcue1GotoAndLoop->slotSet(0);
    ProcessBuffer();
    EXPECT_LE(cuePosition, getCurrentSample());

    EXPECT_DOUBLE_EQ(static_cast<double>(HotcueControl::Status::Valid), m_pHotcue1Enabled->get());
    EXPECT_DOUBLE_EQ(cuePosition, m_pHotcue1Position->get());
    EXPECT_DOUBLE_EQ(Cue::kNoPosition, m_pHotcue1EndPosition->get());

    EXPECT_DOUBLE_EQ(1.0, m_pLoopEnabled->get());
    EXPECT_DOUBLE_EQ(cuePosition, m_pLoopStartPosition->get());
    EXPECT_DOUBLE_EQ(cuePosition + beatloopLength, m_pLoopEndPosition->get());
}

TEST_F(HotcueControlTest, SavedLoopGoto) {
    // Setup fake track with 120 bpm can calculate loop size
    TrackPointer pTrack = createTestTrack();
    pTrack->setBpm(120.0);

    m_pBeatloopSize->slotSet(4);
    const double beatLength = getBeatLengthFrames(pTrack);
    const double loopLength = m_pBeatloopSize->get() * beatLength;
    const double cuePosition = 8 * beatLength;

    loadTrack(pTrack);
    ProcessBuffer();

    EXPECT_DOUBLE_EQ(static_cast<double>(HotcueControl::Status::Invalid), m_pHotcue1Enabled->get());
    EXPECT_DOUBLE_EQ(Cue::kNoPosition, m_pHotcue1Position->get());
    EXPECT_DOUBLE_EQ(Cue::kNoPosition, m_pHotcue1EndPosition->get());

    // Seek to cue Position (8th beat)
    setCurrentSample(cuePosition);
    ProcessBuffer();
    EXPECT_DOUBLE_EQ(cuePosition, getCurrentSample());

    // Set a beatloop this position
    m_pBeatloopActivate->slotSet(1);
    m_pBeatloopActivate->slotSet(0);
    m_pHotcue1SetLoop->slotSet(1);
    m_pHotcue1SetLoop->slotSet(0);
    EXPECT_DOUBLE_EQ(1.0, m_pLoopEnabled->get());

    // Save loop to hotcue slot 1
    EXPECT_DOUBLE_EQ(static_cast<double>(HotcueControl::Status::Active), m_pHotcue1Enabled->get());
    EXPECT_DOUBLE_EQ(cuePosition, m_pHotcue1Position->get());
    EXPECT_DOUBLE_EQ(cuePosition + loopLength, m_pHotcue1EndPosition->get());

    // Disable loop
    m_pLoopToggle->slotSet(1);
    m_pLoopToggle->slotSet(0);
    EXPECT_DOUBLE_EQ(0.0, m_pLoopEnabled->get());
    EXPECT_DOUBLE_EQ(static_cast<double>(HotcueControl::Status::Valid), m_pHotcue1Enabled->get());

    // Seek to start of track
    setCurrentSample(0);
    ProcessBuffer();
    EXPECT_DOUBLE_EQ(0, getCurrentSample());

    m_pHotcue1Goto->slotSet(1);
    m_pHotcue1Goto->slotSet(0);
    ProcessBuffer();
    EXPECT_DOUBLE_EQ(cuePosition, getCurrentSample());

    EXPECT_DOUBLE_EQ(static_cast<double>(HotcueControl::Status::Valid), m_pHotcue1Enabled->get());
    EXPECT_DOUBLE_EQ(cuePosition, m_pHotcue1Position->get());
    EXPECT_DOUBLE_EQ(cuePosition + loopLength, m_pHotcue1EndPosition->get());
    EXPECT_DOUBLE_EQ(0.0, m_pLoopEnabled->get());
}

TEST_F(HotcueControlTest, SavedLoopGotoAndPlay) {
    // Setup fake track with 120 bpm can calculate loop size
    TrackPointer pTrack = createTestTrack();
    pTrack->setBpm(120.0);

    m_pBeatloopSize->slotSet(4);
    const double beatLength = getBeatLengthFrames(pTrack);
    const double loopLength = m_pBeatloopSize->get() * beatLength;
    const double cuePosition = 8 * beatLength;

    loadTrack(pTrack);
    ProcessBuffer();

    EXPECT_DOUBLE_EQ(static_cast<double>(HotcueControl::Status::Invalid), m_pHotcue1Enabled->get());
    EXPECT_DOUBLE_EQ(Cue::kNoPosition, m_pHotcue1Position->get());
    EXPECT_DOUBLE_EQ(Cue::kNoPosition, m_pHotcue1EndPosition->get());

    // Seek to cue Position (8th beat)
    setCurrentSample(cuePosition);
    ProcessBuffer();
    EXPECT_DOUBLE_EQ(cuePosition, getCurrentSample());

    // Set a beatloop this position
    m_pBeatloopActivate->slotSet(1);
    m_pBeatloopActivate->slotSet(0);
    m_pHotcue1SetLoop->slotSet(1);
    m_pHotcue1SetLoop->slotSet(0);
    EXPECT_DOUBLE_EQ(1.0, m_pLoopEnabled->get());

    // Save loop to hotcue slot 1
    EXPECT_DOUBLE_EQ(static_cast<double>(HotcueControl::Status::Active), m_pHotcue1Enabled->get());
    EXPECT_DOUBLE_EQ(cuePosition, m_pHotcue1Position->get());
    EXPECT_DOUBLE_EQ(cuePosition + loopLength, m_pHotcue1EndPosition->get());

    // Disable loop
    m_pLoopToggle->slotSet(1);
    m_pLoopToggle->slotSet(0);
    EXPECT_DOUBLE_EQ(0.0, m_pLoopEnabled->get());
    EXPECT_DOUBLE_EQ(static_cast<double>(HotcueControl::Status::Valid), m_pHotcue1Enabled->get());

    // Seek to start of track
    setCurrentSample(0);
    ProcessBuffer();
    EXPECT_DOUBLE_EQ(0, getCurrentSample());

    m_pHotcue1GotoAndPlay->slotSet(1);
    m_pHotcue1GotoAndPlay->slotSet(0);
    ProcessBuffer();
    EXPECT_LE(cuePosition, getCurrentSample());

    EXPECT_DOUBLE_EQ(static_cast<double>(HotcueControl::Status::Valid), m_pHotcue1Enabled->get());
    EXPECT_DOUBLE_EQ(cuePosition, m_pHotcue1Position->get());
    EXPECT_DOUBLE_EQ(cuePosition + loopLength, m_pHotcue1EndPosition->get());
    EXPECT_DOUBLE_EQ(0.0, m_pLoopEnabled->get());
}

TEST_F(HotcueControlTest, SavedLoopGotoAndLoop) {
    // Setup fake track with 120 bpm can calculate loop size
    TrackPointer pTrack = createTestTrack();
    pTrack->setBpm(120.0);

    m_pBeatloopSize->slotSet(4);
    const double beatLength = getBeatLengthFrames(pTrack);
    const double loopLength = m_pBeatloopSize->get() * beatLength;
    const double cuePosition = 8 * beatLength;

    loadTrack(pTrack);
    ProcessBuffer();

    EXPECT_DOUBLE_EQ(static_cast<double>(HotcueControl::Status::Invalid), m_pHotcue1Enabled->get());
    EXPECT_DOUBLE_EQ(Cue::kNoPosition, m_pHotcue1Position->get());
    EXPECT_DOUBLE_EQ(Cue::kNoPosition, m_pHotcue1EndPosition->get());

    // Seek to cue Position (8th beat)
    setCurrentSample(cuePosition);
    ProcessBuffer();
    EXPECT_DOUBLE_EQ(cuePosition, getCurrentSample());

    // Set a beatloop this position
    m_pBeatloopActivate->slotSet(1);
    m_pBeatloopActivate->slotSet(0);
    m_pHotcue1SetLoop->slotSet(1);
    m_pHotcue1SetLoop->slotSet(0);
    EXPECT_DOUBLE_EQ(1.0, m_pLoopEnabled->get());

    // Save loop to hotcue slot 1
    EXPECT_DOUBLE_EQ(static_cast<double>(HotcueControl::Status::Active), m_pHotcue1Enabled->get());
    EXPECT_DOUBLE_EQ(cuePosition, m_pHotcue1Position->get());
    EXPECT_DOUBLE_EQ(cuePosition + loopLength, m_pHotcue1EndPosition->get());

    // Disable loop
    m_pLoopToggle->slotSet(1);
    m_pLoopToggle->slotSet(0);
    EXPECT_DOUBLE_EQ(0.0, m_pLoopEnabled->get());
    EXPECT_DOUBLE_EQ(static_cast<double>(HotcueControl::Status::Valid), m_pHotcue1Enabled->get());

    // Seek to start of track
    setCurrentSample(0);
    ProcessBuffer();
    EXPECT_DOUBLE_EQ(0, getCurrentSample());

    m_pHotcue1GotoAndLoop->slotSet(1);
    m_pHotcue1GotoAndLoop->slotSet(0);
    ProcessBuffer();
    EXPECT_LE(cuePosition, getCurrentSample());

    EXPECT_DOUBLE_EQ(static_cast<double>(HotcueControl::Status::Active), m_pHotcue1Enabled->get());
    EXPECT_DOUBLE_EQ(cuePosition, m_pHotcue1Position->get());
    EXPECT_DOUBLE_EQ(cuePosition + loopLength, m_pHotcue1EndPosition->get());
    EXPECT_DOUBLE_EQ(1.0, m_pLoopEnabled->get());
    EXPECT_DOUBLE_EQ(cuePosition, m_pLoopStartPosition->get());
    EXPECT_DOUBLE_EQ(cuePosition + loopLength, m_pLoopEndPosition->get());
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
    // Setup fake track with 120 bpm can calculate loop size
    TrackPointer pTrack = createTestTrack();
    pTrack->setBpm(120.0);

    m_pBeatloopSize->slotSet(4);
    const double beatLength = getBeatLengthFrames(pTrack);
    const double loopLength = m_pBeatloopSize->get() * beatLength;

    loadTrack(pTrack);
    ProcessBuffer();

    EXPECT_DOUBLE_EQ(static_cast<double>(HotcueControl::Status::Invalid), m_pHotcue1Enabled->get());
    EXPECT_DOUBLE_EQ(Cue::kNoPosition, m_pHotcue1Position->get());
    EXPECT_DOUBLE_EQ(Cue::kNoPosition, m_pHotcue1EndPosition->get());

    // Set a beatloop (4 beats)
    m_pBeatloopActivate->slotSet(1);
    m_pBeatloopActivate->slotSet(0);
    m_pPlay->slotSet(1);
    ProcessBuffer();

    // Save currently active loop to hotcue slot 1
    m_pHotcue1SetLoop->slotSet(1);
    m_pHotcue1SetLoop->slotSet(0);
    EXPECT_DOUBLE_EQ(static_cast<double>(HotcueControl::Status::Active), m_pHotcue1Enabled->get());
    EXPECT_DOUBLE_EQ(0, m_pHotcue1Position->get());
    EXPECT_DOUBLE_EQ(loopLength, m_pHotcue1EndPosition->get());

    // Double loop size (4 => 8 beats)
    m_pLoopDouble->slotSet(1);
    m_pLoopDouble->slotSet(0);
    EXPECT_DOUBLE_EQ(0, m_pHotcue1Position->get());
    EXPECT_DOUBLE_EQ(2 * loopLength, m_pHotcue1EndPosition->get());

    // Halve loop size (8 => 4 beats)
    m_pLoopHalve->slotSet(1);
    m_pLoopHalve->slotSet(0);
    EXPECT_DOUBLE_EQ(0, m_pHotcue1Position->get());
    EXPECT_DOUBLE_EQ(loopLength, m_pHotcue1EndPosition->get());

    // Halve loop size (4 => 2 beats)
    m_pLoopHalve->slotSet(1);
    m_pLoopHalve->slotSet(0);
    EXPECT_DOUBLE_EQ(0, m_pHotcue1Position->get());
    EXPECT_DOUBLE_EQ(loopLength / 2, m_pHotcue1EndPosition->get());

    m_pPlay->slotSet(0);
}

TEST_F(HotcueControlTest, SavedLoopMove) {
    // Setup fake track with 120 bpm can calculate loop size
    TrackPointer pTrack = createTestTrack();
    pTrack->setBpm(120.0);

    constexpr double loopSize = 4;
    m_pBeatloopSize->slotSet(loopSize);
    const double beatLength = getBeatLengthFrames(pTrack);
    const double loopLength = loopSize * beatLength;

    loadTrack(pTrack);
    ProcessBuffer();

    EXPECT_DOUBLE_EQ(static_cast<double>(HotcueControl::Status::Invalid), m_pHotcue1Enabled->get());
    EXPECT_DOUBLE_EQ(Cue::kNoPosition, m_pHotcue1Position->get());
    EXPECT_DOUBLE_EQ(Cue::kNoPosition, m_pHotcue1EndPosition->get());

    // Set a beatloop at position 0
    m_pBeatloopActivate->slotSet(1);
    m_pBeatloopActivate->slotSet(0);
    m_pPlay->slotSet(1);

    // Save currently active loop to hotcue slot 1
    m_pHotcue1SetLoop->slotSet(1);
    m_pHotcue1SetLoop->slotSet(0);
    ProcessBuffer();
    EXPECT_DOUBLE_EQ(static_cast<double>(HotcueControl::Status::Active), m_pHotcue1Enabled->get());
    EXPECT_DOUBLE_EQ(0, m_pHotcue1Position->get());
    EXPECT_DOUBLE_EQ(loopLength, m_pHotcue1EndPosition->get());

    // Move loop right (0 => 4 beats)
    m_pLoopMove->slotSet(loopSize);
    ProcessBuffer();
    EXPECT_DOUBLE_EQ(static_cast<double>(HotcueControl::Status::Active), m_pHotcue1Enabled->get());
    EXPECT_DOUBLE_EQ(loopLength, m_pHotcue1Position->get());
    EXPECT_DOUBLE_EQ(2 * loopLength, m_pHotcue1EndPosition->get());

    // Move loop left (4 => 0 beats)
    m_pLoopMove->slotSet(-loopSize);
    ProcessBuffer();
    EXPECT_DOUBLE_EQ(static_cast<double>(HotcueControl::Status::Active), m_pHotcue1Enabled->get());
    EXPECT_DOUBLE_EQ(0, m_pHotcue1Position->get());
    EXPECT_DOUBLE_EQ(loopLength, m_pHotcue1EndPosition->get());

    // Move loop left (0 => -4 beats)
    m_pLoopMove->slotSet(-loopSize);
    ProcessBuffer();
    EXPECT_DOUBLE_EQ(static_cast<double>(HotcueControl::Status::Active), m_pHotcue1Enabled->get());
    EXPECT_DOUBLE_EQ(-loopLength, m_pHotcue1Position->get());
    EXPECT_DOUBLE_EQ(0, m_pHotcue1EndPosition->get());

    m_pPlay->slotSet(0);
}

TEST_F(HotcueControlTest, SavedLoopNoScaleIfDisabled) {
    // Setup fake track with 120 bpm can calculate loop size
    TrackPointer pTrack = createTestTrack();
    pTrack->setBpm(120.0);

    m_pBeatloopSize->slotSet(4);
    const double beatLength = getBeatLengthFrames(pTrack);
    const double loopLength = m_pBeatloopSize->get() * beatLength;

    loadTrack(pTrack);
    ProcessBuffer();

    EXPECT_DOUBLE_EQ(static_cast<double>(HotcueControl::Status::Invalid), m_pHotcue1Enabled->get());
    EXPECT_DOUBLE_EQ(Cue::kNoPosition, m_pHotcue1Position->get());
    EXPECT_DOUBLE_EQ(Cue::kNoPosition, m_pHotcue1EndPosition->get());

    // Set a beatloop (4 beats)
    m_pBeatloopActivate->slotSet(1);
    m_pBeatloopActivate->slotSet(0);
    m_pPlay->slotSet(1);
    ProcessBuffer();

    // Save currently active loop to hotcue slot 1
    m_pHotcue1SetLoop->slotSet(1);
    m_pHotcue1SetLoop->slotSet(0);
    EXPECT_DOUBLE_EQ(static_cast<double>(HotcueControl::Status::Active), m_pHotcue1Enabled->get());
    EXPECT_DOUBLE_EQ(0, m_pHotcue1Position->get());
    EXPECT_DOUBLE_EQ(loopLength, m_pHotcue1EndPosition->get());

    // Disable loop
    m_pLoopToggle->slotSet(1);
    m_pLoopToggle->slotSet(0);
    EXPECT_DOUBLE_EQ(static_cast<double>(HotcueControl::Status::Valid), m_pHotcue1Enabled->get());

    // Double loop size (4 => 8 beats) while saved loop is disabled
    m_pLoopDouble->slotSet(1);
    m_pLoopDouble->slotSet(0);
    EXPECT_DOUBLE_EQ(0, m_pHotcue1Position->get());
    EXPECT_DOUBLE_EQ(loopLength, m_pHotcue1EndPosition->get());

    // Halve loop size (8 => 4 beats) while saved loop is disabled
    m_pLoopHalve->slotSet(1);
    m_pLoopHalve->slotSet(0);
    EXPECT_DOUBLE_EQ(0, m_pHotcue1Position->get());
    EXPECT_DOUBLE_EQ(loopLength, m_pHotcue1EndPosition->get());

    // Halve loop size (4 => 2 beats) while saved loop is disabled
    m_pLoopHalve->slotSet(1);
    m_pLoopHalve->slotSet(0);
    EXPECT_DOUBLE_EQ(0, m_pHotcue1Position->get());
    EXPECT_DOUBLE_EQ(loopLength, m_pHotcue1EndPosition->get());

    m_pPlay->slotSet(0);
}

TEST_F(HotcueControlTest, SavedLoopNoMoveIfDisabled) {
    // Setup fake track with 120 bpm can calculate loop size
    TrackPointer pTrack = createTestTrack();
    pTrack->setBpm(120.0);

    constexpr double loopSize = 4;
    m_pBeatloopSize->slotSet(loopSize);
    const double beatLength = getBeatLengthFrames(pTrack);
    const double loopLength = loopSize * beatLength;

    loadTrack(pTrack);
    ProcessBuffer();

    EXPECT_DOUBLE_EQ(static_cast<double>(HotcueControl::Status::Invalid), m_pHotcue1Enabled->get());
    EXPECT_DOUBLE_EQ(Cue::kNoPosition, m_pHotcue1Position->get());
    EXPECT_DOUBLE_EQ(Cue::kNoPosition, m_pHotcue1EndPosition->get());

    // Set a beatloop at position 0
    m_pBeatloopActivate->slotSet(1);
    m_pBeatloopActivate->slotSet(0);
    m_pPlay->slotSet(1);

    // Save currently active loop to hotcue slot 1
    m_pHotcue1SetLoop->slotSet(1);
    m_pHotcue1SetLoop->slotSet(0);
    EXPECT_DOUBLE_EQ(static_cast<double>(HotcueControl::Status::Active), m_pHotcue1Enabled->get());
    EXPECT_DOUBLE_EQ(0, m_pHotcue1Position->get());
    EXPECT_DOUBLE_EQ(loopLength, m_pHotcue1EndPosition->get());

    // Disable Loop
    m_pLoopToggle->slotSet(1);
    m_pLoopToggle->slotSet(0);
    ProcessBuffer();
    EXPECT_DOUBLE_EQ(static_cast<double>(HotcueControl::Status::Valid), m_pHotcue1Enabled->get());

    // Move loop right (0 => 4 beats) while saved loop is disabled
    m_pLoopMove->slotSet(loopSize);
    EXPECT_DOUBLE_EQ(0, m_pHotcue1Position->get());
    EXPECT_DOUBLE_EQ(loopLength, m_pHotcue1EndPosition->get());

    // Move loop left (4 => 0 beats) while saved loop is disabled
    m_pLoopMove->slotSet(-loopSize);
    EXPECT_DOUBLE_EQ(0, m_pHotcue1Position->get());
    EXPECT_DOUBLE_EQ(loopLength, m_pHotcue1EndPosition->get());

    // Move loop left (0 => -4 beats) while saved loop is disabled
    m_pLoopMove->slotSet(-loopSize);
    EXPECT_DOUBLE_EQ(0, m_pHotcue1Position->get());
    EXPECT_DOUBLE_EQ(loopLength, m_pHotcue1EndPosition->get());

    m_pPlay->slotSet(0);
}

TEST_F(HotcueControlTest, SavedLoopReset) {
    // Setup fake track with 120 bpm can calculate loop size
    TrackPointer pTrack = createTestTrack();
    pTrack->setBpm(120.0);

    m_pBeatloopSize->slotSet(4);
    const double beatLength = getBeatLengthFrames(pTrack);
    const double loopLength = m_pBeatloopSize->get() * beatLength;

    loadTrack(pTrack);
    ProcessBuffer();

    EXPECT_DOUBLE_EQ(static_cast<double>(HotcueControl::Status::Invalid), m_pHotcue1Enabled->get());
    EXPECT_DOUBLE_EQ(Cue::kNoPosition, m_pHotcue1Position->get());
    EXPECT_DOUBLE_EQ(Cue::kNoPosition, m_pHotcue1EndPosition->get());

    // Set a beatloop
    m_pBeatloopActivate->slotSet(1);
    m_pBeatloopActivate->slotSet(0);
    ProcessBuffer();

    // Save currently active loop to hotcue slot 1
    m_pHotcue1SetLoop->slotSet(1);
    m_pHotcue1SetLoop->slotSet(0);
    EXPECT_DOUBLE_EQ(static_cast<double>(HotcueControl::Status::Active), m_pHotcue1Enabled->get());
    EXPECT_DOUBLE_EQ(0, m_pHotcue1Position->get());
    EXPECT_DOUBLE_EQ(loopLength, m_pHotcue1EndPosition->get());

    // Set a new beatloop
    setCurrentSample(loopLength);
    m_pBeatloopActivate->slotSet(1);
    m_pBeatloopActivate->slotSet(0);
    ProcessBuffer();

    // Check if setting the new beatloop disabled the current saved loop
    EXPECT_DOUBLE_EQ(static_cast<double>(HotcueControl::Status::Valid), m_pHotcue1Enabled->get());
    EXPECT_DOUBLE_EQ(0, m_pHotcue1Position->get());
    EXPECT_DOUBLE_EQ(loopLength, m_pHotcue1EndPosition->get());
}

TEST_F(HotcueControlTest, SavedLoopToggle) {
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
    m_pHotcue1LoopToggle->slotSet(1);
    m_pHotcue1LoopToggle->slotSet(0);

    EXPECT_DOUBLE_EQ(0.0, m_pLoopEnabled->get());
    EXPECT_DOUBLE_EQ(static_cast<double>(HotcueControl::Status::Valid), m_pHotcue1Enabled->get());
    EXPECT_DOUBLE_EQ(100, m_pHotcue1Position->get());
    EXPECT_DOUBLE_EQ(200, m_pHotcue1EndPosition->get());

    // Re-Enable Loop
    m_pHotcue1LoopToggle->slotSet(1);
    m_pHotcue1LoopToggle->slotSet(0);

    EXPECT_DOUBLE_EQ(1.0, m_pLoopEnabled->get());
    EXPECT_DOUBLE_EQ(static_cast<double>(HotcueControl::Status::Active), m_pHotcue1Enabled->get());
    EXPECT_DOUBLE_EQ(100, m_pHotcue1Position->get());
    EXPECT_DOUBLE_EQ(200, m_pHotcue1EndPosition->get());
}

TEST_F(HotcueControlTest, SavedLoopToggleDoesNotSeek) {
    // Setup fake track with 120 bpm can calculate loop size
    TrackPointer pTrack = createTestTrack();
    pTrack->setBpm(120.0);

    constexpr double loopSize = 4;
    const double beatLength = getBeatLengthFrames(pTrack);
    const double loopLength = loopSize * beatLength;

    const double beforeLoopPosition = 0;
    const double loopStartPosition = 8 * beatLength;
    const double afterLoopPosition = 16 * beatLength;

    loadTrack(pTrack);
    ProcessBuffer();

    EXPECT_DOUBLE_EQ(static_cast<double>(HotcueControl::Status::Invalid), m_pHotcue1Enabled->get());
    EXPECT_DOUBLE_EQ(Cue::kNoPosition, m_pHotcue1Position->get());
    EXPECT_DOUBLE_EQ(Cue::kNoPosition, m_pHotcue1EndPosition->get());

    // Seek to loop start position
    setCurrentSample(loopStartPosition);
    ProcessBuffer();

    // Set a beatloop
    m_pBeatloopActivate->slotSet(1);
    m_pBeatloopActivate->slotSet(0);

    // Save currently active loop to hotcue slot 1
    m_pHotcue1SetLoop->slotSet(1);
    m_pHotcue1SetLoop->slotSet(0);
    EXPECT_DOUBLE_EQ(static_cast<double>(HotcueControl::Status::Active), m_pHotcue1Enabled->get());
    EXPECT_DOUBLE_EQ(loopStartPosition, m_pHotcue1Position->get());
    EXPECT_DOUBLE_EQ(loopStartPosition + loopLength, m_pHotcue1EndPosition->get());

    // Seek to start of track
    setCurrentSample(beforeLoopPosition);
    ProcessBuffer();
    EXPECT_DOUBLE_EQ(beforeLoopPosition, getCurrentSample());

    // Check that the previous seek disabled the loop
    EXPECT_DOUBLE_EQ(static_cast<double>(HotcueControl::Status::Valid), m_pHotcue1Enabled->get());
    EXPECT_DOUBLE_EQ(loopStartPosition, m_pHotcue1Position->get());
    EXPECT_DOUBLE_EQ(loopStartPosition + loopLength, m_pHotcue1EndPosition->get());

    // Re-Enable loop
    m_pHotcue1LoopToggle->slotSet(1);
    m_pHotcue1LoopToggle->slotSet(0);
    ProcessBuffer();
    EXPECT_DOUBLE_EQ(static_cast<double>(HotcueControl::Status::Active), m_pHotcue1Enabled->get());
    EXPECT_DOUBLE_EQ(loopStartPosition, m_pHotcue1Position->get());
    EXPECT_DOUBLE_EQ(loopStartPosition + loopLength, m_pHotcue1EndPosition->get());

    // Check that re-enabling loop didn't seek
    EXPECT_DOUBLE_EQ(beforeLoopPosition, getCurrentSample());

    // Disable loop
    m_pHotcue1LoopToggle->slotSet(1);
    m_pHotcue1LoopToggle->slotSet(0);
    ProcessBuffer();
    EXPECT_DOUBLE_EQ(static_cast<double>(HotcueControl::Status::Valid), m_pHotcue1Enabled->get());
    EXPECT_DOUBLE_EQ(loopStartPosition, m_pHotcue1Position->get());
    EXPECT_DOUBLE_EQ(loopStartPosition + loopLength, m_pHotcue1EndPosition->get());

    // Check that re-enabling loop didn't seek
    EXPECT_DOUBLE_EQ(beforeLoopPosition, getCurrentSample());

    // Seek to position after saved loop
    setCurrentSample(afterLoopPosition);
    ProcessBuffer();

    // Check that the previous seek disabled the loop
    EXPECT_DOUBLE_EQ(static_cast<double>(HotcueControl::Status::Valid), m_pHotcue1Enabled->get());
    EXPECT_DOUBLE_EQ(loopStartPosition, m_pHotcue1Position->get());
    EXPECT_DOUBLE_EQ(loopStartPosition + loopLength, m_pHotcue1EndPosition->get());

    // Re-Enable loop
    m_pHotcue1LoopToggle->slotSet(1);
    m_pHotcue1LoopToggle->slotSet(0);
    ProcessBuffer();
    EXPECT_DOUBLE_EQ(static_cast<double>(HotcueControl::Status::Active), m_pHotcue1Enabled->get());
    EXPECT_DOUBLE_EQ(loopStartPosition, m_pHotcue1Position->get());
    EXPECT_DOUBLE_EQ(loopStartPosition + loopLength, m_pHotcue1EndPosition->get());

    // Check that re-enabling loop didn't seek
    EXPECT_DOUBLE_EQ(afterLoopPosition, getCurrentSample());

    // Disable loop
    m_pHotcue1LoopToggle->slotSet(1);
    m_pHotcue1LoopToggle->slotSet(0);
    ProcessBuffer();
    EXPECT_DOUBLE_EQ(static_cast<double>(HotcueControl::Status::Valid), m_pHotcue1Enabled->get());
    EXPECT_DOUBLE_EQ(loopStartPosition, m_pHotcue1Position->get());
    EXPECT_DOUBLE_EQ(loopStartPosition + loopLength, m_pHotcue1EndPosition->get());

    // Check that re-enabling loop didn't seek
    EXPECT_DOUBLE_EQ(afterLoopPosition, getCurrentSample());
}

TEST_F(HotcueControlTest, SavedLoopActivate) {
    // Setup fake track with 120 bpm can calculate loop size
    TrackPointer pTrack = createTestTrack();
    pTrack->setBpm(120.0);

    m_pBeatloopSize->slotSet(4);
    const double beatLength = getBeatLengthFrames(pTrack);
    const double loopLength = m_pBeatloopSize->get() * beatLength;

    const double beforeLoopPosition = 0;
    const double loopStartPosition = 8 * beatLength;
    const double afterLoopPosition = 16 * beatLength;

    loadTrack(pTrack);
    ProcessBuffer();

    EXPECT_DOUBLE_EQ(static_cast<double>(HotcueControl::Status::Invalid), m_pHotcue1Enabled->get());
    EXPECT_DOUBLE_EQ(Cue::kNoPosition, m_pHotcue1Position->get());
    EXPECT_DOUBLE_EQ(Cue::kNoPosition, m_pHotcue1EndPosition->get());

    // Seek to loop start position
    setCurrentSample(loopStartPosition);
    ProcessBuffer();

    // Set a beatloop
    m_pBeatloopActivate->slotSet(1);
    m_pBeatloopActivate->slotSet(0);

    // Save currently active loop to hotcue slot 1
    m_pHotcue1Activate->slotSet(1);
    m_pHotcue1Activate->slotSet(0);
    EXPECT_DOUBLE_EQ(static_cast<double>(HotcueControl::Status::Active), m_pHotcue1Enabled->get());
    EXPECT_DOUBLE_EQ(loopStartPosition, m_pHotcue1Position->get());
    EXPECT_DOUBLE_EQ(loopStartPosition + loopLength, m_pHotcue1EndPosition->get());

    // Seek to start of track
    setCurrentSample(beforeLoopPosition);
    ProcessBuffer();
    EXPECT_DOUBLE_EQ(beforeLoopPosition, getCurrentSample());

    // Check that the previous seek disabled the loop
    EXPECT_DOUBLE_EQ(static_cast<double>(HotcueControl::Status::Valid), m_pHotcue1Enabled->get());
    EXPECT_DOUBLE_EQ(loopStartPosition, m_pHotcue1Position->get());
    EXPECT_DOUBLE_EQ(loopStartPosition + loopLength, m_pHotcue1EndPosition->get());

    // Activate saved loop (implies seeking to loop start)
    m_pHotcue1Activate->slotSet(1);
    m_pHotcue1Activate->slotSet(0);
    ProcessBuffer();
    EXPECT_DOUBLE_EQ(static_cast<double>(HotcueControl::Status::Active), m_pHotcue1Enabled->get());
    EXPECT_DOUBLE_EQ(loopStartPosition, m_pHotcue1Position->get());
    EXPECT_DOUBLE_EQ(loopStartPosition + loopLength, m_pHotcue1EndPosition->get());
    EXPECT_DOUBLE_EQ(loopStartPosition, getCurrentSample());

    // Disable loop
    // Seek to position after saved loop
    setCurrentSample(afterLoopPosition);
    ProcessBuffer();

    // Check that the previous seek disabled the loop
    EXPECT_DOUBLE_EQ(static_cast<double>(HotcueControl::Status::Valid), m_pHotcue1Enabled->get());
    EXPECT_DOUBLE_EQ(loopStartPosition, m_pHotcue1Position->get());
    EXPECT_DOUBLE_EQ(loopStartPosition + loopLength, m_pHotcue1EndPosition->get());

    // Activate saved loop (implies seeking to loop start)
    m_pHotcue1Activate->slotSet(1);
    m_pHotcue1Activate->slotSet(0);
    ProcessBuffer();
    EXPECT_DOUBLE_EQ(static_cast<double>(HotcueControl::Status::Active), m_pHotcue1Enabled->get());
    EXPECT_DOUBLE_EQ(loopStartPosition, m_pHotcue1Position->get());
    EXPECT_DOUBLE_EQ(loopStartPosition + loopLength, m_pHotcue1EndPosition->get());
    EXPECT_DOUBLE_EQ(loopStartPosition, getCurrentSample());
}
