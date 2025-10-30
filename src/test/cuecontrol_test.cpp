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
        const QString kTrackLocationTest = getTestDir().filePath(QStringLiteral("sine-30.wav"));
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
        m_pMixerDeck1->slotLoadTrack(TrackPointer(),
#ifdef __STEM__
                mixxx::StemChannelSelection(),
#endif
                false);
    }

    mixxx::audio::FramePos getCurrentFramePos() {
        return m_pChannel1->getEngineBuffer()->m_pCueControl->frameInfo().currentPosition;
    }

    void setCurrentFramePos(mixxx::audio::FramePos position) {
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
    constexpr auto kCuePosition = mixxx::audio::FramePos(100);
    constexpr auto kIntroStartPosition = mixxx::audio::FramePos(150);
    constexpr auto kIntroEndPosition = mixxx::audio::FramePos(200);
    constexpr auto kOutroStartPosition = mixxx::audio::FramePos(250);
    constexpr auto kOutroEndPosition = mixxx::audio::FramePos(300);

    TrackPointer pTrack = createTestTrack();
    pTrack->setMainCuePosition(kCuePosition);
    auto pIntro = pTrack->createAndAddCue(
            mixxx::CueType::Intro,
            Cue::kNoHotCue,
            kIntroStartPosition,
            kIntroEndPosition);
    auto pOutro = pTrack->createAndAddCue(
            mixxx::CueType::Outro,
            Cue::kNoHotCue,
            kOutroStartPosition,
            kOutroEndPosition);

    loadTrack(pTrack);

    EXPECT_FRAMEPOS_EQ_CONTROL(kCuePosition, m_pCuePoint);
    EXPECT_FRAMEPOS_EQ_CONTROL(kIntroStartPosition, m_pIntroStartPosition);
    EXPECT_FRAMEPOS_EQ_CONTROL(kIntroEndPosition, m_pIntroEndPosition);
    EXPECT_FRAMEPOS_EQ_CONTROL(kOutroStartPosition, m_pOutroStartPosition);
    EXPECT_FRAMEPOS_EQ_CONTROL(kOutroEndPosition, m_pOutroEndPosition);
    EXPECT_TRUE(m_pIntroStartEnabled->toBool());
    EXPECT_TRUE(m_pIntroEndEnabled->toBool());
    EXPECT_TRUE(m_pOutroStartEnabled->toBool());
    EXPECT_TRUE(m_pOutroEndEnabled->toBool());

    unloadTrack();

    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::kInvalidFramePos, m_pCuePoint);
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::kInvalidFramePos, m_pIntroStartPosition);
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::kInvalidFramePos, m_pIntroEndPosition);
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::kInvalidFramePos, m_pOutroStartPosition);
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::kInvalidFramePos, m_pOutroEndPosition);
    EXPECT_FALSE(m_pIntroStartEnabled->toBool());
    EXPECT_FALSE(m_pIntroEndEnabled->toBool());
    EXPECT_FALSE(m_pOutroStartEnabled->toBool());
    EXPECT_FALSE(m_pOutroEndEnabled->toBool());
}

TEST_F(CueControlTest, LoadTrackWithDetectedCues) {
    constexpr auto kCuePosition = mixxx::audio::FramePos(100);
    constexpr auto kOutroEndPosition = mixxx::audio::FramePos(200);

    TrackPointer pTrack = createTestTrack();
    pTrack->setMainCuePosition(kCuePosition);
    auto pIntro = pTrack->createAndAddCue(
            mixxx::CueType::Intro,
            Cue::kNoHotCue,
            kCuePosition,
            mixxx::audio::kInvalidFramePos);
    auto pOutro = pTrack->createAndAddCue(
            mixxx::CueType::Outro,
            Cue::kNoHotCue,
            mixxx::audio::kInvalidFramePos,
            kOutroEndPosition);

    loadTrack(pTrack);

    EXPECT_FRAMEPOS_EQ_CONTROL(kCuePosition, m_pCuePoint);
    EXPECT_FRAMEPOS_EQ_CONTROL(kCuePosition, m_pIntroStartPosition);
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::kInvalidFramePos, m_pIntroEndPosition);
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::kInvalidFramePos, m_pOutroStartPosition);
    EXPECT_FRAMEPOS_EQ_CONTROL(kOutroEndPosition, m_pOutroEndPosition);
    EXPECT_TRUE(m_pIntroStartEnabled->toBool());
    EXPECT_FALSE(m_pIntroEndEnabled->toBool());
    EXPECT_FALSE(m_pOutroStartEnabled->toBool());
    EXPECT_TRUE(m_pOutroEndEnabled->toBool());
}

