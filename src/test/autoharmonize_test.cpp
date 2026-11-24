#include <gtest/gtest.h>

#include <QCheckBox>
#include <QLabel>

#include "control/controlobject.h"
#include "engine/controls/keycontrol.h"
#include "mixer/basetrackplayer.h"
#include "preferences/dialog/dlgprefdeck.h"
// Named here so mixxx-test's AUTOUIC generates it. The dialog header
// includes the same file, but that include is not scanned on this target.
#include "preferences/dialog/ui_dlgprefdeckdlg.h"
#include "proto/keys.pb.h"
#include "test/mixxxtest.h"
#include "test/mockedenginebackendtest.h"
#include "track/keyutils.h"

namespace {

const ConfigKey kRetiredAutoDjKey(QStringLiteral("[Auto DJ]"),
        QStringLiteral("AutoHarmonizeToPrevious"));
const ConfigKey kRetiredManualKey(QStringLiteral("[Key]"),
        QStringLiteral("AutoHarmonizeOnManualLoad"));

const mixxx::track::io::key::ChromaticKey kLoadedKey =
        mixxx::track::io::key::F_SHARP_MAJOR;
const mixxx::track::io::key::ChromaticKey kOtherKey =
        mixxx::track::io::key::C_MAJOR;

double compatibleOffset(mixxx::track::io::key::ChromaticKey fileKey,
        mixxx::track::io::key::ChromaticKey otherKey,
        double otherDistance) {
    return static_cast<double>(KeyUtils::shortestStepsToCompatibleKey(fileKey, otherKey)) +
            otherDistance;
}

double exactOffset(mixxx::track::io::key::ChromaticKey fileKey,
        mixxx::track::io::key::ChromaticKey otherKey,
        double otherDistance) {
    return static_cast<double>(KeyUtils::shortestStepsToKey(fileKey, otherKey)) +
            otherDistance;
}

// KeyControl stores pow(2, semitones/12) converted back to semitones.
// That is not always the requested double, and EXPECT_DOUBLE_EQ allows
// only 4 ULPs, which this round trip can use up.
double storedMatchKeyPitch(double requestedSemitones) {
    return KeyUtils::powerOf2ToSemitoneChange(
            KeyUtils::semitoneChangeToPowerOf2(requestedSemitones));
}

// Publish an effective key on a real deck the same way KeyControl does when
// its pitch control moves, then read back the key and visual distance sync_key
// would use. The expected pitch is computed from those readbacks.
struct OtherDeckKey {
    mixxx::track::io::key::ChromaticKey key;
    double distance;
};

OtherDeckKey publishOtherDeckKey(const QString& group, double pitchSemitones) {
    ControlObject::set(ConfigKey(group, "file_key"), KeyUtils::keyToNumericValue(kOtherKey));
    ControlObject::set(ConfigKey(group, "pitch"), pitchSemitones);
    OtherDeckKey result;
    result.key = KeyUtils::keyFromNumericValue(ControlObject::get(ConfigKey(group, "key")));
    result.distance = ControlObject::get(ConfigKey(group, "visual_key_distance"));
    return result;
}

} // namespace

class AutoHarmonizeManualLoadTest : public MockedEngineBackendTest {
  protected:
    void SetUp() override {
        MockedEngineBackendTest::SetUp();
        config()->setValue(ConfigKey("[Controls]", "SpeedAutoReset"),
                BaseTrackPlayer::TrackLoadReset::RESET_NONE);
    }

    void setOption(bool enabled) {
        config()->setValue(kAutoHarmonizeOnLoadConfigKey, enabled);
    }

    void setExact(bool enabled) {
        config()->setValue(kExactKeyMatchConfigKey, enabled);
    }

    struct LoadResult {
        // Pitch after the pre-load write, once KeyControl has absorbed it,
        // and before slotTrackLoaded. Harmonize is the only later pitch write
        // when SpeedAutoReset is RESET_NONE and the file key does not change.
        double pitchBefore;
        double pitch;
    };

    // Calls the shipped main-deck load path (slotTrackLoaded) after the file
    // key is on the track.
    LoadResult loadDeck1(mixxx::track::io::key::ChromaticKey key, double initialPitch) {
        if (key == mixxx::track::io::key::INVALID) {
            m_pTrack1->resetKeys();
        } else {
            m_pTrack1->setKey(key, mixxx::track::io::key::USER);
        }
        const ConfigKey pitchKey(m_sGroup1, "pitch");
        ControlObject::set(pitchKey, initialPitch);
        LoadResult result;
        result.pitchBefore = ControlObject::get(pitchKey);
        m_pMixerDeck1->slotTrackLoaded(m_pTrack1, TrackPointer());
        EXPECT_EQ(m_pTrack1, m_pMixerDeck1->getLoadedTrack());
        result.pitch = ControlObject::get(pitchKey);
        return result;
    }
};

TEST_F(AutoHarmonizeManualLoadTest, ManualLoadHarmonizesWhenOptionOn) {
    setOption(true);
    const OtherDeckKey other = publishOtherDeckKey(m_sGroup2, 0.3);
    ASSERT_NE(mixxx::track::io::key::INVALID, other.key);
    const double expected = compatibleOffset(kLoadedKey, other.key, other.distance);
    const auto helper = KeyUtils::matchKeyPitchSemitones(kLoadedKey, other.key, other.distance);
    ASSERT_TRUE(helper.has_value());
    EXPECT_DOUBLE_EQ(expected, *helper);
    ASSERT_NE(expected * 2.0, expected);

    const LoadResult loaded = loadDeck1(kLoadedKey, 4.0);
    ASSERT_NE(loaded.pitchBefore, expected);
    EXPECT_DOUBLE_EQ(storedMatchKeyPitch(expected), loaded.pitch);
    EXPECT_NE(expected * 2.0, loaded.pitch);
    EXPECT_EQ(kLoadedKey, m_pTrack1->getKey());
}

TEST_F(AutoHarmonizeManualLoadTest, ManualLoadLeavesPitchWhenOptionOff) {
    setOption(false);
    publishOtherDeckKey(m_sGroup2, 0.3);
    const LoadResult loaded = loadDeck1(kLoadedKey, 4.0);
    EXPECT_DOUBLE_EQ(loaded.pitchBefore, loaded.pitch);
    EXPECT_EQ(kLoadedKey, m_pTrack1->getKey());
}

