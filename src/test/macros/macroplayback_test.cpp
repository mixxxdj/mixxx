#include "macro_test.h"
#include "test/signalpathtest.h"

class MacroPlaybackTest : public BaseSignalPathTest {
};

TEST_F(MacroPlaybackTest, Playback) {
    MacroAction action(0, 2000);
    QList<MacroAction> actions{MacroAction(0, 0), action};

    TrackPointer pTrack = getTestTrack();
    pTrack->setMacros({{1, std::make_shared<Macro>(actions, "Test1", Macro::StateFlag::Enabled)}});

    loadTrack(m_pMixerDeck1, pTrack);
    EngineBuffer* pEngineBuffer = m_pMixerDeck1->getEngineDeck()->getEngineBuffer();
    EXPECT_EQ(pEngineBuffer->getExactPlayPos() / mixxx::kEngineChannelCount, action.sourceFrame);

    ProcessBuffer();
    // We have to do a second call: First one only queues the seek
    ProcessBuffer();
    EXPECT_EQ(pEngineBuffer->getExactPlayPos() / mixxx::kEngineChannelCount, action.targetFrame);
}