TEST_F(CueControlTest, LoadTrackWithIntroEndAndOutroStart) {
    constexpr auto kIntroEndPosition = mixxx::audio::FramePos(150);
    constexpr auto kOutroStartPosition = mixxx::audio::FramePos(250);

    TrackPointer pTrack = createTestTrack();
    auto pIntro = pTrack->createAndAddCue(
            mixxx::CueType::Intro,
            Cue::kNoHotCue,
            mixxx::audio::kInvalidFramePos,
            kIntroEndPosition);
    auto pOutro = pTrack->createAndAddCue(
            mixxx::CueType::Outro,
            Cue::kNoHotCue,
            kOutroStartPosition,
            mixxx::audio::kInvalidFramePos);

    loadTrack(pTrack);

    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::kStartFramePos, m_pCuePoint);
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::kInvalidFramePos, m_pIntroStartPosition);
    EXPECT_FRAMEPOS_EQ_CONTROL(kIntroEndPosition, m_pIntroEndPosition);
    EXPECT_FRAMEPOS_EQ_CONTROL(kOutroStartPosition, m_pOutroStartPosition);
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::kInvalidFramePos, m_pOutroEndPosition);
    EXPECT_FALSE(m_pIntroStartEnabled->toBool());
    EXPECT_TRUE(m_pIntroEndEnabled->toBool());
    EXPECT_TRUE(m_pOutroStartEnabled->toBool());
    EXPECT_FALSE(m_pOutroEndEnabled->toBool());
}

TEST_F(CueControlTest, LoadAutodetectedCues_QuantizeEnabled) {
    m_pQuantizeEnabled->set(1);

    TrackPointer pTrack = createTestTrack();
    pTrack->trySetBpm(120.0);

    const mixxx::audio::SampleRate sampleRate = pTrack->getSampleRate();
    const double bpm = pTrack->getBpm();
    const double beatLengthFrames = (60.0 * sampleRate / bpm);

    const auto kIntroStartPosition = mixxx::audio::FramePos(2.1 * beatLengthFrames);
    const auto kQuantizedIntroStartPosition = mixxx::audio::FramePos(2.0 * beatLengthFrames);
    const auto kIntroEndPosition = mixxx::audio::FramePos(3.7 * beatLengthFrames);
    const auto kQuantizedIntroEndPosition = mixxx::audio::FramePos(4.0 * beatLengthFrames);
    const auto kOutroStartPosition = mixxx::audio::FramePos(11.1 * beatLengthFrames);
    const auto kQuantizedOutroStartPosition = mixxx::audio::FramePos(11.0 * beatLengthFrames);
    const auto kOutroEndPosition = mixxx::audio::FramePos(15.5 * beatLengthFrames);
    const auto kQuantizedOutroEndPosition = mixxx::audio::FramePos(16.0 * beatLengthFrames);

    pTrack->setMainCuePosition(mixxx::audio::FramePos(1.9 * beatLengthFrames));

    auto pIntro = pTrack->createAndAddCue(
            mixxx::CueType::Intro,
            Cue::kNoHotCue,
            kIntroStartPosition,
            kIntroEndPosition);

    auto pOutro = pTrack->createAndAddCue(
            mixxx::CueType::Outro,
            Cue::kNoHotCue,
            kOutroStartPosition,
            kOutroEndPosition);

    loadTrack(pTrack);

    EXPECT_FRAMEPOS_EQ_CONTROL(kQuantizedIntroStartPosition, m_pCuePoint);
    EXPECT_FRAMEPOS_EQ_CONTROL(kQuantizedIntroStartPosition, m_pIntroStartPosition);
    EXPECT_FRAMEPOS_EQ_CONTROL(kQuantizedIntroEndPosition, m_pIntroEndPosition);
    EXPECT_FRAMEPOS_EQ_CONTROL(kQuantizedOutroStartPosition, m_pOutroStartPosition);
    EXPECT_FRAMEPOS_EQ_CONTROL(kQuantizedOutroEndPosition, m_pOutroEndPosition);
}

