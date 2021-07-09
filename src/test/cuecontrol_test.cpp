#include "engine/controls/cuecontrol.h"
#include "test/signalpathtest.h"

class CueControlTest : public BaseSignalPathTest {
  protected:
    void SetUp() override {
        BaseSignalPathTest::SetUp();

        m_pQuantizeEnabled = std::make_unique<ControlProxy>(m_sGroup1, "quantize");
        m_pCuePoint = std::make_unique<ControlProxy>(m_sGroup1, "cue_point");
        m_pIntroStartPosition = std::make_unique<ControlProxy>(m_sGroup1, "intro_start_position");
        m_pIntroStartEnabled = std::make_unique<ControlProxy>(m_sGroup1, "intro_start_enabled");
        m_pIntroStartSet = std::make_unique<ControlProxy>(m_sGroup1, "intro_start_set");
        m_pIntroStartClear = std::make_unique<ControlProxy>(m_sGroup1, "intro_start_clear");
        m_pIntroEndPosition = std::make_unique<ControlProxy>(m_sGroup1, "intro_end_position");
        m_pIntroEndEnabled = std::make_unique<ControlProxy>(m_sGroup1, "intro_end_enabled");
        m_pIntroEndSet = std::make_unique<ControlProxy>(m_sGroup1, "intro_end_set");
        m_pIntroEndClear = std::make_unique<ControlProxy>(m_sGroup1, "intro_end_clear");
        m_pOutroStartPosition = std::make_unique<ControlProxy>(m_sGroup1, "outro_start_position");
        m_pOutroStartEnabled = std::make_unique<ControlProxy>(m_sGroup1, "outro_start_enabled");
        m_pOutroStartSet = std::make_unique<ControlProxy>(m_sGroup1, "outro_start_set");
        m_pOutroStartClear = std::make_unique<ControlProxy>(m_sGroup1, "outro_start_clear");
        m_pOutroEndPosition = std::make_unique<ControlProxy>(m_sGroup1, "outro_end_position");
        m_pOutroEndEnabled = std::make_unique<ControlProxy>(m_sGroup1, "outro_end_enabled");
        m_pOutroEndSet = std::make_unique<ControlProxy>(m_sGroup1, "outro_end_set");
        m_pOutroEndClear = std::make_unique<ControlProxy>(m_sGroup1, "outro_end_clear");
    }

