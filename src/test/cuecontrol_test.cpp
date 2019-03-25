#include "test/signalpathtest.h"

class CueControlTest : public BaseSignalPathTest {
  protected:
    void SetUp() override {
        BaseSignalPathTest::SetUp();

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
        return std::make_unique<Track>(kTrackLocationTest, SecurityTokenPointer());
    }

    void loadTrack(TrackPointer pTrack) {
        BaseSignalPathTest::loadTrack(m_pMixerDeck1, pTrack);
    }

    TrackPointer createAndLoadFakeTrack() {
        return m_pMixerDeck1->loadFakeTrack(false, 0.0);
    }

    void unloadTrack() {
        m_pMixerDeck1->slotLoadTrack(TrackPointer(), false);
    }

    void setCurrentSample(double sample) {
        m_pChannel1->getEngineBuffer()->queueNewPlaypos(sample, EngineBuffer::SEEK_STANDARD);
        ProcessBuffer();
    }

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
    pTrack->setCuePoint(CuePosition(100.0, Cue::MANUAL));
    auto pIntro = pTrack->createAndAddCue();
    pIntro->setType(Cue::INTRO);
    pIntro->setSource(Cue::MANUAL);
    pIntro->setPosition(150.0);
    pIntro->setLength(50.0);
    auto pOutro = pTrack->createAndAddCue();
    pOutro->setType(Cue::OUTRO);
    pOutro->setSource(Cue::MANUAL);
    pOutro->setPosition(250.0);
    pOutro->setLength(50.0);

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

    EXPECT_DOUBLE_EQ(-1.0, m_pCuePoint->get());
    EXPECT_DOUBLE_EQ(-1.0, m_pIntroStartPosition->get());
    EXPECT_DOUBLE_EQ(-1.0, m_pIntroEndPosition->get());
    EXPECT_DOUBLE_EQ(-1.0, m_pOutroStartPosition->get());
    EXPECT_DOUBLE_EQ(-1.0, m_pOutroEndPosition->get());
    EXPECT_FALSE(m_pIntroStartEnabled->toBool());
    EXPECT_FALSE(m_pIntroEndEnabled->toBool());
    EXPECT_FALSE(m_pOutroStartEnabled->toBool());
    EXPECT_FALSE(m_pOutroEndEnabled->toBool());
}

TEST_F(CueControlTest, LoadTrackWithDetectedCues) {
    TrackPointer pTrack = createTestTrack();
    pTrack->setCuePoint(CuePosition(100.0, Cue::AUTOMATIC));
    auto pIntro = pTrack->createAndAddCue();
    pIntro->setType(Cue::INTRO);
    pIntro->setSource(Cue::AUTOMATIC);
    pIntro->setPosition(100.0);
    pIntro->setLength(0.0);
    auto pOutro = pTrack->createAndAddCue();
    pOutro->setType(Cue::OUTRO);
    pOutro->setSource(Cue::AUTOMATIC);
    pOutro->setPosition(-1.0);
    pOutro->setLength(200.0);

    loadTrack(pTrack);

    EXPECT_DOUBLE_EQ(100.0, m_pCuePoint->get());
    EXPECT_DOUBLE_EQ(100.0, m_pIntroStartPosition->get());
    EXPECT_DOUBLE_EQ(-1.0, m_pIntroEndPosition->get());
    EXPECT_DOUBLE_EQ(-1.0, m_pOutroStartPosition->get());
    EXPECT_DOUBLE_EQ(200.0, m_pOutroEndPosition->get());
    EXPECT_TRUE(m_pIntroStartEnabled->toBool());
    EXPECT_FALSE(m_pIntroEndEnabled->toBool());
    EXPECT_FALSE(m_pOutroStartEnabled->toBool());
    EXPECT_TRUE(m_pOutroEndEnabled->toBool());
}