TEST_F(CueControlTest, LoadAutodetectedCues_QuantizeEnabledNoBeats) {
    m_pQuantizeEnabled->set(1);

    TrackPointer pTrack = createTestTrack();
    pTrack->trySetBpm(0.0);

    constexpr auto kCuePosition = mixxx::audio::FramePos(100);
    pTrack->setMainCuePosition(kCuePosition);

    auto pIntro = pTrack->createAndAddCue(
            mixxx::CueType::Intro,
            Cue::kNoHotCue,
            mixxx::audio::FramePos(250.0),
            mixxx::audio::FramePos(400.0));

    auto pOutro = pTrack->createAndAddCue(
            mixxx::CueType::Outro,
            Cue::kNoHotCue,
            mixxx::audio::FramePos(550.0),
            mixxx::audio::FramePos(800.0));

    loadTrack(pTrack);

    EXPECT_FRAMEPOS_EQ_CONTROL(kCuePosition, m_pCuePoint);
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::FramePos(250.0), m_pIntroStartPosition);
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::FramePos(400.0), m_pIntroEndPosition);
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::FramePos(550.0), m_pOutroStartPosition);
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::FramePos(800.0), m_pOutroEndPosition);
}

TEST_F(CueControlTest, LoadAutodetectedCues_QuantizeDisabled) {
    m_pQuantizeEnabled->set(0);

    TrackPointer pTrack = createTestTrack();
    pTrack->trySetBpm(120.0);

    pTrack->setMainCuePosition(mixxx::audio::FramePos(240.0));

    auto pIntro = pTrack->createAndAddCue(
            mixxx::CueType::Intro,
            Cue::kNoHotCue,
            mixxx::audio::FramePos(210.0),
            mixxx::audio::FramePos(330.0));

    auto pOutro = pTrack->createAndAddCue(
            mixxx::CueType::Outro,
            Cue::kNoHotCue,
            mixxx::audio::FramePos(770.0),
            mixxx::audio::FramePos(990.0));

    loadTrack(pTrack);

    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::FramePos(240.0), m_pCuePoint);
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::FramePos(210.0), m_pIntroStartPosition);
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::FramePos(330.0), m_pIntroEndPosition);
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::FramePos(770.0), m_pOutroStartPosition);
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::FramePos(990.0), m_pOutroEndPosition);
}

TEST_F(CueControlTest, SeekOnLoadDefault) {
    // Default is to load at the intro start
    TrackPointer pTrack = createTestTrack();
    auto pIntro = pTrack->createAndAddCue(
            mixxx::CueType::Intro,
            Cue::kNoHotCue,
            mixxx::audio::FramePos(250.0),
            mixxx::audio::FramePos(400.0));

    loadTrack(pTrack);

    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::FramePos(250.0), m_pIntroStartPosition);
    EXPECT_FRAMEPOS_EQ(mixxx::audio::FramePos(250.0), getCurrentFramePos());
}

TEST_F(CueControlTest, SeekOnLoadMainCue) {
    config()->set(ConfigKey("[Controls]", "CueRecall"),
            ConfigValue(static_cast<int>(SeekOnLoadMode::MainCue)));
    TrackPointer pTrack = createTestTrack();
    loadTrack(pTrack);

    // We expect a cue point at the very beginning
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::kStartFramePos, m_pCuePoint);
    EXPECT_FRAMEPOS_EQ(mixxx::audio::kStartFramePos, getCurrentFramePos());

    // Move cue like silence analysis does and check if track is following it
    pTrack->setMainCuePosition(mixxx::audio::FramePos(200.0));
    pTrack->analysisFinished();
    ProcessBuffer();

    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::FramePos(200.0), m_pCuePoint);
    EXPECT_FRAMEPOS_EQ(mixxx::audio::FramePos(200.0), getCurrentFramePos());
}

TEST_F(CueControlTest, DontSeekOnLoadMainCue) {
    config()->set(ConfigKey("[Controls]", "CueRecall"),
            ConfigValue(static_cast<int>(SeekOnLoadMode::MainCue)));
    TrackPointer pTrack = createTestTrack();
    pTrack->setMainCuePosition(mixxx::audio::FramePos(100.0));

    // The Track should not follow cue changes due to the analyzer if the
    // track has been manual seeked before.
    loadTrack(pTrack);

    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::FramePos(100.0), m_pCuePoint);
    EXPECT_FRAMEPOS_EQ(mixxx::audio::FramePos(100.0), getCurrentFramePos());

    // Manually seek  the track
    setCurrentFramePos(mixxx::audio::FramePos(200.0));

    // Move cue like silence analysis does and check if track is following it
    pTrack->setMainCuePosition(mixxx::audio::FramePos(400.0));
    pTrack->analysisFinished();
    ProcessBuffer();

    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::FramePos(400.0), m_pCuePoint);
    EXPECT_FRAMEPOS_EQ(mixxx::audio::FramePos(200.0), getCurrentFramePos());
}

