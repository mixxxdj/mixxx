#include "test/mockedenginebackendtest.h"
#include "track/beats.h"
#include "util/memory.h"

using namespace mixxx;

class BeatsTranslateTest : public MockedEngineBackendTest {
};

TEST_F(BeatsTranslateTest, SimpleTranslateMatch) {
    const mixxx::Bpm bpm = Bpm(60.0);
    const FramePos firstBeat(0.0);

    // Set up Beats for decks 1 and 2.
    m_pTrack1->setBeats(mixxx::BeatsInternal());
    m_pTrack1->getBeats()->setGrid(bpm, firstBeat);
    ASSERT_DOUBLE_EQ(firstBeat.getValue(),
            m_pTrack1->getBeats()->findClosestBeat(firstBeat).getValue());
    m_pTrack2->setBeats(mixxx::BeatsInternal());
    m_pTrack2->getBeats()->setGrid(bpm, firstBeat);
    ASSERT_DOUBLE_EQ(firstBeat.getValue(),
            m_pTrack2->getBeats()->findClosestBeat(firstBeat).getValue());

    const FrameDiff_t deltaFrames = 1111.0;
    // Seek deck 1 forward a bit
    m_pChannel1->getEngineBuffer()->slotControlSeekAbs(deltaFrames * 2);
    ProcessBuffer();
    EXPECT_TRUE(m_pChannel1->getEngineBuffer()->getVisualPlayPos() > 0);
    ASSERT_DOUBLE_EQ(m_pChannel1->getEngineBuffer()->getExactPlayPos(), deltaFrames * 2);

    // Make both decks playing.
    ControlObject::getControl(m_sGroup1, "play")->set(1.0);
    ControlObject::getControl(m_sGroup2, "play")->set(1.0);
    ProcessBuffer();
    ASSERT_DOUBLE_EQ(m_pChannel1->getEngineBuffer()->getExactPlayPos(),
            m_pChannel2->getEngineBuffer()->getExactPlayPos() +
                    deltaFrames * 2);

    // TODO(XXX) Manually set the "bpm" control... I would like to figure out
    // why this doesn't get set naturally, but this will do for now.
    auto pBpm1 = std::make_unique<ControlProxy>(m_sGroup1, "bpm");
    auto pBpm2 = std::make_unique<ControlProxy>(m_sGroup1, "bpm");
    pBpm1->set(bpm.getValue());
    pBpm2->set(bpm.getValue());
    ProcessBuffer();

    // Push the button on deck 2.
    auto pTranslateMatchAlignment = std::make_unique<ControlProxy>(
        m_sGroup2, "beats_translate_match_alignment");
    pTranslateMatchAlignment->set(1.0);
    ProcessBuffer();

    // Deck 1 is +delta away from its closest beat (which is at 0).
    // Deck 2 was left at 0.
    // We translated grid 2 so that it is also +delta away from its closest beat
    // So that beat should be at deck 1 position -delta.
    ASSERT_DOUBLE_EQ(-deltaFrames,
            m_pTrack2->getBeats()->findClosestBeat(kStartFramePos).getValue());
}
