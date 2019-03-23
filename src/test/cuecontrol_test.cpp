#include "test/signalpathtest.h"

class CueControlTest : public BaseSignalPathTest {
  protected:
    void SetUp() override {
        BaseSignalPathTest::SetUp();

        m_pCuePoint = std::make_unique<ControlProxy>(m_sGroup1, "cue_point");
        m_pIntroStartPosition = std::make_unique<ControlProxy>(m_sGroup1, "intro_start_position");
        m_pIntroStartEnabled = std::make_unique<ControlProxy>(m_sGroup1, "intro_start_enabled");
        m_pIntroEndPosition = std::make_unique<ControlProxy>(m_sGroup1, "intro_end_position");
        m_pIntroEndEnabled = std::make_unique<ControlProxy>(m_sGroup1, "intro_end_enabled");
        m_pOutroStartPosition = std::make_unique<ControlProxy>(m_sGroup1, "outro_start_position");
        m_pOutroStartEnabled = std::make_unique<ControlProxy>(m_sGroup1, "outro_start_enabled");
        m_pOutroEndPosition = std::make_unique<ControlProxy>(m_sGroup1, "outro_end_position");
        m_pOutroEndEnabled = std::make_unique<ControlProxy>(m_sGroup1, "outro_end_enabled");
    }

    TrackPointer createTestTrack() const {
        const QString kTrackLocationTest = QDir::currentPath() + "/src/test/sine-30.wav";
        return std::make_unique<Track>(kTrackLocationTest, SecurityTokenPointer());
    }

    void loadTrack(TrackPointer pTrack) {
        BaseSignalPathTest::loadTrack(m_pMixerDeck1, pTrack);
    }

    void unloadTrack() {
        m_pMixerDeck1->slotLoadTrack(TrackPointer(), false);
    }

    std::unique_ptr<ControlProxy> m_pCuePoint;
    std::unique_ptr<ControlProxy> m_pIntroStartPosition;
    std::unique_ptr<ControlProxy> m_pIntroStartEnabled;
    std::unique_ptr<ControlProxy> m_pIntroEndPosition;
    std::unique_ptr<ControlProxy> m_pIntroEndEnabled;
    std::unique_ptr<ControlProxy> m_pOutroStartPosition;
    std::unique_ptr<ControlProxy> m_pOutroStartEnabled;
    std::unique_ptr<ControlProxy> m_pOutroEndPosition;
    std::unique_ptr<ControlProxy> m_pOutroEndEnabled;
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