TEST_F(AutoHarmonizeManualLoadTest, MatchKeyClickLeavesSyncOff) {
    setOption(false);
    const OtherDeckKey other = publishOtherDeckKey(m_sGroup2, 0.0);
    ASSERT_NE(mixxx::track::io::key::INVALID, other.key);
    // No leader makes slotSyncKey call pickSyncTarget, which needs a playing
    // deck with a BPM. The crown selects the target without that.
    ControlObject::set(ConfigKey(m_sGroup2, "sync_key_leader"), 2.0);
    ControlObject::set(ConfigKey(m_sGroup1, "file_key"),
            KeyUtils::keyToNumericValue(kLoadedKey));
    const double expected = compatibleOffset(kLoadedKey, other.key, other.distance);
    ASSERT_NE(0.0, expected);
    const ConfigKey pitchKey(m_sGroup1, "pitch");
    ControlObject::set(pitchKey, 4.0);
    ASSERT_NE(ControlObject::get(pitchKey), storedMatchKeyPitch(expected));

    ControlObject::set(ConfigKey(m_sGroup1, "match_key"), 1.0);
    EXPECT_DOUBLE_EQ(storedMatchKeyPitch(expected), ControlObject::get(pitchKey));
    EXPECT_DOUBLE_EQ(0.0, ControlObject::get(ConfigKey(m_sGroup1, "sync_key")));
    EXPECT_DOUBLE_EQ(0.0, ControlObject::get(ConfigKey(m_sGroup1, "sync_key_leader")));

    ControlObject::set(ConfigKey(m_sGroup1, "match_key"), 0.0);
    EXPECT_DOUBLE_EQ(0.0, ControlObject::get(ConfigKey(m_sGroup1, "sync_key")));
    EXPECT_DOUBLE_EQ(storedMatchKeyPitch(expected), ControlObject::get(pitchKey));

    // The click must not latch later loads.
    const LoadResult loaded = loadDeck1(kLoadedKey, 4.0);
    EXPECT_DOUBLE_EQ(loaded.pitchBefore, loaded.pitch);
    EXPECT_DOUBLE_EQ(0.0, ControlObject::get(ConfigKey(m_sGroup1, "sync_key")));
}

TEST_F(AutoHarmonizeManualLoadTest, SyncKeyFirstPressBecomesLeader) {
    setOption(false);
    publishOtherDeckKey(m_sGroup2, 0.3);
    ControlObject::set(ConfigKey(m_sGroup1, "file_key"),
            KeyUtils::keyToNumericValue(kLoadedKey));
    const ConfigKey pitchKey(m_sGroup1, "pitch");
    ControlObject::set(pitchKey, 4.0);
    const double pitch = ControlObject::get(pitchKey);

    ControlObject::set(ConfigKey(m_sGroup1, "sync_key"), 1.0);
    EXPECT_DOUBLE_EQ(pitch, ControlObject::get(pitchKey));
    EXPECT_DOUBLE_EQ(1.0, ControlObject::get(ConfigKey(m_sGroup1, "sync_key_leader")));
    EXPECT_DOUBLE_EQ(0.0, ControlObject::get(ConfigKey(m_sGroup2, "sync_key_leader")));

    // A short tap leaves sync, which clears the crown. The key stays.
    ControlObject::set(ConfigKey(m_sGroup1, "sync_key"), 0.0);
    EXPECT_DOUBLE_EQ(pitch, ControlObject::get(pitchKey));
    EXPECT_DOUBLE_EQ(0.0, ControlObject::get(ConfigKey(m_sGroup1, "sync_key")));
    EXPECT_DOUBLE_EQ(0.0, ControlObject::get(ConfigKey(m_sGroup1, "sync_key_leader")));

    const LoadResult loaded = loadDeck1(kLoadedKey, 4.0);
    EXPECT_DOUBLE_EQ(loaded.pitchBefore, loaded.pitch);
    EXPECT_DOUBLE_EQ(0.0, ControlObject::get(ConfigKey(m_sGroup1, "sync_key_leader")));
}

TEST_F(AutoHarmonizeManualLoadTest, FollowerSyncMatchesLeader) {
    setOption(false);
    const OtherDeckKey other = publishOtherDeckKey(m_sGroup2, 0.0);
    ASSERT_NE(mixxx::track::io::key::INVALID, other.key);
    const double leaderPitch = ControlObject::get(ConfigKey(m_sGroup2, "pitch"));
    ControlObject::set(ConfigKey(m_sGroup2, "sync_key"), 1.0);
    EXPECT_DOUBLE_EQ(leaderPitch, ControlObject::get(ConfigKey(m_sGroup2, "pitch")));
    EXPECT_DOUBLE_EQ(1.0, ControlObject::get(ConfigKey(m_sGroup2, "sync_key_leader")));

    ControlObject::set(ConfigKey(m_sGroup1, "file_key"),
            KeyUtils::keyToNumericValue(kLoadedKey));
    const double expected = compatibleOffset(kLoadedKey, other.key, other.distance);
    ASSERT_NE(0.0, expected);
    const ConfigKey pitchKey(m_sGroup1, "pitch");
    ControlObject::set(pitchKey, 4.0);
    ASSERT_NE(ControlObject::get(pitchKey), storedMatchKeyPitch(expected));

    ControlObject::set(ConfigKey(m_sGroup1, "sync_key"), 1.0);
    EXPECT_DOUBLE_EQ(storedMatchKeyPitch(expected), ControlObject::get(pitchKey));
    EXPECT_DOUBLE_EQ(0.0, ControlObject::get(ConfigKey(m_sGroup1, "sync_key_leader")));
    EXPECT_DOUBLE_EQ(1.0, ControlObject::get(ConfigKey(m_sGroup2, "sync_key_leader")));

    // Turning the follower's sync off leaves the leader's crown alone.
    const double leaderPitchAfter = ControlObject::get(ConfigKey(m_sGroup2, "pitch"));
    ControlObject::set(ConfigKey(m_sGroup1, "sync_key"), 0.0);
    EXPECT_DOUBLE_EQ(0.0, ControlObject::get(ConfigKey(m_sGroup1, "sync_key")));
    EXPECT_DOUBLE_EQ(0.0, ControlObject::get(ConfigKey(m_sGroup1, "sync_key_leader")));
    EXPECT_DOUBLE_EQ(1.0, ControlObject::get(ConfigKey(m_sGroup2, "sync_key")));
    EXPECT_DOUBLE_EQ(1.0, ControlObject::get(ConfigKey(m_sGroup2, "sync_key_leader")));
    EXPECT_DOUBLE_EQ(leaderPitchAfter, ControlObject::get(ConfigKey(m_sGroup2, "pitch")));
    EXPECT_DOUBLE_EQ(storedMatchKeyPitch(expected), ControlObject::get(pitchKey));
}

