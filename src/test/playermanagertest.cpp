#include <gtest/gtest.h>

#include <QTest>

#include "engine/enginebuffer.h"
#include "engine/enginemaster.h"
#include "mixer/basetrackplayer.h"
#include "mixer/deck.h"
#include "mixer/playermanager.h"
#include "test/mixxxtest.h"
#include "track/track.h"
#include "util/cmdlineargs.h"

class PlayerManagerTest : public MixxxTest {
    void SetUp() override {
        auto pChannelHandleFactory = std::make_shared<ChannelHandleFactory>();
        m_pEffectsManager = std::make_shared<EffectsManager>(m_pConfig, pChannelHandleFactory);
        m_pEngine = std::make_shared<EngineMaster>(
                m_pConfig,
                "[Master]",
                m_pEffectsManager.get(),
                pChannelHandleFactory,
                true);
        m_pSoundManager = std::make_shared<SoundManager>(m_pConfig, m_pEngine.get());
        m_pEngine->registerNonEngineChannelSoundIO(m_pSoundManager.get());
        m_pPlayerManager = std::make_shared<PlayerManager>(m_pConfig,
                m_pSoundManager.get(),
                m_pEffectsManager.get(),
                m_pEngine.get());

        m_pPlayerManager->addConfiguredDecks();
        m_pPlayerManager->addSampler();
    }

  protected:
    std::shared_ptr<EffectsManager> m_pEffectsManager;
    std::shared_ptr<EngineMaster> m_pEngine;
    std::shared_ptr<SoundManager> m_pSoundManager;
    std::shared_ptr<PlayerManager> m_pPlayerManager;
};

TEST_F(PlayerManagerTest, UnEjectTest) {
    // Ejecting an empty deck with no previously-recorded ejected track has no effect.
    auto deck1 = m_pPlayerManager->getDeck(1);
    deck1->slotEjectTrack(1.0);
    ASSERT_TRUE(deck1->getLoadedTrack() == nullptr);

    // Load a track and eject it
    deck1->loadFakeTrack(false, 130.0);
    // Wait for the track to load.
    m_pEngine->process(1024);
    while (!deck1->getEngineDeck()->getEngineBuffer()->isTrackLoaded()) {
        QTest::qSleep(1); // millis
    }
    deck1->slotEjectTrack(1.0);
    // Load another track with a different bpm.
    deck1->loadFakeTrack(false, 100.0);

    // Ejecting in an empty deck loads the last-ejected track.
    auto deck2 = m_pPlayerManager->getDeck(2);
    ASSERT_TRUE(deck2->getLoadedTrack() == nullptr);
    deck2->slotEjectTrack(2.0);
    ASSERT_TRUE(deck2->getLoadedTrack() != nullptr);
    ASSERT_EQ(130, deck2->getLoadedTrack()->getBpm());
}