TEST_F(CueControlTest, LoadTrackWithIntroEndAndOutroStart) {
    TrackPointer pTrack = createTestTrack();
    auto pIntro = pTrack->createAndAddCue();
    pIntro->setType(Cue::INTRO);
    pIntro->setSource(Cue::MANUAL);
    pIntro->setPosition(-1.0);
    pIntro->setLength(150.0);
    auto pOutro = pTrack->createAndAddCue();
    pOutro->setType(Cue::OUTRO);
    pOutro->setSource(Cue::MANUAL);
    pOutro->setPosition(250.0);
    pOutro->setLength(0.0);

    loadTrack(pTrack);

    EXPECT_DOUBLE_EQ(-1.0, m_pCuePoint->get());
    EXPECT_DOUBLE_EQ(-1.0, m_pIntroStartPosition->get());
    EXPECT_DOUBLE_EQ(150.0, m_pIntroEndPosition->get());
    EXPECT_DOUBLE_EQ(250.0, m_pOutroStartPosition->get());
    EXPECT_DOUBLE_EQ(-1.0, m_pOutroEndPosition->get());
    EXPECT_FALSE(m_pIntroStartEnabled->toBool());
    EXPECT_TRUE(m_pIntroEndEnabled->toBool());
    EXPECT_TRUE(m_pOutroStartEnabled->toBool());
    EXPECT_FALSE(m_pOutroEndEnabled->toBool());
}

TEST_F(CueControlTest, IntroCue_SetStartEnd_ClearStartEnd) {
    TrackPointer pTrack = createAndLoadFakeTrack();

    // Set intro start cue
    setCurrentSample(100.0);
    m_pIntroStartSet->slotSet(1);
    m_pIntroStartSet->slotSet(0);
    EXPECT_DOUBLE_EQ(100.0, m_pIntroStartPosition->get());
    EXPECT_TRUE(m_pIntroStartEnabled->toBool());
    EXPECT_DOUBLE_EQ(-1.0, m_pIntroEndPosition->get());
    EXPECT_FALSE(m_pIntroEndEnabled->toBool());

    CuePointer pCue = pTrack->findCueByType(Cue::INTRO);
    EXPECT_NE(nullptr, pCue);
    if (pCue != nullptr) {
        EXPECT_DOUBLE_EQ(100.0, pCue->getPosition());
        EXPECT_DOUBLE_EQ(0.0, pCue->getLength());
        EXPECT_DOUBLE_EQ(Cue::MANUAL, pCue->getSource());
    }

    // Set intro end cue
    setCurrentSample(500.0);
    m_pIntroEndSet->slotSet(1);
    m_pIntroEndSet->slotSet(0);
    EXPECT_DOUBLE_EQ(100.0, m_pIntroStartPosition->get());
    EXPECT_TRUE(m_pIntroStartEnabled->toBool());
    EXPECT_DOUBLE_EQ(500.0, m_pIntroEndPosition->get());
    EXPECT_TRUE(m_pIntroEndEnabled->toBool());

    pCue = pTrack->findCueByType(Cue::INTRO);
    EXPECT_NE(nullptr, pCue);
    if (pCue != nullptr) {
        EXPECT_DOUBLE_EQ(100.0, pCue->getPosition());
        EXPECT_DOUBLE_EQ(400.0, pCue->getLength());
        EXPECT_DOUBLE_EQ(Cue::MANUAL, pCue->getSource());
    }

    // Clear intro start cue
    m_pIntroStartClear->slotSet(1);
    m_pIntroStartClear->slotSet(0);
    EXPECT_DOUBLE_EQ(-1.0, m_pIntroStartPosition->get());
    EXPECT_FALSE(m_pIntroStartEnabled->toBool());
    EXPECT_DOUBLE_EQ(500.0, m_pIntroEndPosition->get());
    EXPECT_TRUE(m_pIntroEndEnabled->toBool());

    pCue = pTrack->findCueByType(Cue::INTRO);
    EXPECT_NE(nullptr, pCue);
    if (pCue != nullptr) {
        EXPECT_DOUBLE_EQ(-1.0, pCue->getPosition());
        EXPECT_DOUBLE_EQ(500.0, pCue->getLength());
        EXPECT_DOUBLE_EQ(Cue::MANUAL, pCue->getSource());
    }

    // Clear intro end cue
    m_pIntroEndClear->slotSet(1);
    m_pIntroEndClear->slotSet(0);
    EXPECT_DOUBLE_EQ(-1.0, m_pIntroStartPosition->get());
    EXPECT_FALSE(m_pIntroStartEnabled->toBool());
    EXPECT_DOUBLE_EQ(-1.0, m_pIntroEndPosition->get());
    EXPECT_FALSE(m_pIntroEndEnabled->toBool());

    EXPECT_EQ(nullptr, pTrack->findCueByType(Cue::INTRO));
}

