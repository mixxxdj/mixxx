#include <gtest/gtest.h>

#include <QSignalSpy>
#include <memory>

#include "control/controlobject.h"
#include "control/controlpushbutton.h"
#include "library/keyhighlightmanager.h"
#include "mixer/playermanager.h"
#include "proto/keys.pb.h"
#include "test/mixxxtest.h"
#include "track/keyutils.h"

namespace {

using mixxx::KeyHighlightManager;
using Class = KeyUtils::KeyHighlightClass;
using Shift = KeyUtils::YellowShift;
namespace key = mixxx::track::io::key;

class KeyHighlightManagerTest : public MixxxTest {
  protected:
    void SetUp() override {
        m_pNumDecks = std::make_unique<ControlObject>(
                ConfigKey(QStringLiteral("[App]"), QStringLiteral("num_decks")));
        // Create the per-deck controls the manager subscribes to. Two decks is
        // enough to cover the multi-deck union; key_highlight is a toggle
        // button just like the real KeyControl control.
        for (int i = 0; i < kDecks; ++i) {
            const QString group = PlayerManager::groupForDeck(i);
            m_highlight[i] = std::make_unique<ControlPushButton>(
                    ConfigKey(group, QStringLiteral("key_highlight")));
            m_highlight[i]->setButtonMode(mixxx::control::ButtonMode::Toggle);
            m_key[i] = std::make_unique<ControlObject>(
                    ConfigKey(group, QStringLiteral("key")));
        }
        m_pNumDecks->set(kDecks);
    }

    void setDeckKey(int deck, key::ChromaticKey k) {
        m_key[deck]->set(KeyUtils::keyToNumericValue(k));
    }
    void setHighlight(int deck, bool on) {
        m_highlight[deck]->set(on ? 1.0 : 0.0);
    }

    static constexpr int kDecks = 2;
    std::unique_ptr<ControlObject> m_pNumDecks;
    std::unique_ptr<ControlPushButton> m_highlight[kDecks];
    std::unique_ptr<ControlObject> m_key[kDecks];
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

} // namespace
