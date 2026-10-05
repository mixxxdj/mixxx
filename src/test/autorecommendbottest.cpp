#include <gtest/gtest.h>

#include <QString>

#include "control/controlobject.h"
#include "control/controllinpotmeter.h"
#include "control/controlpotmeter.h"
#include "control/controlpushbutton.h"
#include "dialog/autorecommendbot.h"
#include "engine/channels/enginechannel.h"
#include "library/dao/playlistdao.h"
#include "library/dao/trackschema.h"
#include "mixer/basetrackplayer.h"
#include "mixer/playerinfo.h"
#include "mixer/playermanager.h"
#include "test/librarytest.h"
#include "track/track.h"

namespace {

const QString kAppGroup = QStringLiteral("[App]");

// Provides the mixer controls that PlayerInfo looks up on
// construction ([Master] crossfader), mirroring FakeMixer in
// autodjprocessor_test.cpp.
class FakeMixer {
  public:
    FakeMixer()
            : crossfader(ConfigKey(QStringLiteral("[Master]"),
                      QStringLiteral("crossfader")),
                    -1.0,
                    1.0),
              crossfaderReverse(ConfigKey(QStringLiteral("[Mixer Profile]"),
                      QStringLiteral("xFaderReverse"))) {
        crossfaderReverse.setButtonMode(mixxx::control::ButtonMode::Toggle);
    }

    ControlPotmeter crossfader;
    ControlPushButton crossfaderReverse;
};

// A deck stand-in that provides just what the Auto-Recommendation bot
// touches: a play control under the deck group, a synchronously
// storable loaded track, and the pure virtuals of BaseTrackPlayer.
// Modeled on FakeDeck in autodjprocessor_test.cpp.
class FakeDeck : public BaseTrackPlayer {
  public:
    FakeDeck(const QString& group, EngineChannel::ChannelOrientation orient)
            : BaseTrackPlayer(nullptr, group),
              trackSamples(ConfigKey(group, "track_samples")),
              samplerate(ConfigKey(group, "track_samplerate")),
              rateratio(ConfigKey(group, "rate_ratio"), true, false, false, 1.0),
              playposition(ConfigKey(group, "playposition"), 0.0, 1.0, 0, 0, true),
              play(ConfigKey(group, "play")),
              repeat(ConfigKey(group, "repeat")),
              introStartPos(ConfigKey(group, "intro_start_position")),
              introEndPos(ConfigKey(group, "intro_end_position")),
              outroStartPos(ConfigKey(group, "outro_start_position")),
              outroEndPos(ConfigKey(group, "outro_end_position")),
              orientation(ConfigKey(group, "orientation")) {
        play.setButtonMode(mixxx::control::ButtonMode::Toggle);
        repeat.setButtonMode(mixxx::control::ButtonMode::Toggle);
        outroStartPos.set(Cue::kNoPosition);
        outroEndPos.set(Cue::kNoPosition);
        orientation.set(orient);
    }

    TrackPointer getLoadedTrack() const override {
        return loadedTrack;
    }

    void setupEqControls() override {
    }

    void slotLoadTrack(TrackPointer pTrack,
#ifdef __STEM__
            mixxx::StemChannelSelection,
#endif
            bool bPlay) override {
        loadedTrack = pTrack;
        // Mirror BaseTrackPlayer: register the track with PlayerInfo,
        // which is where the bot reads loaded tracks from (to exclude
        // them from recommendations).
        PlayerInfo::instance().setTrackInfo(getGroup(), pTrack);
        samplerate.set(pTrack->getSampleRate());
        play.set(bPlay);
    }

    void slotEjectTrack(double val) override {
        if (val > 0) {
            loadedTrack = nullptr;
            PlayerInfo::instance().setTrackInfo(getGroup(), TrackPointer());
        }
    }

    void slotCloneFromGroup(const QString&) override {
    }

    void slotCloneDeck() override {
    }