TEST_F(AutoHarmonizeManualLoadTest, LatchedFollowerHarmonizesWhenOptionOff) {
    setOption(false);
    const OtherDeckKey other = publishOtherDeckKey(m_sGroup2, 0.3);
    const double expected = compatibleOffset(kLoadedKey, other.key, other.distance);
    ASSERT_NE(0.0, expected);
    ControlObject::set(ConfigKey(m_sGroup2, "sync_key"), 1.0);
    EXPECT_DOUBLE_EQ(1.0, ControlObject::get(ConfigKey(m_sGroup2, "sync_key_leader")));

    ControlObject::set(ConfigKey(m_sGroup1, "sync_key"), 1.0);
    const LoadResult loaded = loadDeck1(kLoadedKey, 4.0);
    EXPECT_DOUBLE_EQ(storedMatchKeyPitch(expected), loaded.pitch);
    EXPECT_NE(expected * 2.0, loaded.pitch);
    // The load must not release the latch, and must not take the crown.
    EXPECT_DOUBLE_EQ(1.0, ControlObject::get(ConfigKey(m_sGroup1, "sync_key")));
    EXPECT_DOUBLE_EQ(0.0, ControlObject::get(ConfigKey(m_sGroup1, "sync_key_leader")));
    EXPECT_EQ(kLoadedKey, m_pTrack1->getKey());
}

TEST_F(AutoHarmonizeManualLoadTest, LeaderPitchChangeRetunesSyncedDeck) {
    setOption(false);
    publishOtherDeckKey(m_sGroup2, 0.0);
    ControlObject::set(ConfigKey(m_sGroup2, "sync_key"), 1.0);
    EXPECT_DOUBLE_EQ(1.0, ControlObject::get(ConfigKey(m_sGroup2, "sync_key_leader")));

    ControlObject::set(ConfigKey(m_sGroup1, "file_key"),
            KeyUtils::keyToNumericValue(kLoadedKey));
    ControlObject::set(ConfigKey(m_sGroup3, "file_key"),
            KeyUtils::keyToNumericValue(kLoadedKey));
    const ConfigKey followerPitch(m_sGroup1, "pitch");
    const ConfigKey idlePitch(m_sGroup3, "pitch");
    ControlObject::set(followerPitch, 4.0);
    ControlObject::set(idlePitch, 4.0);
    const double idleBefore = ControlObject::get(idlePitch);
    ControlObject::set(ConfigKey(m_sGroup1, "sync_key"), 1.0);
    const double matched = ControlObject::get(followerPitch);

    ControlObject::set(ConfigKey(m_sGroup2, "pitch"), 1.0);
    const auto leaderKey = KeyUtils::keyFromNumericValue(
            ControlObject::get(ConfigKey(m_sGroup2, "key")));
    const double leaderDistance =
            ControlObject::get(ConfigKey(m_sGroup2, "visual_key_distance"));
    ASSERT_NE(mixxx::track::io::key::INVALID, leaderKey);
    const double expected = compatibleOffset(kLoadedKey, leaderKey, leaderDistance);
    ASSERT_NE(matched, storedMatchKeyPitch(expected));
    EXPECT_DOUBLE_EQ(storedMatchKeyPitch(expected), ControlObject::get(followerPitch));
    EXPECT_DOUBLE_EQ(idleBefore, ControlObject::get(idlePitch));
    EXPECT_DOUBLE_EQ(1.0, ControlObject::get(ConfigKey(m_sGroup2, "sync_key_leader")));
    EXPECT_DOUBLE_EQ(0.0, ControlObject::get(ConfigKey(m_sGroup1, "sync_key_leader")));

    // Editing the follower must not drag the leader.
    const double leaderPitch = ControlObject::get(ConfigKey(m_sGroup2, "pitch"));
    ControlObject::set(followerPitch, 3.0);
    EXPECT_DOUBLE_EQ(leaderPitch, ControlObject::get(ConfigKey(m_sGroup2, "pitch")));
}

TEST_F(AutoHarmonizeManualLoadTest, FollowerSyncExactKeyMatchesTonic) {
    setOption(false);
    setExact(true);
    const OtherDeckKey other = publishOtherDeckKey(m_sGroup2, 0.0);
    ASSERT_NE(mixxx::track::io::key::INVALID, other.key);
    const double exact = exactOffset(kLoadedKey, other.key, other.distance);
    const double compatible = compatibleOffset(kLoadedKey, other.key, other.distance);
    ASSERT_NE(exact, compatible);

    ControlObject::set(ConfigKey(m_sGroup2, "sync_key"), 1.0);
    EXPECT_DOUBLE_EQ(1.0, ControlObject::get(ConfigKey(m_sGroup2, "sync_key_leader")));

    ControlObject::set(ConfigKey(m_sGroup1, "file_key"),
            KeyUtils::keyToNumericValue(kLoadedKey));
    const ConfigKey pitchKey(m_sGroup1, "pitch");
    ControlObject::set(pitchKey, 4.0);

    ControlObject::set(ConfigKey(m_sGroup1, "sync_key"), 1.0);
    EXPECT_DOUBLE_EQ(storedMatchKeyPitch(exact), ControlObject::get(pitchKey));
    EXPECT_NE(storedMatchKeyPitch(compatible), ControlObject::get(pitchKey));
    EXPECT_DOUBLE_EQ(0.0, ControlObject::get(ConfigKey(m_sGroup1, "sync_key_leader")));
    EXPECT_DOUBLE_EQ(1.0, ControlObject::get(ConfigKey(m_sGroup2, "sync_key_leader")));
}

TEST_F(AutoHarmonizeManualLoadTest, ManualLoadExactKeyWhenBothOptionsOn) {
    setOption(true);
    setExact(true);
    const OtherDeckKey other = publishOtherDeckKey(m_sGroup2, 0.0);
    const double exact = exactOffset(kLoadedKey, other.key, other.distance);
    const double compatible = compatibleOffset(kLoadedKey, other.key, other.distance);
    ASSERT_NE(exact, compatible);
    ASSERT_NE(0.0, exact);

    const LoadResult loaded = loadDeck1(kLoadedKey, 4.0);
    EXPECT_DOUBLE_EQ(storedMatchKeyPitch(exact), loaded.pitch);
    EXPECT_NE(storedMatchKeyPitch(compatible), loaded.pitch);
    EXPECT_EQ(kLoadedKey, m_pTrack1->getKey());
}

