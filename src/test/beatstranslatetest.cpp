#include "test/mockedenginebackendtest.h"
#include "track/beatgrid.h"
#include "util/memory.h"

class BeatsTranslateTest : public MockedEngineBackendTest {
};

TEST_F(BeatsTranslateTest, SimpleTranslateMatch) {
    // Set up BeatGrids for decks 1 and 2.
    const auto bpm = mixxx::Bpm(60.0);
    constexpr auto firstBeat = mixxx::audio::kStartFramePos;
    auto grid1 = mixxx::BeatGrid::makeBeatGrid(
            m_pTrack1->getSampleRate(), QString(), bpm, firstBeat);
    m_pTrack1->trySetBeats(grid1);
    ASSERT_DOUBLE_EQ(firstBeat.value(),
            grid1->findClosestBeat(mixxx::audio::kStartFramePos).value());

    auto grid2 = mixxx::BeatGrid::makeBeatGrid(
            m_pTrack2->getSampleRate(), QString(), bpm, firstBeat);
    m_pTrack2->trySetBeats(grid2);
    ASSERT_DOUBLE_EQ(firstBeat.value(),
            grid2->findClosestBeat(mixxx::audio::kStartFramePos).value());

    // Seek deck 1 forward a bit.
    const double deltaFrames = 1111.0;
    m_pChannel1->getEngineBuffer()->slotControlSeekAbs(deltaFrames * 2.0);
    ProcessBuffer();
    EXPECT_TRUE(m_pChannel1->getEngineBuffer()->getVisualPlayPos() > 0);

    // Make both decks playing.
    ControlObject::getControl(m_sGroup1, "play")->set(1.0);
    ControlObject::getControl(m_sGroup2, "play")->set(1.0);
    ProcessBuffer();
    // Manually set the "bpm" control... I would like to figure out why this
    // doesn't get set naturally, but this will do for now.
    auto pBpm1 = std::make_unique<ControlProxy>(m_sGroup1, "bpm");
    auto pBpm2 = std::make_unique<ControlProxy>(m_sGroup1, "bpm");
    pBpm1->set(bpm.value());
    pBpm2->set(bpm.value());
    ProcessBuffer();

    // Push the button on deck 2.
    auto pTranslateMatchAlignment = std::make_unique<ControlProxy>(
        m_sGroup2, "beats_translate_match_alignment");
    pTranslateMatchAlignment->set(1.0);
    ProcessBuffer();

    // Deck 1 is +deltaFrames away from its closest beat (which is at 0).
    // Deck 2 was left at 0. We translated grid 2 so that it is also +deltaFrames
    // away from its closest beat, so that beat should be at -deltaFrames.
    mixxx::BeatsPointer beats = m_pTrack2->getBeats();
    ASSERT_DOUBLE_EQ(-deltaFrames, beats->findClosestBeat(mixxx::audio::kStartFramePos).value());
}