    TrackPointer loadedTrack;
    ControlObject trackSamples;
    ControlObject samplerate;
    ControlObject rateratio;
    ControlLinPotmeter playposition;
    ControlPushButton play;
    ControlPushButton repeat;
    ControlObject introStartPos;
    ControlObject introEndPos;
    ControlObject outroStartPos;
    ControlObject outroEndPos;
    ControlObject orientation;
};

// Deterministic stand-in for PlayerManager, implementing the same
// interface AutoDJProcessor tests mock. The bot depends on
// PlayerManagerInterface exactly so tests can supply this.
class TestPlayerManager : public PlayerManagerInterface {
  public:
    TestPlayerManager()
            : numDecks(ConfigKey(kAppGroup, QStringLiteral("num_decks")), true),
              numSamplers(ConfigKey(kAppGroup, QStringLiteral("num_samplers")), true),
              numPreviewDecks(ConfigKey(kAppGroup, QStringLiteral("num_preview_decks")),
                      true) {
    }

    void addDeck(BaseTrackPlayer* pDeck) {
        m_decks.append(pDeck);
    }

    // Mirrors PlayerManager::addDeck(): once the deck exists the
    // count change is announced. Emitting this is what makes the bot
    // rebind its deck play watchers.
    void addDeckAtRuntime(BaseTrackPlayer* pDeck) {
        addDeck(pDeck);
        emit numberOfDecksChanged(static_cast<int>(m_decks.size()));
    }

    // Mirrors a runtime deck removal: the last deck is dropped and
    // the smaller count is announced, which must make the bot rebind
    // to the remaining decks.
    void removeDeckAtRuntime() {
        if (m_decks.isEmpty()) {
            return;
        }
        m_decks.removeLast();
        emit numberOfDecksChanged(static_cast<int>(m_decks.size()));
    }

    BaseTrackPlayer* getPlayer(const QString& group) const override {
        for (BaseTrackPlayer* pDeck : m_decks) {
            if (pDeck->getGroup() == group) {
                return pDeck;
            }
        }
        return nullptr;
    }

    BaseTrackPlayer* getPlayer(const ChannelHandle& channelHandle) const override {
        Q_UNUSED(channelHandle);
        return nullptr;
    }

    BaseTrackPlayer* getDeckBase(int deckIndex) const override {
        if (deckIndex < 0 || deckIndex >= m_decks.size()) {
            return nullptr;
        }
        return m_decks.at(deckIndex);
    }

    int numberOfDecks() const override {
        return static_cast<int>(m_decks.size());
    }

    PreviewDeck* getPreviewDeck(int) const override {
        return nullptr;
    }

    int numberOfPreviewDecks() const override {
        return 0;
    }

    Sampler* getSampler(int) const override {
        return nullptr;
    }

    int numberOfSamplers() const override {
        return 0;
    }

    // The count controls other components (e.g. PlayerInfo) look up.
    ControlObject numDecks;
    ControlObject numSamplers;
    ControlObject numPreviewDecks;

  private:
    QList<BaseTrackPlayer*> m_decks;
};

class AutoRecommendBotTest : public LibraryTest {
  public:
    AutoRecommendBotTest()
            : deck1(QStringLiteral("[Channel1]"), EngineChannel::LEFT),
              deck2(QStringLiteral("[Channel2]"), EngineChannel::RIGHT),
              deck3(QStringLiteral("[Channel3]"), EngineChannel::CENTER) {
        qRegisterMetaType<TrackPointer>("TrackPointer");
    }

    void SetUp() override {
        // The AutoDJ queue silently drops tracks if its playlist does
        // not exist; create it like the AutoDJ tests do.
        PlaylistDAO& playlistDao = internalCollection()->getPlaylistDAO();
        if (playlistDao.getPlaylistIdFromName(AUTODJ_TABLE) < 0) {
            playlistDao.createPlaylist(AUTODJ_TABLE, PlaylistDAO::PLHT_AUTO_DJ);
        }
        PlayerInfo::create();
        m_playerManager.addDeck(&deck1);
        m_playerManager.addDeck(&deck2);
    }