TEST_F(AutoHarmonizeManualLoadTest, LeaderPitchChangeExactKeyRetunesSyncedDeck) {
    setOption(false);
    setExact(true);
    publishOtherDeckKey(m_sGroup2, 0.0);
    ControlObject::set(ConfigKey(m_sGroup2, "sync_key"), 1.0);
    EXPECT_DOUBLE_EQ(1.0, ControlObject::get(ConfigKey(m_sGroup2, "sync_key_leader")));

    ControlObject::set(ConfigKey(m_sGroup1, "file_key"),
            KeyUtils::keyToNumericValue(kLoadedKey));
    const ConfigKey followerPitch(m_sGroup1, "pitch");
    ControlObject::set(followerPitch, 4.0);
    ControlObject::set(ConfigKey(m_sGroup1, "sync_key"), 1.0);
    const double matched = ControlObject::get(followerPitch);

    ControlObject::set(ConfigKey(m_sGroup2, "pitch"), 1.0);
    const auto leaderKey = KeyUtils::keyFromNumericValue(
            ControlObject::get(ConfigKey(m_sGroup2, "key")));
    const double leaderDistance =
            ControlObject::get(ConfigKey(m_sGroup2, "visual_key_distance"));
    ASSERT_NE(mixxx::track::io::key::INVALID, leaderKey);
    const double exact = exactOffset(kLoadedKey, leaderKey, leaderDistance);
    const double compatible = compatibleOffset(kLoadedKey, leaderKey, leaderDistance);
    ASSERT_NE(exact, compatible);
    ASSERT_NE(matched, storedMatchKeyPitch(exact));
    EXPECT_DOUBLE_EQ(storedMatchKeyPitch(exact), ControlObject::get(followerPitch));
    EXPECT_NE(storedMatchKeyPitch(compatible), ControlObject::get(followerPitch));
}

TEST_F(AutoHarmonizeManualLoadTest, ResetKeyReleasesSyncLatch) {
    setOption(false);
    publishOtherDeckKey(m_sGroup2, 0.0);
    ControlObject::set(ConfigKey(m_sGroup2, "sync_key"), 1.0);
    EXPECT_DOUBLE_EQ(1.0, ControlObject::get(ConfigKey(m_sGroup2, "sync_key_leader")));

    ControlObject::set(ConfigKey(m_sGroup1, "file_key"),
            KeyUtils::keyToNumericValue(kLoadedKey));
    const ConfigKey pitchKey(m_sGroup1, "pitch");
    ControlObject::set(pitchKey, 4.0);
    ControlObject::set(ConfigKey(m_sGroup1, "sync_key"), 1.0);
    const double matched = ControlObject::get(pitchKey);
    ASSERT_NE(0.0, matched);
    ASSERT_NE(4.0, matched);

    // Right-click reset on the note. The latch must drop or the leader
    // would pull this deck back onto the harmonized key.
    ControlObject::set(ConfigKey(m_sGroup1, "reset_key"), 1.0);
    EXPECT_DOUBLE_EQ(0.0, ControlObject::get(pitchKey));
    EXPECT_DOUBLE_EQ(0.0, ControlObject::get(ConfigKey(m_sGroup1, "sync_key")));
    EXPECT_DOUBLE_EQ(0.0, ControlObject::get(ConfigKey(m_sGroup1, "sync_key_leader")));
    EXPECT_DOUBLE_EQ(1.0, ControlObject::get(ConfigKey(m_sGroup2, "sync_key")));
    EXPECT_DOUBLE_EQ(1.0, ControlObject::get(ConfigKey(m_sGroup2, "sync_key_leader")));

    ControlObject::set(ConfigKey(m_sGroup2, "pitch"), 1.0);
    const auto leaderKey = KeyUtils::keyFromNumericValue(
            ControlObject::get(ConfigKey(m_sGroup2, "key")));
    const double leaderDistance =
            ControlObject::get(ConfigKey(m_sGroup2, "visual_key_distance"));
    ASSERT_NE(mixxx::track::io::key::INVALID, leaderKey);
    const double pulled = compatibleOffset(kLoadedKey, leaderKey, leaderDistance);
    // That leader move would have changed this deck if sync were still on.
    ASSERT_NE(0.0, storedMatchKeyPitch(pulled));
    EXPECT_DOUBLE_EQ(0.0, ControlObject::get(pitchKey));
    EXPECT_DOUBLE_EQ(1.0, ControlObject::get(ConfigKey(m_sGroup2, "pitch")));

    ControlObject::set(ConfigKey(m_sGroup1, "reset_key"), 0.0);
    EXPECT_DOUBLE_EQ(0.0, ControlObject::get(ConfigKey(m_sGroup1, "sync_key")));
    EXPECT_DOUBLE_EQ(0.0, ControlObject::get(pitchKey));
}

TEST_F(AutoHarmonizeManualLoadTest, KeyLeaderAboveSoftStaysOn) {
    setOption(false);
    publishOtherDeckKey(m_sGroup2, 0.0);
    ControlObject::set(ConfigKey(m_sGroup1, "file_key"),
            KeyUtils::keyToNumericValue(kLoadedKey));
    // A request for the explicit light is folded back to the one crown.
    ControlObject::set(ConfigKey(m_sGroup1, "sync_key_leader"), 2.0);
    EXPECT_DOUBLE_EQ(1.0, ControlObject::get(ConfigKey(m_sGroup1, "sync_key_leader")));
    EXPECT_DOUBLE_EQ(1.0, ControlObject::get(ConfigKey(m_sGroup1, "sync_key")));
    const ConfigKey pitchKey(m_sGroup1, "pitch");
    ControlObject::set(pitchKey, 4.0);
    const double pitch = ControlObject::get(pitchKey);

    ControlObject::set(ConfigKey(m_sGroup1, "sync_key"), 1.0);
    EXPECT_DOUBLE_EQ(pitch, ControlObject::get(pitchKey));
    EXPECT_DOUBLE_EQ(1.0, ControlObject::get(ConfigKey(m_sGroup1, "sync_key_leader")));

    // Clicking the lit crown asks for 2 and stays on this key.
    ControlObject::set(ConfigKey(m_sGroup1, "sync_key_leader"), 2.0);
    EXPECT_DOUBLE_EQ(1.0, ControlObject::get(ConfigKey(m_sGroup1, "sync_key_leader")));
    EXPECT_DOUBLE_EQ(pitch, ControlObject::get(pitchKey));
    EXPECT_DOUBLE_EQ(1.0, ControlObject::get(ConfigKey(m_sGroup1, "sync_key")));
}

