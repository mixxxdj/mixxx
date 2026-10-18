#include <gtest/gtest.h>

#include <QSignalSpy>
#include <limits>

#include "control/controlobject.h"
#include "library/keyhighlightmanager.h"
#include "mixer/playerinfo.h"
#include "proto/keys.pb.h"
#include "test/keyhighlighttestcontrols.h"
#include "test/mixxxtest.h"
#include "track/keyutils.h"
#include "track/trackid.h"

namespace {

using mixxx::KeyHighlightManager;
using KeyMatch = KeyHighlightManager::KeyMatch;
using BpmMatch = KeyHighlightManager::BpmMatch;
namespace key = mixxx::track::io::key;

TEST(KeyHighlightClassifyTest, Key) {
    const struct {
        key::ChromaticKey trackKey;
        key::ChromaticKey referenceKey;
        KeyMatch expected;
    } cases[] = {
            {key::A_MINOR, key::A_MINOR, KeyMatch::Perfect},
            {key::C_MAJOR, key::A_MINOR, KeyMatch::Perfect},
            {key::E_MINOR, key::A_MINOR, KeyMatch::Neighbour},
            {key::G_MAJOR, key::A_MINOR, KeyMatch::Neighbour},
            {key::B_FLAT_MINOR, key::A_MINOR, KeyMatch::ShiftDown},
            {key::G_SHARP_MINOR, key::A_MINOR, KeyMatch::ShiftUp},
            {key::E_FLAT_MINOR, key::A_MINOR, KeyMatch::ShiftEither},
            {key::B_MINOR, key::A_MINOR, KeyMatch::None},
            {key::B_MAJOR, key::C_MAJOR, KeyMatch::ShiftUp},
            {key::F_SHARP_MAJOR, key::C_MAJOR, KeyMatch::ShiftEither},
            {key::D_FLAT_MAJOR, key::C_MAJOR, KeyMatch::ShiftDown},
            {key::D_MAJOR, key::C_MAJOR, KeyMatch::None},
            {key::B_FLAT_MINOR, key::C_MAJOR, KeyMatch::ShiftDown},
            {key::INVALID, key::C_MAJOR, KeyMatch::None},
            {key::C_MAJOR, key::INVALID, KeyMatch::None},
            {key::INVALID, key::INVALID, KeyMatch::None},
    };
    for (const auto& c : cases) {
        EXPECT_EQ(c.expected, KeyHighlightManager::classifyKey(c.trackKey, c.referenceKey))
                << KeyUtils::keyDebugName(c.trackKey).toStdString() << " vs "
                << KeyUtils::keyDebugName(c.referenceKey).toStdString();
    }
}

TEST(KeyHighlightClassifyTest, KeyAgreesWithCompatibleKeys) {
    for (int r = key::C_MAJOR; r <= key::B_MINOR; ++r) {
        const auto referenceKey = static_cast<key::ChromaticKey>(r);
        const auto compatible = KeyUtils::getCompatibleKeys(referenceKey);
        for (int t = key::C_MAJOR; t <= key::B_MINOR; ++t) {
            const auto trackKey = static_cast<key::ChromaticKey>(t);
            const KeyMatch match = KeyHighlightManager::classifyKey(trackKey, referenceKey);
            const bool up = compatible.contains(KeyUtils::scaleKeySteps(trackKey, 1));
            const bool down = compatible.contains(KeyUtils::scaleKeySteps(trackKey, -1));
            SCOPED_TRACE(KeyUtils::keyDebugName(trackKey).toStdString() + " vs " +
                    KeyUtils::keyDebugName(referenceKey).toStdString());
            if (compatible.contains(trackKey)) {
                EXPECT_TRUE(match == KeyMatch::Perfect || match == KeyMatch::Neighbour);
            } else if (up && down) {
                EXPECT_EQ(KeyMatch::ShiftEither, match);
            } else if (up) {
                EXPECT_EQ(KeyMatch::ShiftUp, match);
            } else if (down) {
                EXPECT_EQ(KeyMatch::ShiftDown, match);
            } else {
                EXPECT_EQ(KeyMatch::None, match);
            }
        }
    }
}

TEST(KeyHighlightClassifyTest, BpmDirect) {
    EXPECT_EQ(BpmMatch::Direct, KeyHighlightManager::classifyBpm(100.0, 100.0, 6.0));
    EXPECT_EQ(BpmMatch::Direct, KeyHighlightManager::classifyBpm(103.0, 100.0, 6.0));
    // The window edges are inclusive.
    EXPECT_EQ(BpmMatch::Direct, KeyHighlightManager::classifyBpm(106.0, 100.0, 6.0));
    EXPECT_EQ(BpmMatch::Direct, KeyHighlightManager::classifyBpm(94.0, 100.0, 6.0));
    // Just outside.
    EXPECT_EQ(BpmMatch::None, KeyHighlightManager::classifyBpm(106.5, 100.0, 6.0));
    EXPECT_EQ(BpmMatch::None, KeyHighlightManager::classifyBpm(93.5, 100.0, 6.0));
}

TEST(KeyHighlightClassifyTest, BpmHalfDouble) {
    // Half tempo: 50 BPM +/- 6 % = 47..53.
    EXPECT_EQ(BpmMatch::HalfDouble, KeyHighlightManager::classifyBpm(50.0, 100.0, 6.0));
    EXPECT_EQ(BpmMatch::HalfDouble, KeyHighlightManager::classifyBpm(53.0, 100.0, 6.0));
    EXPECT_EQ(BpmMatch::HalfDouble, KeyHighlightManager::classifyBpm(47.0, 100.0, 6.0));
    EXPECT_EQ(BpmMatch::None, KeyHighlightManager::classifyBpm(53.5, 100.0, 6.0));
    // Double tempo: 200 BPM +/- 6 % = 188..212.
    EXPECT_EQ(BpmMatch::HalfDouble, KeyHighlightManager::classifyBpm(200.0, 100.0, 6.0));
    EXPECT_EQ(BpmMatch::HalfDouble, KeyHighlightManager::classifyBpm(212.0, 100.0, 6.0));
    EXPECT_EQ(BpmMatch::HalfDouble, KeyHighlightManager::classifyBpm(188.0, 100.0, 6.0));
    EXPECT_EQ(BpmMatch::None, KeyHighlightManager::classifyBpm(213.0, 100.0, 6.0));
    // Between the half and the direct window.
    EXPECT_EQ(BpmMatch::None, KeyHighlightManager::classifyBpm(75.0, 100.0, 6.0));
}

TEST(KeyHighlightClassifyTest, BpmDirectWinsOverHalfDouble) {
    // With a wide tolerance 66 BPM is within both 100 +/- 50 % and 50 +/- 50 %.
    EXPECT_EQ(BpmMatch::Direct, KeyHighlightManager::classifyBpm(66.0, 100.0, 50.0));
}

TEST(KeyHighlightClassifyTest, BpmNonPositiveIsNone) {
    EXPECT_EQ(BpmMatch::None, KeyHighlightManager::classifyBpm(100.0, 100.0, 0.0));
    EXPECT_EQ(BpmMatch::None, KeyHighlightManager::classifyBpm(100.0, 100.0, -6.0));
    EXPECT_EQ(BpmMatch::None, KeyHighlightManager::classifyBpm(0.0, 100.0, 6.0));
    EXPECT_EQ(BpmMatch::None, KeyHighlightManager::classifyBpm(-100.0, 100.0, 6.0));
    EXPECT_EQ(BpmMatch::None, KeyHighlightManager::classifyBpm(100.0, 0.0, 6.0));
    EXPECT_EQ(BpmMatch::None, KeyHighlightManager::classifyBpm(100.0, -100.0, 6.0));
}

class KeyHighlightManagerTest : public MixxxTest, protected KeyHighlightTestControls {
  protected:
    void SetUp() override {
        // PlayerInfo binds to the controls on construction, and the manager
        // to PlayerInfo. Recreate it so loaded tracks don't leak between tests.
        PlayerInfo::destroy();
        PlayerInfo::create();
    }