    ~AutoRecommendBotTest() override {
        PlayerInfo::destroy();
    }

  protected:
    // Loads a track into the deck and marks the deck as playing.
    // Returns the deck's play control.
    ControlObject* loadAndMarkPlaying(FakeDeck& deck, const TrackPointer& pTrack) {
        deck.slotLoadTrack(pTrack,
#ifdef __STEM__
                mixxx::StemChannelSelection(),
#endif
                false);
        EXPECT_EQ(pTrack, deck.getLoadedTrack());
        deck.play.set(1.0);
        ControlObject* pPlayControl = ControlObject::getControl(
                ConfigKey(deck.getGroup(), QStringLiteral("play")));
        EXPECT_NE(nullptr, pPlayControl);
        return pPlayControl;
    }

    void enableBot(bool deckPreloading) {
        config()->setValue(ConfigKey(QStringLiteral("[AutoRecommend]"),
                              QStringLiteral("Enabled")),
                true);
        config()->setValue(ConfigKey(QStringLiteral("[AutoRecommend]"),
                              QStringLiteral("DeckPreloading")),
                deckPreloading);
    }

    FakeMixer mixer;
    FakeDeck deck1;
    FakeDeck deck2;
    // Not registered with the player manager: tests add it through
    // TestPlayerManager::addDeckAtRuntime() to simulate a runtime
    // deck addition.
    FakeDeck deck3;
    TestPlayerManager m_playerManager;
};

} // namespace

// A deck stopping with the bot enabled preloads the best eligible
// recommendation into the first idle deck: the reference track itself
// and tracks that are already queued are excluded from the choice.
TEST_F(AutoRecommendBotTest, DeckStopPreloadsBestRecommendation) {
    const TrackPointer pReference =
            getOrAddTrackByLocation(getTestDir().filePath(kTestFile1));
    const TrackPointer pBest =
            getOrAddTrackByLocation(getTestDir().filePath(kTestFile2));
    const TrackPointer pQueued =
            getOrAddTrackByLocation(
                    getTestDir().filePath(QStringLiteral("id3-test-data/all.mp3")));
    ASSERT_TRUE(pReference && pReference->getId().isValid());
    ASSERT_TRUE(pBest && pBest->getId().isValid());
    ASSERT_TRUE(pQueued && pQueued->getId().isValid());

    // A queued track must never be recommended.
    PlaylistDAO& playlistDao = internalCollection()->getPlaylistDAO();
    playlistDao.addTracksToAutoDJQueue(
            {pQueued->getId()}, PlaylistDAO::AutoDJSendLoc::BOTTOM);
    EXPECT_TRUE(playlistDao.getAutoDJTrackIds().contains(pQueued->getId()));

    // Load the reference track into deck 1 and mark it playing.
    ControlObject* pPlayControl = loadAndMarkPlaying(deck1, pReference);
    ASSERT_EQ(pReference, deck1.getLoadedTrack());

    // The bot reads its flags from the config at construction.
    enableBot(true);
    AutoRecommendBot bot(trackCollectionManager(), &m_playerManager, config());
    ASSERT_TRUE(bot.isEnabled());
    ASSERT_TRUE(bot.isDeckPreloadingEnabled());
    ASSERT_EQ(2, m_playerManager.numberOfDecks());
    bot.bindDeckPlayControls(m_playerManager.numberOfDecks());

    bool preloaded = false;
    QString preloadedGroup;
    QString preloadedTitle;
    QObject::connect(&bot,
            &AutoRecommendBot::trackPreloaded,
            [&](const QString& group, const QString& title) {
                preloaded = true;
                preloadedGroup = group;
                preloadedTitle = title;
            });

    // Deck 2 is empty and stopped: it is the preload target.
    ASSERT_EQ(nullptr, deck2.getLoadedTrack());
    ASSERT_FALSE(deck2.play.toBool());

    // Simulate deck 1 stopping through the real play control, which
    // fires the bot's watcher.
    ASSERT_TRUE(pPlayControl->toBool());
    pPlayControl->set(0.0);

    // The best eligible recommendation has been preloaded into the
    // idle deck; the reference and the queued track were not chosen.
    ASSERT_TRUE(preloaded);
    EXPECT_EQ(deck2.getGroup(), preloadedGroup);
    ASSERT_NE(nullptr, deck2.getLoadedTrack());
    EXPECT_EQ(pBest->getId(), deck2.getLoadedTrack()->getId());
    EXPECT_EQ(pBest->getTitle(), preloadedTitle);
    // Deck 1 keeps its reference track.
    EXPECT_EQ(pReference, deck1.getLoadedTrack());
    EXPECT_FALSE(deck1.play.toBool());
}