TEST_F(AutoHarmonizeManualLoadTest, CrownClickTurnsSyncOn) {
    setOption(false);
    publishOtherDeckKey(m_sGroup2, 0.0);
    ControlObject::set(ConfigKey(m_sGroup1, "file_key"),
            KeyUtils::keyToNumericValue(kLoadedKey));
    const ConfigKey pitchKey(m_sGroup1, "pitch");
    ControlObject::set(pitchKey, 4.0);
    const double pitch = ControlObject::get(pitchKey);

    // The crown latches sync immediately and does not retune.
    ControlObject::set(ConfigKey(m_sGroup1, "sync_key_leader"), 1.0);
    EXPECT_DOUBLE_EQ(1.0, ControlObject::get(ConfigKey(m_sGroup1, "sync_key")));
    EXPECT_DOUBLE_EQ(pitch, ControlObject::get(pitchKey));
    EXPECT_DOUBLE_EQ(1.0, ControlObject::get(ConfigKey(m_sGroup1, "sync_key_leader")));
    EXPECT_DOUBLE_EQ(0.0, ControlObject::get(ConfigKey(m_sGroup2, "sync_key")));

    // Crown off leaves sync latched, the way a beat follower keeps sync.
    ControlObject::set(ConfigKey(m_sGroup1, "sync_key_leader"), 0.0);
    EXPECT_DOUBLE_EQ(1.0, ControlObject::get(ConfigKey(m_sGroup1, "sync_key")));
    EXPECT_DOUBLE_EQ(0.0, ControlObject::get(ConfigKey(m_sGroup1, "sync_key_leader")));
    EXPECT_DOUBLE_EQ(pitch, ControlObject::get(pitchKey));
}

TEST_F(AutoHarmonizeManualLoadTest, SyncOffClearsCrownAndPromotesFollower) {
    setOption(false);
    publishOtherDeckKey(m_sGroup2, 0.0);
    const double leaderPitch = ControlObject::get(ConfigKey(m_sGroup2, "pitch"));
    ControlObject::set(ConfigKey(m_sGroup2, "sync_key"), 1.0);
    EXPECT_DOUBLE_EQ(1.0, ControlObject::get(ConfigKey(m_sGroup2, "sync_key_leader")));
    EXPECT_DOUBLE_EQ(leaderPitch, ControlObject::get(ConfigKey(m_sGroup2, "pitch")));

    ControlObject::set(ConfigKey(m_sGroup1, "file_key"),
            KeyUtils::keyToNumericValue(kLoadedKey));
    const ConfigKey pitchKey(m_sGroup1, "pitch");
    ControlObject::set(pitchKey, 4.0);
    ControlObject::set(ConfigKey(m_sGroup1, "sync_key"), 1.0);
    const double followerPitch = ControlObject::get(pitchKey);
    ASSERT_NE(0.0, followerPitch);
    EXPECT_DOUBLE_EQ(0.0, ControlObject::get(ConfigKey(m_sGroup1, "sync_key_leader")));

    // The old leader drops the crown. The follower takes it and keeps its key.
    ControlObject::set(ConfigKey(m_sGroup2, "sync_key"), 0.0);
    EXPECT_DOUBLE_EQ(0.0, ControlObject::get(ConfigKey(m_sGroup2, "sync_key")));
    EXPECT_DOUBLE_EQ(0.0, ControlObject::get(ConfigKey(m_sGroup2, "sync_key_leader")));
    EXPECT_DOUBLE_EQ(leaderPitch, ControlObject::get(ConfigKey(m_sGroup2, "pitch")));
    EXPECT_DOUBLE_EQ(1.0, ControlObject::get(ConfigKey(m_sGroup1, "sync_key")));
    EXPECT_DOUBLE_EQ(1.0, ControlObject::get(ConfigKey(m_sGroup1, "sync_key_leader")));
    EXPECT_DOUBLE_EQ(followerPitch, ControlObject::get(pitchKey));
}

TEST_F(AutoHarmonizeManualLoadTest, ResetLeaderKeepsSyncAndRetunesFollower) {
    setOption(false);
    publishOtherDeckKey(m_sGroup2, 0.0);
    ControlObject::set(ConfigKey(m_sGroup2, "sync_key"), 1.0);
    EXPECT_DOUBLE_EQ(1.0, ControlObject::get(ConfigKey(m_sGroup2, "sync_key_leader")));

    ControlObject::set(ConfigKey(m_sGroup1, "file_key"),
            KeyUtils::keyToNumericValue(kLoadedKey));
    ControlObject::set(ConfigKey(m_sGroup3, "file_key"),
            KeyUtils::keyToNumericValue(kLoadedKey));
    const ConfigKey pitchKey(m_sGroup1, "pitch");
    const ConfigKey idlePitch(m_sGroup3, "pitch");
    ControlObject::set(pitchKey, 4.0);
    ControlObject::set(idlePitch, 4.0);
    ControlObject::set(ConfigKey(m_sGroup1, "sync_key"), 1.0);
    const double idleBefore = ControlObject::get(idlePitch);
    // Move the leader off the original key so the follower is not already
    // on the reset target.
    ControlObject::set(ConfigKey(m_sGroup2, "pitch"), 1.0);
    const double followerPitch = ControlObject::get(pitchKey);
    ASSERT_NE(0.0, followerPitch);
    ASSERT_NE(0.0, ControlObject::get(ConfigKey(m_sGroup2, "pitch")));

    // The leader keeps sync and the crown. Other synced decks match the
    // restored key. A deck with sync off stays put.
    ControlObject::set(ConfigKey(m_sGroup2, "reset_key"), 1.0);
    EXPECT_DOUBLE_EQ(0.0, ControlObject::get(ConfigKey(m_sGroup2, "pitch")));
    EXPECT_DOUBLE_EQ(1.0, ControlObject::get(ConfigKey(m_sGroup2, "sync_key")));
    EXPECT_DOUBLE_EQ(1.0, ControlObject::get(ConfigKey(m_sGroup2, "sync_key_leader")));
    EXPECT_DOUBLE_EQ(1.0, ControlObject::get(ConfigKey(m_sGroup1, "sync_key")));
    EXPECT_DOUBLE_EQ(0.0, ControlObject::get(ConfigKey(m_sGroup1, "sync_key_leader")));
    const auto leaderKey = KeyUtils::keyFromNumericValue(
            ControlObject::get(ConfigKey(m_sGroup2, "key")));
    const double leaderDistance =
            ControlObject::get(ConfigKey(m_sGroup2, "visual_key_distance"));
    ASSERT_NE(mixxx::track::io::key::INVALID, leaderKey);
    const double expected = compatibleOffset(kLoadedKey, leaderKey, leaderDistance);
    ASSERT_NE(followerPitch, storedMatchKeyPitch(expected));
    EXPECT_DOUBLE_EQ(storedMatchKeyPitch(expected), ControlObject::get(pitchKey));
    EXPECT_DOUBLE_EQ(idleBefore, ControlObject::get(idlePitch));
    EXPECT_DOUBLE_EQ(0.0, ControlObject::get(ConfigKey(m_sGroup3, "sync_key")));

    ControlObject::set(ConfigKey(m_sGroup2, "reset_key"), 0.0);
    EXPECT_DOUBLE_EQ(1.0, ControlObject::get(ConfigKey(m_sGroup2, "sync_key")));
    EXPECT_DOUBLE_EQ(1.0, ControlObject::get(ConfigKey(m_sGroup2, "sync_key_leader")));
    EXPECT_DOUBLE_EQ(storedMatchKeyPitch(expected), ControlObject::get(pitchKey));
}