TEST_F(CueControlTest, OutroCue_SetStartEnd_ClearStartEnd) {
    TrackPointer pTrack = createAndLoadFakeTrack();

    // Set outro start cue
    setCurrentSample(750.0);
    m_pOutroStartSet->slotSet(1);
    m_pOutroStartSet->slotSet(0);
    EXPECT_DOUBLE_EQ(750.0, m_pOutroStartPosition->get());
    EXPECT_TRUE(m_pOutroStartEnabled->toBool());
    EXPECT_DOUBLE_EQ(-1.0, m_pOutroEndPosition->get());
    EXPECT_FALSE(m_pOutroEndEnabled->toBool());

    CuePointer pCue = pTrack->findCueByType(Cue::OUTRO);
    EXPECT_NE(nullptr, pCue);
    if (pCue != nullptr) {
        EXPECT_DOUBLE_EQ(750.0, pCue->getPosition());
        EXPECT_DOUBLE_EQ(0.0, pCue->getLength());
        EXPECT_DOUBLE_EQ(Cue::MANUAL, pCue->getSource());
    }

    // Set outro end cue
    setCurrentSample(1000.0);
    m_pOutroEndSet->slotSet(1);
    m_pOutroEndSet->slotSet(0);
    EXPECT_DOUBLE_EQ(750.0, m_pOutroStartPosition->get());
    EXPECT_TRUE(m_pOutroStartEnabled->toBool());
    EXPECT_DOUBLE_EQ(1000.0, m_pOutroEndPosition->get());
    EXPECT_TRUE(m_pOutroEndEnabled->toBool());

    pCue = pTrack->findCueByType(Cue::OUTRO);
    EXPECT_NE(nullptr, pCue);
    if (pCue != nullptr) {
        EXPECT_DOUBLE_EQ(750.0, pCue->getPosition());
        EXPECT_DOUBLE_EQ(250.0, pCue->getLength());
        EXPECT_DOUBLE_EQ(Cue::MANUAL, pCue->getSource());
    }

    // Clear outro start cue
    m_pOutroStartClear->slotSet(1);
    m_pOutroStartClear->slotSet(0);
    EXPECT_DOUBLE_EQ(-1.0, m_pOutroStartPosition->get());
    EXPECT_FALSE(m_pOutroStartEnabled->toBool());
    EXPECT_DOUBLE_EQ(1000.0, m_pOutroEndPosition->get());
    EXPECT_TRUE(m_pOutroEndEnabled->toBool());

    pCue = pTrack->findCueByType(Cue::OUTRO);
    EXPECT_NE(nullptr, pCue);
    if (pCue != nullptr) {
        EXPECT_DOUBLE_EQ(-1.0, pCue->getPosition());
        EXPECT_DOUBLE_EQ(1000.0, pCue->getLength());
        EXPECT_DOUBLE_EQ(Cue::MANUAL, pCue->getSource());
    }

    // Clear outro end cue
    m_pOutroEndClear->slotSet(1);
    m_pOutroEndClear->slotSet(0);
    EXPECT_DOUBLE_EQ(-1.0, m_pOutroStartPosition->get());
    EXPECT_FALSE(m_pOutroStartEnabled->toBool());
    EXPECT_DOUBLE_EQ(-1.0, m_pOutroEndPosition->get());
    EXPECT_FALSE(m_pOutroEndEnabled->toBool());

    EXPECT_EQ(nullptr, pTrack->findCueByType(Cue::OUTRO));
}