TEST_F(CueControlTest, SeekOnLoadDefault_CueInPreroll) {
    config()->set(ConfigKey("[Controls]", "CueRecall"),
            ConfigValue(static_cast<int>(SeekOnLoadMode::MainCue)));
    TrackPointer pTrack = createTestTrack();
    pTrack->setMainCuePosition(mixxx::audio::FramePos(-100.0));

    loadTrack(pTrack);

    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::FramePos(-100.0), m_pCuePoint);
    EXPECT_FRAMEPOS_EQ(mixxx::audio::FramePos(-100.0), getCurrentFramePos());

    // Move cue like silence analysis does and check if track is following it
    pTrack->setMainCuePosition(mixxx::audio::FramePos(-200.0));
    pTrack->analysisFinished();
    ProcessBuffer();

    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::FramePos(-200.0), m_pCuePoint);
    EXPECT_FRAMEPOS_EQ(mixxx::audio::FramePos(-200.0), getCurrentFramePos());
}

TEST_F(CueControlTest, FollowCueOnQuantize) {
    m_pQuantizeEnabled->set(0);
    config()->set(ConfigKey("[Controls]", "CueRecall"),
            ConfigValue(static_cast<int>(SeekOnLoadMode::MainCue)));
    TrackPointer pTrack = createTestTrack();
    pTrack->trySetBpm(120.0);

    const mixxx::audio::SampleRate sampleRate = pTrack->getSampleRate();
    const double bpm = pTrack->getBpm();
    const mixxx::audio::FrameDiff_t beatLengthFrames = (60.0 * sampleRate / bpm);
    const auto cuePos = mixxx::audio::FramePos(1.8 * beatLengthFrames);
    const auto quantizedCuePos = mixxx::audio::FramePos(2.0 * beatLengthFrames);
    pTrack->setMainCuePosition(cuePos);

    loadTrack(pTrack);

    EXPECT_FRAMEPOS_EQ_CONTROL(cuePos, m_pCuePoint);
    EXPECT_FRAMEPOS_EQ(cuePos, getCurrentFramePos());

    // enable quantization and expect current position to follow
    m_pQuantizeEnabled->set(1);
    ProcessBuffer();
    EXPECT_FRAMEPOS_EQ_CONTROL(quantizedCuePos, m_pCuePoint);
    EXPECT_FRAMEPOS_EQ(quantizedCuePos, getCurrentFramePos());

    // move current position to track start
    m_pQuantizeEnabled->set(0);
    ProcessBuffer();
    setCurrentFramePos(mixxx::audio::kStartFramePos);
    ProcessBuffer();
    EXPECT_FRAMEPOS_EQ(mixxx::audio::kStartFramePos, getCurrentFramePos());

    // enable quantization again and expect play position to stay at track start
    m_pQuantizeEnabled->set(1);
    ProcessBuffer();
    EXPECT_FRAMEPOS_EQ_CONTROL(quantizedCuePos, m_pCuePoint);
    EXPECT_FRAMEPOS_EQ(mixxx::audio::kStartFramePos, getCurrentFramePos());
}