TEST_F(AutoHarmonizeManualLoadTest, CrownTakeoverKeepsOwnKey) {
    setOption(false);
    publishOtherDeckKey(m_sGroup2, 0.0);
    ControlObject::set(ConfigKey(m_sGroup2, "sync_key_leader"), 2.0);
    EXPECT_DOUBLE_EQ(1.0, ControlObject::get(ConfigKey(m_sGroup2, "sync_key_leader")));
    EXPECT_DOUBLE_EQ(1.0, ControlObject::get(ConfigKey(m_sGroup2, "sync_key")));
    const double previousLeaderPitch = ControlObject::get(ConfigKey(m_sGroup2, "pitch"));

    ControlObject::set(ConfigKey(m_sGroup1, "file_key"),
            KeyUtils::keyToNumericValue(kLoadedKey));
    const ConfigKey pitchKey(m_sGroup1, "pitch");
    ControlObject::set(pitchKey, 4.0);
    const double pitch = ControlObject::get(pitchKey);
    const auto leaderKey = KeyUtils::keyFromNumericValue(
            ControlObject::get(ConfigKey(m_sGroup1, "key")));
    const double leaderDistance =
            ControlObject::get(ConfigKey(m_sGroup1, "visual_key_distance"));
    ASSERT_NE(mixxx::track::io::key::INVALID, leaderKey);
    const double expected = compatibleOffset(kOtherKey, leaderKey, leaderDistance);
    ASSERT_NE(previousLeaderPitch, storedMatchKeyPitch(expected));

    // Taking the crown does not retune this deck. The old leader keeps sync
    // and matches this key.
    ControlObject::set(ConfigKey(m_sGroup1, "sync_key_leader"), 1.0);
    EXPECT_DOUBLE_EQ(1.0, ControlObject::get(ConfigKey(m_sGroup1, "sync_key")));
    EXPECT_DOUBLE_EQ(pitch, ControlObject::get(pitchKey));
    EXPECT_DOUBLE_EQ(1.0, ControlObject::get(ConfigKey(m_sGroup1, "sync_key_leader")));
    EXPECT_DOUBLE_EQ(0.0, ControlObject::get(ConfigKey(m_sGroup2, "sync_key_leader")));
    EXPECT_DOUBLE_EQ(1.0, ControlObject::get(ConfigKey(m_sGroup2, "sync_key")));
    EXPECT_DOUBLE_EQ(storedMatchKeyPitch(expected),
            ControlObject::get(ConfigKey(m_sGroup2, "pitch")));
}

TEST_F(AutoHarmonizeManualLoadTest, ExplicitCrownClickTurnsSyncOn) {
    setOption(false);
    publishOtherDeckKey(m_sGroup2, 0.0);
    ControlObject::set(ConfigKey(m_sGroup2, "sync_key_leader"), 1.0);
    EXPECT_DOUBLE_EQ(1.0, ControlObject::get(ConfigKey(m_sGroup2, "sync_key")));

    ControlObject::set(ConfigKey(m_sGroup1, "file_key"),
            KeyUtils::keyToNumericValue(kLoadedKey));
    const ConfigKey pitchKey(m_sGroup1, "pitch");
    ControlObject::set(pitchKey, 4.0);
    const double pitch = ControlObject::get(pitchKey);

    ControlObject::set(ConfigKey(m_sGroup1, "sync_key_leader"), 2.0);
    EXPECT_DOUBLE_EQ(1.0, ControlObject::get(ConfigKey(m_sGroup1, "sync_key")));
    EXPECT_DOUBLE_EQ(pitch, ControlObject::get(pitchKey));
    EXPECT_DOUBLE_EQ(1.0, ControlObject::get(ConfigKey(m_sGroup1, "sync_key_leader")));
    EXPECT_DOUBLE_EQ(0.0, ControlObject::get(ConfigKey(m_sGroup2, "sync_key_leader")));
    // Demoting the other crown does not release its sync latch.
    EXPECT_DOUBLE_EQ(1.0, ControlObject::get(ConfigKey(m_sGroup2, "sync_key")));
}

TEST_F(AutoHarmonizeManualLoadTest, MatchKeyDoesNotAdoptLeader) {
    setOption(false);
    publishOtherDeckKey(m_sGroup2, 0.0);
    ControlObject::set(ConfigKey(m_sGroup1, "file_key"),
            KeyUtils::keyToNumericValue(kLoadedKey));
    ControlObject::set(ConfigKey(m_sGroup1, "pitch"), 4.0);

    ControlObject::set(ConfigKey(m_sGroup1, "match_key"), 1.0);
    EXPECT_DOUBLE_EQ(0.0, ControlObject::get(ConfigKey(m_sGroup1, "sync_key")));
    EXPECT_DOUBLE_EQ(0.0, ControlObject::get(ConfigKey(m_sGroup1, "sync_key_leader")));
    EXPECT_DOUBLE_EQ(0.0, ControlObject::get(ConfigKey(m_sGroup2, "sync_key_leader")));
}

