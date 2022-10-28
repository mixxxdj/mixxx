#include <gtest/gtest.h>

#include <QTest>

#include "control/controlindicatortimer.h"
#include "database/mixxxdb.h"
#include "engine/enginebuffer.h"
#include "engine/enginemaster.h"
#include "mixer/basetrackplayer.h"
#include "mixer/deck.h"
#include "mixer/playerinfo.h"
#include "mixer/playermanager.h"
#include "sources/soundsourceproxy.h"
#include "test/mixxxdbtest.h"
#include "test/soundsourceproviderregistration.h"
#include "track/track.h"
#include "util/cmdlineargs.h"

namespace {

const QString kTrackLocationTest1 = QStringLiteral("id3-test-data/cover-test-png.mp3");
const QString kTrackLocationTest2 = QStringLiteral("id3-test-data/cover-test-vbr.mp3");

void deleteTrack(Track* pTrack) {
    // Delete track objects directly in unit tests with
    // no main event loop
    delete pTrack;
};

} // namespace

// We can't inherit from LibraryTest because that creates a key_notation control object that is also
// created by the Library object itself. The duplicated CO creation causes a debug assert.
class PlayerManagerTest : public MixxxDbTest, SoundSourceProviderRegistration {
  public:
    PlayerManagerTest()
            : MixxxDbTest(true) {
    }

    void SetUp() override {
        // This setup mirrors coreservices -- it would be nice if we could use coreservices instead
        // but it does a lot of local disk / settings setup.
        auto pChannelHandleFactory = std::make_shared<ChannelHandleFactory>();
        m_pEffectsManager = std::make_shared<EffectsManager>(m_pConfig, pChannelHandleFactory);
        m_pEngine = std::make_shared<EngineMaster>(
                m_pConfig,
                "[Master]",
                m_pEffectsManager.get(),
                pChannelHandleFactory,
                true);
        m_pSoundManager = std::make_shared<SoundManager>(m_pConfig, m_pEngine.get());
        m_pControlIndicatorTimer = std::make_shared<mixxx::ControlIndicatorTimer>(nullptr);
        m_pEngine->registerNonEngineChannelSoundIO(m_pSoundManager.get());
        m_pPlayerManager = std::make_shared<PlayerManager>(m_pConfig,
                m_pSoundManager.get(),
                m_pEffectsManager.get(),
                m_pEngine.get());

        m_pPlayerManager->addConfiguredDecks();
        m_pPlayerManager->addSampler();
        PlayerInfo::create();
        m_pEffectsManager->setup();

        const auto dbConnection = mixxx::DbConnectionPooled(dbConnectionPooler());
        if (!MixxxDb::initDatabaseSchema(dbConnection)) {
            exit(1);
        }
        m_pTrackCollectionManager = std::make_unique<TrackCollectionManager>(
                nullptr,
                m_pConfig,
                dbConnectionPooler(),
                deleteTrack);

        m_pRecordingManager = std::make_shared<RecordingManager>(m_pConfig, m_pEngine.get());
        m_pLibrary = std::make_shared<Library>(
                nullptr,
                m_pConfig,
                dbConnectionPooler(),
                m_pTrackCollectionManager.get(),
                m_pPlayerManager.get(),
                m_pRecordingManager.get());

        m_pPlayerManager->bindToLibrary(m_pLibrary.get());
    }

    ~PlayerManagerTest() {
        m_pSoundManager.reset();
        m_pPlayerManager.reset();
        PlayerInfo::destroy();
        m_pLibrary.reset();
        m_pRecordingManager.reset();
        m_pEngine.reset();
        m_pEffectsManager.reset();
        m_pTrackCollectionManager.reset();
        m_pControlIndicatorTimer.reset();
    }

  protected:
    TrackPointer getOrAddTrackByLocation(
            const QString& trackLocation) const {
        return m_pTrackCollectionManager->getOrAddTrack(
                TrackRef::fromFilePath(trackLocation));
    }

    std::shared_ptr<EffectsManager> m_pEffectsManager;
    std::shared_ptr<mixxx::ControlIndicatorTimer> m_pControlIndicatorTimer;
    std::shared_ptr<EngineMaster> m_pEngine;
    std::shared_ptr<SoundManager> m_pSoundManager;
    std::shared_ptr<PlayerManager> m_pPlayerManager;
    std::unique_ptr<TrackCollectionManager> m_pTrackCollectionManager;
    std::shared_ptr<RecordingManager> m_pRecordingManager;
    std::shared_ptr<Library> m_pLibrary;
};

TEST_F(PlayerManagerTest, UnEjectTest) {
    // Ejecting an empty deck with no previously-recorded ejected track has no effect.
    auto deck1 = m_pPlayerManager->getDeck(1);
    deck1->slotEjectTrack(1.0);
    ASSERT_EQ(nullptr, deck1->getLoadedTrack());

    // Load a track and eject it
    TrackPointer pTrack1 = getOrAddTrackByLocation(getTestDir().filePath(kTrackLocationTest1));
    ASSERT_NE(nullptr, pTrack1);
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
    TrackPointer pTrack2 = getOrAddTrackByLocation(getTestDir().filePath(kTrackLocationTest2));
    ASSERT_NE(nullptr, pTrack2);
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
    TrackPointer pTrack1 = getOrAddTrackByLocation(getTestDir().filePath(kTrackLocationTest1));
    ASSERT_NE(nullptr, pTrack1);
    TrackId testId1 = pTrack1->getId();
    ASSERT_TRUE(testId1.isValid());
    deck1->slotLoadTrack(pTrack1, false);
    ASSERT_NE(nullptr, deck1->getLoadedTrack());

    m_pEngine->process(1024);
    while (!deck1->getEngineDeck()->getEngineBuffer()->isTrackLoaded()) {
        QTest::qSleep(100); // millis
    }

    // Load another track, replacing the first, causing it to be unloaded.
    TrackPointer pTrack2 = getOrAddTrackByLocation(getTestDir().filePath(kTrackLocationTest2));
    ASSERT_NE(nullptr, pTrack2);
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
    auto pTrack = Track::newDummy(getTestDir().filePath(kTrackLocationTest1), TrackId(10));
    ASSERT_NE(nullptr, pTrack);
    m_pPlayerManager->slotSaveEjectedTrack(pTrack);
    auto deck1 = m_pPlayerManager->getDeck(1);
    // Does nothing -- no crash.
    deck1->slotEjectTrack(1.0);
    ASSERT_EQ(nullptr, deck1->getLoadedTrack());
}