TEST_F(CueControlTest, SeekOnSetCueCDJ) {
    // Regression test for https://github.com/mixxxdj/mixxx/issues/10551
    config()->set(ConfigKey("[Controls]", "CueRecall"),
            ConfigValue(static_cast<int>(SeekOnLoadMode::MainCue)));
    m_pQuantizeEnabled->set(1);
    TrackPointer pTrack = createTestTrack();
    pTrack->trySetBpm(120.0);

    const mixxx::audio::SampleRate sampleRate = pTrack->getSampleRate();
    const double bpm = pTrack->getBpm();
    const mixxx::audio::FrameDiff_t beatLengthFrames = (60.0 * sampleRate / bpm);
    const auto cuePos = mixxx::audio::FramePos(10 * beatLengthFrames);
    pTrack->setMainCuePosition(cuePos);

    loadTrack(pTrack);

    EXPECT_FRAMEPOS_EQ_CONTROL(cuePos, m_pCuePoint);
    EXPECT_FRAMEPOS_EQ(cuePos, getCurrentFramePos());

    // Change the playpos and move the cue point there.
    const auto newCuePos = mixxx::audio::FramePos(2.0 * beatLengthFrames);
    setCurrentFramePos(newCuePos);
    m_pChannel1->getEngineBuffer()->m_pCueControl->cueCDJ(1.0);
    ProcessBuffer();

    // Cue point and playpos should be at the same position.
    EXPECT_FRAMEPOS_EQ_CONTROL(newCuePos, m_pCuePoint);
    EXPECT_FRAMEPOS_EQ(newCuePos, getCurrentFramePos());
}

TEST_F(CueControlTest, SeekOnSetCuePlay) {
    // Regression test for https://github.com/mixxxdj/mixxx/issues/10551
    config()->set(ConfigKey("[Controls]", "CueRecall"),
            ConfigValue(static_cast<int>(SeekOnLoadMode::MainCue)));
    m_pQuantizeEnabled->set(1);
    TrackPointer pTrack = createTestTrack();
    pTrack->trySetBpm(120.0);

    const mixxx::audio::SampleRate sampleRate = pTrack->getSampleRate();
    const double bpm = pTrack->getBpm();
    const mixxx::audio::FrameDiff_t beatLengthFrames = (60.0 * sampleRate / bpm);
    const auto cuePos = mixxx::audio::FramePos(10 * beatLengthFrames);
    pTrack->setMainCuePosition(cuePos);

    loadTrack(pTrack);

    EXPECT_FRAMEPOS_EQ_CONTROL(cuePos, m_pCuePoint);
    EXPECT_FRAMEPOS_EQ(cuePos, getCurrentFramePos());

    // Change the playpos and do a cuePlay.
    const auto newCuePos = mixxx::audio::FramePos(2.0 * beatLengthFrames);
    setCurrentFramePos(newCuePos);
    m_pChannel1->getEngineBuffer()->m_pCueControl->cuePlay(1.0);
    ProcessBuffer();

    // Cue point and playpos should be at the same position.
    EXPECT_FRAMEPOS_EQ_CONTROL(newCuePos, m_pCuePoint);
    EXPECT_FRAMEPOS_EQ(newCuePos, getCurrentFramePos());
}

TEST_F(CueControlTest, IntroCue_SetStartEnd_ClearStartEnd) {
    TrackPointer pTrack = createAndLoadFakeTrack();

    // Set intro start cue
    setCurrentFramePos(mixxx::audio::FramePos(100.0));
    m_pIntroStartSet->set(1);
    m_pIntroStartSet->set(0);
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::FramePos(100.0), m_pIntroStartPosition);
    EXPECT_TRUE(m_pIntroStartEnabled->toBool());
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::kInvalidFramePos, m_pIntroEndPosition);
    EXPECT_FALSE(m_pIntroEndEnabled->toBool());

    CuePointer pCue = pTrack->findCueByType(mixxx::CueType::Intro);
    EXPECT_NE(nullptr, pCue);
    if (pCue != nullptr) {
        EXPECT_FRAMEPOS_EQ(mixxx::audio::FramePos(100.0), pCue->getPosition());
        EXPECT_DOUBLE_EQ(0.0, pCue->getLengthFrames());
    }

    // Set intro end cue
    setCurrentFramePos(mixxx::audio::FramePos(500.0));
    m_pIntroEndSet->set(1);
    m_pIntroEndSet->set(0);
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::FramePos(100.0), m_pIntroStartPosition);
    EXPECT_TRUE(m_pIntroStartEnabled->toBool());
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::FramePos(500.0), m_pIntroEndPosition);
    EXPECT_TRUE(m_pIntroEndEnabled->toBool());

    pCue = pTrack->findCueByType(mixxx::CueType::Intro);
    EXPECT_NE(nullptr, pCue);
    if (pCue != nullptr) {
        EXPECT_FRAMEPOS_EQ(mixxx::audio::FramePos(100.0), pCue->getPosition());
        EXPECT_DOUBLE_EQ(400.0, pCue->getLengthFrames());
    }

    // Clear intro start cue
    m_pIntroStartClear->set(1);
    m_pIntroStartClear->set(0);
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::kInvalidFramePos, m_pIntroStartPosition);
    EXPECT_FALSE(m_pIntroStartEnabled->toBool());
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::FramePos(500.0), m_pIntroEndPosition);
    EXPECT_TRUE(m_pIntroEndEnabled->toBool());

    pCue = pTrack->findCueByType(mixxx::CueType::Intro);
    EXPECT_NE(nullptr, pCue);
    if (pCue != nullptr) {
        EXPECT_FRAMEPOS_EQ(mixxx::audio::kInvalidFramePos, pCue->getPosition());
        EXPECT_DOUBLE_EQ(500.0, pCue->getLengthFrames());
    }

    // Clear intro end cue
    m_pIntroEndClear->set(1);
    m_pIntroEndClear->set(0);
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::kInvalidFramePos, m_pIntroStartPosition);
    EXPECT_FALSE(m_pIntroStartEnabled->toBool());
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::kInvalidFramePos, m_pIntroEndPosition);
    EXPECT_FALSE(m_pIntroEndEnabled->toBool());

    EXPECT_EQ(nullptr, pTrack->findCueByType(mixxx::CueType::Intro));
}