TEST_F(AutoHarmonizeManualLoadTest, ExplicitLeaderIsMatchTarget) {
    setOption(true);
    const OtherDeckKey other = publishOtherDeckKey(m_sGroup2, 0.0);
    ControlObject::set(ConfigKey(m_sGroup3, "file_key"),
            KeyUtils::keyToNumericValue(mixxx::track::io::key::A_MINOR));
    ControlObject::set(ConfigKey(m_sGroup3, "pitch"), 0.3);
    const auto leaderKey = KeyUtils::keyFromNumericValue(
            ControlObject::get(ConfigKey(m_sGroup3, "key")));
    const double leaderDistance =
            ControlObject::get(ConfigKey(m_sGroup3, "visual_key_distance"));
    ASSERT_NE(mixxx::track::io::key::INVALID, leaderKey);
    const double expected = compatibleOffset(kLoadedKey, leaderKey, leaderDistance);
    const double otherOffset = compatibleOffset(kLoadedKey, other.key, other.distance);
    ASSERT_NE(expected, otherOffset);

    ControlObject::set(ConfigKey(m_sGroup3, "sync_key_leader"), 2.0);
    EXPECT_DOUBLE_EQ(0.0, ControlObject::get(ConfigKey(m_sGroup1, "sync_key_leader")));
    EXPECT_DOUBLE_EQ(0.0, ControlObject::get(ConfigKey(m_sGroup2, "sync_key_leader")));

    const LoadResult loaded = loadDeck1(kLoadedKey, 4.0);
    EXPECT_DOUBLE_EQ(storedMatchKeyPitch(expected), loaded.pitch);
    EXPECT_EQ(kLoadedKey, m_pTrack1->getKey());
}

TEST_F(AutoHarmonizeManualLoadTest, SoftLeaderIsMatchTarget) {
    setOption(true);
    const OtherDeckKey other = publishOtherDeckKey(m_sGroup2, 0.0);
    ControlObject::set(ConfigKey(m_sGroup3, "file_key"),
            KeyUtils::keyToNumericValue(mixxx::track::io::key::A_MINOR));
    ControlObject::set(ConfigKey(m_sGroup3, "pitch"), 0.3);
    const auto leaderKey = KeyUtils::keyFromNumericValue(
            ControlObject::get(ConfigKey(m_sGroup3, "key")));
    const double leaderDistance =
            ControlObject::get(ConfigKey(m_sGroup3, "visual_key_distance"));
    const double expected = compatibleOffset(kLoadedKey, leaderKey, leaderDistance);
    const double otherOffset = compatibleOffset(kLoadedKey, other.key, other.distance);
    ASSERT_NE(expected, otherOffset);

    ControlObject::set(ConfigKey(m_sGroup3, "sync_key_leader"), 1.0);

    const LoadResult loaded = loadDeck1(kLoadedKey, 4.0);
    EXPECT_DOUBLE_EQ(storedMatchKeyPitch(expected), loaded.pitch);
}

TEST_F(AutoHarmonizeManualLoadTest, LeaderDeckLoadKeepsPitch) {
    setOption(true);
    publishOtherDeckKey(m_sGroup2, 0.3);
    ControlObject::set(ConfigKey(m_sGroup1, "sync_key_leader"), 2.0);

    const LoadResult loaded = loadDeck1(kLoadedKey, 4.0);
    EXPECT_DOUBLE_EQ(loaded.pitchBefore, loaded.pitch);
    EXPECT_EQ(kLoadedKey, m_pTrack1->getKey());
}

TEST_F(AutoHarmonizeManualLoadTest, NextLoadFollowsChangedLeaderKey) {
    setOption(true);
    publishOtherDeckKey(m_sGroup2, 0.0);
    const LoadResult first = loadDeck1(kLoadedKey, 4.0);

    const OtherDeckKey shifted = publishOtherDeckKey(m_sGroup2, 0.3);
    const double expected = compatibleOffset(kLoadedKey, shifted.key, shifted.distance);
    ASSERT_NE(storedMatchKeyPitch(expected), first.pitch);

    const LoadResult second = loadDeck1(kLoadedKey, first.pitch);
    EXPECT_DOUBLE_EQ(storedMatchKeyPitch(expected), second.pitch);
    EXPECT_NE(expected * 2.0, second.pitch);
    EXPECT_EQ(kLoadedKey, m_pTrack1->getKey());
}

TEST_F(AutoHarmonizeManualLoadTest, RetiredAutoDjKeyHarmonizesUntilDecksKeyExists) {
    config()->setValue(kRetiredAutoDjKey, true);
    const OtherDeckKey other = publishOtherDeckKey(m_sGroup2, 0.0);
    const double expected = compatibleOffset(kLoadedKey, other.key, other.distance);
    ASSERT_NE(0.0, expected);

    const LoadResult loaded = loadDeck1(kLoadedKey, 4.0);
    EXPECT_DOUBLE_EQ(storedMatchKeyPitch(expected), loaded.pitch);
}

TEST_F(AutoHarmonizeManualLoadTest, DecksKeyOffIgnoresRetiredKeys) {
    setOption(false);
    config()->setValue(kRetiredAutoDjKey, true);
    config()->setValue(kRetiredManualKey, true);
    publishOtherDeckKey(m_sGroup2, 0.3);

    const LoadResult loaded = loadDeck1(kLoadedKey, 4.0);
    EXPECT_DOUBLE_EQ(loaded.pitchBefore, loaded.pitch);
}

TEST_F(AutoHarmonizeManualLoadTest, AutoDjMarkBlocksManualHarmonize) {
    setOption(true);
    publishOtherDeckKey(m_sGroup2, 0.3);
    m_pMixerDeck1->setPendingAutoDjAutoload(true);

    const LoadResult loaded = loadDeck1(kLoadedKey, 4.0);
    EXPECT_DOUBLE_EQ(loaded.pitchBefore, loaded.pitch);
    // The manual path must not consume the AutoDJ autoload mark.
    EXPECT_TRUE(m_pMixerDeck1->isPendingAutoDjAutoload());
    EXPECT_EQ(kLoadedKey, m_pTrack1->getKey());
}

TEST_F(AutoHarmonizeManualLoadTest, InvalidKeyLeavesPitchAndStoredKey) {
    setOption(true);
    publishOtherDeckKey(m_sGroup2, 0.0);
    const LoadResult loaded = loadDeck1(mixxx::track::io::key::INVALID, 4.0);
    EXPECT_DOUBLE_EQ(loaded.pitchBefore, loaded.pitch);
    EXPECT_EQ(mixxx::track::io::key::INVALID, m_pTrack1->getKey());
    EXPECT_EQ(m_pTrack1, m_pMixerDeck1->getLoadedTrack());
}

