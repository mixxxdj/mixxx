#include <gtest/gtest.h>

#include <QSignalSpy>
#include <memory>

#include "control/controlobject.h"
#include "control/controlpushbutton.h"
#include "library/keyhighlightmanager.h"
#include "mixer/playerinfo.h"
#include "mixer/playermanager.h"
#include "proto/keys.pb.h"
#include "test/mixxxtest.h"
#include "track/keyutils.h"
#include "track/track.h"

namespace {

using mixxx::KeyHighlightManager;
using Class = KeyUtils::KeyHighlightClass;
using Shift = KeyUtils::YellowShift;
namespace key = mixxx::track::io::key;

const QString kTrackLocation =
        QStringLiteral("/test/keyhighlight/track.mp3");

class KeyHighlightManagerTest : public MixxxTest {
  protected:
    void SetUp() override {
        m_pNumDecks = std::make_unique<ControlObject>(
                ConfigKey(QStringLiteral("[App]"), QStringLiteral("num_decks")));
        // Create the per-deck controls the manager subscribes to. Two decks is
        // enough to cover the multi-deck union; key_highlight is a toggle
        // button just like the real KeyControl control. file_key is the stored
        // key; key is the (possibly pitch-shifted) engine key.
        // PlayerInfo's idle timer polls the crossfader and each deck's
        // play/pregain/volume/orientation to pick the "loudest playing" deck -
        // machinery unrelated to the highlighter. A full harness (e.g.
        // PlayerManagerTest) gets these from a real EngineMixer; our lightweight
        // fixture stubs just the controls that path reads so its debug asserts
        // for missing controls don't fire. Their values don't matter here.
        m_pCrossfader = std::make_unique<ControlObject>(ConfigKey(
                QStringLiteral("[Master]"), QStringLiteral("crossfader")));
        // PlayerInfo's ctor also binds these [App] counters.
        m_pNumSamplers = std::make_unique<ControlObject>(ConfigKey(
                QStringLiteral("[App]"), QStringLiteral("num_samplers")));
        m_pNumPreviewDecks = std::make_unique<ControlObject>(ConfigKey(
                QStringLiteral("[App]"), QStringLiteral("num_preview_decks")));
        for (int i = 0; i < kDecks; ++i) {
            const QString group = PlayerManager::groupForDeck(i);
            m_highlight[i] = std::make_unique<ControlPushButton>(
                    ConfigKey(group, QStringLiteral("key_highlight")));
            m_highlight[i]->setButtonMode(mixxx::control::ButtonMode::Toggle);
            m_key[i] = std::make_unique<ControlObject>(
                    ConfigKey(group, QStringLiteral("key")));
            m_fileKey[i] = std::make_unique<ControlObject>(
                    ConfigKey(group, QStringLiteral("file_key")));
            m_playerInfoEnv[i][0] = std::make_unique<ControlObject>(
                    ConfigKey(group, QStringLiteral("play")));
            m_playerInfoEnv[i][1] = std::make_unique<ControlObject>(
                    ConfigKey(group, QStringLiteral("pregain")));
            m_playerInfoEnv[i][2] = std::make_unique<ControlObject>(
                    ConfigKey(group, QStringLiteral("volume")));
            m_playerInfoEnv[i][3] = std::make_unique<ControlObject>(
                    ConfigKey(group, QStringLiteral("orientation")));
        }
        m_pNumDecks->set(kDecks);

        // Create PlayerInfo only after the controls its proxies bind to exist
        // (PollingControlProxy binds at construction). The manager connects to
        // PlayerInfo::trackChanged in its ctor; own the singleton's lifecycle
        // here so loaded-track state never leaks between tests.
        PlayerInfo::destroy();
        PlayerInfo::create();
    }

    void TearDown() override {
        PlayerInfo::destroy();
    }

