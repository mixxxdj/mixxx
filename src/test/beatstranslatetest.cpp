#include "test/mockedenginebackendtest.h"
#include "track/beats.h"
#include "util/memory.h"

using namespace mixxx;

class BeatsTranslateTest : public MockedEngineBackendTest {
};

TEST_F(BeatsTranslateTest, SimpleTranslateMatch) {
    // Set up Beats for decks 1 and 2.
    const double bpm = 60.0;
    const FrameNum firstBeat = 0.0;
    auto beats1 = new Beats(m_pTrack1.get());
    beats1->setGrid(bpm, firstBeat);
    m_pTrack1->setBeats(BeatsPointer(beats1));
    ASSERT_DOUBLE_EQ(firstBeat, beats1->findClosestBeat(0));

    auto beats2 = new Beats(m_pTrack2.get());
    beats2->setGrid(bpm, firstBeat);
    m_pTrack2->setBeats(BeatsPointer(beats2));
    beats2->printDebugInfo();
    ASSERT_DOUBLE_EQ(firstBeat, beats2->findClosestBeat(0));

    // Seek deck 1 forward a bit.
    const double delta = 2222.0;
    m_pChannel1->getEngineBuffer()->slotControlSeekAbs(delta);
    ProcessBuffer();
    EXPECT_TRUE(m_pChannel1->getEngineBuffer()->getVisualPlayPos() > 0);

    // Make both decks playing.
    ControlObject::getControl(m_sGroup1, "play", true)->set(1.0);
    ControlObject::getControl(m_sGroup2, "play", true)->set(1.0);
    ProcessBuffer();
    // TODO(XXX) Manually set the "bpm" control... I would like to figure out why this
    // doesn't get set naturally, but this will do for now.
    auto pBpm1 = std::make_unique<ControlProxy>(m_sGroup1, "bpm");
    auto pBpm2 = std::make_unique<ControlProxy>(m_sGroup1, "bpm");
    pBpm1->set(bpm);
    pBpm2->set(bpm);
    ProcessBuffer();

    // Push the button on deck 2.
    beats1->printDebugInfo();
    beats2->printDebugInfo();
    auto pTranslateMatchAlignment = std::make_unique<ControlProxy>(
        m_sGroup2, "beats_translate_match_alignment");
    pTranslateMatchAlignment->set(1.0);
    ProcessBuffer();

    // Deck 1 is +delta away from its closest beat (which is at 0).
    // Deck 2 was left at 0. We translated grid 2 so that it is also +delta
    // away from its closest beat, so that beat should be at -delta.
    beats2->printDebugInfo();
    ASSERT_DOUBLE_EQ(-delta, beats2->findClosestBeat(0));
}