TEST_F(AutoHarmonizeManualLoadTest, CompatiblePairWritesZeroOffset) {
    setOption(true);
    const OtherDeckKey other = publishOtherDeckKey(m_sGroup2, 0.0);
    const double expected = compatibleOffset(kOtherKey, other.key, other.distance);
    EXPECT_DOUBLE_EQ(0.0, expected);

    const LoadResult loaded = loadDeck1(kOtherKey, 4.0);
    EXPECT_DOUBLE_EQ(expected, loaded.pitch);
    EXPECT_EQ(kOtherKey, m_pTrack1->getKey());
}

TEST_F(AutoHarmonizeManualLoadTest, PreviewLoadIsNotHarmonized) {
    setOption(true);
    publishOtherDeckKey(m_sGroup2, 0.3);
    TrackPointer pPreview = m_pPreview1->loadFakeTrack(false, 120.0);
    pPreview->setKey(kLoadedKey, mixxx::track::io::key::USER);
    const ConfigKey previewPitch(m_sPreviewGroup, "pitch");
    ControlObject::set(previewPitch, 2.5);
    const double pitchBefore = ControlObject::get(previewPitch);

    m_pPreview1->slotTrackLoaded(pPreview, TrackPointer());

    EXPECT_DOUBLE_EQ(pitchBefore, ControlObject::get(previewPitch));
    EXPECT_EQ(kLoadedKey, pPreview->getKey());
    EXPECT_EQ(pPreview, m_pPreview1->getLoadedTrack());
}

TEST_F(MixxxTest, HarmonizePreferenceIsOnDecksPage) {
    ControlObject numDecks(ConfigKey(QStringLiteral("[App]"), QStringLiteral("num_decks")));
    ControlObject numSamplers(ConfigKey(QStringLiteral("[App]"), QStringLiteral("num_samplers")));
    numDecks.set(1.0);
    numSamplers.set(0.0);
    ControlObject rate(ConfigKey(QStringLiteral("[Channel1]"), QStringLiteral("rate")));
    ControlObject rateRange(ConfigKey(QStringLiteral("[Channel1]"), QStringLiteral("rateRange")));
    ControlObject rateDir(ConfigKey(QStringLiteral("[Channel1]"), QStringLiteral("rate_dir")));
    ControlObject cueMode(ConfigKey(QStringLiteral("[Channel1]"), QStringLiteral("cue_mode")));
    ControlObject keylockMode(ConfigKey(
            QStringLiteral("[Channel1]"), QStringLiteral("keylockMode")));
    ControlObject keyunlockMode(
            ConfigKey(QStringLiteral("[Channel1]"), QStringLiteral("keyunlockMode")));

    DlgPrefDeck deckPage(nullptr, config());
    QCheckBox* pBox = deckPage.findChild<QCheckBox*>(
            QStringLiteral("checkBoxAutoHarmonizeOnLoad"));
    ASSERT_NE(pBox, nullptr);
    EXPECT_FALSE(pBox->isChecked());
    // Unset key uses the false default. A true default here would hide that.
    EXPECT_FALSE(config()->getValue<bool>(kAutoHarmonizeOnLoadConfigKey, false));
    EXPECT_FALSE(config()->exists(kRetiredAutoDjKey));
    EXPECT_FALSE(config()->exists(kRetiredManualKey));

    pBox->setChecked(true);
    deckPage.slotApply();
    EXPECT_TRUE(config()->getValue<bool>(kAutoHarmonizeOnLoadConfigKey, false));
    EXPECT_FALSE(config()->exists(kRetiredAutoDjKey));
    EXPECT_FALSE(config()->exists(kRetiredManualKey));

    deckPage.slotResetToDefaults();
    EXPECT_FALSE(pBox->isChecked());
    // Reset changes the widget only until Apply.
    EXPECT_TRUE(config()->getValue<bool>(kAutoHarmonizeOnLoadConfigKey, false));
    deckPage.slotApply();
    // Stored false must win over a true fallback, so this is not "unset".
    EXPECT_FALSE(config()->getValue<bool>(kAutoHarmonizeOnLoadConfigKey, true));

    pBox->setChecked(true);
    deckPage.slotUpdate();
    EXPECT_FALSE(pBox->isChecked());
    EXPECT_FALSE(config()->getValue<bool>(kAutoHarmonizeOnLoadConfigKey, true));

    QCheckBox* pExact = deckPage.findChild<QCheckBox*>(
            QStringLiteral("checkBoxExactKeyMatch"));
    QLabel* pLabel = deckPage.findChild<QLabel*>(
            QStringLiteral("labelExactKeyMatch"));
    ASSERT_NE(pExact, nullptr);
    ASSERT_NE(pLabel, nullptr);
    EXPECT_EQ(QStringLiteral("Exact key"), pLabel->text());
    EXPECT_EQ(QStringLiteral("Match the other deck's key exactly"), pExact->text());
    EXPECT_FALSE(pExact->isChecked());
    EXPECT_FALSE(config()->getValue<bool>(kExactKeyMatchConfigKey, false));
    EXPECT_FALSE(KeyControl::exactKeyMatchEnabled(config()));
    EXPECT_FALSE(KeyControl::exactKeyMatchEnabled(UserSettingsPointer()));

    pExact->setChecked(true);
    deckPage.slotApply();
    EXPECT_TRUE(config()->getValue<bool>(kExactKeyMatchConfigKey, false));
    EXPECT_TRUE(KeyControl::exactKeyMatchEnabled(config()));
    // Exact key does not turn auto-harmonize on.
    EXPECT_FALSE(pBox->isChecked());
    EXPECT_FALSE(config()->getValue<bool>(kAutoHarmonizeOnLoadConfigKey, true));

    deckPage.slotResetToDefaults();
    EXPECT_FALSE(pExact->isChecked());
    // Reset changes the widget only until Apply.
    EXPECT_TRUE(config()->getValue<bool>(kExactKeyMatchConfigKey, false));
    deckPage.slotApply();
    EXPECT_FALSE(config()->getValue<bool>(kExactKeyMatchConfigKey, true));
    EXPECT_FALSE(KeyControl::exactKeyMatchEnabled(config()));

    pExact->setChecked(true);
    deckPage.slotUpdate();
    EXPECT_FALSE(pExact->isChecked());
    EXPECT_FALSE(config()->getValue<bool>(kExactKeyMatchConfigKey, true));
}
