#include "test/mockedenginebackendtest.h"
#include "track/beatgrid.h"

class BeatsTranslateTest : public MockedEngineBackendTest {
};

TEST_F(BeatsTranslateTest, SimpleTranslateMatch) {

    // Set up BeatGrids for decks 1 and 2.
    const double bpm = 60.0;
    const double firstBeat = 0.0;
    BeatGrid grid1(m_pTrack1.data(), m_pTrack1->getSampleRate());
    grid1.setGrid(bpm, firstBeat);
    m_pTrack1->setBeats(QSharedPointer<Beats>(&grid1));
    ASSERT_DOUBLE_EQ(firstBeat, grid1.findClosestBeat(0));

    BeatGrid grid2(m_pTrack2.data(), m_pTrack2->getSampleRate());
    grid2.setGrid(bpm, firstBeat);
    m_pTrack2->setBeats(QSharedPointer<Beats>(&grid2));
    ASSERT_DOUBLE_EQ(firstBeat, grid2.findClosestBeat(0));

    // Seek deck 1 forward a bit.
    const double delta = 2222.0;
    m_pChannel1->getEngineBuffer()->slotControlSeekAbs(delta);
    ProcessBuffer();
    EXPECT_TRUE(m_pChannel1->getEngineBuffer()->getVisualPlayPos() > 0);

    // Make both decks playing.
    ControlObject::getControl(m_sGroup1, "play", true)->set(1.0);
    ControlObject::getControl(m_sGroup2, "play", true)->set(1.0);
    ProcessBuffer();
    // Manually set the "bpm" control... I would like to figure out why this
    // doesn't get set naturally, but this will do for now.
    QScopedPointer<ControlObjectThread> pBpm1(getControlObjectThread(
            ConfigKey(m_sGroup1, "bpm")));
    QScopedPointer<ControlObjectThread> pBpm2(getControlObjectThread(
            ConfigKey(m_sGroup1, "bpm")));
    pBpm1->set(bpm);
    pBpm2->set(bpm);
    ProcessBuffer();

    // Push the button on deck 2.
    QScopedPointer<ControlObjectThread> (getControlObjectThread(
            ConfigKey(m_sGroup2, "beats_translate_match_alignment")))->set(1.0);
    ProcessBuffer();

    // Deck 1 is +delta away from its closest beat (which is at 0).
    // Deck 2 was left at 0. We translated grid 2 so that it is also +delta
    // away from its closest beat, so that beat should be at -delta.
    ASSERT_DOUBLE_EQ(-delta, grid2.findClosestBeat(0));
}