    TrackPointer createTestTrack() const {
        const QString kTrackLocationTest = QDir::currentPath() + "/src/test/sine-30.wav";
        const auto pTrack = Track::newTemporary(
                mixxx::FileAccess(mixxx::FileInfo(kTrackLocationTest)));
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

    void setCurrentSample(double samplePosition) {
        const auto position = mixxx::audio::FramePos::fromEngineSamplePos(samplePosition);
        m_pChannel1->getEngineBuffer()->queueNewPlaypos(position, EngineBuffer::SEEK_STANDARD);
        ProcessBuffer();
    }

    std::unique_ptr<ControlProxy> m_pQuantizeEnabled;
    std::unique_ptr<ControlProxy> m_pCuePoint;
    std::unique_ptr<ControlProxy> m_pIntroStartPosition;
    std::unique_ptr<ControlProxy> m_pIntroStartEnabled;
    std::unique_ptr<ControlProxy> m_pIntroStartSet;
    std::unique_ptr<ControlProxy> m_pIntroStartClear;
    std::unique_ptr<ControlProxy> m_pIntroEndPosition;
    std::unique_ptr<ControlProxy> m_pIntroEndEnabled;
    std::unique_ptr<ControlProxy> m_pIntroEndSet;
    std::unique_ptr<ControlProxy> m_pIntroEndClear;
    std::unique_ptr<ControlProxy> m_pOutroStartPosition;
    std::unique_ptr<ControlProxy> m_pOutroStartEnabled;
    std::unique_ptr<ControlProxy> m_pOutroStartSet;
    std::unique_ptr<ControlProxy> m_pOutroStartClear;
    std::unique_ptr<ControlProxy> m_pOutroEndPosition;
    std::unique_ptr<ControlProxy> m_pOutroEndEnabled;
    std::unique_ptr<ControlProxy> m_pOutroEndSet;
    std::unique_ptr<ControlProxy> m_pOutroEndClear;
};

TEST_F(CueControlTest, LoadUnloadTrack) {
    TrackPointer pTrack = createTestTrack();
    pTrack->setCuePoint(CuePosition(100.0));
    auto pIntro = pTrack->createAndAddCue(
            mixxx::CueType::Intro,
            Cue::kNoHotCue,
            150.0,
            200.0);
    auto pOutro = pTrack->createAndAddCue(
            mixxx::CueType::Outro,
            Cue::kNoHotCue,
            250.0,
            300.0);

    loadTrack(pTrack);

    EXPECT_DOUBLE_EQ(100.0, m_pCuePoint->get());
    EXPECT_DOUBLE_EQ(150.0, m_pIntroStartPosition->get());
    EXPECT_DOUBLE_EQ(200.0, m_pIntroEndPosition->get());
    EXPECT_DOUBLE_EQ(250.0, m_pOutroStartPosition->get());
    EXPECT_DOUBLE_EQ(300.0, m_pOutroEndPosition->get());
    EXPECT_TRUE(m_pIntroStartEnabled->toBool());
    EXPECT_TRUE(m_pIntroEndEnabled->toBool());
    EXPECT_TRUE(m_pOutroStartEnabled->toBool());
    EXPECT_TRUE(m_pOutroEndEnabled->toBool());

    unloadTrack();

    EXPECT_DOUBLE_EQ(Cue::kNoPosition, m_pCuePoint->get());
    EXPECT_DOUBLE_EQ(Cue::kNoPosition, m_pIntroStartPosition->get());
    EXPECT_DOUBLE_EQ(Cue::kNoPosition, m_pIntroEndPosition->get());
    EXPECT_DOUBLE_EQ(Cue::kNoPosition, m_pOutroStartPosition->get());
    EXPECT_DOUBLE_EQ(Cue::kNoPosition, m_pOutroEndPosition->get());
    EXPECT_FALSE(m_pIntroStartEnabled->toBool());
    EXPECT_FALSE(m_pIntroEndEnabled->toBool());
    EXPECT_FALSE(m_pOutroStartEnabled->toBool());
    EXPECT_FALSE(m_pOutroEndEnabled->toBool());
}

TEST_F(CueControlTest, LoadTrackWithDetectedCues) {
    TrackPointer pTrack = createTestTrack();
    pTrack->setCuePoint(CuePosition(100.0));
    auto pIntro = pTrack->createAndAddCue(
            mixxx::CueType::Intro,
            Cue::kNoHotCue,
            100.0,
            Cue::kNoPosition);
    auto pOutro = pTrack->createAndAddCue(
            mixxx::CueType::Outro,
            Cue::kNoHotCue,
            Cue::kNoPosition,
            200.0);

    loadTrack(pTrack);

    EXPECT_DOUBLE_EQ(100.0, m_pCuePoint->get());
    EXPECT_DOUBLE_EQ(100.0, m_pIntroStartPosition->get());
    EXPECT_DOUBLE_EQ(Cue::kNoPosition, m_pIntroEndPosition->get());
    EXPECT_DOUBLE_EQ(Cue::kNoPosition, m_pOutroStartPosition->get());
    EXPECT_DOUBLE_EQ(200.0, m_pOutroEndPosition->get());
    EXPECT_TRUE(m_pIntroStartEnabled->toBool());
    EXPECT_FALSE(m_pIntroEndEnabled->toBool());
    EXPECT_FALSE(m_pOutroStartEnabled->toBool());
    EXPECT_TRUE(m_pOutroEndEnabled->toBool());
}

TEST_F(CueControlTest, LoadTrackWithIntroEndAndOutroStart) {
    TrackPointer pTrack = createTestTrack();
    auto pIntro = pTrack->createAndAddCue(
            mixxx::CueType::Intro,
            Cue::kNoHotCue,
            Cue::kNoPosition,
            150.0);
    auto pOutro = pTrack->createAndAddCue(
            mixxx::CueType::Outro,
            Cue::kNoHotCue,
            250.0,
            Cue::kNoPosition);

    loadTrack(pTrack);

    EXPECT_DOUBLE_EQ(0.0, m_pCuePoint->get());
    EXPECT_DOUBLE_EQ(Cue::kNoPosition, m_pIntroStartPosition->get());
    EXPECT_DOUBLE_EQ(150.0, m_pIntroEndPosition->get());
    EXPECT_DOUBLE_EQ(250.0, m_pOutroStartPosition->get());
    EXPECT_DOUBLE_EQ(Cue::kNoPosition, m_pOutroEndPosition->get());
    EXPECT_FALSE(m_pIntroStartEnabled->toBool());
    EXPECT_TRUE(m_pIntroEndEnabled->toBool());
    EXPECT_TRUE(m_pOutroStartEnabled->toBool());
    EXPECT_FALSE(m_pOutroEndEnabled->toBool());
}

TEST_F(CueControlTest, LoadAutodetectedCues_QuantizeEnabled) {
    m_pQuantizeEnabled->set(1);

    TrackPointer pTrack = createTestTrack();
    pTrack->trySetBpm(120.0);

    const int frameSize = 2;
    const int sampleRate = pTrack->getSampleRate();
    const double bpm = pTrack->getBpm();
    const double beatLength = (60.0 * sampleRate / bpm) * frameSize;

    pTrack->setCuePoint(CuePosition(1.9 * beatLength));

    auto pIntro = pTrack->createAndAddCue(
            mixxx::CueType::Intro,
            Cue::kNoHotCue,
            2.1 * beatLength,
            3.7 * beatLength);

    auto pOutro = pTrack->createAndAddCue(
            mixxx::CueType::Outro,
            Cue::kNoHotCue,
            11.1 * beatLength,
            15.5 * beatLength);

    loadTrack(pTrack);

    EXPECT_DOUBLE_EQ(2.0 * beatLength, m_pCuePoint->get());
    EXPECT_DOUBLE_EQ(2.0 * beatLength, m_pIntroStartPosition->get());
    EXPECT_DOUBLE_EQ(4.0 * beatLength, m_pIntroEndPosition->get());
    EXPECT_DOUBLE_EQ(11.0 * beatLength, m_pOutroStartPosition->get());
    EXPECT_DOUBLE_EQ(16.0 * beatLength, m_pOutroEndPosition->get());
}

TEST_F(CueControlTest, LoadAutodetectedCues_QuantizeEnabledNoBeats) {
    m_pQuantizeEnabled->set(1);

    TrackPointer pTrack = createTestTrack();
    pTrack->trySetBpm(0.0);

    pTrack->setCuePoint(CuePosition(100.0));

    auto pIntro = pTrack->createAndAddCue(
            mixxx::CueType::Intro,
            Cue::kNoHotCue,
            250.0,
            400.0);

    auto pOutro = pTrack->createAndAddCue(
            mixxx::CueType::Outro,
            Cue::kNoHotCue,
            550.0,
            800.0);

    loadTrack(pTrack);

    EXPECT_DOUBLE_EQ(100.0, m_pCuePoint->get());
    EXPECT_DOUBLE_EQ(250.0, m_pIntroStartPosition->get());
    EXPECT_DOUBLE_EQ(400.0, m_pIntroEndPosition->get());
    EXPECT_DOUBLE_EQ(550.0, m_pOutroStartPosition->get());
    EXPECT_DOUBLE_EQ(800.0, m_pOutroEndPosition->get());
}

TEST_F(CueControlTest, LoadAutodetectedCues_QuantizeDisabled) {
    m_pQuantizeEnabled->set(0);

    TrackPointer pTrack = createTestTrack();
    pTrack->trySetBpm(120.0);

    pTrack->setCuePoint(CuePosition(240.0));

    auto pIntro = pTrack->createAndAddCue(
            mixxx::CueType::Intro,
            Cue::kNoHotCue,
            210.0,
            330.0);

    auto pOutro = pTrack->createAndAddCue(
            mixxx::CueType::Outro,
            Cue::kNoHotCue,
            770.0,
            990.0);

    loadTrack(pTrack);

    EXPECT_DOUBLE_EQ(240.0, m_pCuePoint->get());
    EXPECT_DOUBLE_EQ(210.0, m_pIntroStartPosition->get());
    EXPECT_DOUBLE_EQ(330.0, m_pIntroEndPosition->get());
    EXPECT_DOUBLE_EQ(770.0, m_pOutroStartPosition->get());
    EXPECT_DOUBLE_EQ(990.0, m_pOutroEndPosition->get());
}

TEST_F(CueControlTest, SeekOnLoadDefault) {
    // Default is to load at the intro start
    TrackPointer pTrack = createTestTrack();
    auto pIntro = pTrack->createAndAddCue(
            mixxx::CueType::Intro,
            Cue::kNoHotCue,
            250.0,
            400.0);

    loadTrack(pTrack);

    EXPECT_DOUBLE_EQ(250.0, m_pIntroStartPosition->get());
    EXPECT_DOUBLE_EQ(250.0, getCurrentSample());
}

TEST_F(CueControlTest, SeekOnLoadMainCue) {
    config()->set(ConfigKey("[Controls]", "CueRecall"),
            ConfigValue(static_cast<int>(SeekOnLoadMode::MainCue)));
    TrackPointer pTrack = createTestTrack();
    loadTrack(pTrack);

    // We expect a cue point at the very beginning
    EXPECT_DOUBLE_EQ(0.0, m_pCuePoint->get());
    EXPECT_DOUBLE_EQ(0.0, getCurrentSample());

    // Move cue like silence analysis does and check if track is following it
    pTrack->setCuePoint(CuePosition(200.0));
    pTrack->analysisFinished();
    ProcessBuffer();

    EXPECT_DOUBLE_EQ(200.0, m_pCuePoint->get());
    EXPECT_DOUBLE_EQ(200.0, getCurrentSample());
}

TEST_F(CueControlTest, DontSeekOnLoadMainCue) {
    config()->set(ConfigKey("[Controls]", "CueRecall"),
            ConfigValue(static_cast<int>(SeekOnLoadMode::MainCue)));
    TrackPointer pTrack = createTestTrack();
    pTrack->setCuePoint(CuePosition(100.0));

    // The Track should not follow cue changes due to the analyzer if the
    // track has been manual seeked before.
    loadTrack(pTrack);

    EXPECT_DOUBLE_EQ(100.0, m_pCuePoint->get());
    EXPECT_DOUBLE_EQ(100.0, getCurrentSample());

    // Manually seek  the track
    setCurrentSample(200.0);

    // Move cue like silence analysis does and check if track is following it
    pTrack->setCuePoint(CuePosition(400.0));
    pTrack->analysisFinished();
    ProcessBuffer();

    EXPECT_DOUBLE_EQ(400.0, m_pCuePoint->get());
    EXPECT_DOUBLE_EQ(200.0, getCurrentSample());
}

TEST_F(CueControlTest, SeekOnLoadDefault_CueInPreroll) {
    config()->set(ConfigKey("[Controls]", "CueRecall"),
            ConfigValue(static_cast<int>(SeekOnLoadMode::MainCue)));
    TrackPointer pTrack = createTestTrack();
    pTrack->setCuePoint(CuePosition(-100.0));

    loadTrack(pTrack);

    EXPECT_DOUBLE_EQ(-100.0, m_pCuePoint->get());
    EXPECT_DOUBLE_EQ(-100.0, getCurrentSample());

    // Move cue like silence analysis does and check if track is following it
    pTrack->setCuePoint(CuePosition(-200.0));
    pTrack->analysisFinished();
    ProcessBuffer();

    EXPECT_DOUBLE_EQ(-200.0, m_pCuePoint->get());
    EXPECT_DOUBLE_EQ(-200.0, getCurrentSample());
}

TEST_F(CueControlTest, FollowCueOnQuantize) {
    config()->set(ConfigKey("[Controls]", "CueRecall"),
            ConfigValue(static_cast<int>(SeekOnLoadMode::MainCue)));
    TrackPointer pTrack = createTestTrack();
    pTrack->trySetBpm(120.0);

    const int frameSize = 2;
    const int sampleRate = pTrack->getSampleRate();
    const double bpm = pTrack->getBpm();
    const double beatLength = (60.0 * sampleRate / bpm) * frameSize;
    double cuePos = 1.8 * beatLength;
    double quantizedCuePos = 2.0 * beatLength;
    pTrack->setCuePoint(cuePos);

    loadTrack(pTrack);

    EXPECT_DOUBLE_EQ(cuePos, m_pCuePoint->get());
    EXPECT_DOUBLE_EQ(cuePos, getCurrentSample());

    // enable quantization and expect current position to follow
    m_pQuantizeEnabled->set(1);
    ProcessBuffer();
    EXPECT_DOUBLE_EQ(quantizedCuePos, m_pCuePoint->get());
    EXPECT_DOUBLE_EQ(quantizedCuePos, getCurrentSample());

    // move current position to track start
    m_pQuantizeEnabled->set(0);
    ProcessBuffer();
    setCurrentSample(0.0);
    ProcessBuffer();
    EXPECT_DOUBLE_EQ(0.0, getCurrentSample());

    // enable quantization again and expect play position to stay at track start
    m_pQuantizeEnabled->set(1);
    ProcessBuffer();
    EXPECT_DOUBLE_EQ(quantizedCuePos, m_pCuePoint->get());
    EXPECT_DOUBLE_EQ(0.0, getCurrentSample());
}

TEST_F(CueControlTest, IntroCue_SetStartEnd_ClearStartEnd) {
    TrackPointer pTrack = createAndLoadFakeTrack();

    // Set intro start cue
    setCurrentSample(100.0);
    m_pIntroStartSet->slotSet(1);
    m_pIntroStartSet->slotSet(0);
    EXPECT_DOUBLE_EQ(100.0, m_pIntroStartPosition->get());
    EXPECT_TRUE(m_pIntroStartEnabled->toBool());
    EXPECT_DOUBLE_EQ(Cue::kNoPosition, m_pIntroEndPosition->get());
    EXPECT_FALSE(m_pIntroEndEnabled->toBool());

    CuePointer pCue = pTrack->findCueByType(mixxx::CueType::Intro);
    EXPECT_NE(nullptr, pCue);
    if (pCue != nullptr) {
        EXPECT_DOUBLE_EQ(100.0, pCue->getPosition().toEngineSamplePos());
        EXPECT_DOUBLE_EQ(0.0, pCue->getLengthFrames() * mixxx::kEngineChannelCount);
    }

    // Set intro end cue
    setCurrentSample(500.0);
    m_pIntroEndSet->slotSet(1);
    m_pIntroEndSet->slotSet(0);
    EXPECT_DOUBLE_EQ(100.0, m_pIntroStartPosition->get());
    EXPECT_TRUE(m_pIntroStartEnabled->toBool());
    EXPECT_DOUBLE_EQ(500.0, m_pIntroEndPosition->get());
    EXPECT_TRUE(m_pIntroEndEnabled->toBool());

    pCue = pTrack->findCueByType(mixxx::CueType::Intro);
    EXPECT_NE(nullptr, pCue);
    if (pCue != nullptr) {
        EXPECT_DOUBLE_EQ(100.0, pCue->getPosition().toEngineSamplePos());
        EXPECT_DOUBLE_EQ(400.0, pCue->getLengthFrames() * mixxx::kEngineChannelCount);
    }

    // Clear intro start cue
    m_pIntroStartClear->slotSet(1);
    m_pIntroStartClear->slotSet(0);
    EXPECT_DOUBLE_EQ(Cue::kNoPosition, m_pIntroStartPosition->get());
    EXPECT_FALSE(m_pIntroStartEnabled->toBool());
    EXPECT_DOUBLE_EQ(500.0, m_pIntroEndPosition->get());
    EXPECT_TRUE(m_pIntroEndEnabled->toBool());

    pCue = pTrack->findCueByType(mixxx::CueType::Intro);
    EXPECT_NE(nullptr, pCue);
    if (pCue != nullptr) {
        EXPECT_EQ(mixxx::audio::kInvalidFramePos, pCue->getPosition());
        EXPECT_DOUBLE_EQ(500.0, pCue->getLengthFrames() * mixxx::kEngineChannelCount);
    }

    // Clear intro end cue
    m_pIntroEndClear->slotSet(1);
    m_pIntroEndClear->slotSet(0);
    EXPECT_DOUBLE_EQ(Cue::kNoPosition, m_pIntroStartPosition->get());
    EXPECT_FALSE(m_pIntroStartEnabled->toBool());
    EXPECT_DOUBLE_EQ(Cue::kNoPosition, m_pIntroEndPosition->get());
    EXPECT_FALSE(m_pIntroEndEnabled->toBool());

    EXPECT_EQ(nullptr, pTrack->findCueByType(mixxx::CueType::Intro));
}

TEST_F(CueControlTest, OutroCue_SetStartEnd_ClearStartEnd) {
    TrackPointer pTrack = createAndLoadFakeTrack();

    // Set outro start cue
    setCurrentSample(750.0);
    m_pOutroStartSet->slotSet(1);
    m_pOutroStartSet->slotSet(0);
    EXPECT_DOUBLE_EQ(750.0, m_pOutroStartPosition->get());
    EXPECT_TRUE(m_pOutroStartEnabled->toBool());
    EXPECT_DOUBLE_EQ(Cue::kNoPosition, m_pOutroEndPosition->get());
    EXPECT_FALSE(m_pOutroEndEnabled->toBool());

    CuePointer pCue = pTrack->findCueByType(mixxx::CueType::Outro);
    EXPECT_NE(nullptr, pCue);
    if (pCue != nullptr) {
        EXPECT_DOUBLE_EQ(750.0, pCue->getPosition().toEngineSamplePos());
        EXPECT_DOUBLE_EQ(0.0, pCue->getLengthFrames() * mixxx::kEngineChannelCount);
    }

    // Set outro end cue
    setCurrentSample(1000.0);
    m_pOutroEndSet->slotSet(1);
    m_pOutroEndSet->slotSet(0);
    EXPECT_DOUBLE_EQ(750.0, m_pOutroStartPosition->get());
    EXPECT_TRUE(m_pOutroStartEnabled->toBool());
    EXPECT_DOUBLE_EQ(1000.0, m_pOutroEndPosition->get());
    EXPECT_TRUE(m_pOutroEndEnabled->toBool());

    pCue = pTrack->findCueByType(mixxx::CueType::Outro);
    EXPECT_NE(nullptr, pCue);
    if (pCue != nullptr) {
        EXPECT_DOUBLE_EQ(750.0, pCue->getPosition().toEngineSamplePos());
        EXPECT_DOUBLE_EQ(250.0, pCue->getLengthFrames() * mixxx::kEngineChannelCount);
    }

    // Clear outro start cue
    m_pOutroStartClear->slotSet(1);
    m_pOutroStartClear->slotSet(0);
    EXPECT_DOUBLE_EQ(Cue::kNoPosition, m_pOutroStartPosition->get());
    EXPECT_FALSE(m_pOutroStartEnabled->toBool());
    EXPECT_DOUBLE_EQ(1000.0, m_pOutroEndPosition->get());
    EXPECT_TRUE(m_pOutroEndEnabled->toBool());

    pCue = pTrack->findCueByType(mixxx::CueType::Outro);
    EXPECT_NE(nullptr, pCue);
    if (pCue != nullptr) {
        EXPECT_EQ(mixxx::audio::kInvalidFramePos, pCue->getPosition());
        EXPECT_DOUBLE_EQ(1000.0, pCue->getLengthFrames() * mixxx::kEngineChannelCount);
    }

    // Clear outro end cue
    m_pOutroEndClear->slotSet(1);
    m_pOutroEndClear->slotSet(0);
    EXPECT_DOUBLE_EQ(Cue::kNoPosition, m_pOutroStartPosition->get());
    EXPECT_FALSE(m_pOutroStartEnabled->toBool());
    EXPECT_DOUBLE_EQ(Cue::kNoPosition, m_pOutroEndPosition->get());
    EXPECT_FALSE(m_pOutroEndEnabled->toBool());

    EXPECT_EQ(nullptr, pTrack->findCueByType(mixxx::CueType::Outro));
}
