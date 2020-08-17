#include "macros_test.h"
#include "test/signalpathtest.h"

class MacroPlaybackTest : public BaseSignalPathTest {
};

TEST_F(MacroPlaybackTest, Playback) {
    MacroAction action(0, 2000);
    QVector<MacroAction> actions{action};
    Macro macro(actions, "Test1", Macro::StateFlag::Enabled);

    TrackPointer pTrack = getTestTrack();
    pTrack->setMacros({{1, macro}});

    loadTrack(m_pMixerDeck1, pTrack);
    EngineBuffer* pEngineBuffer = m_pMixerDeck1->getEngineDeck()->getEngineBuffer();
    EXPECT_EQ(pEngineBuffer->getExactPlayPos() / mixxx::kEngineChannelCount, action.position);

    ProcessBuffer();
    // We have to do a second call: First one only queues the seek
    ProcessBuffer();
    EXPECT_EQ(pEngineBuffer->getExactPlayPos() / mixxx::kEngineChannelCount, action.target);
}