    void TearDown() override {
        if (KeyHighlightManager::isCreated()) {
            KeyHighlightManager::destroy();
        }
        PlayerInfo::destroy();
    }

    static KeyHighlightManager& createManager() {
        return *KeyHighlightManager::createInstance();
    }
};

TEST_F(KeyHighlightManagerTest, CreatesControlPerDeck) {
    m_pNumDecks->set(1);
    createManager();
    EXPECT_TRUE(ControlObject::exists(highlightKey(0)));
    EXPECT_FALSE(ControlObject::exists(highlightKey(1)));

    m_pNumDecks->set(2);
    EXPECT_TRUE(ControlObject::exists(highlightKey(1)));
    EXPECT_FALSE(isHighlightOn(1));

    KeyHighlightManager::destroy();
    EXPECT_FALSE(ControlObject::exists(highlightKey(0)));
}

TEST_F(KeyHighlightManagerTest, InactiveByDefault) {
    setDeckKey(0, key::A_MINOR);
    setDeckBpm(0, 100.0);
    KeyHighlightManager& manager = createManager();
    EXPECT_FALSE(manager.isKeyActive());
    EXPECT_FALSE(manager.isBpmActive());
    EXPECT_EQ(KeyMatch::None, manager.keyMatch(key::A_MINOR));
    EXPECT_EQ(BpmMatch::None, manager.bpmMatch(100.0));
    EXPECT_FALSE(manager.referenceTrack());
}

TEST_F(KeyHighlightManagerTest, MatchesEnabledDeckKey) {
    setDeckKey(0, key::A_MINOR);
    KeyHighlightManager& manager = createManager();
    setHighlight(0, true);
    ASSERT_TRUE(manager.isKeyActive());
    EXPECT_EQ(KeyMatch::Perfect, manager.keyMatch(key::A_MINOR));
    EXPECT_EQ(KeyMatch::Neighbour, manager.keyMatch(key::E_MINOR));
    EXPECT_EQ(KeyMatch::ShiftDown, manager.keyMatch(key::B_FLAT_MINOR));
    EXPECT_EQ(KeyMatch::None, manager.keyMatch(key::B_MINOR));
    EXPECT_EQ(KeyMatch::None, manager.keyMatch(key::INVALID));
}

TEST_F(KeyHighlightManagerTest, ToggleOnOff) {
    setDeckKey(0, key::A_MINOR);
    KeyHighlightManager& manager = createManager();
    QSignalSpy spy(&manager, &KeyHighlightManager::keyHighlightChanged);

    setHighlight(0, true);
    EXPECT_TRUE(manager.isKeyActive());
    EXPECT_EQ(1, spy.count());

    setHighlight(0, false);
    EXPECT_FALSE(manager.isKeyActive());
    EXPECT_EQ(KeyMatch::None, manager.keyMatch(key::A_MINOR));
    EXPECT_EQ(2, spy.count());
}

TEST_F(KeyHighlightManagerTest, MutuallyExclusiveAcrossDecks) {
    setDeckKey(0, key::A_MINOR);
    setDeckKey(1, key::F_SHARP_MAJOR);
    KeyHighlightManager& manager = createManager();

    setHighlight(0, true);
    ASSERT_EQ(KeyMatch::Perfect, manager.keyMatch(key::A_MINOR));

    // Enabling deck 1 turns deck 0 off.
    QSignalSpy spy(&manager, &KeyHighlightManager::keyHighlightChanged);
    setHighlight(1, true);
    EXPECT_FALSE(isHighlightOn(0));
    EXPECT_TRUE(isHighlightOn(1));
    EXPECT_EQ(KeyMatch::Perfect, manager.keyMatch(key::F_SHARP_MAJOR));
    EXPECT_EQ(KeyMatch::ShiftEither, manager.keyMatch(key::A_MINOR));
    EXPECT_EQ(1, spy.count());

    // Turning off a deck that isn't highlighting changes nothing.
    setHighlight(0, false);
    EXPECT_TRUE(isHighlightOn(1));
    EXPECT_EQ(KeyMatch::Perfect, manager.keyMatch(key::F_SHARP_MAJOR));
    EXPECT_EQ(1, spy.count());
}

TEST_F(KeyHighlightManagerTest, KeylessDeck) {
    setDeckKey(0, key::INVALID);
    setDeckKey(1, key::A_MINOR);
    KeyHighlightManager& manager = createManager();

    setHighlight(0, true);
    EXPECT_FALSE(manager.isKeyActive());
    EXPECT_EQ(KeyMatch::None, manager.keyMatch(key::C_MAJOR));

    setHighlight(1, true);
    EXPECT_TRUE(manager.isKeyActive());
    EXPECT_EQ(KeyMatch::Neighbour, manager.keyMatch(key::E_MINOR));
}

TEST_F(KeyHighlightManagerTest, OnlyDiscreteKeyChangesEmit) {
    setDeckKey(0, key::A_MINOR);
    KeyHighlightManager& manager = createManager();
    setHighlight(0, true);
    QSignalSpy spy(&manager, &KeyHighlightManager::keyHighlightChanged);

    // Fractions of a semitone don't change the key.
    const double base = KeyUtils::keyToNumericValue(key::A_MINOR);
    m_key[0]->set(base + 0.4);
    m_key[0]->set(base + 0.9);
    EXPECT_EQ(0, spy.count());

    setDeckKey(0, key::B_MINOR);
    EXPECT_EQ(1, spy.count());

    // Another deck's key doesn't matter.
    setDeckKey(1, key::C_MAJOR);
    EXPECT_EQ(1, spy.count());
}

TEST_F(KeyHighlightManagerTest, NewDeckTakesPart) {
    m_pNumDecks->set(1);
    setDeckKey(0, key::A_MINOR);
    setDeckKey(1, key::F_SHARP_MAJOR);
    KeyHighlightManager& manager = createManager();
    setHighlight(0, true);

    m_pNumDecks->set(2);
    setHighlight(1, true);
    EXPECT_FALSE(isHighlightOn(0));
    EXPECT_EQ(KeyMatch::Perfect, manager.keyMatch(key::F_SHARP_MAJOR));
}

TEST_F(KeyHighlightManagerTest, ReferenceKeyIsPlayingKey) {
    // A minor pitched up a semitone plays B flat minor.
    setDeckKeys(0, key::A_MINOR, key::B_FLAT_MINOR);
    KeyHighlightManager& manager = createManager();
    setHighlight(0, true);
    EXPECT_EQ(KeyMatch::Perfect, manager.keyMatch(key::B_FLAT_MINOR));
    EXPECT_EQ(KeyMatch::ShiftUp, manager.keyMatch(key::A_MINOR));
}

TEST_F(KeyHighlightManagerTest, PitchedKey) {
    setDeckKeys(0, key::A_MINOR, key::C_MINOR);
    setDeckKeys(1, key::C_MAJOR, key::C_MAJOR);
    KeyHighlightManager& manager = createManager();
    EXPECT_EQ(key::INVALID, manager.pitchedKey());

    setHighlight(0, true);
    EXPECT_EQ(key::C_MINOR, manager.pitchedKey());

    // Re-pitching emits.
    QSignalSpy spy(&manager, &KeyHighlightManager::keyHighlightChanged);
    setDeckKey(0, key::B_FLAT_MINOR);
    EXPECT_EQ(key::B_FLAT_MINOR, manager.pitchedKey());
    EXPECT_EQ(1, spy.count());

    // An unpitched deck.
    setHighlight(1, true);
    EXPECT_EQ(key::INVALID, manager.pitchedKey());

    setHighlight(1, false);
    EXPECT_EQ(key::INVALID, manager.pitchedKey());
}

TEST_F(KeyHighlightManagerTest, NoPitchedKeyWithoutBothKeys) {
    setDeckKeys(0, key::INVALID, key::B_FLAT_MINOR);
    KeyHighlightManager& manager = createManager();
    setHighlight(0, true);
    EXPECT_TRUE(manager.isKeyActive());
    EXPECT_EQ(key::INVALID, manager.pitchedKey());

    setDeckKeys(0, key::A_MINOR, key::INVALID);
    EXPECT_FALSE(manager.isKeyActive());
    EXPECT_EQ(key::INVALID, manager.pitchedKey());
}

TEST_F(KeyHighlightManagerTest, ReferenceTrack) {
    KeyHighlightManager& manager = createManager();
    const TrackPointer pTrack0 = loadTrack(0, 1);
    loadTrack(1, 2);
    EXPECT_FALSE(manager.referenceTrack());

    QSignalSpy spy(&manager, &KeyHighlightManager::referenceTrackChanged);
    setHighlight(0, true);
    EXPECT_EQ(pTrack0, manager.referenceTrack());
    EXPECT_EQ(1, spy.count());

    // Only the reference deck's track matters.
    loadTrack(1, 3);
    EXPECT_EQ(1, spy.count());

    setHighlight(1, true);
    EXPECT_EQ(2, spy.count());
    const TrackPointer pTrack3 = manager.referenceTrack();
    ASSERT_TRUE(pTrack3);
    EXPECT_EQ(TrackId(QVariant(3)), pTrack3->getId());

    ejectTrack(1);
    EXPECT_FALSE(manager.referenceTrack());
    EXPECT_EQ(3, spy.count());

    const TrackPointer pTrack4 = loadTrack(1, 4);
    EXPECT_EQ(pTrack4, manager.referenceTrack());
    EXPECT_EQ(4, spy.count());

    setHighlight(1, false);
    EXPECT_FALSE(manager.referenceTrack());
    EXPECT_EQ(5, spy.count());
}

TEST_F(KeyHighlightManagerTest, BpmFollowsEnabledDeck) {
    setDeckBpm(0, 100.0);
    setDeckBpm(1, 140.0);
    KeyHighlightManager& manager = createManager();

    setHighlight(0, true);
    ASSERT_TRUE(manager.isBpmActive());
    EXPECT_EQ(BpmMatch::Direct, manager.bpmMatch(104.0));
    EXPECT_EQ(BpmMatch::None, manager.bpmMatch(140.0));

    setHighlight(1, true);
    ASSERT_TRUE(manager.isBpmActive());
    EXPECT_EQ(BpmMatch::Direct, manager.bpmMatch(140.0));
    EXPECT_EQ(BpmMatch::None, manager.bpmMatch(104.0));

    setHighlight(1, false);
    EXPECT_FALSE(manager.isBpmActive());
    EXPECT_EQ(BpmMatch::None, manager.bpmMatch(140.0));
}

TEST_F(KeyHighlightManagerTest, BpmFollowsTempoChange) {
    setDeckBpm(0, 100.0);
    KeyHighlightManager& manager = createManager();
    setHighlight(0, true);
    ASSERT_EQ(BpmMatch::None, manager.bpmMatch(110.0));

    setDeckBpm(0, 105.0);
    EXPECT_EQ(BpmMatch::Direct, manager.bpmMatch(110.0));

    // Ejecting resets the BPM to 0.
    setDeckBpm(0, 0.0);
    EXPECT_FALSE(manager.isBpmActive());
    EXPECT_EQ(BpmMatch::None, manager.bpmMatch(100.0));
}

TEST_F(KeyHighlightManagerTest, BpmWithoutKey) {
    setDeckKey(0, key::INVALID);
    setDeckBpm(0, 120.0);
    setDeckKey(1, key::A_MINOR);
    KeyHighlightManager& manager = createManager();

    setHighlight(0, true);
    EXPECT_FALSE(manager.isKeyActive());
    EXPECT_TRUE(manager.isBpmActive());
    EXPECT_EQ(BpmMatch::Direct, manager.bpmMatch(120.0));

    // A deck without a BPM, e.g. with no track loaded.
    setHighlight(1, true);
    EXPECT_TRUE(manager.isKeyActive());
    EXPECT_FALSE(manager.isBpmActive());
}

TEST_F(KeyHighlightManagerTest, SignalsAreSeparate) {
    setDeckKey(0, key::A_MINOR);
    setDeckBpm(0, 100.0);
    KeyHighlightManager& manager = createManager();
    setHighlight(0, true);

    QSignalSpy keySpy(&manager, &KeyHighlightManager::keyHighlightChanged);
    QSignalSpy bpmSpy(&manager, &KeyHighlightManager::bpmHighlightChanged);
    QSignalSpy trackSpy(&manager, &KeyHighlightManager::referenceTrackChanged);

    setDeckBpm(0, 101.0);
    EXPECT_EQ(0, keySpy.count());
    EXPECT_EQ(1, bpmSpy.count());

    setDeckKey(0, key::B_MINOR);
    EXPECT_EQ(1, keySpy.count());
    EXPECT_EQ(1, bpmSpy.count());

    EXPECT_EQ(0, trackSpy.count());
}

TEST_F(KeyHighlightManagerTest, NoBpmEmitWhenOffOrOtherDeck) {
    setDeckBpm(0, 100.0);
    setDeckBpm(1, 140.0);
    KeyHighlightManager& manager = createManager();
    QSignalSpy spy(&manager, &KeyHighlightManager::bpmHighlightChanged);

    setDeckBpm(0, 101.0);
    EXPECT_EQ(0, spy.count());

    setHighlight(0, true);
    ASSERT_EQ(1, spy.count());

    setDeckBpm(1, 141.0);
    EXPECT_EQ(1, spy.count());
    EXPECT_EQ(BpmMatch::Direct, manager.bpmMatch(101.0));
}

TEST_F(KeyHighlightManagerTest, SetBpmRange) {
    setDeckBpm(0, 100.0);
    KeyHighlightManager& manager = createManager();
    setHighlight(0, true);
    // The default tolerance is 6 %.
    ASSERT_EQ(BpmMatch::None, manager.bpmMatch(108.0));

    QSignalSpy spy(&manager, &KeyHighlightManager::bpmHighlightChanged);
    manager.setBpmRange(10.0);
    EXPECT_EQ(1, spy.count());
    EXPECT_EQ(BpmMatch::Direct, manager.bpmMatch(108.0));

    manager.setBpmRange(10.0);
    EXPECT_EQ(1, spy.count());

    manager.setBpmRange(0.0);
    EXPECT_EQ(2, spy.count());
    EXPECT_FALSE(manager.isBpmActive());
    EXPECT_EQ(BpmMatch::None, manager.bpmMatch(100.0));
}

TEST_F(KeyHighlightManagerTest, SetBpmRangeClamps) {
    setDeckBpm(0, 100.0);
    KeyHighlightManager& manager = createManager();
    setHighlight(0, true);

    // The tolerance comes from the config file, which may have been edited by
    // hand.
    manager.setBpmRange(-1.0);
    EXPECT_FALSE(manager.isBpmActive());
    manager.setBpmRange(std::numeric_limits<double>::quiet_NaN());
    EXPECT_FALSE(manager.isBpmActive());
    manager.setBpmRange(std::numeric_limits<double>::infinity());
    EXPECT_FALSE(manager.isBpmActive());

    // 100 BPM +/- 20 % = 80..120.
    manager.setBpmRange(50.0);
    ASSERT_TRUE(manager.isBpmActive());
    EXPECT_EQ(BpmMatch::Direct, manager.bpmMatch(120.0));
    EXPECT_EQ(BpmMatch::None, manager.bpmMatch(121.0));
}

} // namespace