// With preloading disabled the same deck-stop transition must not
// touch the idle deck, even though an eligible candidate exists.
TEST_F(AutoRecommendBotTest, NoPreloadWhenDeckPreloadingDisabled) {
    const TrackPointer pReference =
            getOrAddTrackByLocation(getTestDir().filePath(kTestFile1));
    const TrackPointer pCandidate =
            getOrAddTrackByLocation(getTestDir().filePath(kTestFile2));
    ASSERT_TRUE(pReference && pReference->getId().isValid());
    ASSERT_TRUE(pCandidate && pCandidate->getId().isValid());

    ControlObject* pPlayControl = loadAndMarkPlaying(deck1, pReference);
    ASSERT_EQ(pReference, deck1.getLoadedTrack());

    enableBot(false);
    AutoRecommendBot bot(trackCollectionManager(), &m_playerManager, config());
    ASSERT_TRUE(bot.isEnabled());
    ASSERT_FALSE(bot.isDeckPreloadingEnabled());
    bot.bindDeckPlayControls(m_playerManager.numberOfDecks());

    int preloadedCount = 0;
    QObject::connect(&bot,
            &AutoRecommendBot::trackPreloaded,
            [&](const QString&, const QString&) { ++preloadedCount; });

    ASSERT_EQ(nullptr, deck2.getLoadedTrack());
    ASSERT_TRUE(pPlayControl->toBool());
    pPlayControl->set(0.0);

    // No preload happened.
    EXPECT_EQ(0, preloadedCount);
    EXPECT_EQ(nullptr, deck2.getLoadedTrack());
}