    void setDeckKey(int deck, key::ChromaticKey k) {
        m_key[deck]->set(KeyUtils::keyToNumericValue(k));
    }
    void setDeckFileKey(int deck, key::ChromaticKey k) {
        m_fileKey[deck]->set(KeyUtils::keyToNumericValue(k));
    }
    // Convenience: set both the stored and playing key to the same value (an
    // unpitched deck) so a test only has to override `key` to simulate pitch.
    void setDeckKeys(int deck, key::ChromaticKey fileKey, key::ChromaticKey playingKey) {
        setDeckFileKey(deck, fileKey);
        setDeckKey(deck, playingKey);
    }
    void setHighlight(int deck, bool on) {
        m_highlight[deck]->set(on ? 1.0 : 0.0);
    }
    // Load (or, with a null id, eject) a track on a deck. We emit
    // PlayerInfo::trackChanged directly - the exact signal the manager listens
    // to in production - rather than going through setTrackInfo(), whose
    // additional "loudest playing deck" bookkeeping reads crossfader/play/volume
    // controls that are unrelated to this feature.
    TrackPointer loadTrack(int deck, int trackId) {
        auto pTrack = Track::newDummy(kTrackLocation, TrackId(QVariant(trackId)));
        emit PlayerInfo::instance().trackChanged(
                PlayerManager::groupForDeck(deck), pTrack, m_loaded[deck]);
        m_loaded[deck] = pTrack;
        return pTrack;
    }
    void ejectTrack(int deck) {
        emit PlayerInfo::instance().trackChanged(
                PlayerManager::groupForDeck(deck), TrackPointer(), m_loaded[deck]);
        m_loaded[deck] = TrackPointer();
    }