TEST_F(CueControlTest, OutroCue_SetStartEnd_ClearStartEnd) {
    TrackPointer pTrack = createAndLoadFakeTrack();

    // Set outro start cue
    setCurrentFramePos(mixxx::audio::FramePos(750.0));
    m_pOutroStartSet->set(1);
    m_pOutroStartSet->set(0);
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::FramePos(750.0), m_pOutroStartPosition);
    EXPECT_TRUE(m_pOutroStartEnabled->toBool());
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::kInvalidFramePos, m_pOutroEndPosition);
    EXPECT_FALSE(m_pOutroEndEnabled->toBool());

    CuePointer pCue = pTrack->findCueByType(mixxx::CueType::Outro);
    EXPECT_NE(nullptr, pCue);
    if (pCue != nullptr) {
        EXPECT_FRAMEPOS_EQ(mixxx::audio::FramePos(750.0), pCue->getPosition());
        EXPECT_DOUBLE_EQ(0.0, pCue->getLengthFrames());
    }

    // Set outro end cue
    setCurrentFramePos(mixxx::audio::FramePos(1000.0));
    m_pOutroEndSet->set(1);
    m_pOutroEndSet->set(0);
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::FramePos(750.0), m_pOutroStartPosition);
    EXPECT_TRUE(m_pOutroStartEnabled->toBool());
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::FramePos(1000.0), m_pOutroEndPosition);
    EXPECT_TRUE(m_pOutroEndEnabled->toBool());

    pCue = pTrack->findCueByType(mixxx::CueType::Outro);
    EXPECT_NE(nullptr, pCue);
    if (pCue != nullptr) {
        EXPECT_FRAMEPOS_EQ(mixxx::audio::FramePos(750.0), pCue->getPosition());
        EXPECT_DOUBLE_EQ(250.0, pCue->getLengthFrames());
    }

    // Clear outro start cue
    m_pOutroStartClear->set(1);
    m_pOutroStartClear->set(0);
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::kInvalidFramePos, m_pOutroStartPosition);
    EXPECT_FALSE(m_pOutroStartEnabled->toBool());
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::FramePos(1000.0), m_pOutroEndPosition);
    EXPECT_TRUE(m_pOutroEndEnabled->toBool());

    pCue = pTrack->findCueByType(mixxx::CueType::Outro);
    EXPECT_NE(nullptr, pCue);
    if (pCue != nullptr) {
        EXPECT_FRAMEPOS_EQ(mixxx::audio::kInvalidFramePos, pCue->getPosition());
        EXPECT_DOUBLE_EQ(1000.0, pCue->getLengthFrames());
    }

    // Clear outro end cue
    m_pOutroEndClear->set(1);
    m_pOutroEndClear->set(0);
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::kInvalidFramePos, m_pOutroStartPosition);
    EXPECT_FALSE(m_pOutroStartEnabled->toBool());
    EXPECT_FRAMEPOS_EQ_CONTROL(mixxx::audio::kInvalidFramePos, m_pOutroEndPosition);
    EXPECT_FALSE(m_pOutroEndEnabled->toBool());

    EXPECT_EQ(nullptr, pTrack->findCueByType(mixxx::CueType::Outro));
}