// A deck added at runtime is picked up: the deck-count change makes
// the bot rebind its play watchers, so a stop on the newly added
// deck triggers a preload just like on the original decks. Before
// the count change the bot must not watch the deck at all.
TEST_F(AutoRecommendBotTest, DeckAddedAtRuntimeBecomesWatched) {
    const TrackPointer pRef1 =
            getOrAddTrackByLocation(getTestDir().filePath(kTestFile1));
    const TrackPointer pRef3 =
            getOrAddTrackByLocation(getTestDir().filePath(kTestFile2));
    const TrackPointer pCandidate =
            getOrAddTrackByLocation(
                    getTestDir().filePath(QStringLiteral("id3-test-data/all.mp3")));
    ASSERT_TRUE(pRef1 && pRef1->getId().isValid());
    ASSERT_TRUE(pRef3 && pRef3->getId().isValid());
    ASSERT_TRUE(pCandidate && pCandidate->getId().isValid());

    // Deck 1 plays the reference track; deck 2 is the idle target.
    ControlObject* pDeck1Play = loadAndMarkPlaying(deck1, pRef1);
    // The future third deck already holds a track while stopped, but
    // it is not registered with the player manager yet.
    deck3.slotLoadTrack(pRef3,
#ifdef __STEM__
            mixxx::StemChannelSelection(),
#endif
            false);
    ASSERT_EQ(pRef3, deck3.getLoadedTrack());

    enableBot(true);
    AutoRecommendBot bot(trackCollectionManager(), &m_playerManager, config());
    ASSERT_TRUE(bot.isEnabled());
    ASSERT_TRUE(bot.isDeckPreloadingEnabled());
    ASSERT_EQ(2, m_playerManager.numberOfDecks());
    bot.bindDeckPlayControls(m_playerManager.numberOfDecks());

    int preloadedCount = 0;
    QString preloadedGroup;
    QObject::connect(&bot,
            &AutoRecommendBot::trackPreloaded,
            [&](const QString& group, const QString&) {
                ++preloadedCount;
                preloadedGroup = group;
            });

    // Before the deck-count change the bot does not watch deck 3:
    // a full play/stop cycle on it preloads nothing.
    deck3.play.set(1.0);
    deck3.play.set(0.0);
    EXPECT_EQ(0, preloadedCount);
    EXPECT_EQ(nullptr, deck2.getLoadedTrack());

    // The player manager registers the third deck and announces the
    // new count, like PlayerManager::addDeck() does.
    m_playerManager.addDeckAtRuntime(&deck3);
    ASSERT_EQ(3, m_playerManager.numberOfDecks());

    // Deck 3 starting to play never preloads anything...
    deck3.play.set(1.0);
    EXPECT_EQ(0, preloadedCount);

    // ...but its subsequent stop is now watched: the only eligible
    // candidate (loaded decks and the reference track are excluded)
    // is preloaded into the idle deck.
    deck3.play.set(0.0);
    ASSERT_EQ(1, preloadedCount);
    EXPECT_EQ(deck2.getGroup(), preloadedGroup);
    ASSERT_NE(nullptr, deck2.getLoadedTrack());
    EXPECT_EQ(pCandidate->getId(), deck2.getLoadedTrack()->getId());
    // The other decks are untouched.
    EXPECT_EQ(pRef1, deck1.getLoadedTrack());
    EXPECT_EQ(pRef3, deck3.getLoadedTrack());
    EXPECT_TRUE(pDeck1Play->toBool());
}

// The rebind replaces the watchers of all decks, not just the new
// one: after a deck-count change a stop on an original deck is still
// detected and preloads the idle deck.
TEST_F(AutoRecommendBotTest, ExistingDecksStayWatchedAfterDeckAddedAtRuntime) {
    const TrackPointer pRef1 =
            getOrAddTrackByLocation(getTestDir().filePath(kTestFile1));
    const TrackPointer pCandidate =
            getOrAddTrackByLocation(getTestDir().filePath(kTestFile2));
    ASSERT_TRUE(pRef1 && pRef1->getId().isValid());
    ASSERT_TRUE(pCandidate && pCandidate->getId().isValid());

    ControlObject* pDeck1Play = loadAndMarkPlaying(deck1, pRef1);

    enableBot(true);
    AutoRecommendBot bot(trackCollectionManager(), &m_playerManager, config());
    ASSERT_TRUE(bot.isEnabled());
    ASSERT_EQ(2, m_playerManager.numberOfDecks());
    bot.bindDeckPlayControls(m_playerManager.numberOfDecks());

    int preloadedCount = 0;
    QString preloadedGroup;
    QObject::connect(&bot,
            &AutoRecommendBot::trackPreloaded,
            [&](const QString& group, const QString&) {
                ++preloadedCount;
                preloadedGroup = group;
            });
    ASSERT_EQ(nullptr, deck2.getLoadedTrack());

    // The deck-count change rebinds the watchers of every deck.
    m_playerManager.addDeckAtRuntime(&deck3);
    ASSERT_EQ(3, m_playerManager.numberOfDecks());

    // Stopping the original deck is still detected after the rebind.
    ASSERT_TRUE(pDeck1Play->toBool());
    pDeck1Play->set(0.0);

    ASSERT_EQ(1, preloadedCount);
    EXPECT_EQ(deck2.getGroup(), preloadedGroup);
    ASSERT_NE(nullptr, deck2.getLoadedTrack());
    EXPECT_EQ(pCandidate->getId(), deck2.getLoadedTrack()->getId());
    EXPECT_EQ(pRef1, deck1.getLoadedTrack());
}