    static constexpr int kDecks = 2;
    std::unique_ptr<ControlObject> m_pNumDecks;
    std::unique_ptr<ControlObject> m_pCrossfader;
    std::unique_ptr<ControlObject> m_pNumSamplers;
    std::unique_ptr<ControlObject> m_pNumPreviewDecks;
    std::unique_ptr<ControlPushButton> m_highlight[kDecks];
    std::unique_ptr<ControlObject> m_key[kDecks];
    std::unique_ptr<ControlObject> m_fileKey[kDecks];
    // play/pregain/volume/orientation per deck - PlayerInfo's loudest-deck
    // environment, not part of this feature (see SetUp()).
    std::unique_ptr<ControlObject> m_playerInfoEnv[kDecks][4];
    // Tracks the last loaded track per deck so we can pass a correct "old track"
    // when emitting trackChanged.
    TrackPointer m_loaded[kDecks];
};

TEST_F(KeyHighlightManagerTest, InactiveByDefault) {
    KeyHighlightManager manager;
    EXPECT_FALSE(manager.isActive());
    EXPECT_EQ(Class::None, manager.classOf(key::C_MAJOR));
    EXPECT_EQ(Class::None, manager.classOf(key::A_MINOR));
}

TEST_F(KeyHighlightManagerTest, SingleDeckClassification) {
    const auto ref = key::A_MINOR;
    setDeckKey(0, ref);
    setHighlight(0, true);

    KeyHighlightManager manager;
    ASSERT_TRUE(manager.isActive());

    // classOf must agree with the pure classifier for every valid key.
    for (int i = static_cast<int>(key::C_MAJOR);
            i <= static_cast<int>(key::B_MINOR);
            ++i) {
        const auto trackKey = static_cast<key::ChromaticKey>(i);
        EXPECT_EQ(KeyUtils::classifyAgainst(trackKey, ref),
                manager.classOf(trackKey))
                << "track=" << i;
    }
    // INVALID is never highlighted.
    EXPECT_EQ(Class::None, manager.classOf(key::INVALID));
}

TEST_F(KeyHighlightManagerTest, DirectionMatchesClassifier) {
    const auto ref = key::A_MINOR;
    setDeckKey(0, ref);
    setHighlight(0, true);

    KeyHighlightManager manager;
    ASSERT_TRUE(manager.isActive());

    // directionOf must agree with the pure classifier for every valid key, and
    // be None for any non-Yellow key (the table only carries directions on
    // Yellow cells).
    for (int i = static_cast<int>(key::C_MAJOR);
            i <= static_cast<int>(key::B_MINOR);
            ++i) {
        const auto trackKey = static_cast<key::ChromaticKey>(i);
        EXPECT_EQ(KeyUtils::yellowShiftDirection(trackKey, ref),
                manager.directionOf(trackKey))
                << "track=" << i;
        if (manager.classOf(trackKey) != Class::Yellow) {
            EXPECT_EQ(Shift::None, manager.directionOf(trackKey))
                    << "non-yellow track=" << i;
        }
    }
    // INVALID never carries a direction.
    EXPECT_EQ(Shift::None, manager.directionOf(key::INVALID));
}

TEST_F(KeyHighlightManagerTest, DirectionInactiveIsNone) {
    setDeckKey(0, key::A_MINOR);
    KeyHighlightManager manager;
    ASSERT_FALSE(manager.isActive());

    // With the highlighter off, no key carries a direction.
    for (int i = static_cast<int>(key::C_MAJOR);
            i <= static_cast<int>(key::B_MINOR);
            ++i) {
        EXPECT_EQ(Shift::None,
                manager.directionOf(static_cast<key::ChromaticKey>(i)))
                << "track=" << i;
    }
}

TEST_F(KeyHighlightManagerTest, DirectionConsistentWithChosenClass) {
    // After a deck switch, the surviving reference key must drive the direction,
    // not a stale one. Mirrors MutuallyExclusiveAcrossDecks but checks the
    // direction table.
    const auto ref1 = key::F_SHARP_MAJOR;
    setDeckKey(0, key::A_MINOR);
    setDeckKey(1, ref1);

    KeyHighlightManager manager;
    setHighlight(0, true);
    setHighlight(1, true); // clears deck 0; F# major is now the sole reference

    ASSERT_TRUE(manager.isActive());
    EXPECT_DOUBLE_EQ(0.0, m_highlight[0]->get());

    for (int i = static_cast<int>(key::C_MAJOR);
            i <= static_cast<int>(key::B_MINOR);
            ++i) {
        const auto trackKey = static_cast<key::ChromaticKey>(i);
        EXPECT_EQ(KeyUtils::yellowShiftDirection(trackKey, ref1),
                manager.directionOf(trackKey))
                << "track=" << i;
    }
}

TEST_F(KeyHighlightManagerTest, ToggleOnOff) {
    setDeckKey(0, key::A_MINOR);
    KeyHighlightManager manager;
    ASSERT_FALSE(manager.isActive());

    QSignalSpy spy(&manager, &KeyHighlightManager::highlightChanged);

    setHighlight(0, true);
    EXPECT_TRUE(manager.isActive());
    EXPECT_EQ(1, spy.count());

    setHighlight(0, false);
    EXPECT_FALSE(manager.isActive());
    EXPECT_EQ(Class::None, manager.classOf(key::C_MAJOR));
    EXPECT_EQ(2, spy.count());
}

TEST_F(KeyHighlightManagerTest, MutuallyExclusiveAcrossDecks) {
    // The highlighter is single-deck: enabling one deck's highlight turns any
    // other deck's off, so the classification reflects exactly one reference
    // key at a time.
    const auto ref0 = key::A_MINOR;
    const auto ref1 = key::F_SHARP_MAJOR; // harmonically distant from A minor
    setDeckKey(0, ref0);
    setDeckKey(1, ref1);

    KeyHighlightManager manager;
    ASSERT_FALSE(manager.isActive());

    // Enable deck 0: classifies against A minor.
    setHighlight(0, true);
    ASSERT_TRUE(manager.isActive());
    EXPECT_EQ(Class::GreenPerfect, manager.classOf(key::A_MINOR));

    // Enable deck 1 while deck 0 is on: deck 0 must be cleared, and the
    // classification must now reflect F# major only.
    setHighlight(1, true);
    EXPECT_TRUE(manager.isActive());
    // The manager cleared deck 0's control as a side effect.
    EXPECT_DOUBLE_EQ(0.0, m_highlight[0]->get());
    EXPECT_DOUBLE_EQ(1.0, m_highlight[1]->get());
    // F# major is now the sole reference -> GreenPerfect for itself.
    EXPECT_EQ(Class::GreenPerfect, manager.classOf(key::F_SHARP_MAJOR));
    // A minor is no longer a reference, so it is classified *against* F# major
    // (distant), not GreenPerfect anymore.
    EXPECT_EQ(KeyUtils::classifyAgainst(key::A_MINOR, ref1),
            manager.classOf(key::A_MINOR));

    // Every track key now matches the single-deck (F# major) classification.
    for (int i = static_cast<int>(key::C_MAJOR);
            i <= static_cast<int>(key::B_MINOR);
            ++i) {
        const auto trackKey = static_cast<key::ChromaticKey>(i);
        EXPECT_EQ(KeyUtils::classifyAgainst(trackKey, ref1),
                manager.classOf(trackKey))
                << "track=" << i;
    }
}

TEST_F(KeyHighlightManagerTest, MutualExclusionEmitsAndDoesNotOscillate) {
    setDeckKey(0, key::A_MINOR);
    setDeckKey(1, key::F_SHARP_MAJOR);

    KeyHighlightManager manager;
    QSignalSpy spy(&manager, &KeyHighlightManager::highlightChanged);

    setHighlight(0, true);
    ASSERT_TRUE(manager.isActive());
    const int afterDeck0 = spy.count();
    EXPECT_GT(afterDeck0, 0);

    // Switching to deck 1 clears deck 0. This involves a re-entrant write to
    // deck 0's control; the re-entrancy guard must keep this convergent (a
    // bounded number of emits, no infinite loop) and leave a single active deck.
    setHighlight(1, true);
    EXPECT_TRUE(manager.isActive());
    EXPECT_DOUBLE_EQ(0.0, m_highlight[0]->get());
    EXPECT_DOUBLE_EQ(1.0, m_highlight[1]->get());
    EXPECT_EQ(Class::GreenPerfect, manager.classOf(key::F_SHARP_MAJOR));
    // Convergent: a switch produces only a small, bounded number of repaints.
    EXPECT_LE(spy.count() - afterDeck0, 3);

    // Turning the sole active deck off deactivates the highlighter entirely.
    setHighlight(1, false);
    EXPECT_FALSE(manager.isActive());
    EXPECT_EQ(Class::None, manager.classOf(key::F_SHARP_MAJOR));
}

TEST_F(KeyHighlightManagerTest, KeylessDeckContributesNothing) {
    // Deck 0 enabled but has no key -> not active, nothing highlighted.
    setDeckKey(0, key::INVALID);
    setHighlight(0, true);

    KeyHighlightManager manager;
    EXPECT_FALSE(manager.isActive());
    EXPECT_EQ(Class::None, manager.classOf(key::C_MAJOR));

    // Add a second, keyed+enabled deck -> only that one contributes.
    setDeckKey(1, key::A_MINOR);
    setHighlight(1, true);
    EXPECT_TRUE(manager.isActive());
    EXPECT_EQ(KeyUtils::classifyAgainst(key::E_MINOR, key::A_MINOR),
            manager.classOf(key::E_MINOR));
}

TEST_F(KeyHighlightManagerTest, DiscreteKeyDebounce) {
    setDeckKey(0, key::A_MINOR);
    setHighlight(0, true);
    KeyHighlightManager manager;
    ASSERT_TRUE(manager.isActive());

    QSignalSpy spy(&manager, &KeyHighlightManager::highlightChanged);

    // A sub-semitone change that still floors to the same ChromaticKey must
    // NOT trigger a recompute/emit (this guards the perf claim).
    const double base = KeyUtils::keyToNumericValue(key::A_MINOR);
    m_key[0]->set(base + 0.4);
    EXPECT_EQ(0, spy.count());
    m_key[0]->set(base + 0.9);
    EXPECT_EQ(0, spy.count());

    // Crossing into a new discrete key emits exactly once.
    setDeckKey(0, key::B_MINOR);
    EXPECT_EQ(1, spy.count());
}

TEST_F(KeyHighlightManagerTest, NumDecksGrowth) {
    // Start the manager when num_decks reports a single deck, then grow.
    m_pNumDecks->set(1);
    setDeckKey(0, key::A_MINOR);
    setHighlight(0, true);

    KeyHighlightManager manager;
    ASSERT_TRUE(manager.isActive());

    QSignalSpy spy(&manager, &KeyHighlightManager::highlightChanged);

    // Grow to two decks; the newly-added deck must start participating. Enabling
    // deck 1 clears deck 0 (mutual exclusion), so F# major becomes the sole
    // reference.
    m_pNumDecks->set(2);
    setDeckKey(1, key::F_SHARP_MAJOR);
    setHighlight(1, true);

    EXPECT_TRUE(manager.isActive());
    EXPECT_EQ(Class::GreenPerfect, manager.classOf(key::F_SHARP_MAJOR));
    EXPECT_DOUBLE_EQ(0.0, m_highlight[0]->get()); // deck 0 was cleared
    EXPECT_GT(spy.count(), 0);

    // Lowering num_decks must drop the now-out-of-range deck's contribution.
    // Only deck 1 (F# major) was enabled; shrinking below it must deactivate
    // the highlighter rather than leave a stale reference key active.
    m_pNumDecks->set(1);
    EXPECT_FALSE(manager.isActive());
    EXPECT_EQ(Class::None, manager.classOf(key::F_SHARP_MAJOR));
}

TEST_F(KeyHighlightManagerTest, NumDecksShrinkKeepsInRangeDeck) {
    // Two decks, deck 0 enabled (A minor). Shrinking to one deck must keep the
    // still-in-range deck 0 active.
    setDeckKey(0, key::A_MINOR);
    KeyHighlightManager manager;
    setHighlight(0, true);
    ASSERT_TRUE(manager.isActive());

    m_pNumDecks->set(1);
    EXPECT_TRUE(manager.isActive());
    EXPECT_EQ(Class::GreenPerfect, manager.classOf(key::A_MINOR));
}

// ---------------------------------------------------------------------------
// playingKeyForTrack(): the pitched "OriginalKey (PlayingKey)" suffix source.
// ---------------------------------------------------------------------------

TEST_F(KeyHighlightManagerTest, PlayingKeyDiffersWhenPitched) {
    // Deck 0 highlighting, track loaded, stored key A minor but pitched up one
    // semitone so the engine (playing) key is A# minor -> the suffix key is the
    // playing key.
    setDeckKeys(0, key::A_MINOR, key::B_FLAT_MINOR);
    setHighlight(0, true);
    KeyHighlightManager manager;
    ASSERT_TRUE(manager.isActive());

    const int kId = 7;
    loadTrack(0, kId);

    EXPECT_EQ(key::B_FLAT_MINOR,
            manager.playingKeyForTrack(TrackId(QVariant(kId))));
}

TEST_F(KeyHighlightManagerTest, NoSuffixWhenUnpitched) {
    // Playing key == stored key -> no parenthetical.
    setDeckKeys(0, key::A_MINOR, key::A_MINOR);
    setHighlight(0, true);
    KeyHighlightManager manager;
    ASSERT_TRUE(manager.isActive());

    const int kId = 7;
    loadTrack(0, kId);

    EXPECT_EQ(key::INVALID,
            manager.playingKeyForTrack(TrackId(QVariant(kId))));
}

TEST_F(KeyHighlightManagerTest, NoSuffixWhenHighlighterOff) {
    // Pitched, track loaded, but the deck's highlighter is OFF: the suffix is a
    // highlighter feature, so it must not show.
    setDeckKeys(0, key::A_MINOR, key::B_FLAT_MINOR);
    KeyHighlightManager manager;
    const int kId = 7;
    loadTrack(0, kId);
    ASSERT_FALSE(manager.isActive());

    EXPECT_EQ(key::INVALID,
            manager.playingKeyForTrack(TrackId(QVariant(kId))));
}

TEST_F(KeyHighlightManagerTest, NoSuffixForUnloadedTrack) {
    // A track id that is not loaded on any deck never gets a suffix.
    setDeckKeys(0, key::A_MINOR, key::B_FLAT_MINOR);
    setHighlight(0, true);
    KeyHighlightManager manager;
    ASSERT_TRUE(manager.isActive());
    loadTrack(0, 7);

    EXPECT_EQ(key::INVALID,
            manager.playingKeyForTrack(TrackId(QVariant(999))));
    // An invalid TrackId is also never matched.
    EXPECT_EQ(key::INVALID, manager.playingKeyForTrack(TrackId()));
}

TEST_F(KeyHighlightManagerTest, SuffixAppearsOnLoadAfterConstruction) {
    // Loading a track after the manager is constructed must make the suffix
    // appear (the PlayerInfo::trackChanged wiring drives this) and emit so the
    // model repaints.
    setDeckKeys(0, key::A_MINOR, key::B_FLAT_MINOR);
    setHighlight(0, true);
    KeyHighlightManager manager;
    ASSERT_TRUE(manager.isActive());

    const int kId = 11;
    EXPECT_EQ(key::INVALID,
            manager.playingKeyForTrack(TrackId(QVariant(kId))));

    QSignalSpy spy(&manager, &KeyHighlightManager::highlightChanged);
    loadTrack(0, kId);
    EXPECT_GT(spy.count(), 0);
    EXPECT_EQ(key::B_FLAT_MINOR,
            manager.playingKeyForTrack(TrackId(QVariant(kId))));
}

TEST_F(KeyHighlightManagerTest, EjectClearsSuffix) {
    setDeckKeys(0, key::A_MINOR, key::B_FLAT_MINOR);
    setHighlight(0, true);
    KeyHighlightManager manager;
    ASSERT_TRUE(manager.isActive());

    const int kId = 7;
    loadTrack(0, kId);
    ASSERT_EQ(key::B_FLAT_MINOR,
            manager.playingKeyForTrack(TrackId(QVariant(kId))));

    ejectTrack(0);
    EXPECT_EQ(key::INVALID,
            manager.playingKeyForTrack(TrackId(QVariant(kId))));
}

TEST_F(KeyHighlightManagerTest, SuffixFollowsRefDeckOnSwitch) {
    // Both decks hold a (different) pitched track. Switching the active deck
    // must move the suffix to the surviving reference deck's track, and clear it
    // from the deck that was turned off (mutual exclusion).
    setDeckKeys(0, key::A_MINOR, key::B_FLAT_MINOR); // deck 0 pitched +1
    setDeckKeys(1, key::C_MAJOR, key::D_FLAT_MAJOR); // deck 1 pitched +1
    KeyHighlightManager manager;
    const int kId0 = 1;
    const int kId1 = 2;
    loadTrack(0, kId0);
    loadTrack(1, kId1);

    setHighlight(0, true);
    ASSERT_TRUE(manager.isActive());
    EXPECT_EQ(key::B_FLAT_MINOR,
            manager.playingKeyForTrack(TrackId(QVariant(kId0))));
    EXPECT_EQ(key::INVALID,
            manager.playingKeyForTrack(TrackId(QVariant(kId1))));

    // Switch to deck 1: deck 0 is cleared, so only deck 1's track gets a suffix.
    setHighlight(1, true);
    ASSERT_TRUE(manager.isActive());
    EXPECT_DOUBLE_EQ(0.0, m_highlight[0]->get());
    EXPECT_EQ(key::INVALID,
            manager.playingKeyForTrack(TrackId(QVariant(kId0))));
    EXPECT_EQ(key::D_FLAT_MAJOR,
            manager.playingKeyForTrack(TrackId(QVariant(kId1))));
}

TEST_F(KeyHighlightManagerTest, AnyPitchAmountNotJustOneSemitone) {
    // The suffix reflects whatever the discrete playing key is, for any pitch
    // amount - here +3 semitones (A minor -> C minor) and -2 (C major -> A#
    // major), not only +/-1.
    setDeckKeys(0, key::A_MINOR, key::C_MINOR);
    setHighlight(0, true);
    KeyHighlightManager manager;
    ASSERT_TRUE(manager.isActive());
    loadTrack(0, 5);
    EXPECT_EQ(key::C_MINOR, manager.playingKeyForTrack(TrackId(QVariant(5))));

    // Re-pitch the same deck down two semitones from C major.
    setDeckKeys(0, key::C_MAJOR, key::B_FLAT_MAJOR);
    EXPECT_EQ(key::B_FLAT_MAJOR,
            manager.playingKeyForTrack(TrackId(QVariant(5))));
}

TEST_F(KeyHighlightManagerTest, ReferenceKeyIsPlayingKeyNotFileKey) {
    // Requirement-1 regression: with the deck pitched, the *classification* of
    // the rest of the library must be computed against the playing (engine) key,
    // not the stored file key. Pick a track key that classifies differently
    // against the two so a regression to file_key would be caught.
    const auto fileKey = key::A_MINOR;
    const auto playingKey = key::B_FLAT_MINOR; // pitched +1
    setDeckKeys(0, fileKey, playingKey);
    setHighlight(0, true);
    KeyHighlightManager manager;
    ASSERT_TRUE(manager.isActive());

    // The manager must agree with classifying against the PLAYING key for every
    // track key, and differ from the file-key classification on at least one
    // (proving it is not silently using file_key).
    bool sawDifference = false;
    for (int i = static_cast<int>(key::C_MAJOR);
            i <= static_cast<int>(key::B_MINOR);
            ++i) {
        const auto trackKey = static_cast<key::ChromaticKey>(i);
        EXPECT_EQ(KeyUtils::classifyAgainst(trackKey, playingKey),
                manager.classOf(trackKey))
                << "track=" << i;
        if (KeyUtils::classifyAgainst(trackKey, playingKey) !=
                KeyUtils::classifyAgainst(trackKey, fileKey)) {
            sawDifference = true;
        }
    }
    EXPECT_TRUE(sawDifference)
            << "test keys chosen so playing vs file classification differ";
}

TEST_F(KeyHighlightManagerTest, LoadedPitchedRowMatchesItself) {
    // F7 regression: the loaded, pitched deck's OWN library row must not be
    // labelled Yellow with a "pitch me one semitone" arrow it is already playing
    // past. The model classifies that row by the key it is *playing*
    // (playingKeyForTrack), which is the highlighter's own reference key, so the
    // self-comparison is the strongest match and carries no transpose hint.
    //
    // Worked example (the common case): stored A minor pitched +1 semitone plays
    // B flat minor; that is the reference. classifyAgainst(playing, playing) is
    // GreenPerfect and directionOf(playing) is None - whereas classifying the
    // STORED key A minor against the playing B flat minor would (wrongly) be
    // Yellow + an Up arrow.
    const auto fileKey = key::A_MINOR;
    const auto playingKey = key::B_FLAT_MINOR; // pitched +1
    setDeckKeys(0, fileKey, playingKey);
    setHighlight(0, true);
    KeyHighlightManager manager;
    ASSERT_TRUE(manager.isActive());

    const int kId = 7;
    loadTrack(0, kId);

    // Guard the premise: the stored key really would mislabel against the
    // playing reference, so the substitution below is doing real work.
    ASSERT_EQ(Class::Yellow, manager.classOf(fileKey));
    ASSERT_NE(Shift::None, manager.directionOf(fileKey));

    // The model substitutes the playing key for this row; that is what it
    // classifies. It must read GreenPerfect with no arrow.
    const auto rowKey = manager.playingKeyForTrack(TrackId(QVariant(kId)));
    ASSERT_EQ(playingKey, rowKey);
    EXPECT_EQ(Class::GreenPerfect, manager.classOf(rowKey));
    EXPECT_EQ(Shift::None, manager.directionOf(rowKey));
}

TEST_F(KeyHighlightManagerTest, PlayingKeyInvalidGuards) {
    // Invalid file or engine key -> no suffix even when loaded+highlighting.
    setDeckKeys(0, key::INVALID, key::B_FLAT_MINOR);
    setHighlight(0, true);
    KeyHighlightManager manager;
    loadTrack(0, 3);
    // Deck has no valid stored key; even if active via another path, the suffix
    // must be suppressed.
    EXPECT_EQ(key::INVALID, manager.playingKeyForTrack(TrackId(QVariant(3))));

    // Valid stored key but invalid engine key -> also suppressed.
    setDeckKeys(0, key::A_MINOR, key::INVALID);
    EXPECT_EQ(key::INVALID, manager.playingKeyForTrack(TrackId(QVariant(3))));
}

} // namespace
