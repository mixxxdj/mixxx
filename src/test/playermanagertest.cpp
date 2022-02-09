#include <gtest/gtest.h>

#include <QTest>

#include "database/mixxxdb.h"
#include "engine/enginebuffer.h"
#include "engine/enginemaster.h"
#include "mixer/basetrackplayer.h"
#include "mixer/deck.h"
#include "mixer/playermanager.h"
#include "test/librarytest.h"
#include "track/track.h"
#include "util/cmdlineargs.h"

const QString kTrackLocationTest1(QDir::currentPath() %
        "/src/test/id3-test-data/cover-test-png.mp3");
const QString kTrackLocationTest2(QDir::currentPath() %
        "/src/test/id3-test-data/cover-test-vbr.mp3");

class PlayerManagerTest : public LibraryTest { //, SoundSourceProviderRegistration {
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

        m_pRecordingManager = std::make_shared<RecordingManager>(m_pConfig, m_pEngine.get());
        m_pLibrary = std::make_shared<Library>(
                nullptr,
                m_pConfig,
                dbConnectionPooler(),
                trackCollectionManager(),
                m_pPlayerManager.get(),
                m_pRecordingManager.get());

        m_pPlayerManager->addConfiguredDecks();
        m_pPlayerManager->addSampler();
        m_pPlayerManager->bindToLibrary(m_pLibrary.get());
    }

  protected:
    std::shared_ptr<EffectsManager> m_pEffectsManager;
    std::shared_ptr<EngineMaster> m_pEngine;
    std::shared_ptr<SoundManager> m_pSoundManager;
    std::shared_ptr<PlayerManager> m_pPlayerManager;
    std::shared_ptr<RecordingManager> m_pRecordingManager;
    std::shared_ptr<Library> m_pLibrary;
};

TEST_F(PlayerManagerTest, UnEjectTest) {
    // Ejecting an empty deck with no previously-recorded ejected track has no effect.
    auto deck1 = m_pPlayerManager->getDeck(1);
    deck1->slotEjectTrack(1.0);
    ASSERT_EQ(nullptr, deck1->getLoadedTrack());

    // Load a track and eject it
    TrackPointer pTrack1 = getOrAddTrackByLocation(kTrackLocationTest1);
    TrackId testId1 = pTrack1->getId();
    ASSERT_TRUE(testId1.isValid());
    deck1->slotLoadTrack(pTrack1, false);
    ASSERT_NE(nullptr, deck1->getLoadedTrack());

    m_pEngine->process(1024);
    while (!deck1->getEngineDeck()->getEngineBuffer()->isTrackLoaded()) {
        QTest::qSleep(100); // millis
    }
    deck1->slotEjectTrack(1.0);

    // Load another track.
    TrackPointer pTrack2 = getOrAddTrackByLocation(kTrackLocationTest2);
    deck1->slotLoadTrack(pTrack2, false);

    // Ejecting in an empty deck loads the last-ejected track.
    auto deck2 = m_pPlayerManager->getDeck(2);
    ASSERT_EQ(nullptr, deck2->getLoadedTrack());
    deck2->slotEjectTrack(2.0);
    ASSERT_NE(nullptr, deck2->getLoadedTrack());
    ASSERT_EQ(testId1, deck2->getLoadedTrack()->getId());
}

// Loading a new track in a deck causes the old one to be ejected.
// That old track can be unejected into a different deck.
TEST_F(PlayerManagerTest, UnEjectReplaceTrackTest) {
    auto deck1 = m_pPlayerManager->getDeck(1);
    // Load a track and the load another one
    TrackPointer pTrack1 = getOrAddTrackByLocation(kTrackLocationTest1);
    TrackId testId1 = pTrack1->getId();
    ASSERT_TRUE(testId1.isValid());
    deck1->slotLoadTrack(pTrack1, false);
    ASSERT_NE(nullptr, deck1->getLoadedTrack());

    m_pEngine->process(1024);
    while (!deck1->getEngineDeck()->getEngineBuffer()->isTrackLoaded()) {
        QTest::qSleep(100); // millis
    }

    // Load another track, replacing the first, causing it to be unloaded.
    TrackPointer pTrack2 = getOrAddTrackByLocation(kTrackLocationTest2);
    deck1->slotLoadTrack(pTrack2, false);
    m_pEngine->process(1024);
    while (!deck1->getEngineDeck()->getEngineBuffer()->isTrackLoaded()) {
        QTest::qSleep(100); // millis
    }

    // Ejecting in an empty deck loads the last-ejected track.
    auto deck2 = m_pPlayerManager->getDeck(2);
    ASSERT_EQ(nullptr, deck2->getLoadedTrack());
    deck2->slotEjectTrack(1.0);
    ASSERT_NE(nullptr, deck2->getLoadedTrack());
    ASSERT_EQ(testId1, deck2->getLoadedTrack()->getId());
}

TEST_F(PlayerManagerTest, UnEjectInvalidTrackIdTest) {
    // Save an invalid trackid in playermanager.
    auto pTrack = Track::newDummy(kTrackLocationTest1, TrackId(10));
    m_pPlayerManager->slotSaveEjectedTrack(pTrack);
    auto deck1 = m_pPlayerManager->getDeck(1);
    // Does nothing -- no crash.
    deck1->slotEjectTrack(1.0);
    ASSERT_EQ(nullptr, deck1->getLoadedTrack());
}