// When the deck count shrinks the bot re-binds to the smaller set:
// the removed deck's watcher is dropped (a play/stop cycle on its
// control becomes inert) and the surviving decks keep working,
// including a deck that was playing across the rebind.
TEST_F(AutoRecommendBotTest, DeckRemovedAtRuntimeUnwatchesItAndKeepsOthers) {
    const TrackPointer pRef1 =
            getOrAddTrackByLocation(getTestDir().filePath(kTestFile1));
    const TrackPointer pBest =
            getOrAddTrackByLocation(getTestDir().filePath(kTestFile2));
    const TrackPointer pRemoved =
            getOrAddTrackByLocation(
                    getTestDir().filePath(QStringLiteral("id3-test-data/all.mp3")));
    ASSERT_TRUE(pRef1 && pRef1->getId().isValid());
    ASSERT_TRUE(pBest && pBest->getId().isValid());
    ASSERT_TRUE(pRemoved && pRemoved->getId().isValid());

    // Deck 1 plays the reference; deck 2 is the idle target.
    ControlObject* pDeck1Play = loadAndMarkPlaying(deck1, pRef1);

    enableBot(true);
    AutoRecommendBot bot(trackCollectionManager(), &m_playerManager, config());
    ASSERT_TRUE(bot.isEnabled());
    ASSERT_TRUE(bot.isDeckPreloadingEnabled());
    bot.bindDeckPlayControls(m_playerManager.numberOfDecks());

    int preloadedCount = 0;
    QString preloadedGroup;
    QObject::connect(&bot,
            &AutoRecommendBot::trackPreloaded,
            [&](const QString& group, const QString&) {
                ++preloadedCount;
                preloadedGroup = group;
            });

    // Grow to three decks (the bot re-binds automatically) and give
    // the third deck a track while it is stopped.
    m_playerManager.addDeckAtRuntime(&deck3);
    ASSERT_EQ(3, m_playerManager.numberOfDecks());
    deck3.slotLoadTrack(pRemoved,
#ifdef __STEM__
            mixxx::StemChannelSelection(),
#endif
            false);
    ASSERT_EQ(pRemoved, deck3.getLoadedTrack());

    // Then the deck count shrinks again and deck 3 is removed.
    m_playerManager.removeDeckAtRuntime();
    ASSERT_EQ(2, m_playerManager.numberOfDecks());

    // The removed deck is inert: a full play/stop cycle on its
    // control preloads nothing and touches no other deck.
    deck3.play.set(1.0);
    deck3.play.set(0.0);
    EXPECT_EQ(0, preloadedCount);
    EXPECT_EQ(nullptr, deck2.getLoadedTrack());

    // The surviving decks stayed watched across the rebind: deck 1
    // was playing when the count shrank, and its stop still preloads
    // the idle deck with the only eligible candidate (the reference
    // and the removed deck's track are excluded).
    ASSERT_TRUE(pDeck1Play->toBool());
    pDeck1Play->set(0.0);
    ASSERT_EQ(1, preloadedCount);
    EXPECT_EQ(deck2.getGroup(), preloadedGroup);
    ASSERT_NE(nullptr, deck2.getLoadedTrack());
    EXPECT_EQ(pBest->getId(), deck2.getLoadedTrack()->getId());
    EXPECT_EQ(pRef1, deck1.getLoadedTrack());
    // The removed deck kept its track; nothing touched it.
    EXPECT_EQ(pRemoved, deck3.getLoadedTrack());
}
