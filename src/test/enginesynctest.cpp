#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <memory>
#include <string>

#include "control/controlobject.h"
#include "engine/controls/bpmcontrol.h"
#include "engine/sync/synccontrol.h"
#include "mixer/basetrackplayer.h"
#include "preferences/usersettings.h"
#include "test/mixxxtest.h"
#include "test/mockedenginebackendtest.h"
#include "track/beats.h"

namespace {
constexpr double kMaxFloatingPointErrorLowPrecision = 0.005;
constexpr double kMaxFloatingPointErrorHighPrecision = 0.0000000000000005;
constexpr double kMaxBeatDistanceEpsilon = 1e-9;
} // namespace

/// Tests for Sync Lock.
/// The following manual tests should probably be performed:
/// * Quantize mode nudges tracks in sync, whether internal or deck leader.
/// * Flinging tracks with the waveform should work.
/// * vinyl??
class EngineSyncTest : public MockedEngineBackendTest {
  public:
    QString getLeaderGroup() {
        Syncable* pLeaderSyncable = m_pEngineSync->getLeaderSyncable();
        if (pLeaderSyncable) {
            return pLeaderSyncable->getGroup();
        }
        return QString();
    }

    bool isExplicitLeader(const QString& group) {
        return isLeader(group, SyncMode::LeaderExplicit);
    }

    bool isSoftLeader(const QString& group) {
        return isLeader(group, SyncMode::LeaderSoft);
    }

    bool isFollower(const QString& group) {
        if (group == m_sInternalClockGroup) {
            return !ControlObject::getControl(ConfigKey(m_sInternalClockGroup,
                                                      "sync_leader"))
                            ->toBool();
        }
        if (auto mode = ControlObject::getControl(ConfigKey(group, "sync_mode"))
                                ->get();
                static_cast<SyncMode>(mode) != SyncMode::Follower) {
            qWarning() << "expected mode Follower, got" << mode;
            return false;
        }
        if (!ControlObject::getControl(ConfigKey(group, "sync_enabled"))->toBool()) {
            qWarning() << "sync_enabled should be on, isn't";
            return false;
        }
        if (double leader = ControlObject::getControl(
                    ConfigKey(group, "sync_leader"))
                                    ->get();
                leader != 0.0) {
            qWarning() << "sync_leader should be 0.0, is" << leader;
            return false;
        }
        return true;
    }

    void assertSyncOff(const QString& group) {
        if (group == m_sInternalClockGroup) {
            EXPECT_EQ(0,
                    ControlObject::getControl(
                            ConfigKey(m_sInternalClockGroup, "sync_leader"))
                            ->get());
        } else {
            EXPECT_EQ(SyncMode::None,
                    static_cast<SyncMode>(ControlObject::getControl(ConfigKey(group, "sync_mode"))
                                                  ->get()));
            EXPECT_EQ(0,
                    ControlObject::getControl(ConfigKey(group, "sync_enabled"))
                            ->get());
            EXPECT_EQ(0,
                    ControlObject::getControl(ConfigKey(group, "sync_leader"))
                            ->get());
        }
    }

    void assertNoLeader() {
        EXPECT_EQ(NULL, m_pEngineSync->getLeaderChannel());
        EXPECT_EQ(NULL, m_pEngineSync->getLeaderSyncable());
    }

  private:
    bool isLeader(const QString& group, SyncMode leaderType) {
        if (group == m_sInternalClockGroup) {
            double leader = ControlObject::getControl(ConfigKey(m_sInternalClockGroup,
                                                              "sync_leader"))
                                    ->get();
            if (leaderType == SyncMode::LeaderSoft && leader != 1.0) {
                qWarning() << "internal clock sync_leader should be 1.0, is" << leader;
                return false;
            } else if (leaderType == SyncMode::LeaderExplicit && leader != 2.0) {
                qWarning() << "internal clock sync_leader should be 2.0, is" << leader;
                return false;
            }
            if (m_pEngineSync->getLeaderChannel()) {
                qWarning() << "no current leader";
                return false;
            }
            if (m_sInternalClockGroup != getLeaderGroup()) {
                qWarning() << "internal clock is not leader, it's" << getLeaderGroup();
                return false;
            }
            return true;
        }
        if (group == m_sGroup1) {
            if (m_pEngineSync->getLeaderChannel() != m_pChannel1) {
                qWarning() << "leader pointer should be channel 1, is "
                           << (m_pEngineSync->getLeaderChannel()
                                              ? m_pEngineSync->getLeaderChannel()
                                                        ->getGroup()
                                              : "null");
                return false;
            }
        } else if (group == m_sGroup2) {
            if (m_pEngineSync->getLeaderChannel() != m_pChannel2) {
                qWarning() << "leader pointer should be channel 2, is "
                           << (m_pEngineSync->getLeaderChannel()
                                              ? m_pEngineSync->getLeaderChannel()
                                                        ->getGroup()
                                              : "null");
                return false;
            }
        } else if (group == m_sGroup3) {
            if (m_pEngineSync->getLeaderChannel() != m_pChannel3) {
                qWarning() << "leader pointer should be channel 3, is "
                           << (m_pEngineSync->getLeaderChannel()
                                              ? m_pEngineSync->getLeaderChannel()
                                                        ->getGroup()
                                              : "null");
                return false;
            }
        }
        if (getLeaderGroup() != group) {
            qWarning() << "leader group should be" << group << ", is" << getLeaderGroup();
            return false;
        }

        if (auto mode = ControlObject::getControl(ConfigKey(group, "sync_mode"))
                                ->get();
                static_cast<SyncMode>(mode) != leaderType) {
            qWarning() << "mode should be" << leaderType << ", is" << mode;
            return false;
        }
        if (!ControlObject::getControl(ConfigKey(group, "sync_enabled"))->toBool()) {
            qWarning() << "sync_enabled should be true, isn't";
            return false;
        }
        switch (leaderType) {
        case SyncMode::LeaderSoft: {
            if (double leader = ControlObject::getControl(
                        ConfigKey(group, "sync_leader"))
                                        ->get();
                    leader != 1.0) {
                qWarning() << "leader should be 1.0, is" << leader;
                return false;
            }
            break;
        }
        case SyncMode::LeaderExplicit: {
            if (double leader = ControlObject::getControl(
                        ConfigKey(group, "sync_leader"))
                                        ->get();
                    leader != 2.0) {
                qWarning() << "leader should be 2.0, is" << leader;
                return false;
            }
            break;
        }
        default:
            qWarning() << "bad leader type specified";
            return false;
        }
        return true;
    }
};

TEST_F(EngineSyncTest, ControlObjectsExist) {
    // This isn't exhaustive, but certain COs have a habit of not being set up properly.
    EXPECT_TRUE(ControlObject::getControl(ConfigKey(m_sGroup1, "file_bpm")) !=
            NULL);
}

TEST_F(EngineSyncTest, SetLeaderSuccess) {
    // If we set the first channel to leader, EngineSync should get that message.

    auto pButtonLeaderSync1 =
            std::make_unique<ControlProxy>(m_sGroup1, "sync_mode");
    pButtonLeaderSync1->set(static_cast<double>(SyncMode::LeaderExplicit));
    ProcessBuffer();

    // No tracks are playing and we have no beats, LeaderExplicit state is in stand-by
    EXPECT_DOUBLE_EQ(
            0.0, ControlObject::getControl(ConfigKey(m_sGroup1, "bpm"))->get());
    // The sync lock should now be internal clock, with group 1 waiting for play.
    EXPECT_TRUE(isFollower(m_sInternalClockGroup));
    EXPECT_TRUE(isFollower(m_sGroup1));

    auto pButtonLeaderSync2 =
            std::make_unique<ControlProxy>(m_sGroup2, "sync_mode");
    pButtonLeaderSync2->set(static_cast<double>(SyncMode::Follower));
    ProcessBuffer();

    EXPECT_TRUE(isFollower(m_sGroup2));

    // Now set channel 2 to be leader.
    pButtonLeaderSync2->set(static_cast<double>(SyncMode::LeaderExplicit));
    ProcessBuffer();

    // Now channel 2 should be waiting leader, and channel 1 should be a follower.
    EXPECT_TRUE(isFollower(m_sGroup2));
    EXPECT_TRUE(isFollower(m_sGroup1));

    // Now back again.
    pButtonLeaderSync1->set(static_cast<double>(SyncMode::LeaderExplicit));
    ProcessBuffer();

    // Now channel 1 should be waiting leader, and channel 2 should be a follower.
    EXPECT_TRUE(isFollower(m_sGroup1));
    EXPECT_TRUE(isFollower(m_sGroup2));

    // Now set channel 1 to follower, now all are followers, waiting for a tempo to adopt.
    pButtonLeaderSync1->set(static_cast<double>(SyncMode::Follower));
    ProcessBuffer();

    EXPECT_TRUE(isFollower(m_sInternalClockGroup));
    EXPECT_TRUE(isFollower(m_sGroup1));
    EXPECT_TRUE(isFollower(m_sGroup2));
}

TEST_F(EngineSyncTest, ExplicitLeaderPersists) {
    // If we set an explicit leader, enabling sync or pressing play on other decks
    // doesn't cause the leader to move around.
    mixxx::BeatsPointer pBeats1 = mixxx::Beats::fromConstTempo(
            m_pTrack1->getSampleRate(), mixxx::audio::kStartFramePos, mixxx::Bpm(120));
    m_pTrack1->trySetBeats(pBeats1);
    mixxx::BeatsPointer pBeats2 = mixxx::Beats::fromConstTempo(
            m_pTrack2->getSampleRate(), mixxx::audio::kStartFramePos, mixxx::Bpm(124));
    m_pTrack2->trySetBeats(pBeats2);

    auto pButtonLeaderSync1 =
            std::make_unique<ControlProxy>(m_sGroup1, "sync_mode");
    ControlObject::getControl(ConfigKey(m_sGroup1, "play"))->set(1.0);
    ProcessBuffer();
    pButtonLeaderSync1->set(static_cast<double>(SyncMode::LeaderExplicit));
    ProcessBuffer();
    // The sync lock should now be channel 1.
    EXPECT_TRUE(isExplicitLeader(m_sGroup1));

    auto pButtonLeaderSync2 =
            std::make_unique<ControlProxy>(m_sGroup2, "sync_enabled");
    ControlObject::getControl(ConfigKey(m_sGroup2, "play"))->set(1.0);
    pButtonLeaderSync2->set(1.0);
    ProcessBuffer();
    EXPECT_TRUE(isExplicitLeader(m_sGroup1));
    EXPECT_TRUE(isFollower(m_sGroup2));

    // Stop deck 2, and restart it, no change.
    ControlObject::getControl(ConfigKey(m_sGroup2, "play"))->set(0.0);
    ProcessBuffer();
    ControlObject::getControl(ConfigKey(m_sGroup2, "play"))->set(1.0);
    ProcessBuffer();
    EXPECT_TRUE(isExplicitLeader(m_sGroup1));
    EXPECT_TRUE(isFollower(m_sGroup2));
}

TEST_F(EngineSyncTest, SetLeaderWhilePlaying) {
    // Make sure we don't get two leader lights if we change leaders while playing.
    mixxx::BeatsPointer pBeats1 = mixxx::Beats::fromConstTempo(
            m_pTrack1->getSampleRate(), mixxx::audio::kStartFramePos, mixxx::Bpm(120));
    m_pTrack1->trySetBeats(pBeats1);
    mixxx::BeatsPointer pBeats2 = mixxx::Beats::fromConstTempo(
            m_pTrack2->getSampleRate(), mixxx::audio::kStartFramePos, mixxx::Bpm(124));
    m_pTrack2->trySetBeats(pBeats2);
    mixxx::BeatsPointer pBeats3 = mixxx::Beats::fromConstTempo(
            m_pTrack3->getSampleRate(), mixxx::audio::kStartFramePos, mixxx::Bpm(128));
    m_pTrack3->trySetBeats(pBeats3);

    auto pButtonLeaderSync1 =
            std::make_unique<ControlProxy>(m_sGroup1, "sync_mode");
    pButtonLeaderSync1->set(static_cast<double>(SyncMode::LeaderExplicit));
    auto pButtonLeaderSync2 =
            std::make_unique<ControlProxy>(m_sGroup2, "sync_mode");
    pButtonLeaderSync2->set(static_cast<double>(SyncMode::Follower));
    auto pButtonLeaderSync3 =
            std::make_unique<ControlProxy>(m_sGroup3, "sync_mode");
    pButtonLeaderSync3->set(static_cast<double>(SyncMode::Follower));

    ControlObject::getControl(ConfigKey(m_sGroup1, "play"))->set(1.0);
    ControlObject::getControl(ConfigKey(m_sGroup2, "play"))->set(1.0);
    ControlObject::getControl(ConfigKey(m_sGroup3, "play"))->set(1.0);

    ProcessBuffer();

    pButtonLeaderSync3->set(static_cast<double>(SyncMode::LeaderExplicit));

    ProcessBuffer();

    EXPECT_TRUE(isFollower(m_sGroup1));
    EXPECT_TRUE(isFollower(m_sGroup2));
    EXPECT_TRUE(isExplicitLeader(m_sGroup3));
}

TEST_F(EngineSyncTest, SetEnabledBecomesLeader) {
    // If we set the first channel with a valid tempo to follower, it should be leader.
    mixxx::BeatsPointer pBeats1 = mixxx::Beats::fromConstTempo(
            m_pTrack1->getSampleRate(), mixxx::audio::kStartFramePos, mixxx::Bpm(80));
    m_pTrack1->trySetBeats(pBeats1);
    ProcessBuffer();
    auto pButtonLeaderSync1 =
            std::make_unique<ControlProxy>(m_sGroup1, "sync_mode");
    pButtonLeaderSync1->set(static_cast<double>(SyncMode::Follower));
    ProcessBuffer();

    EXPECT_TRUE(isSoftLeader(m_sGroup1));
    EXPECT_TRUE(isFollower(m_sInternalClockGroup));
}

TEST_F(EngineSyncTest, DisableInternalLeaderWhilePlaying) {
    auto pButtonLeaderSync = std::make_unique<ControlProxy>(
            m_sInternalClockGroup, "sync_leader");
    pButtonLeaderSync->set(1.0);
    auto pButtonSyncMode1 =
            std::make_unique<ControlProxy>(m_sGroup1, "sync_mode");
    pButtonSyncMode1->set(static_cast<double>(SyncMode::Follower));
    auto pButtonSyncMode2 =
            std::make_unique<ControlProxy>(m_sGroup2, "sync_mode");
    pButtonSyncMode2->set(static_cast<double>(SyncMode::Follower));
    ProcessBuffer();
    // The sync lock should now be Internal.
    EXPECT_TRUE(isExplicitLeader(m_sInternalClockGroup));

    // Make sure both decks are playing.
    mixxx::BeatsPointer pBeats1 = mixxx::Beats::fromConstTempo(
            m_pTrack1->getSampleRate(), mixxx::audio::kStartFramePos, mixxx::Bpm(80));
    m_pTrack1->trySetBeats(pBeats1);
    ControlObject::getControl(ConfigKey(m_sGroup1, "play"))->set(1.0);
    mixxx::BeatsPointer pBeats2 = mixxx::Beats::fromConstTempo(
            m_pTrack2->getSampleRate(), mixxx::audio::kStartFramePos, mixxx::Bpm(80));
    m_pTrack2->trySetBeats(pBeats2);
    ControlObject::getControl(ConfigKey(m_sGroup2, "play"))->set(1.0);
    ProcessBuffer();

    // Now unset Internal leader.
    pButtonLeaderSync->set(0.0);
    ProcessBuffer();

    // This is not allowed, Internal should still be leader.
    EXPECT_TRUE(isSoftLeader(m_sInternalClockGroup));
    EXPECT_EQ(1, pButtonLeaderSync->get());
}

TEST_F(EngineSyncTest, DisableSyncOnLeader) {
    // Channel 1 follower, channel 2 leader.
    mixxx::BeatsPointer pBeats1 = mixxx::Beats::fromConstTempo(
            m_pTrack1->getSampleRate(), mixxx::audio::kStartFramePos, mixxx::Bpm(130));
    m_pTrack1->trySetBeats(pBeats1);
    auto pButtonSyncMode1 =
            std::make_unique<ControlProxy>(m_sGroup1, "sync_mode");
    pButtonSyncMode1->set(static_cast<double>(SyncMode::Follower));

    mixxx::BeatsPointer pBeats2 = mixxx::Beats::fromConstTempo(
            m_pTrack2->getSampleRate(), mixxx::audio::kStartFramePos, mixxx::Bpm(130));
    m_pTrack2->trySetBeats(pBeats2);
    // Set deck two to leader, but this is not allowed because it's stopped.
    ProcessBuffer();
    auto pButtonSyncLeader2 =
            std::make_unique<ControlProxy>(m_sGroup2, "sync_leader");
    pButtonSyncLeader2->set(1.0);
    ProcessBuffer();
    EXPECT_TRUE(isSoftLeader(m_sGroup1));
    EXPECT_TRUE(isFollower(m_sGroup2));

    // Set deck 2 to playing, now it becomes explicit leader.
    ControlObject::getControl(ConfigKey(m_sGroup2, "play"))->set(1.0);
    // The request to become leader is queued, so we have to process a buffer.
    ProcessBuffer();
    EXPECT_TRUE(isFollower(m_sGroup1));
    EXPECT_TRUE(isSoftLeader(m_sGroup2));

    // Unset enabled on channel2, it should work.
    auto pButtonSyncEnabled2 =
            std::make_unique<ControlProxy>(m_sGroup2, "sync_enabled");
    pButtonSyncEnabled2->set(0.0);
    ProcessBuffer();
    EXPECT_TRUE(isSoftLeader(m_sGroup1));
    EXPECT_EQ(0, ControlObject::getControl(ConfigKey(m_sGroup2, "sync_enabled"))->get());
    EXPECT_EQ(0, ControlObject::getControl(ConfigKey(m_sGroup2, "sync_leader"))->get());
}

TEST_F(EngineSyncTest, InternalLeaderSetFollowerSliderMoves) {
    // If internal is leader, and we turn on a follower, the slider should move.
    auto pButtonLeaderSyncInternal = std::make_unique<ControlProxy>(
            m_sInternalClockGroup, "sync_leader");
    auto pInternalClockBpm =
            std::make_unique<ControlProxy>(m_sInternalClockGroup, "bpm");

    pInternalClockBpm->set(100.0);
    // Request internal clock to become SyncMode::LeaderExplicit
    pButtonLeaderSyncInternal->set(1);
    EXPECT_TRUE(isExplicitLeader(m_sInternalClockGroup));

    // Set the file bpm of channel 1 to 80 bpm.
    mixxx::BeatsPointer pBeats1 = mixxx::Beats::fromConstTempo(
            m_pTrack1->getSampleRate(), mixxx::audio::kStartFramePos, mixxx::Bpm(80));
    m_pTrack1->trySetBeats(pBeats1);

    auto pButtonLeaderSync1 =
            std::make_unique<ControlProxy>(m_sGroup1, "sync_mode");
    pButtonLeaderSync1->set(static_cast<double>(SyncMode::Follower));
    ProcessBuffer();

    EXPECT_TRUE(isFollower(m_sGroup1));
    EXPECT_TRUE(isExplicitLeader(m_sInternalClockGroup));

    EXPECT_DOUBLE_EQ(getRateSliderValue(1.25),
            ControlObject::getControl(ConfigKey(m_sGroup1, "rate"))->get());
    EXPECT_DOUBLE_EQ(100.0,
            ControlObject::getControl(ConfigKey(m_sGroup1, "bpm"))->get());
}

TEST_F(EngineSyncTest, AnySyncDeckSliderStays) {
    // If there exists a sync deck, even if it's not playing, don't change the
    // leader BPM if a new deck enables sync.

    mixxx::BeatsPointer pBeats1 = mixxx::Beats::fromConstTempo(
            m_pTrack1->getSampleRate(), mixxx::audio::kStartFramePos, mixxx::Bpm(80));
    m_pTrack1->trySetBeats(pBeats1);
    ProcessBuffer();
    auto pButtonSyncEnabled1 =
            std::make_unique<ControlProxy>(m_sGroup1, "sync_enabled");
    pButtonSyncEnabled1->set(1.0);

    // After setting up the first deck, the internal BPM should be 80.
    EXPECT_DOUBLE_EQ(80.0,
            ControlObject::getControl(ConfigKey(m_sInternalClockGroup, "bpm"))
                    ->get());

    mixxx::BeatsPointer pBeats2 = mixxx::Beats::fromConstTempo(
            m_pTrack2->getSampleRate(), mixxx::audio::kStartFramePos, mixxx::Bpm(100));
    m_pTrack2->trySetBeats(pBeats2);
    auto pButtonSyncEnabled2 =
            std::make_unique<ControlProxy>(m_sGroup2, "sync_enabled");
    pButtonSyncEnabled2->set(1.0);

    // After the second one, though, the internal BPM should still be 80.
    EXPECT_DOUBLE_EQ(80.0,
            ControlObject::getControl(ConfigKey(m_sInternalClockGroup, "bpm"))
                    ->get());
}

TEST_F(EngineSyncTest, InternalClockFollowsFirstPlayingDeck) {
    // Same as above, except we use the midi lights to change state.
    auto pButtonLeaderSync1 =
            std::make_unique<ControlProxy>(m_sGroup1, "sync_mode");
    auto pButtonLeaderSync2 =
            std::make_unique<ControlProxy>(m_sGroup2, "sync_mode");
    auto pButtonSyncEnabled1 =
            std::make_unique<ControlProxy>(m_sGroup1, "sync_enabled");
    auto pButtonSyncEnabled2 =
            std::make_unique<ControlProxy>(m_sGroup2, "sync_enabled");

    // Set up decks so they can be playing, and start deck 1.
    mixxx::BeatsPointer pBeats1 = mixxx::Beats::fromConstTempo(
            m_pTrack1->getSampleRate(), mixxx::audio::kStartFramePos, mixxx::Bpm(100));
    m_pTrack1->trySetBeats(pBeats1);
    ControlObject::set(ConfigKey(m_sGroup1, "rate"), getRateSliderValue(1.0));
    ControlObject::set(ConfigKey(m_sGroup1, "play"), 1.0);
    mixxx::BeatsPointer pBeats2 = mixxx::Beats::fromConstTempo(
            m_pTrack2->getSampleRate(), mixxx::audio::kStartFramePos, mixxx::Bpm(130));
    m_pTrack2->trySetBeats(pBeats2);
    ControlObject::set(ConfigKey(m_sGroup2, "rate"), getRateSliderValue(1.0));
    ControlObject::set(ConfigKey(m_sGroup2, "play"), 0.0);
    ProcessBuffer();

    // Set channel 1 to be enabled
    pButtonSyncEnabled1->set(1.0);
    ProcessBuffer();

    // The sync lock should now be deck 1.
    EXPECT_TRUE(isSoftLeader(m_sGroup1));
    EXPECT_DOUBLE_EQ(130.0,
            ControlObject::get(ConfigKey(m_sInternalClockGroup, "bpm")));

    // Set channel 2 to be enabled.
    pButtonSyncEnabled2->set(1);
    ProcessBuffer();

    // channel 1 still leader while 2 is not playing
    EXPECT_TRUE(isSoftLeader(m_sGroup1));
    EXPECT_TRUE(isFollower(m_sInternalClockGroup));
    EXPECT_TRUE(isFollower(m_sGroup2));

    // The rate should not have changed -- deck 1 still matches deck 2.
    EXPECT_DOUBLE_EQ(getRateSliderValue(1.3),
            ControlObject::getControl(ConfigKey(m_sGroup1, "rate"))->get());

    // Reset channel 2 rate, set channel 2 to play, and process a buffer.
    ControlObject::set(ConfigKey(m_sGroup2, "rate"), getRateSliderValue(1.0));
    ControlObject::set(ConfigKey(m_sGroup2, "play"), 1.0);
    ProcessBuffer();
    // Deck 1 still leader
    EXPECT_TRUE(isFollower(m_sInternalClockGroup));
    EXPECT_TRUE(isSoftLeader(m_sGroup1));
    EXPECT_TRUE(isFollower(m_sGroup2));

    // Now disable sync on channel 1.
    pButtonSyncEnabled1->set(0);
    ProcessBuffer();

    // Leader flips to deck 2
    EXPECT_TRUE(isSoftLeader(m_sGroup2));
    EXPECT_TRUE(isFollower(m_sInternalClockGroup));

    // Rate should now match channel 2.
    EXPECT_DOUBLE_EQ(
            130.0, ControlObject::get(ConfigKey(m_sInternalClockGroup, "bpm")));
}

TEST_F(EngineSyncTest, SetExplicitLeaderByLights) {
    // Same as above, except we use the midi lights to change state.
    auto pButtonLeaderSync1 =
            std::make_unique<ControlProxy>(m_sGroup1, "sync_mode");
    auto pButtonLeaderSync2 =
            std::make_unique<ControlProxy>(m_sGroup2, "sync_mode");
    auto pButtonSyncEnabled1 =
            std::make_unique<ControlProxy>(m_sGroup1, "sync_enabled");
    auto pButtonSyncEnabled2 =
            std::make_unique<ControlProxy>(m_sGroup2, "sync_enabled");
    auto pButtonSyncLeader1 =
            std::make_unique<ControlProxy>(m_sGroup1, "sync_leader");
    auto pButtonSyncLeader2 =
            std::make_unique<ControlProxy>(m_sGroup2, "sync_leader");

    // Set the file bpm of channel 1 to 160bpm.
    mixxx::BeatsPointer pBeats1 = mixxx::Beats::fromConstTempo(
            m_pTrack1->getSampleRate(), mixxx::audio::kStartFramePos, mixxx::Bpm(160));
    m_pTrack1->trySetBeats(pBeats1);
    // Set the file bpm of channel 2 to 150bpm.
    mixxx::BeatsPointer pBeats2 = mixxx::Beats::fromConstTempo(
            m_pTrack2->getSampleRate(), mixxx::audio::kStartFramePos, mixxx::Bpm(150));
    m_pTrack2->trySetBeats(pBeats2);

    ControlObject::set(ConfigKey(m_sGroup1, "play"), 1.0);
    ControlObject::set(ConfigKey(m_sGroup2, "play"), 1.0);
    ProcessBuffer();

    // Set channel 1 to be explicit leader.
    pButtonSyncLeader1->set(1.0);
    ProcessBuffer();

    // The sync lock should now be channel 1.
    EXPECT_TRUE(isSoftLeader(m_sGroup1));

    // Set channel 2 to be follower.
    pButtonSyncEnabled2->set(1);
    ProcessBuffer();

    EXPECT_TRUE(isFollower(m_sGroup2));

    // Now set channel 2 to be leader.
    pButtonSyncLeader2->set(1);
    ProcessBuffer();

    // Now channel 2 should be leader, and channel 1 should be a follower.
    EXPECT_TRUE(isFollower(m_sGroup1));
    EXPECT_TRUE(isSoftLeader(m_sGroup2));

    // Now back again.
    pButtonSyncLeader1->set(1);
    ProcessBuffer();

    // Now channel 1 should be leader, and channel 2 should be a follower.
    EXPECT_TRUE(isSoftLeader(m_sGroup1));
    EXPECT_TRUE(isFollower(m_sGroup2));

    // Now set channel 1 to not-leader.
    // This will choose automatically a Soft Leader, so it chooses the other deck.
    pButtonSyncLeader1->set(0);
    ProcessBuffer();

    EXPECT_TRUE(isFollower(m_sInternalClockGroup));
    EXPECT_TRUE(isFollower(m_sGroup1));
    EXPECT_TRUE(isSoftLeader(m_sGroup2));

    // Try again without playing
    pButtonSyncLeader1->set(1);
    ProcessBuffer();

    EXPECT_TRUE(isSoftLeader(m_sGroup1));
    EXPECT_TRUE(isFollower(m_sGroup2));

    ControlObject::set(ConfigKey(m_sGroup1, "play"), 0.0);
    pButtonSyncLeader1->set(0);
    ProcessBuffer();

    // Now the m_sGroup2 should be leader because m_sGroup1 can't lead without playing
    EXPECT_TRUE(isFollower(m_sInternalClockGroup));
    EXPECT_TRUE(isSoftLeader(m_sGroup2));
    EXPECT_TRUE(isFollower(m_sGroup1));
}

TEST_F(EngineSyncTest, SetExplicitLeaderByLightsNoTracks) {
    // Same as above, except we use the midi lights to change state.
    auto pButtonSyncEnabled2 =
            std::make_unique<ControlProxy>(m_sGroup2, "sync_enabled");
    auto pButtonSyncLeader1 =
            std::make_unique<ControlProxy>(m_sGroup1, "sync_leader");

    pButtonSyncLeader1->set(1);

    // Set channel 2 to be follower.
    pButtonSyncEnabled2->set(1);

    // Without a track loaded, deck 1 can't be an explicit leader.
    EXPECT_TRUE(isFollower(m_sGroup1));
    EXPECT_TRUE(isFollower(m_sGroup2));

    pButtonSyncLeader1->set(0);

    EXPECT_TRUE(isFollower(m_sGroup1));
    EXPECT_TRUE(isFollower(m_sGroup2));
    EXPECT_TRUE(isFollower(m_sInternalClockGroup));
}

TEST_F(EngineSyncTest, RateChangeTest) {
    auto pButtonLeaderSync1 =
            std::make_unique<ControlProxy>(m_sGroup1, "sync_mode");
    pButtonLeaderSync1->set(static_cast<double>(SyncMode::LeaderExplicit));
    auto pButtonLeaderSync2 =
            std::make_unique<ControlProxy>(m_sGroup2, "sync_mode");
    pButtonLeaderSync2->set(static_cast<double>(SyncMode::Follower));
    ProcessBuffer();

    // Set the file bpm of channel 1 to 160bpm.
    mixxx::BeatsPointer pBeats1 = mixxx::Beats::fromConstTempo(
            m_pTrack1->getSampleRate(), mixxx::audio::kStartFramePos, mixxx::Bpm(160));
    m_pTrack1->trySetBeats(pBeats1);
    EXPECT_DOUBLE_EQ(
            160.0, ControlObject::get(ConfigKey(m_sGroup1, "file_bpm")));
    ProcessBuffer();
    EXPECT_DOUBLE_EQ(
            160.0, ControlObject::get(ConfigKey(m_sInternalClockGroup, "bpm")));

    // Set the rate of channel 1 to 1.2.
    ControlObject::set(ConfigKey(m_sGroup1, "rate"), getRateSliderValue(1.2));
    EXPECT_DOUBLE_EQ(getRateSliderValue(1.2),
            ControlObject::get(ConfigKey(m_sGroup1, "rate")));
    EXPECT_DOUBLE_EQ(192.0, ControlObject::get(ConfigKey(m_sGroup1, "bpm")));

    // Internal leader should also be 192.
    EXPECT_DOUBLE_EQ(
            192.0, ControlObject::get(ConfigKey(m_sInternalClockGroup, "bpm")));

    // Set the file bpm of channel 2 to 120bpm.
    mixxx::BeatsPointer pBeats2 = mixxx::Beats::fromConstTempo(
            m_pTrack2->getSampleRate(), mixxx::audio::kStartFramePos, mixxx::Bpm(120));
    m_pTrack2->trySetBeats(pBeats2);
    EXPECT_DOUBLE_EQ(
            120.0, ControlObject::get(ConfigKey(m_sGroup2, "file_bpm")));

    EXPECT_DOUBLE_EQ(getRateSliderValue(0.8),
            ControlObject::get(ConfigKey(m_sGroup2, "rate")));
    // Leader is currently 192, BPM before sync is 120, so the closer sync should be 96
    EXPECT_DOUBLE_EQ(96.0, ControlObject::get(ConfigKey(m_sGroup2, "bpm")));
}

TEST_F(EngineSyncTest, RateChangeTestWeirdOrder) {
    // This is like the test above, but the user loads the track after the slider has been tweaked.
    auto pButtonLeaderSync1 =
            std::make_unique<ControlProxy>(m_sGroup1, "sync_mode");
    pButtonLeaderSync1->set(static_cast<double>(SyncMode::LeaderExplicit));
    auto pButtonLeaderSync2 =
            std::make_unique<ControlProxy>(m_sGroup2, "sync_mode");
    pButtonLeaderSync2->set(static_cast<double>(SyncMode::Follower));
    ProcessBuffer();

    // Set the file bpm of channel 1 to 160bpm.
    mixxx::BeatsPointer pBeats1 = mixxx::Beats::fromConstTempo(
            m_pTrack1->getSampleRate(), mixxx::audio::kStartFramePos, mixxx::Bpm(160));
    m_pTrack1->trySetBeats(pBeats1);
    EXPECT_DOUBLE_EQ(
            160.0, ControlObject::get(ConfigKey(m_sInternalClockGroup, "bpm")));

    // Set the file bpm of channel 2 to 120bpm.
    mixxx::BeatsPointer pBeats2 = mixxx::Beats::fromConstTempo(
            m_pTrack2->getSampleRate(), mixxx::audio::kStartFramePos, mixxx::Bpm(120));
    m_pTrack2->trySetBeats(pBeats2);

    // Set the rate slider of channel 1 to 1.2.
    ControlObject::set(ConfigKey(m_sGroup1, "rate"), getRateSliderValue(1.2));

    // Rate slider for channel 2 should now be 1.6 = (160 * 1.2 / 120) - 1.0.
    EXPECT_DOUBLE_EQ(getRateSliderValue(1.6),
            ControlObject::get(ConfigKey(m_sGroup2, "rate")));
    EXPECT_DOUBLE_EQ(192.0, ControlObject::get(ConfigKey(m_sGroup2, "bpm")));

    // Internal Leader BPM should read the same.
    EXPECT_DOUBLE_EQ(
            192.0, ControlObject::get(ConfigKey(m_sInternalClockGroup, "bpm")));
}

TEST_F(EngineSyncTest, RateChangeTestOrder3) {
    // Set the file bpm of channel 1 to 160bpm.
    mixxx::BeatsPointer pBeats1 = mixxx::Beats::fromConstTempo(
            m_pTrack1->getSampleRate(), mixxx::audio::kStartFramePos, mixxx::Bpm(160));
    m_pTrack1->trySetBeats(pBeats1);
    EXPECT_DOUBLE_EQ(
            160.0, ControlObject::get(ConfigKey(m_sGroup1, "file_bpm")));

    // Set the file bpm of channel 2 to 120bpm.
    mixxx::BeatsPointer pBeats2 = mixxx::Beats::fromConstTempo(
            m_pTrack2->getSampleRate(), mixxx::audio::kStartFramePos, mixxx::Bpm(120));
    m_pTrack2->trySetBeats(pBeats2);
    EXPECT_DOUBLE_EQ(
            120.0, ControlObject::get(ConfigKey(m_sGroup2, "file_bpm")));
    ProcessBuffer();

    // Turn on Leader. Setting leader explicitly means we don't match the tempo of
    // another deck.
    auto pButtonLeaderSync1 =
            std::make_unique<ControlProxy>(m_sGroup1, "sync_mode");
    pButtonLeaderSync1->set(static_cast<double>(SyncMode::LeaderExplicit));
    ProcessBuffer();
    EXPECT_TRUE(isExplicitLeader(m_sGroup1));
    EXPECT_DOUBLE_EQ(160.0, ControlObject::get(ConfigKey(m_sGroup1, "bpm")));

    // Turn on follower.
    auto pButtonLeaderSync2 =
            std::make_unique<ControlProxy>(m_sGroup2, "sync_mode");
    pButtonLeaderSync2->set(static_cast<double>(SyncMode::Follower));
    ProcessBuffer();

    // Follower should immediately set its slider.
    EXPECT_NEAR(getRateSliderValue(1.0),
            ControlObject::get(ConfigKey(m_sGroup1, "rate")),
            kMaxFloatingPointErrorLowPrecision);
    EXPECT_DOUBLE_EQ(160.0, ControlObject::get(ConfigKey(m_sGroup2, "bpm")));
    EXPECT_DOUBLE_EQ(
            160.0, ControlObject::get(ConfigKey(m_sInternalClockGroup, "bpm")));
}

TEST_F(EngineSyncTest, FollowerRateChange) {
    // Confirm that followers can change sync lock rate as well.
    auto pButtonLeaderSync1 =
            std::make_unique<ControlProxy>(m_sGroup1, "sync_mode");
    pButtonLeaderSync1->set(static_cast<double>(SyncMode::LeaderExplicit));
    auto pButtonLeaderSync2 =
            std::make_unique<ControlProxy>(m_sGroup2, "sync_mode");
    pButtonLeaderSync2->set(static_cast<double>(SyncMode::Follower));
    ProcessBuffer();

    // Set the file bpm of channel 1 to 160bpm.
    mixxx::BeatsPointer pBeats1 = mixxx::Beats::fromConstTempo(
            m_pTrack1->getSampleRate(), mixxx::audio::kStartFramePos, mixxx::Bpm(160));
    m_pTrack1->trySetBeats(pBeats1);

    // Set the file bpm of channel 2 to 120bpm.
    mixxx::BeatsPointer pBeats2 = mixxx::Beats::fromConstTempo(
            m_pTrack2->getSampleRate(), mixxx::audio::kStartFramePos, mixxx::Bpm(120));
    m_pTrack2->trySetBeats(pBeats2);

    // Set the rate slider of channel 1 to 1.2.
    ControlObject::set(ConfigKey(m_sGroup1, "rate"), getRateSliderValue(1.2));

    // Rate slider for channel 2 should now be 1.6 = (160 * 1.2 / 120).
    EXPECT_DOUBLE_EQ(getRateSliderValue(1.6),
            ControlObject::get(ConfigKey(m_sGroup2, "rate")));
    EXPECT_DOUBLE_EQ(192.0, ControlObject::get(ConfigKey(m_sGroup2, "bpm")));

    // Try to twiddle the rate slider on channel 2.
    auto pSlider2 = std::make_unique<ControlProxy>(m_sGroup2, "rate");
    pSlider2->set(getRateSliderValue(0.8));
    ProcessBuffer();

    // Rates should still be changed even though it's a follower.
    EXPECT_DOUBLE_EQ(getRateSliderValue(0.8),
            ControlObject::get(ConfigKey(m_sGroup2, "rate")));
    EXPECT_DOUBLE_EQ(96.0, ControlObject::get(ConfigKey(m_sGroup2, "bpm")));
    EXPECT_DOUBLE_EQ(getRateSliderValue(0.6),
            ControlObject::get(ConfigKey(m_sGroup1, "rate")));
    EXPECT_DOUBLE_EQ(96.0, ControlObject::get(ConfigKey(m_sGroup1, "bpm")));
}

TEST_F(EngineSyncTest, InternalRateChangeTest) {
    auto pButtonLeaderSyncInternal = std::make_unique<ControlProxy>(
            m_sInternalClockGroup, "sync_leader");
    pButtonLeaderSyncInternal->set(1.0);
    auto pButtonLeaderSync1 =
            std::make_unique<ControlProxy>(m_sGroup1, "sync_mode");
    pButtonLeaderSync1->set(static_cast<double>(SyncMode::Follower));
    auto pButtonLeaderSync2 =
            std::make_unique<ControlProxy>(m_sGroup2, "sync_mode");
    pButtonLeaderSync2->set(static_cast<double>(SyncMode::Follower));
    ProcessBuffer();

    EXPECT_TRUE(isExplicitLeader(m_sInternalClockGroup));
    EXPECT_TRUE(isFollower(m_sGroup1));
    EXPECT_TRUE(isFollower(m_sGroup2));

    // Set the file bpm of channel 1 to 160bpm.
    mixxx::BeatsPointer pBeats1 = mixxx::Beats::fromConstTempo(
            m_pTrack1->getSampleRate(), mixxx::audio::kStartFramePos, mixxx::Bpm(160));
    m_pTrack1->trySetBeats(pBeats1);
    EXPECT_DOUBLE_EQ(160.0,
            ControlObject::getControl(ConfigKey(m_sGroup1, "file_bpm"))->get());

    // Set the file bpm of channel 2 to 120bpm.
    mixxx::BeatsPointer pBeats2 = mixxx::Beats::fromConstTempo(
            m_pTrack2->getSampleRate(), mixxx::audio::kStartFramePos, mixxx::Bpm(120));
    m_pTrack2->trySetBeats(pBeats2);
    EXPECT_DOUBLE_EQ(120.0,
            ControlObject::getControl(ConfigKey(m_sGroup2, "file_bpm"))->get());

    // Set the internal rate to 150.
    auto pLeaderSyncSlider =
            std::make_unique<ControlProxy>(m_sInternalClockGroup, "bpm");
    pLeaderSyncSlider->set(150.0);
    EXPECT_DOUBLE_EQ(150.0,
            ControlObject::getControl(ConfigKey(m_sInternalClockGroup, "bpm"))
                    ->get());
    // Set decks playing, and process a buffer to update all the COs.
    ControlObject::getControl(ConfigKey(m_sGroup1, "play"))->set(1.0);
    ControlObject::getControl(ConfigKey(m_sGroup2, "play"))->set(1.0);
    ProcessBuffer();
    // Rate sliders for channels 1 and 2 should change appropriately.
    EXPECT_DOUBLE_EQ(getRateSliderValue(0.9375),
            ControlObject::getControl(ConfigKey(m_sGroup1, "rate"))->get());
    EXPECT_DOUBLE_EQ(150.0,
            ControlObject::getControl(ConfigKey(m_sGroup1, "bpm"))->get());
    EXPECT_DOUBLE_EQ(getRateSliderValue(1.25),
            ControlObject::getControl(ConfigKey(m_sGroup2, "rate"))->get());
    EXPECT_DOUBLE_EQ(150.0,
            ControlObject::getControl(ConfigKey(m_sGroup2, "bpm"))->get());

    // Set the internal rate to 140.
    pLeaderSyncSlider->set(140.0);

    // Update COs again.
    ProcessBuffer();

    // Rate sliders for channels 1 and 2 should change appropriately.
    EXPECT_DOUBLE_EQ(getRateSliderValue(.875),
            ControlObject::getControl(ConfigKey(m_sGroup1, "rate"))->get());
    EXPECT_DOUBLE_EQ(140.0,
            ControlObject::getControl(ConfigKey(m_sGroup1, "bpm"))->get());
    EXPECT_NEAR(getRateSliderValue(1.16666667),
            ControlObject::getControl(ConfigKey(m_sGroup2, "rate"))->get(),
            kMaxFloatingPointErrorLowPrecision);
    EXPECT_DOUBLE_EQ(140.0,
            ControlObject::getControl(ConfigKey(m_sGroup2, "bpm"))->get());
}

TEST_F(EngineSyncTest, LeaderStopSliderCheck) {
    // If the leader is playing, and stop is pushed, the sliders should stay the same.
    mixxx::BeatsPointer pBeats1 = mixxx::Beats::fromConstTempo(
            m_pTrack1->getSampleRate(), mixxx::audio::kStartFramePos, mixxx::Bpm(120));
    m_pTrack1->trySetBeats(pBeats1);
    mixxx::BeatsPointer pBeats2 = mixxx::Beats::fromConstTempo(
            m_pTrack2->getSampleRate(), mixxx::audio::kStartFramePos, mixxx::Bpm(128));
    m_pTrack2->trySetBeats(pBeats2);

    auto pButtonLeaderSync2 =
            std::make_unique<ControlProxy>(m_sGroup2, "sync_mode");
    pButtonLeaderSync2->set(static_cast<double>(SyncMode::Follower));
    auto pButtonLeaderSync1 =
            std::make_unique<ControlProxy>(m_sGroup1, "sync_mode");
    pButtonLeaderSync1->set(static_cast<double>(SyncMode::LeaderExplicit));
    ProcessBuffer();

    //EXPECT_TRUE(isExplicitLeader(m_sGroup1));
    EXPECT_TRUE(isFollower(m_sGroup2));

    auto pChannel1Play = std::make_unique<ControlProxy>(m_sGroup1, "play");
    pChannel1Play->set(1.0);
    auto pChannel2Play = std::make_unique<ControlProxy>(m_sGroup2, "play");
    pChannel2Play->set(1.0);

    ProcessBuffer();

    EXPECT_DOUBLE_EQ(120.0, ControlObject::get(ConfigKey(m_sGroup2, "bpm")));
    EXPECT_DOUBLE_EQ(getRateSliderValue(0.9375),
            ControlObject::get(ConfigKey(m_sGroup2, "rate")));

    pChannel1Play->set(0.0);

    ProcessBuffer();

    EXPECT_DOUBLE_EQ(120.0, ControlObject::get(ConfigKey(m_sGroup2, "bpm")));
    EXPECT_DOUBLE_EQ(getRateSliderValue(0.9375),
            ControlObject::get(ConfigKey(m_sGroup2, "rate")));
}

TEST_F(EngineSyncTest, EnableOneDeckInitsLeader) {
    // If Internal is leader, and we turn sync on a playing deck, the playing deck sets the
    // internal leader and the beat distances are now aligned.
    ControlObject::set(ConfigKey(m_sInternalClockGroup, "bpm"), 124.0);
    ControlObject::set(ConfigKey(m_sInternalClockGroup, "beat_distance"), 0.5);
    ProcessBuffer();

    // Set up the deck to play.
    mixxx::BeatsPointer pBeats1 = mixxx::Beats::fromConstTempo(
            m_pTrack1->getSampleRate(), mixxx::audio::kStartFramePos, mixxx::Bpm(130));
    m_pTrack1->trySetBeats(pBeats1);
    ControlObject::getControl(ConfigKey(m_sGroup1, "rate"))
            ->set(getRateSliderValue(1.0));
    ControlObject::getControl(ConfigKey(m_sGroup1, "beat_distance"))->set(0.2);
    ControlObject::getControl(ConfigKey(m_sGroup1, "play"))->set(1.0);

    // Enable Sync.  We have to call requestEnableSync directly
    // because calling ProcessBuffer() tries to advance the beat_distance values.
    m_pEngineSync->requestSyncMode(
            m_pEngineSync->getSyncableForGroup(m_sGroup1), SyncMode::Follower);

    // Internal is no longer leader because there is exactly one playing deck.
    EXPECT_TRUE(isSoftLeader(m_sGroup1));
    EXPECT_TRUE(isFollower(m_sInternalClockGroup));

    // Internal clock rate and beat distance should match that deck.
    EXPECT_DOUBLE_EQ(
            130.0, ControlObject::get(ConfigKey(m_sInternalClockGroup, "bpm")));
    EXPECT_DOUBLE_EQ(130.0, ControlObject::get(ConfigKey(m_sGroup1, "bpm")));
    EXPECT_DOUBLE_EQ(
            0.2, ControlObject::get(ConfigKey(m_sGroup1, "beat_distance")));
    EXPECT_DOUBLE_EQ(0.2,
            ControlObject::get(
                    ConfigKey(m_sInternalClockGroup, "beat_distance")));

    // Enable second deck, bpm and beat distance should still match original setting.
    mixxx::BeatsPointer pBeats2 = mixxx::Beats::fromConstTempo(
            m_pTrack2->getSampleRate(), mixxx::audio::kStartFramePos, mixxx::Bpm(140));
    m_pTrack2->trySetBeats(pBeats2);
    ControlObject::getControl(ConfigKey(m_sGroup2, "rate"))
            ->set(getRateSliderValue(1.0));
    ControlObject::getControl(ConfigKey(m_sGroup2, "beat_distance"))->set(0.2);
    ControlObject::getControl(ConfigKey(m_sGroup2, "play"))->set(1.0);

    m_pEngineSync->requestSyncMode(
            m_pEngineSync->getSyncableForGroup(m_sGroup2), SyncMode::Follower);
    // Deck 1 is still soft leader.
    EXPECT_TRUE(isFollower(m_sInternalClockGroup));
    EXPECT_TRUE(isSoftLeader(m_sGroup1));
    EXPECT_TRUE(isFollower(m_sGroup2));

    EXPECT_DOUBLE_EQ(
            130.0, ControlObject::get(ConfigKey(m_sInternalClockGroup, "bpm")));
    EXPECT_DOUBLE_EQ(130.0, ControlObject::get(ConfigKey(m_sGroup2, "bpm")));
    EXPECT_DOUBLE_EQ(
            0.2, ControlObject::get(ConfigKey(m_sGroup2, "beat_distance")));
    EXPECT_DOUBLE_EQ(0.2,
            ControlObject::get(
                    ConfigKey(m_sInternalClockGroup, "beat_distance")));
}

TEST_F(EngineSyncTest, MomentarySyncAlgorithmTwo) {
    m_pConfig->set(ConfigKey("[BPM]", "sync_lock_algorithm"),
            ConfigValue(EngineSync::PREFER_LOCK_BPM));

    mixxx::BeatsPointer pBeats1 = mixxx::Beats::fromConstTempo(
            m_pTrack1->getSampleRate(), mixxx::audio::kStartFramePos, mixxx::Bpm(120));
    m_pTrack1->trySetBeats(pBeats1);
    mixxx::BeatsPointer pBeats2 = mixxx::Beats::fromConstTempo(
            m_pTrack2->getSampleRate(), mixxx::audio::kStartFramePos, mixxx::Bpm(128));
    m_pTrack2->trySetBeats(pBeats2);

    ControlObject::getControl(ConfigKey(m_sGroup2, "play"))->set(1.0);
    ProcessBuffer();

    auto pButtonSyncEnabled1 =
            std::make_unique<ControlProxy>(m_sGroup1, "sync_enabled");
    pButtonSyncEnabled1->set(1.0);
    pButtonSyncEnabled1->set(0.0);

    ProcessBuffer();

    EXPECT_DOUBLE_EQ(128.0, ControlObject::get(ConfigKey(m_sGroup1, "bpm")));
}

TEST_F(EngineSyncTest, EnableOneDeckInitializesLeader) {
    // Enabling sync on a deck causes it to be leader, and sets bpm and clock.
    // Set the deck to play.
    mixxx::BeatsPointer pBeats1 = mixxx::Beats::fromConstTempo(
            m_pTrack1->getSampleRate(), mixxx::audio::kStartFramePos, mixxx::Bpm(130));
    m_pTrack1->trySetBeats(pBeats1);
    ProcessBuffer();
    ControlObject::getControl(ConfigKey(m_sGroup1, "rate"))
            ->set(getRateSliderValue(1.0));
    ControlObject::getControl(ConfigKey(m_sGroup1, "beat_distance"))->set(0.2);
    ControlObject::getControl(ConfigKey(m_sGroup1, "play"))->set(1.0);

    // Set the deck to follower.
    // As above, use direct call to avoid advancing beat distance.
    m_pEngineSync->requestSyncMode(
            m_pEngineSync->getSyncableForGroup(m_sGroup1), SyncMode::Follower);

    // That first deck is now leader
    EXPECT_TRUE(isSoftLeader(m_sGroup1));

    // Internal clock rate should be set and beat distances reset.
    EXPECT_DOUBLE_EQ(130.0,
            ControlObject::getControl(ConfigKey(m_sInternalClockGroup, "bpm"))
                    ->get());
    EXPECT_DOUBLE_EQ(130.0,
            ControlObject::getControl(ConfigKey(m_sGroup1, "bpm"))->get());
    EXPECT_DOUBLE_EQ(0.2,
            ControlObject::getControl(ConfigKey(m_sGroup1, "beat_distance"))
                    ->get());
    EXPECT_DOUBLE_EQ(0.2,
            ControlObject::getControl(
                    ConfigKey(m_sInternalClockGroup, "beat_distance"))
                    ->get());
}

TEST_F(EngineSyncTest, LoadTrackInitializesLeader) {
    // First eject the fake tracks that come with the testing framework.
    m_pChannel1->getEngineBuffer()->ejectTrack();
    m_pChannel2->getEngineBuffer()->ejectTrack();
    m_pChannel3->getEngineBuffer()->ejectTrack();
    ProcessBuffer();

    auto pButtonSyncEnabled1 =
            std::make_unique<ControlProxy>(m_sGroup1, "sync_enabled");
    pButtonSyncEnabled1->set(1.0);

    // No leader because this deck has no track.
    EXPECT_EQ(nullptr, m_pEngineSync->getLeaderChannel());
    EXPECT_DOUBLE_EQ(
            0.0, ControlObject::getControl(ConfigKey(m_sGroup1, "bpm"))->get());

    // The track load trigger a leader change.
    m_pMixerDeck1->loadFakeTrack(false, 140.0);
    ProcessBuffer();
    ProcessBuffer();

    EXPECT_TRUE(isSoftLeader(m_sGroup1));
    EXPECT_DOUBLE_EQ(140.0,
            ControlObject::getControl(ConfigKey(m_sGroup1, "bpm"))->get());

    // Play button doesn't affect leader.
    ControlObject::getControl(ConfigKey(m_sGroup1, "play"))->set(1.0);
    EXPECT_TRUE(isSoftLeader(m_sGroup1));
    ControlObject::getControl(ConfigKey(m_sGroup1, "play"))->set(0.0);

    // Ejecting the track has no effect on leader status
    m_pChannel1->getEngineBuffer()->ejectTrack();
    ProcessBuffer();
    EXPECT_TRUE(isFollower(m_sGroup1));
    // no relevant tempo available so internal clock is following
    EXPECT_TRUE(isFollower(m_sInternalClockGroup));

    auto pButtonSyncEnabled2 =
            std::make_unique<ControlProxy>(m_sGroup2, "sync_enabled");
    pButtonSyncEnabled2->set(1.0);

    m_pMixerDeck1->loadFakeTrack(false, 128.0);
    ProcessBuffer();
    ProcessBuffer();

    // Deck 2 is still empty so Deck 1 becomes leader again
    EXPECT_TRUE(isSoftLeader(m_sGroup1));
    EXPECT_DOUBLE_EQ(128.0,
            ControlObject::getControl(ConfigKey(m_sGroup1, "bpm"))->get());

    // If sync is on two decks and one deck is loaded but not playing, we should
    // initialize to that deck with internal clock leader.
    m_pMixerDeck2->loadFakeTrack(false, 110.0);
    ProcessBuffer();

    EXPECT_TRUE(isFollower(m_sInternalClockGroup));
    EXPECT_DOUBLE_EQ(128.0,
            ControlObject::getControl(ConfigKey(m_sInternalClockGroup, "bpm"))
                    ->get());
    EXPECT_DOUBLE_EQ(128.0,
            ControlObject::getControl(ConfigKey(m_sGroup1, "bpm"))->get());
    EXPECT_DOUBLE_EQ(128.0,
            ControlObject::getControl(ConfigKey(m_sGroup2, "bpm"))->get());
}

TEST_F(EngineSyncTest, LoadTrackResetTempoOption) {
    // Make sure playing decks with sync lock enabled do not change tempo when
    // the "Reset Speed/Tempo" preference is set and a track is loaded to another
    // deck with sync lock enabled.
    m_pConfig->set(ConfigKey("[Controls]", "SpeedAutoReset"),
            ConfigValue(BaseTrackPlayer::RESET_SPEED));

    // Enable sync on two stopped decks
    auto pButtonSyncEnabled1 =
            std::make_unique<ControlProxy>(m_sGroup1, "sync_enabled");
    pButtonSyncEnabled1->set(1.0);

    auto pButtonSyncEnabled2 =
            std::make_unique<ControlProxy>(m_sGroup2, "sync_enabled");
    pButtonSyncEnabled2->set(1.0);

    // If sync is on and we load a track, it should initialize leader because
    // the other track has no bpm.
    TrackPointer track1 = m_pMixerDeck1->loadFakeTrack(false, 140.0);
    ProcessBuffer();
    ProcessBuffer();

    EXPECT_DOUBLE_EQ(
            140.0, ControlObject::get(ConfigKey(m_sInternalClockGroup, "bpm")));
    EXPECT_DOUBLE_EQ(140.0, ControlObject::get(ConfigKey(m_sGroup1, "bpm")));
    EXPECT_TRUE(isSoftLeader(m_sGroup1));

    // If sync is on two decks and we load a track while one is playing,
    // that should not change the playing deck.
    ControlObject::set(ConfigKey(m_sGroup1, "play"), 1.0);

    TrackPointer track2 = m_pMixerDeck2->loadFakeTrack(false, 128.0);
    ProcessBuffer();

    EXPECT_DOUBLE_EQ(
            140.0, ControlObject::get(ConfigKey(m_sInternalClockGroup, "bpm")));
    EXPECT_DOUBLE_EQ(140.0, ControlObject::get(ConfigKey(m_sGroup1, "bpm")));
    EXPECT_DOUBLE_EQ(140.0, ControlObject::get(ConfigKey(m_sGroup2, "bpm")));

    // Repeat with RESET_PITCH_AND_SPEED
    m_pConfig->set(ConfigKey("[Controls]", "SpeedAutoReset"),
            ConfigValue(BaseTrackPlayer::RESET_PITCH_AND_SPEED));
    ControlObject::set(ConfigKey(m_sGroup1, "play"), 0.0);
    ControlObject::set(ConfigKey(m_sGroup1, "rate"), getRateSliderValue(1.0));
    track1 = m_pMixerDeck1->loadFakeTrack(false, 140.0);
    ProcessBuffer();
    ControlObject::getControl(ConfigKey(m_sGroup1, "play"))->set(1.0);
    track2 = m_pMixerDeck2->loadFakeTrack(false, 128.0);
    ProcessBuffer();

    EXPECT_DOUBLE_EQ(
            140.0, ControlObject::get(ConfigKey(m_sInternalClockGroup, "bpm")));
    EXPECT_DOUBLE_EQ(140.0, ControlObject::get(ConfigKey(m_sGroup1, "bpm")));
    EXPECT_DOUBLE_EQ(140.0, ControlObject::get(ConfigKey(m_sGroup2, "bpm")));

    // Even when RESET_NONE is on, sync lock is more important -- do not change
    // the speed of the playing deck.
    EXPECT_TRUE(isSoftLeader(m_sGroup1));
    m_pConfig->set(ConfigKey("[Controls]", "SpeedAutoReset"),
            ConfigValue(BaseTrackPlayer::RESET_NONE));
    ControlObject::set(ConfigKey(m_sGroup1, "play"), 0.0);
    track1 = m_pMixerDeck1->loadFakeTrack(false, 124.0);
    ProcessBuffer();

    ControlObject::set(ConfigKey(m_sGroup1, "rate"), getRateSliderValue(1.0));
    ControlObject::set(ConfigKey(m_sGroup1, "play"), 1.0);
    track2 = m_pMixerDeck2->loadFakeTrack(false, 128.0);
    ProcessBuffer();

    EXPECT_DOUBLE_EQ(
            124.0, ControlObject::get(ConfigKey(m_sInternalClockGroup, "bpm")));
    EXPECT_DOUBLE_EQ(124.0, ControlObject::get(ConfigKey(m_sGroup1, "bpm")));
    EXPECT_DOUBLE_EQ(124.0, ControlObject::get(ConfigKey(m_sGroup2, "bpm")));

    // Load two tracks with sync off and RESET_SPEED
    m_pConfig->set(ConfigKey("[Controls]", "SpeedAutoReset"),
            ConfigValue(BaseTrackPlayer::RESET_SPEED));
    ControlObject::getControl(ConfigKey(m_sGroup1, "play"))->set(0.0);
    ControlObject::getControl(ConfigKey(m_sGroup1, "rate"))
            ->set(getRateSliderValue(1.5));
    ControlObject::getControl(ConfigKey(m_sGroup2, "rate"))
            ->set(getRateSliderValue(1.5));
    pButtonSyncEnabled1->set(0.0);
    pButtonSyncEnabled2->set(0.0);
    track1 = m_pMixerDeck1->loadFakeTrack(false, 140.0);
    ProcessBuffer();
    ControlObject::set(ConfigKey(m_sGroup1, "play"), 1.0);
    track2 = m_pMixerDeck2->loadFakeTrack(false, 128.0);
    ProcessBuffer();
    EXPECT_DOUBLE_EQ(140.0, ControlObject::get(ConfigKey(m_sGroup1, "bpm")));
    EXPECT_DOUBLE_EQ(128.0, ControlObject::get(ConfigKey(m_sGroup2, "bpm")));

    // Load two tracks with sync off and RESET_PITCH_AND_SPEED
    m_pConfig->set(ConfigKey("[Controls]", "SpeedAutoReset"),
            ConfigValue(BaseTrackPlayer::RESET_PITCH_AND_SPEED));
    ControlObject::getControl(ConfigKey(m_sGroup1, "play"))->set(0.0);
    ControlObject::getControl(ConfigKey(m_sGroup1, "rate"))
            ->set(getRateSliderValue(1.5));
    ControlObject::getControl(ConfigKey(m_sGroup2, "rate"))
            ->set(getRateSliderValue(1.5));
    pButtonSyncEnabled1->set(0.0);
    pButtonSyncEnabled2->set(0.0);
    track1 = m_pMixerDeck1->loadFakeTrack(false, 140.0);
    ProcessBuffer();
    ControlObject::getControl(ConfigKey(m_sGroup1, "play"))->set(1.0);
    track2 = m_pMixerDeck2->loadFakeTrack(false, 128.0);
    ProcessBuffer();
    EXPECT_DOUBLE_EQ(140.0,
            ControlObject::getControl(ConfigKey(m_sGroup1, "bpm"))->get());
    EXPECT_DOUBLE_EQ(128.0,
            ControlObject::getControl(ConfigKey(m_sGroup2, "bpm"))->get());
}

TEST_F(EngineSyncTest, EnableOneDeckSliderUpdates) {
    // If we enable a deck to be leader, the internal slider should immediately update.
    auto pButtonSyncEnabled1 =
            std::make_unique<ControlProxy>(m_sGroup1, "sync_enabled");

    mixxx::BeatsPointer pBeats1 = mixxx::Beats::fromConstTempo(
            m_pTrack1->getSampleRate(), mixxx::audio::kStartFramePos, mixxx::Bpm(130));
    m_pTrack1->trySetBeats(pBeats1);
    ProcessBuffer();
    ControlObject::getControl(ConfigKey(m_sGroup1, "rate"))
            ->set(getRateSliderValue(1.0));

    // Set the deck to sync enabled.
    pButtonSyncEnabled1->set(1.0);
    ProcessBuffer();

    // Group 1 should now be leader (only one sync deck).
    EXPECT_TRUE(isSoftLeader(m_sGroup1));

    // Internal clock rate should be set.
    EXPECT_DOUBLE_EQ(130.0,
            ControlObject::getControl(ConfigKey(m_sInternalClockGroup, "bpm"))
                    ->get());
}

TEST_F(EngineSyncTest, SyncToNonSyncDeck) {
    // If deck 1 is playing, and deck 2 presses sync, deck 2 should sync to deck 1 even if
    // deck 1 is not a sync deck.

    auto pButtonSyncEnabled1 =
            std::make_unique<ControlProxy>(m_sGroup1, "sync_enabled");
    auto pButtonSyncEnabled2 =
            std::make_unique<ControlProxy>(m_sGroup2, "sync_enabled");

    mixxx::BeatsPointer pBeats1 = mixxx::Beats::fromConstTempo(
            m_pTrack1->getSampleRate(), mixxx::audio::kStartFramePos, mixxx::Bpm(130));
    m_pTrack1->trySetBeats(pBeats1);
    ProcessBuffer();
    ControlObject::set(ConfigKey(m_sGroup1, "rate"), getRateSliderValue(1.0));
    mixxx::BeatsPointer pBeats2 = mixxx::Beats::fromConstTempo(
            m_pTrack2->getSampleRate(), mixxx::audio::kStartFramePos, mixxx::Bpm(100));
    m_pTrack2->trySetBeats(pBeats2);
    ControlObject::getControl(ConfigKey(m_sGroup2, "rate"))
            ->set(getRateSliderValue(1.0));

    ControlObject::set(ConfigKey(m_sGroup1, "play"), 1.0);
    ControlObject::set(ConfigKey(m_sGroup2, "play"), 1.0);
    ProcessBuffer();

    pButtonSyncEnabled2->set(1.0);
    pButtonSyncEnabled2->set(0.0);
    ProcessBuffer();

    // There should be no leader, and deck2 should match rate of deck1.  Sync slider should be
    // updated with the value, however.
    assertNoLeader();
    EXPECT_DOUBLE_EQ(
            130.0, ControlObject::get(ConfigKey(m_sInternalClockGroup, "bpm")));
    assertSyncOff(m_sGroup2);
    EXPECT_DOUBLE_EQ(getRateSliderValue(1.3),
            ControlObject::get(ConfigKey(m_sGroup2, "rate")));

    // Reset the pitch of deck 2.
    ControlObject::set(ConfigKey(m_sGroup2, "rate"), getRateSliderValue(1.0));

    // The same should work in reverse.
    pButtonSyncEnabled1->set(1.0);
    pButtonSyncEnabled1->set(0.0);
    ProcessBuffer();

    // There should be no leader, and deck2 should match rate of deck1.
    EXPECT_EQ(0,
            ControlObject::get(
                    ConfigKey(m_sInternalClockGroup, "sync_leader")));
    EXPECT_DOUBLE_EQ(
            100.0, ControlObject::get(ConfigKey(m_sInternalClockGroup, "bpm")));
    EXPECT_EQ(nullptr, m_pEngineSync->getLeaderChannel());
    EXPECT_EQ(NULL, m_pEngineSync->getLeaderSyncable());
    EXPECT_EQ(static_cast<double>(SyncMode::None),
            ControlObject::get(ConfigKey(m_sGroup1, "sync_mode")));
    EXPECT_EQ(0, ControlObject::get(ConfigKey(m_sGroup1, "sync_enabled")));
    EXPECT_EQ(0, ControlObject::get(ConfigKey(m_sGroup1, "sync_leader")));
    EXPECT_DOUBLE_EQ(getRateSliderValue(100.0 / 130.0),
            ControlObject::get(ConfigKey(m_sGroup1, "rate")));

    // Reset again.
    ControlObject::set(ConfigKey(m_sGroup1, "rate"), getRateSliderValue(1.0));

    // If deck 1 is not playing, however, deck 2 should stay at the same rate.
    ControlObject::set(ConfigKey(m_sGroup1, "play"), 0.0);

    // The same should work in reverse.
    pButtonSyncEnabled1->set(1.0);
    pButtonSyncEnabled1->set(0.0);
    ProcessBuffer();

    // There should be no leader, and deck2 should match rate of deck1.
    EXPECT_EQ(0,
            ControlObject::get(
                    ConfigKey(m_sInternalClockGroup, "sync_leader")));
    EXPECT_DOUBLE_EQ(
            100.0, ControlObject::get(ConfigKey(m_sInternalClockGroup, "bpm")));
    EXPECT_EQ(nullptr, m_pEngineSync->getLeaderChannel());
    EXPECT_EQ(NULL, m_pEngineSync->getLeaderSyncable());
    EXPECT_EQ(static_cast<double>(SyncMode::None),
            ControlObject::get(ConfigKey(m_sGroup2, "sync_mode")));
    EXPECT_EQ(0, ControlObject::get(ConfigKey(m_sGroup2, "sync_enabled")));
    EXPECT_EQ(0, ControlObject::get(ConfigKey(m_sGroup2, "sync_leader")));
    EXPECT_DOUBLE_EQ(getRateSliderValue(1.0),
            ControlObject::get(ConfigKey(m_sGroup2, "rate")));
}

TEST_F(EngineSyncTest, MomentarySyncDependsOnPlayingStates) {
    // Like it says -- if the current deck is playing, and the target deck is
    // playing, they should sync even if there's no sync mode enabled.

    auto pButtonSyncEnabled1 =
            std::make_unique<ControlProxy>(m_sGroup1, "sync_enabled");
    auto pButtonSyncEnabled2 =
            std::make_unique<ControlProxy>(m_sGroup2, "sync_enabled");

    // Set up decks so they can be playing, and start deck 1.
    mixxx::BeatsPointer pBeats1 = mixxx::Beats::fromConstTempo(
            m_pTrack1->getSampleRate(), mixxx::audio::kStartFramePos, mixxx::Bpm(100));
    m_pTrack1->trySetBeats(pBeats1);
    ControlObject::set(ConfigKey(m_sGroup1, "rate"), getRateSliderValue(1.0));
    ControlObject::set(ConfigKey(m_sGroup1, "play"), 1.0);
    mixxx::BeatsPointer pBeats2 = mixxx::Beats::fromConstTempo(
            m_pTrack2->getSampleRate(), mixxx::audio::kStartFramePos, mixxx::Bpm(130));
    m_pTrack2->trySetBeats(pBeats2);
    ControlObject::set(ConfigKey(m_sGroup2, "rate"), getRateSliderValue(1.0));
    ControlObject::set(ConfigKey(m_sGroup2, "play"), 1.0);
    ProcessBuffer();
    ProcessBuffer();

    // Set channel 1 to be enabled momentarily.
    pButtonSyncEnabled1->set(1.0);
    pButtonSyncEnabled1->set(0.0);
    ProcessBuffer();

    // The sync lock should still be off and the speed should match deck 2.
    assertNoLeader();
    assertSyncOff(m_sGroup1);
    assertSyncOff(m_sGroup2);
    EXPECT_DOUBLE_EQ(
            130.0, ControlObject::get(ConfigKey(m_sInternalClockGroup, "bpm")));

    // Also works if deck 1 is not playing.
    ControlObject::set(ConfigKey(m_sGroup1, "rate"), getRateSliderValue(1.0));
    ControlObject::set(ConfigKey(m_sGroup1, "play"), 0.0);
    ControlObject::set(ConfigKey(m_sGroup2, "play"), 1.0);
    ProcessBuffer();
    pButtonSyncEnabled1->set(1.0);
    pButtonSyncEnabled1->set(0.0);
    ProcessBuffer();
    assertNoLeader();
    assertSyncOff(m_sGroup1);
    assertSyncOff(m_sGroup2);
    EXPECT_DOUBLE_EQ(
            130.0, ControlObject::get(ConfigKey(m_sInternalClockGroup, "bpm")));

    // Also works if neither deck is playing.
    ControlObject::set(ConfigKey(m_sGroup1, "rate"), getRateSliderValue(1.0));
    ControlObject::set(ConfigKey(m_sGroup1, "play"), 0.0);
    ControlObject::set(ConfigKey(m_sGroup2, "play"), 0.0);
    ProcessBuffer();
    pButtonSyncEnabled1->set(1.0);
    pButtonSyncEnabled1->set(0.0);
    ProcessBuffer();
    assertNoLeader();
    assertSyncOff(m_sGroup1);
    assertSyncOff(m_sGroup2);
    EXPECT_DOUBLE_EQ(
            130.0, ControlObject::get(ConfigKey(m_sInternalClockGroup, "bpm")));

    // But it doesn't work if deck 2 isn't playing and deck 1 is. (This would
    // cause deck1 to suddenly change bpm while playing back).
    ControlObject::set(ConfigKey(m_sGroup1, "rate"), getRateSliderValue(1.0));
    ControlObject::set(ConfigKey(m_sGroup1, "play"), 1.0);
    ControlObject::set(ConfigKey(m_sGroup2, "play"), 0.0);
    ProcessBuffer();
    pButtonSyncEnabled1->set(1.0);
    pButtonSyncEnabled1->set(0.0);
    ProcessBuffer();
    assertNoLeader();
    assertSyncOff(m_sGroup1);
    assertSyncOff(m_sGroup2);
    EXPECT_DOUBLE_EQ(
            100.0, ControlObject::get(ConfigKey(m_sInternalClockGroup, "bpm")));
}

TEST_F(EngineSyncTest, EjectTrackSyncRemains) {
    auto pButtonSyncEnabled1 =
            std::make_unique<ControlProxy>(m_sGroup1, "sync_enabled");
    auto pButtonSyncEnabled2 =
            std::make_unique<ControlProxy>(m_sGroup2, "sync_enabled");

    mixxx::BeatsPointer pBeats1 = mixxx::Beats::fromConstTempo(
            m_pTrack1->getSampleRate(), mixxx::audio::kStartFramePos, mixxx::Bpm(120));
    m_pTrack1->trySetBeats(pBeats1);
    ProcessBuffer();
    pButtonSyncEnabled1->set(1.0);
    ProcessBuffer();

    // m_sGroup1 takes over
    EXPECT_TRUE(isFollower(m_sInternalClockGroup));
    EXPECT_TRUE(isSoftLeader(m_sGroup1));
    assertSyncOff(m_sGroup2);

    m_pChannel1->getEngineBuffer()->ejectTrack();
    // When an eject happens, the bpm gets set to zero.
    ProcessBuffer();

    EXPECT_DOUBLE_EQ(
            0.0, ControlObject::getControl(ConfigKey(m_sGroup1, "bpm"))->get());

    // No valid tempo available all are waiting as follower
    EXPECT_TRUE(isFollower(m_sInternalClockGroup));
    EXPECT_TRUE(isFollower(m_sGroup1));
    assertSyncOff(m_sGroup2);

    m_pMixerDeck1->loadFakeTrack(false, 128.0);
    ProcessBuffer();
    EXPECT_DOUBLE_EQ(128.0,
            ControlObject::getControl(ConfigKey(m_sGroup1, "bpm"))->get());
    mixxx::BeatsPointer pBeats2 = mixxx::Beats::fromConstTempo(
            m_pTrack2->getSampleRate(), mixxx::audio::kStartFramePos, mixxx::Bpm(135));
    m_pTrack2->trySetBeats(pBeats2);
    pButtonSyncEnabled2->set(1.0);
    ProcessBuffer();

    EXPECT_TRUE(isFollower(m_sInternalClockGroup));
    EXPECT_TRUE(isSoftLeader(m_sGroup1));
    EXPECT_TRUE(isFollower(m_sGroup2));

    m_pChannel1->getEngineBuffer()->ejectTrack();
    m_pTrack1->trySetBeats(mixxx::BeatsPointer());
    ProcessBuffer();

    EXPECT_TRUE(isFollower(m_sInternalClockGroup));
    EXPECT_TRUE(isFollower(m_sGroup1));
    EXPECT_TRUE(isSoftLeader(m_sGroup2));
}

TEST_F(EngineSyncTest, FileBpmChangesDontAffectLeader) {
    // If filebpm changes, don't treat it like a rate change.
    mixxx::BeatsPointer pBeats1 = mixxx::Beats::fromConstTempo(
            m_pTrack1->getSampleRate(), mixxx::audio::kStartFramePos, mixxx::Bpm(100));
    m_pTrack1->trySetBeats(pBeats1);
    auto pButtonSyncEnabled1 =
            std::make_unique<ControlProxy>(m_sGroup1, "sync_enabled");
    pButtonSyncEnabled1->set(1.0);
    ProcessBuffer();

    mixxx::BeatsPointer pBeats2 = mixxx::Beats::fromConstTempo(
            m_pTrack2->getSampleRate(), mixxx::audio::kStartFramePos, mixxx::Bpm(120));
    m_pTrack2->trySetBeats(pBeats2);
    auto pButtonSyncEnabled2 =
            std::make_unique<ControlProxy>(m_sGroup2, "sync_enabled");
    pButtonSyncEnabled2->set(1.0);
    ProcessBuffer();

    EXPECT_TRUE(isSoftLeader(m_sGroup1));
    EXPECT_TRUE(isFollower(m_sGroup2));

    // Update the leader's beats -- update the internal clock
    pBeats1 = mixxx::Beats::fromConstTempo(
            m_pTrack1->getSampleRate(), mixxx::audio::kStartFramePos, mixxx::Bpm(160));
    m_pTrack1->trySetBeats(pBeats1);
    EXPECT_DOUBLE_EQ(
            160.0, ControlObject::get(ConfigKey(m_sGroup1, "bpm")));
    EXPECT_DOUBLE_EQ(
            160.0, ControlObject::get(ConfigKey(m_sInternalClockGroup, "bpm")));

    EXPECT_TRUE(isSoftLeader(m_sGroup1));

    // Update follower beats -- don't update internal clock.
    pBeats2 = mixxx::Beats::fromConstTempo(
            m_pTrack2->getSampleRate(), mixxx::audio::kStartFramePos, mixxx::Bpm(140));
    m_pTrack2->trySetBeats(pBeats2);
    EXPECT_DOUBLE_EQ(
            160.0, ControlObject::get(ConfigKey(m_sInternalClockGroup, "bpm")));
}

TEST_F(EngineSyncTest, ExplicitLeaderPostProcessed) {
    // Regression test thanks to a bug.  Make sure that an explicit leader
    // channel gets post-processed.
    auto pButtonLeaderSync1 =
            std::make_unique<ControlProxy>(m_sGroup1, "sync_mode");
    pButtonLeaderSync1->set(static_cast<double>(SyncMode::LeaderExplicit));
    mixxx::BeatsPointer pBeats1 = mixxx::Beats::fromConstTempo(
            m_pTrack1->getSampleRate(), mixxx::audio::kStartFramePos, mixxx::Bpm(160));
    m_pTrack1->trySetBeats(pBeats1);
    ProcessBuffer();
    ControlObject::getControl(ConfigKey(m_sGroup1, "play"))->set(1.0);
    ProcessBuffer();

    EXPECT_NEAR(0.0023219956,
            m_pChannel1->getEngineBuffer()->getVisualPlayPos(),
            kMaxFloatingPointErrorLowPrecision);
}

TEST_F(EngineSyncTest, ZeroBPMRateAdjustIgnored) {
    // If a track isn't loaded (0 bpm), but the deck has sync enabled,
    // don't pay attention to rate changes.
    m_pTrack1->trySetBeats(nullptr);
    auto pButtonSyncEnabled1 =
            std::make_unique<ControlProxy>(m_sGroup1, "sync_enabled");
    pButtonSyncEnabled1->set(1.0);
    ControlObject::getControl(ConfigKey(m_sGroup1, "rate"))
            ->set(getRateSliderValue(1.0));
    ProcessBuffer();

    mixxx::BeatsPointer pBeats2 = mixxx::Beats::fromConstTempo(
            m_pTrack2->getSampleRate(), mixxx::audio::kStartFramePos, mixxx::Bpm(120));
    m_pTrack2->trySetBeats(pBeats2);
    auto pButtonSyncEnabled2 =
            std::make_unique<ControlProxy>(m_sGroup2, "sync_enabled");
    pButtonSyncEnabled2->set(1.0);
    ProcessBuffer();

    ControlObject::getControl(ConfigKey(m_sGroup1, "rate"))
            ->set(getRateSliderValue(1.3));

    EXPECT_DOUBLE_EQ(getRateSliderValue(1.0),
            ControlObject::getControl(ConfigKey(m_sGroup2, "rate"))->get());

    // Also try with explicit leader/follower setting
    pButtonSyncEnabled1->set(0.0);
    ControlObject::getControl(ConfigKey(m_sGroup1, "sync_mode"))
            ->set(static_cast<double>(SyncMode::LeaderExplicit));

    ControlObject::getControl(ConfigKey(m_sGroup1, "rate"))
            ->set(getRateSliderValue(1.4));
    EXPECT_DOUBLE_EQ(getRateSliderValue(1.0),
            ControlObject::getControl(ConfigKey(m_sGroup2, "rate"))->get());

    pButtonSyncEnabled1->set(0.0);
    ControlObject::getControl(ConfigKey(m_sGroup1, "sync_mode"))
            ->set(static_cast<double>(SyncMode::Follower));

    ControlObject::getControl(ConfigKey(m_sGroup1, "rate"))
            ->set(getRateSliderValue(0.9));
    EXPECT_DOUBLE_EQ(getRateSliderValue(1.0),
            ControlObject::getControl(ConfigKey(m_sGroup2, "rate"))->get());
}

TEST_F(EngineSyncTest, BeatDistanceBeforeStart) {
    // https://github.com/mixxxdj/mixxx/issues/10423
    // If the start position is before zero, we should still initialize the beat distance
    // correctly.  Unfortunately, this currently doesn't work.

    mixxx::BeatsPointer pBeats1 = mixxx::Beats::fromConstTempo(
            m_pTrack1->getSampleRate(), mixxx::audio::kStartFramePos, mixxx::Bpm(128));
    m_pTrack1->trySetBeats(pBeats1);
    ControlObject::getControl(ConfigKey(m_sGroup1, "quantize"))->set(1.0);
    ControlObject::set(ConfigKey(m_sGroup1, "playposition"), -.05);
    ProcessBuffer();
    ControlObject::getControl(ConfigKey(m_sGroup1, "play"))->set(1.0);
    ProcessBuffer();
    // This fraction is one buffer beyond the seek position -- indicating that
    // we seeked correctly.
    EXPECT_NEAR(0.49143461829176116,
            ControlObject::getControl(ConfigKey(m_sGroup1, "beat_distance"))->get(),
            kMaxBeatDistanceEpsilon);
}

TEST_F(EngineSyncTest, ZeroLatencyRateChangeNoQuant) {
    // Confirm that a rate change in an explicit leader is instantly communicated
    // to followers.
    mixxx::BeatsPointer pBeats1 = mixxx::Beats::fromConstTempo(
            m_pTrack1->getSampleRate(), mixxx::audio::kStartFramePos, mixxx::Bpm(128));
    m_pTrack1->trySetBeats(pBeats1);
    mixxx::BeatsPointer pBeats2 = mixxx::Beats::fromConstTempo(
            m_pTrack2->getSampleRate(), mixxx::audio::kStartFramePos, mixxx::Bpm(160));
    m_pTrack2->trySetBeats(pBeats2);
    ProcessBuffer();

    // Make Channel2 leader to weed out any channel ordering issues.
    ControlObject::getControl(ConfigKey(m_sGroup2, "sync_mode"))
            ->set(static_cast<double>(SyncMode::Follower));
    ControlObject::getControl(ConfigKey(m_sGroup1, "sync_mode"))
            ->set(static_cast<double>(SyncMode::Follower));
    // Exaggerate the effect with a high rate.
    ControlObject::set(ConfigKey(m_sGroup2, "rate_ratio"), 10.0);

    ControlObject::getControl(ConfigKey(m_sGroup1, "play"))->set(1.0);
    ControlObject::getControl(ConfigKey(m_sGroup2, "play"))->set(1.0);

    EXPECT_EQ(ControlObject::getControl(ConfigKey(m_sGroup2, "beat_distance"))
                      ->get(),
            ControlObject::getControl(ConfigKey(m_sGroup1, "beat_distance"))
                    ->get());

    for (int i = 0; i < 50; ++i) {
        ProcessBuffer();
        // Keep messing with the rate
        double rate = i % 2 == 0 ? i / 10.0 : i / -10.0;
        ControlObject::set(ConfigKey(m_sGroup2, "rate_ratio"), rate);

        // Buffers should be in sync.
        EXPECT_NEAR(
                ControlObject::getControl(ConfigKey(m_sGroup2, "beat_distance"))->get(),
                ControlObject::getControl(ConfigKey(m_sGroup1, "beat_distance"))->get(),
                kMaxBeatDistanceEpsilon);
    }

    // Make sure we're actually going somewhere!
    EXPECT_GT(
            ControlObject::getControl(ConfigKey(m_sGroup1, "beat_distance"))->get(),
            0);
    // Buffers should be in sync.
    EXPECT_NEAR(
            ControlObject::getControl(ConfigKey(m_sGroup2, "beat_distance"))->get(),
            ControlObject::getControl(ConfigKey(m_sGroup1, "beat_distance"))->get(),
            kMaxBeatDistanceEpsilon);
}

TEST_F(EngineSyncTest, ZeroLatencyRateChangeQuant) {
    // Confirm that a rate change in an explicit leader is instantly communicated
    // to followers.
    mixxx::BeatsPointer pBeats1 = mixxx::Beats::fromConstTempo(
            m_pTrack1->getSampleRate(), mixxx::audio::kStartFramePos, mixxx::Bpm(128));
    m_pTrack1->trySetBeats(pBeats1);
    mixxx::BeatsPointer pBeats2 = mixxx::Beats::fromConstTempo(
            m_pTrack2->getSampleRate(), mixxx::audio::kStartFramePos, mixxx::Bpm(160));
    m_pTrack2->trySetBeats(pBeats2);
    ProcessBuffer();

    ControlObject::getControl(ConfigKey(m_sGroup1, "quantize"))->set(1.0);
    ControlObject::getControl(ConfigKey(m_sGroup2, "quantize"))->set(1.0);

    // Make Channel2 leader to weed out any channel ordering issues.
    ControlObject::getControl(ConfigKey(m_sGroup2, "sync_mode"))
            ->set(static_cast<double>(SyncMode::Follower));
    ControlObject::getControl(ConfigKey(m_sGroup1, "sync_mode"))
            ->set(static_cast<double>(SyncMode::Follower));
    // Exaggerate the effect with a high rate.
    ControlObject::set(ConfigKey(m_sGroup2, "rate_ratio"), 10.0);

    ControlObject::getControl(ConfigKey(m_sGroup1, "play"))->set(1.0);
    ControlObject::getControl(ConfigKey(m_sGroup2, "play"))->set(1.0);
    ProcessBuffer();

    EXPECT_EQ(ControlObject::getControl(ConfigKey(m_sGroup2, "beat_distance"))
                      ->get(),
            ControlObject::getControl(ConfigKey(m_sGroup1, "beat_distance"))
                    ->get());

    for (int i = 0; i < 50; ++i) {
        ProcessBuffer();
        // Keep messing with the rate
        double rate = i % 2 == 0 ? i / 10.0 : i / -10.0;
        ControlObject::set(ConfigKey(m_sGroup2, "rate_ratio"), rate);

        // Buffers should be in sync.
        EXPECT_NEAR(
                ControlObject::getControl(ConfigKey(m_sGroup2, "beat_distance"))->get(),
                ControlObject::getControl(ConfigKey(m_sGroup1, "beat_distance"))->get(),
                kMaxBeatDistanceEpsilon);
    }

    // Make sure we're actually going somewhere!
    EXPECT_GT(
            ControlObject::getControl(ConfigKey(m_sGroup1, "beat_distance"))->get(),
            0);
    // Buffers should be in sync.
    EXPECT_NEAR(
            ControlObject::getControl(ConfigKey(m_sGroup2, "beat_distance"))->get(),
            ControlObject::getControl(ConfigKey(m_sGroup1, "beat_distance"))->get(),
            kMaxBeatDistanceEpsilon);
}

TEST_F(EngineSyncTest, ZeroLatencyRateDiffQuant) {
    // Confirm that a rate change in an explicit leader is instantly communicated
    // to followers.
    mixxx::BeatsPointer pBeats1 = mixxx::Beats::fromConstTempo(
            m_pTrack1->getSampleRate(), mixxx::audio::kStartFramePos, mixxx::Bpm(128));
    m_pTrack1->trySetBeats(pBeats1);
    mixxx::BeatsPointer pBeats2 = mixxx::Beats::fromConstTempo(
            m_pTrack2->getSampleRate(), mixxx::audio::kStartFramePos, mixxx::Bpm(160));
    m_pTrack2->trySetBeats(pBeats2);
    ProcessBuffer();

    ControlObject::getControl(ConfigKey(m_sGroup2, "quantize"))->set(1.0);

    // Make Channel2 leader to weed out any channel ordering issues.
    ControlObject::getControl(ConfigKey(m_sGroup2, "sync_mode"))
            ->set(static_cast<double>(SyncMode::Follower));
    ControlObject::getControl(ConfigKey(m_sGroup1, "sync_mode"))
            ->set(static_cast<double>(SyncMode::Follower));
    // Exaggerate the effect with a high rate.
    ControlObject::set(ConfigKey(m_sGroup2, "rate_ratio"), 10.0);

    ControlObject::getControl(ConfigKey(m_sGroup1, "play"))->set(1.0);
    ControlObject::getControl(ConfigKey(m_sGroup2, "play"))->set(1.0);

    EXPECT_EQ(ControlObject::getControl(ConfigKey(m_sGroup2, "beat_distance"))
                      ->get(),
            ControlObject::getControl(ConfigKey(m_sGroup1, "beat_distance"))
                    ->get());

    ProcessBuffer();

    for (int i = 0; i < 50; ++i) {
        ProcessBuffer();
        // Keep messing with the rate
        double rate = i % 2 == 0 ? i / 10.0 : i / -10.0;
        ControlObject::set(ConfigKey(m_sGroup2, "rate_ratio"), rate);

        // Buffers should be in sync.
        EXPECT_NEAR(
                ControlObject::getControl(ConfigKey(m_sGroup2, "beat_distance"))->get(),
                ControlObject::getControl(ConfigKey(m_sGroup1, "beat_distance"))->get(),
                kMaxBeatDistanceEpsilon);
    }

    // Make sure we're actually going somewhere!
    EXPECT_GT(
            ControlObject::getControl(ConfigKey(m_sGroup1, "beat_distance"))->get(),
            0);
    // Buffers should be in sync.
    EXPECT_NEAR(
            ControlObject::getControl(ConfigKey(m_sGroup2, "beat_distance"))->get(),
            ControlObject::getControl(ConfigKey(m_sGroup1, "beat_distance"))->get(),
            kMaxBeatDistanceEpsilon);
}

// In this test, we set play *first* and then turn on sync lock.
// This exercises a slightly different ordering of signals that we
// need to check. The Sync feature is unfortunately brittle.
// This test exercises https://github.com/mixxxdj/mixxx/issues/10024
TEST_F(EngineSyncTest, ActivatingSyncDoesNotCauseDrifting) {
    mixxx::BeatsPointer pBeats1 = mixxx::Beats::fromConstTempo(
            m_pTrack1->getSampleRate(), mixxx::audio::kStartFramePos, mixxx::Bpm(150));
    m_pTrack1->trySetBeats(pBeats1);
    mixxx::BeatsPointer pBeats2 = mixxx::Beats::fromConstTempo(
            m_pTrack2->getSampleRate(), mixxx::audio::kStartFramePos, mixxx::Bpm(150));
    m_pTrack2->trySetBeats(pBeats2);

    ControlObject::getControl(ConfigKey(m_sGroup1, "quantize"))->set(0.0);
    ControlObject::getControl(ConfigKey(m_sGroup2, "quantize"))->set(1.0);

    ControlObject::getControl(ConfigKey(m_sGroup1, "play"))->set(1.0);
    ControlObject::getControl(ConfigKey(m_sGroup2, "play"))->set(1.0);

    ProcessBuffer();

    // make sure we aren't out-of-sync from the start
    EXPECT_EQ(ControlObject::getControl(ConfigKey(m_sGroup2, "beat_distance"))
                      ->get(),
            ControlObject::getControl(ConfigKey(m_sGroup1, "beat_distance"))
                    ->get());

    // engage first sync-leader
    ControlObject::getControl(ConfigKey(m_sGroup1, "sync_mode"))
            ->set(static_cast<double>(SyncMode::Follower));

    // engage second Sync-leader
    ControlObject::getControl(ConfigKey(m_sGroup2, "sync_mode"))
            ->set(static_cast<double>(SyncMode::Follower));

    // Run for a number of buffers
    for (int i = 0; i < 25; ++i) {
        ProcessBuffer();
    }
    // Make sure we're actually going somewhere!
    EXPECT_GT(ControlObject::getControl(ConfigKey(m_sGroup1, "beat_distance"))
                      ->get(),
            0);

    // Buffers should be in sync.
    EXPECT_EQ(ControlObject::getControl(ConfigKey(m_sGroup2, "beat_distance"))
                      ->get(),
            ControlObject::getControl(ConfigKey(m_sGroup1, "beat_distance"))
                    ->get());
}

TEST_F(EngineSyncTest, HalfDoubleBpmTest) {
    mixxx::BeatsPointer pBeats1 = mixxx::Beats::fromConstTempo(
            m_pTrack1->getSampleRate(), mixxx::audio::kStartFramePos, mixxx::Bpm(70));
    m_pTrack1->trySetBeats(pBeats1);
    mixxx::BeatsPointer pBeats2 = mixxx::Beats::fromConstTempo(
            m_pTrack2->getSampleRate(), mixxx::audio::kStartFramePos, mixxx::Bpm(140));
    m_pTrack2->trySetBeats(pBeats2);

    // Mixxx will choose the first playing deck to be leader.  Let's start deck 2 first.
    ControlObject::getControl(ConfigKey(m_sGroup1, "volume"))->set(1.0);
    ControlObject::getControl(ConfigKey(m_sGroup2, "volume"))->set(1.0);
    ProcessBuffer();
    ControlObject::getControl(ConfigKey(m_sGroup1, "quantize"))->set(1.0);
    ControlObject::getControl(ConfigKey(m_sGroup2, "quantize"))->set(1.0);
    ControlObject::getControl(ConfigKey(m_sGroup2, "sync_enabled"))->set(1);
    ControlObject::getControl(ConfigKey(m_sGroup1, "sync_enabled"))->set(1);
    ProcessBuffer();
    ControlObject::getControl(ConfigKey(m_sGroup2, "play"))->set(1.0);
    ControlObject::getControl(ConfigKey(m_sGroup1, "play"))->set(1.0);
    ProcessBuffer();

    ASSERT_TRUE(isSoftLeader(m_sGroup2));
    ASSERT_TRUE(isFollower(m_sGroup1));

    EXPECT_EQ(0.5,
            m_pChannel1->getEngineBuffer()
                    ->m_pSyncControl->m_leaderBpmAdjustFactor);
    EXPECT_EQ(1.0,
            m_pChannel2->getEngineBuffer()
                    ->m_pSyncControl->m_leaderBpmAdjustFactor);
    EXPECT_DOUBLE_EQ(
            m_pChannel1->getEngineBuffer()->m_pSyncControl->getBeatDistance(),
            m_pChannel2->getEngineBuffer()->m_pSyncControl->getBeatDistance());
    EXPECT_EQ(0, ControlObject::get(ConfigKey(m_sGroup1, "rate")));
    EXPECT_EQ(70, ControlObject::get(ConfigKey(m_sGroup1, "bpm")));
    EXPECT_EQ(0, ControlObject::get(ConfigKey(m_sGroup2, "rate")));
    EXPECT_EQ(140, ControlObject::get(ConfigKey(m_sGroup2, "bpm")));

    // Do lots of processing to make sure we get over the 0.5 beat_distance barrier.
    for (int i = 0; i < 50; ++i) {
        ProcessBuffer();
        // The beat distances are NOT as simple as x2 or /2.  Use the built-in functions
        // to do the proper conversion.
        EXPECT_NEAR(
                m_pChannel1->getEngineBuffer()->m_pSyncControl->getBeatDistance(),
                m_pChannel2->getEngineBuffer()->m_pSyncControl->getBeatDistance(),
                kMaxFloatingPointErrorLowPrecision);
    }

    ControlObject::getControl(ConfigKey(m_sGroup1, "play"))->set(0.0);
    ControlObject::getControl(ConfigKey(m_sGroup2, "play"))->set(0.0);

    ControlObject::getControl(ConfigKey(m_sGroup1, "sync_mode"))
            ->set(static_cast<double>(SyncMode::None));
    ControlObject::getControl(ConfigKey(m_sGroup2, "sync_mode"))
            ->set(static_cast<double>(SyncMode::None));

    EXPECT_EQ(1.0,
            m_pChannel1->getEngineBuffer()
                    ->m_pSyncControl->m_leaderBpmAdjustFactor);
    EXPECT_EQ(1.0,
            m_pChannel2->getEngineBuffer()
                    ->m_pSyncControl->m_leaderBpmAdjustFactor);

    ControlObject::getControl(ConfigKey(m_sGroup1, "sync_mode"))
            ->set(static_cast<double>(SyncMode::Follower));
    ControlObject::getControl(ConfigKey(m_sGroup2, "sync_mode"))
            ->set(static_cast<double>(SyncMode::Follower));
    ControlObject::getControl(ConfigKey(m_sGroup1, "rate"))
            ->set(getRateSliderValue(1.0));

    // Now start deck 1 first and check again.
    ControlObject::getControl(ConfigKey(m_sGroup1, "play"))->set(1.0);
    ControlObject::getControl(ConfigKey(m_sGroup2, "play"))->set(1.0);

    ProcessBuffer();

    EXPECT_EQ(1.0,
            m_pChannel1->getEngineBuffer()
                    ->m_pSyncControl->m_leaderBpmAdjustFactor);
    EXPECT_EQ(2.0,
            m_pChannel2->getEngineBuffer()
                    ->m_pSyncControl->m_leaderBpmAdjustFactor);

    // Exaggerate the effect with a high rate.
    ControlObject::getControl(ConfigKey(m_sGroup2, "rate"))
            ->set(getRateSliderValue(2.0));

    for (int i = 0; i < 50; ++i) {
        ProcessBuffer();
        EXPECT_DOUBLE_EQ(m_pChannel1->getEngineBuffer()
                                 ->m_pSyncControl->getBeatDistance(),
                m_pChannel2->getEngineBuffer()
                        ->m_pSyncControl->getBeatDistance());
    }
}

TEST_F(EngineSyncTest, HalfDoubleThenPlay) {
    // If a deck plays that had its multiplier set, we need to reset the
    // internal clock.
    mixxx::BeatsPointer pBeats1 = mixxx::Beats::fromConstTempo(
            m_pTrack1->getSampleRate(), mixxx::audio::kStartFramePos, mixxx::Bpm(80));
    m_pTrack1->trySetBeats(pBeats1);
    ProcessBuffer();
    mixxx::BeatsPointer pBeats2 = mixxx::Beats::fromConstTempo(
            m_pTrack2->getSampleRate(), mixxx::audio::kStartFramePos, mixxx::Bpm(175));
    m_pTrack2->trySetBeats(pBeats2);
    ProcessBuffer();
    ControlObject::getControl(ConfigKey(m_sGroup1, "rate"))
            ->set(getRateSliderValue(1.0));

    auto pButtonSyncEnabled1 =
            std::make_unique<ControlProxy>(m_sGroup1, "sync_enabled");
    pButtonSyncEnabled1->set(1.0);
    auto pButtonSyncEnabled2 =
            std::make_unique<ControlProxy>(m_sGroup2, "sync_enabled");
    pButtonSyncEnabled2->set(1.0);
    ControlObject::getControl(ConfigKey(m_sGroup1, "quantize"))->set(1.0);
    ControlObject::getControl(ConfigKey(m_sGroup2, "quantize"))->set(1.0);

    // We expect that m_sGroup1 has adjusted its own bpm to the second deck and becomes a single leader.
    // The internal clock is initialized right away.
    EXPECT_TRUE(isSoftLeader(m_sGroup1));
    EXPECT_TRUE(isFollower(m_sGroup2));
    EXPECT_DOUBLE_EQ(1.0,
            m_pChannel1->getEngineBuffer()
                    ->m_pSyncControl->m_leaderBpmAdjustFactor);
    EXPECT_DOUBLE_EQ(2.0,
            m_pChannel2->getEngineBuffer()
                    ->m_pSyncControl->m_leaderBpmAdjustFactor);
    EXPECT_DOUBLE_EQ(87.5,
            ControlObject::getControl(ConfigKey(m_sInternalClockGroup, "bpm"))
                    ->get());
    EXPECT_DOUBLE_EQ(87.5,
            ControlObject::getControl(ConfigKey(m_sGroup1, "bpm"))->get());
    EXPECT_DOUBLE_EQ(175.0,
            ControlObject::getControl(ConfigKey(m_sGroup2, "bpm"))->get());
    EXPECT_DOUBLE_EQ(87.5 / 80,
            ControlObject::getControl(ConfigKey(m_sGroup1, "rate_ratio"))
                    ->get());
    EXPECT_DOUBLE_EQ(1.0,
            ControlObject::getControl(ConfigKey(m_sGroup2, "rate_ratio"))
                    ->get());
    // Local bpms are not adjusted by the multiplier
    EXPECT_DOUBLE_EQ(80,
            ControlObject::getControl(ConfigKey(m_sGroup1, "local_bpm"))
                    ->get());
    EXPECT_DOUBLE_EQ(175.0,
            ControlObject::getControl(ConfigKey(m_sGroup2, "local_bpm"))
                    ->get());

    ControlObject::getControl(ConfigKey(m_sGroup2, "play"))->set(1.0);
    ControlObject::getControl(ConfigKey(m_sGroup1, "play"))->set(1.0);

    EXPECT_DOUBLE_EQ(175,
            ControlObject::getControl(ConfigKey(m_sInternalClockGroup, "bpm"))
                    ->get());

    EXPECT_DOUBLE_EQ(
            m_pChannel1->getEngineBuffer()->m_pSyncControl->getBeatDistance(),
            m_pChannel2->getEngineBuffer()->m_pSyncControl->getBeatDistance());

    // Now enable the other deck first.
    // Sync only cares about which deck plays first now, enable order is irrelevant.
    ControlObject::getControl(ConfigKey(m_sGroup1, "play"))->set(0.0);
    ControlObject::getControl(ConfigKey(m_sGroup2, "play"))->set(0.0);
    pButtonSyncEnabled1->set(0.0);
    pButtonSyncEnabled2->set(0.0);
    ProcessBuffer();
    pButtonSyncEnabled2->set(1.0);
    pButtonSyncEnabled1->set(1.0);
    EXPECT_DOUBLE_EQ(0.5,
            m_pChannel1->getEngineBuffer()
                    ->m_pSyncControl->m_leaderBpmAdjustFactor);
    EXPECT_DOUBLE_EQ(1.0,
            m_pChannel2->getEngineBuffer()
                    ->m_pSyncControl->m_leaderBpmAdjustFactor);
    ControlObject::getControl(ConfigKey(m_sGroup1, "play"))->set(1.0);
    ControlObject::getControl(ConfigKey(m_sGroup2, "play"))->set(1.0);

    EXPECT_DOUBLE_EQ(87.5 / 80,
            ControlObject::getControl(ConfigKey(m_sGroup1, "rate_ratio"))
                    ->get());
    EXPECT_DOUBLE_EQ(1.0,
            ControlObject::getControl(ConfigKey(m_sGroup2, "rate_ratio"))
                    ->get());

    EXPECT_NEAR(
            m_pChannel1->getEngineBuffer()->m_pSyncControl->getBeatDistance(),
            m_pChannel2->getEngineBuffer()->m_pSyncControl->getBeatDistance(),
            kMaxFloatingPointErrorHighPrecision);

    // Check if it stays in sync after processing some buffer
    ProcessBuffer();
    ProcessBuffer();
    ProcessBuffer();

    EXPECT_DOUBLE_EQ(87.5,
            ControlObject::getControl(ConfigKey(m_sGroup1, "bpm"))->get());

    EXPECT_NEAR(
            m_pChannel1->getEngineBuffer()->m_pSyncControl->getBeatDistance(),
            m_pChannel2->getEngineBuffer()->m_pSyncControl->getBeatDistance(),
            kMaxFloatingPointErrorHighPrecision);

    // Check if it stays in sync after processing some buffer
    ProcessBuffer();
    ProcessBuffer();
    ProcessBuffer();

    EXPECT_NEAR(
            m_pChannel1->getEngineBuffer()->m_pSyncControl->getBeatDistance(),
            m_pChannel2->getEngineBuffer()->m_pSyncControl->getBeatDistance(),
            kMaxFloatingPointErrorHighPrecision);
}

TEST_F(EngineSyncTest, HalfDoubleInternalClockTest) {
    // If we set the file_bpm CO's directly, the correct signals aren't fired.
    mixxx::BeatsPointer pBeats1 = mixxx::Beats::fromConstTempo(
            m_pTrack1->getSampleRate(), mixxx::audio::kStartFramePos, mixxx::Bpm(70));
    m_pTrack1->trySetBeats(pBeats1);
    ProcessBuffer();
    mixxx::BeatsPointer pBeats2 = mixxx::Beats::fromConstTempo(
            m_pTrack2->getSampleRate(), mixxx::audio::kStartFramePos, mixxx::Bpm(140));
    m_pTrack2->trySetBeats(pBeats2);
    ProcessBuffer();

    ControlObject::getControl(ConfigKey(m_sGroup1, "quantize"))->set(1.0);
    ControlObject::getControl(ConfigKey(m_sGroup2, "quantize"))->set(1.0);
    // Make Channel2 leader to weed out any channel ordering issues.
    ControlObject::getControl(ConfigKey(m_sGroup1, "sync_enabled"))->set(1);
    ControlObject::getControl(ConfigKey(m_sGroup2, "sync_enabled"))->set(1);

    EXPECT_DOUBLE_EQ(70.0,
            ControlObject::getControl(ConfigKey(m_sInternalClockGroup, "bpm"))
                    ->get());
    EXPECT_DOUBLE_EQ(getRateSliderValue(1.0),
            ControlObject::getControl(ConfigKey(m_sGroup1, "rate"))->get());
    EXPECT_DOUBLE_EQ(getRateSliderValue(1.0),
            ControlObject::getControl(ConfigKey(m_sGroup2, "rate"))->get());
}

namespace {
QVector<mixxx::audio::FramePos> createBeatVector(
        mixxx::audio::FramePos first_beat,
        unsigned int num_beats,
        mixxx::audio::FrameDiff_t beat_length) {
    QVector<mixxx::audio::FramePos> beats;
    for (unsigned int i = 0; i < num_beats; ++i) {
        beats.append(first_beat + i * beat_length);
    }
    return beats;
}
} // namespace

TEST_F(EngineSyncTest, HalfDoubleConsistency) {
    // half-double matching should be consistent
    mixxx::audio::FrameDiff_t beatLengthFrames = 60.0 * 44100 / 90.0;
    constexpr auto startOffsetFrames = mixxx::audio::kStartFramePos;
    constexpr int numBeats = 100;
    QVector<mixxx::audio::FramePos> beats1 =
            createBeatVector(startOffsetFrames, numBeats, beatLengthFrames);
    auto pBeats1 = mixxx::Beats::fromBeatPositions(m_pTrack1->getSampleRate(), beats1);
    m_pTrack1->trySetBeats(pBeats1);
    ProcessBuffer();

    beatLengthFrames = 60.0 * 44100 / 145.0;
    QVector<mixxx::audio::FramePos> beats2 =
            createBeatVector(startOffsetFrames, numBeats, beatLengthFrames);
    auto pBeats2 = mixxx::Beats::fromBeatPositions(m_pTrack2->getSampleRate(), beats2);
    m_pTrack2->trySetBeats(pBeats2);
    ProcessBuffer();

    ControlObject::getControl(ConfigKey(m_sGroup1, "play"))->set(1.0);
    ControlObject::getControl(ConfigKey(m_sGroup2, "sync_enabled"))->set(1);
    EXPECT_DOUBLE_EQ(180.0,
            ControlObject::getControl(ConfigKey(m_sGroup2, "bpm"))->get());

    ControlObject::getControl(ConfigKey(m_sGroup1, "play"))->set(1.0);
    ControlObject::getControl(ConfigKey(m_sGroup1, "sync_enabled"))->set(1);
    EXPECT_DOUBLE_EQ(90.0,
            ControlObject::getControl(ConfigKey(m_sGroup1, "bpm"))->get());
    EXPECT_DOUBLE_EQ(180.0,
            ControlObject::getControl(ConfigKey(m_sGroup2, "bpm"))->get());

    ControlObject::getControl(ConfigKey(m_sGroup1, "sync_enabled"))->set(0);
    ProcessBuffer();
    ControlObject::getControl(ConfigKey(m_sGroup1, "sync_enabled"))->set(1);
    ProcessBuffer();
    EXPECT_DOUBLE_EQ(90.0,
            ControlObject::getControl(ConfigKey(m_sGroup1, "bpm"))->get());
    EXPECT_DOUBLE_EQ(180.0,
            ControlObject::getControl(ConfigKey(m_sGroup2, "bpm"))->get());
}

TEST_F(EngineSyncTest, HalfDoubleEachOther) {
    // Confirm that repeated sync with both decks leads to the same
    // Half/Double decision.
    // This test demonstrates https://github.com/mixxxdj/mixxx/issues/10378
    mixxx::BeatsPointer pBeats1 = mixxx::Beats::fromConstTempo(
            m_pTrack1->getSampleRate(), mixxx::audio::kStartFramePos, mixxx::Bpm(144));
    m_pTrack1->trySetBeats(pBeats1);
    ProcessBuffer();
    mixxx::BeatsPointer pBeats2 = mixxx::Beats::fromConstTempo(
            m_pTrack2->getSampleRate(), mixxx::audio::kStartFramePos, mixxx::Bpm(105));
    m_pTrack2->trySetBeats(pBeats2);
    ProcessBuffer();

    // Threshold 1.414 sqrt(2);
    // 144 / 105 = 1.37
    // 105 / 72 = 1.46
    // expect 144 BPM

    ControlObject::getControl(ConfigKey(m_sGroup2, "sync_enabled"))->set(1.0);
    ControlObject::getControl(ConfigKey(m_sGroup2, "sync_enabled"))->set(0.0);

    EXPECT_DOUBLE_EQ(144.0,
            ControlObject::getControl(ConfigKey(m_sGroup2, "bpm"))->get());

    ControlObject::getControl(ConfigKey(m_sGroup1, "sync_enabled"))->set(1.0);
    ControlObject::getControl(ConfigKey(m_sGroup1, "sync_enabled"))->set(0.0);

    EXPECT_DOUBLE_EQ(144.0,
            ControlObject::getControl(ConfigKey(m_sGroup1, "bpm"))->get());

    // Threshold 1.414 sqrt(2);
    // 150 / 144 = 1.04
    // 144 / 75 = 1.92
    // expect 75 BPM

    mixxx::BeatsPointer pBeats1b = mixxx::Beats::fromConstTempo(
            m_pTrack1->getSampleRate(), mixxx::audio::kStartFramePos, mixxx::Bpm(150));
    m_pTrack1->trySetBeats(pBeats1b);
    ProcessBuffer();

    EXPECT_DOUBLE_EQ(150.0,
            ControlObject::getControl(ConfigKey(m_sGroup1, "bpm"))->get());

    ControlObject::getControl(ConfigKey(m_sGroup2, "sync_enabled"))->set(1.0);
    ControlObject::getControl(ConfigKey(m_sGroup2, "sync_enabled"))->set(0.0);

    EXPECT_DOUBLE_EQ(150,
            ControlObject::getControl(ConfigKey(m_sGroup2, "bpm"))->get());

    ControlObject::getControl(ConfigKey(m_sGroup1, "sync_enabled"))->set(1.0);
    ControlObject::getControl(ConfigKey(m_sGroup1, "sync_enabled"))->set(0.0);

    EXPECT_DOUBLE_EQ(150.0,
            ControlObject::getControl(ConfigKey(m_sGroup1, "bpm"))->get());
}

TEST_F(EngineSyncTest, SetFileBpmUpdatesLocalBpm) {
    ControlObject::getControl(ConfigKey(m_sGroup1, "beat_distance"))->set(0.2);
    mixxx::BeatsPointer pBeats1 = mixxx::Beats::fromConstTempo(
            m_pTrack1->getSampleRate(), mixxx::audio::kStartFramePos, mixxx::Bpm(130));
    m_pTrack1->trySetBeats(pBeats1);
    ProcessBuffer();
    EXPECT_EQ(
            mixxx::Bpm(130.0), m_pEngineSync->getSyncableForGroup(m_sGroup1)->getBaseBpm());
}

TEST_F(EngineSyncTest, SyncPhaseToPlayingNonSyncDeck) {
    // If we press play on a sync deck, we will only sync phase to a non-sync
    // deck if there are no sync decks and the non-sync deck is playing.

    auto pButtonSyncEnabled1 =
            std::make_unique<ControlProxy>(m_sGroup1, "sync_enabled");
    mixxx::BeatsPointer pBeats1 = mixxx::Beats::fromConstTempo(
            m_pTrack1->getSampleRate(), mixxx::audio::kStartFramePos, mixxx::Bpm(130));
    m_pTrack1->trySetBeats(pBeats1);
    ProcessBuffer();
    ControlObject::getControl(ConfigKey(m_sGroup1, "quantize"))->set(1.0);

    auto pButtonSyncEnabled2 =
            std::make_unique<ControlProxy>(m_sGroup2, "sync_enabled");
    ControlObject::set(ConfigKey(m_sGroup2, "rate_ratio"), 1.0);
    mixxx::BeatsPointer pBeats2 = mixxx::Beats::fromConstTempo(
            m_pTrack2->getSampleRate(), mixxx::audio::kStartFramePos, mixxx::Bpm(100));
    m_pTrack2->trySetBeats(pBeats2);
    ProcessBuffer();

    // Set the sync deck playing with nothing else active.
    // Next Deck becomes leader and the Leader clock is set to 100 BPM
    // The 130 BPM Track should be played at 100 BPM, rate = 0.769230769
    pButtonSyncEnabled1->set(1.0);
    ControlObject::getControl(ConfigKey(m_sGroup1, "play"))->set(1.0);
    // Beat length: 40707.692307692305
    // Buffer size is 1024 at 44.1 kHz at the given rate = 787.692307692 Samples
    // Expected beat_distance = 0.019349962
    ProcessBuffer();

    // Internal clock rate should be set and beat distance already updated
    EXPECT_DOUBLE_EQ(100.0,
            ControlObject::getControl(ConfigKey(m_sGroup1, "bpm"))->get());
    EXPECT_NEAR(0.019349962,
            ControlObject::getControl(ConfigKey(m_sGroup1, "beat_distance"))->get(),
            kMaxFloatingPointErrorLowPrecision);

    // The internal clock must also have been advanced to the same fraction of a beat.
    EXPECT_DOUBLE_EQ(100.0,
            ControlObject::getControl(ConfigKey(m_sInternalClockGroup, "bpm"))->get());
    EXPECT_NEAR(0.019349962,
            ControlObject::getControl(ConfigKey(m_sInternalClockGroup, "beat_distance"))->get(),
            kMaxFloatingPointErrorLowPrecision);

    ControlObject::getControl(ConfigKey(m_sGroup1, "play"))->set(0.0);

    ProcessBuffer();

    // we expect that Deck 1 distance has not changed but the internal clock keeps going, because
    // the internal clock should continue playing even if the leader is stopped.
    EXPECT_TRUE(isSoftLeader(m_sGroup1));
    EXPECT_NEAR(0.019349962,
            ControlObject::getControl(ConfigKey(m_sGroup1, "beat_distance"))->get(),
            kMaxFloatingPointErrorLowPrecision);
    EXPECT_NEAR(0.038699924,
            ControlObject::getControl(ConfigKey(m_sInternalClockGroup, "beat_distance"))->get(),
            kMaxFloatingPointErrorLowPrecision);

    // Now make the second deck playing and see if it works.
    ControlObject::getControl(ConfigKey(m_sGroup2, "play"))->set(1.0);
    ControlObject::getControl(ConfigKey(m_sGroup1, "play"))->set(1.0);

    // What seems to be happening is a seek is queued when a track is loaded, so all these tracks
    // seek to 0, thus resetting the beat distance.
    ProcessBuffer();

    // The first deck is still the only one with sync active.
    // TODO: It does sounds odd to start the track 1 at a random position and adjust the
    // phase later. Seeking into phase is the best option even with quantize off.
    EXPECT_TRUE(isSoftLeader(m_sGroup1));
    EXPECT_DOUBLE_EQ(100.0,
            ControlObject::getControl(ConfigKey(m_sGroup1, "bpm"))->get());
    // The adjustment is calculated here: BpmControl::calcSyncAdjustment
    EXPECT_GT(0.038699925, ControlObject::getControl(ConfigKey(m_sGroup1, "beat_distance"))->get());
    EXPECT_DOUBLE_EQ(100.0,
            ControlObject::getControl(ConfigKey(m_sGroup2, "bpm"))->get());
    EXPECT_NEAR(
            0.019349962,
            ControlObject::getControl(ConfigKey(m_sGroup2, "beat_distance"))->get(),
            kMaxFloatingPointErrorLowPrecision);
    EXPECT_DOUBLE_EQ(100.0,
            ControlObject::getControl(ConfigKey(m_sInternalClockGroup, "bpm"))->get());
    EXPECT_NEAR(
            0.038699925,
            ControlObject::getControl(ConfigKey(m_sInternalClockGroup, "beat_distance"))->get(),
            kMaxFloatingPointErrorLowPrecision);

    ControlObject::set(ConfigKey(m_sGroup1, "play"), 0.0);
    pButtonSyncEnabled1->set(0.0);
    ControlObject::set(ConfigKey(m_sGroup1, "rate_ratio"), 1.0);

    ControlObject::set(ConfigKey(m_sGroup2, "play"), 0.0);
    pButtonSyncEnabled2->set(0.0);
    ControlObject::set(ConfigKey(m_sGroup2, "rate_ratio"), 1.0);

    // If we enable sync on the third deck, it will match the first, which has been reset to 130.
    auto pButtonSyncEnabled3 =
            std::make_unique<ControlProxy>(m_sGroup3, "sync_enabled");
    ControlObject::set(ConfigKey(m_sGroup3, "beat_distance"), 0.6);
    ControlObject::set(ConfigKey(m_sGroup2, "rate_ratio"), 1.0);
    mixxx::BeatsPointer pBeats3 = mixxx::Beats::fromConstTempo(
            m_pTrack3->getSampleRate(), mixxx::audio::kStartFramePos, mixxx::Bpm(140));
    m_pTrack3->trySetBeats(pBeats3);
    // This will sync to the first deck here and not the second #9397
    pButtonSyncEnabled3->set(1.0);
    EXPECT_TRUE(isSoftLeader(m_sGroup3));
    ProcessBuffer();
    EXPECT_DOUBLE_EQ(130.0, ControlObject::get(ConfigKey(m_sGroup3, "bpm")));
    // revert that
    ControlObject::set(ConfigKey(m_sGroup3, "rate_ratio"), 1.0);
    ProcessBuffer();
    EXPECT_DOUBLE_EQ(140.0, ControlObject::get(ConfigKey(m_sGroup3, "bpm")));
    // now we have Deck 3 with 140 bpm and sync enabled

    pButtonSyncEnabled1->set(1.0);
    ProcessBuffer();
    // Soft leader is now deck one because that was the last one we enabled and none of them
    // are playing.
    EXPECT_TRUE(isSoftLeader(m_sGroup1));

    ControlObject::getControl(ConfigKey(m_sGroup3, "play"))->set(1.0);
    ControlObject::getControl(ConfigKey(m_sGroup2, "play"))->set(1.0);
    ControlObject::getControl(ConfigKey(m_sGroup1, "play"))->set(1.0);
    ProcessBuffer();

    // When deck 1 activated, it should have matched deck 3.
    EXPECT_DOUBLE_EQ(140.0,
            ControlObject::getControl(ConfigKey(m_sInternalClockGroup, "bpm"))
                    ->get());
    EXPECT_DOUBLE_EQ(140.0,
            ControlObject::getControl(ConfigKey(m_sGroup1, "bpm"))->get());
    // The exact beat distance will be one buffer past .6, but this is good
    // enough to confirm that it worked.
    EXPECT_GT(0.7,
            ControlObject::getControl(ConfigKey(m_sGroup1, "beat_distance"))
                    ->get());
    EXPECT_GT(0.7,
            ControlObject::getControl(
                    ConfigKey(m_sInternalClockGroup, "beat_distance"))
                    ->get());
}

TEST_F(EngineSyncTest, UserTweakBeatDistance) {
    // If a deck has a user tweak, and another deck stops such that the first
    // is used to reseed the leader beat distance, make sure the user offset
    // is reset.
    mixxx::BeatsPointer pBeats1 = mixxx::Beats::fromConstTempo(
            m_pTrack1->getSampleRate(), mixxx::audio::kStartFramePos, mixxx::Bpm(128));
    m_pTrack1->trySetBeats(pBeats1);
    mixxx::BeatsPointer pBeats2 = mixxx::Beats::fromConstTempo(
            m_pTrack2->getSampleRate(), mixxx::audio::kStartFramePos, mixxx::Bpm(128));
    m_pTrack2->trySetBeats(pBeats2);

    ControlObject::getControl(ConfigKey(m_sGroup1, "quantize"))->set(1.0);
    ControlObject::getControl(ConfigKey(m_sGroup2, "quantize"))->set(1.0);
    ControlObject::getControl(ConfigKey(m_sGroup1, "sync_enabled"))->set(1);
    ControlObject::getControl(ConfigKey(m_sGroup2, "sync_enabled"))->set(1);
    ControlObject::getControl(ConfigKey(m_sGroup1, "play"))->set(1.0);
    ControlObject::getControl(ConfigKey(m_sGroup2, "play"))->set(1.0);

    // Play some buffers with the wheel at 0 to show the sync works
    ControlObject::getControl(ConfigKey(m_sGroup1, "wheel"))->set(0);
    for (int i = 0; i < 10; ++i) {
        ProcessBuffer();
    }

    // Spin the wheel, causing the useroffset for group1 to get set.
    ControlObject::getControl(ConfigKey(m_sGroup1, "wheel"))->set(0.4);
    for (int i = 0; i < 10; ++i) {
        ProcessBuffer();
    }
    // Play some more buffers with the wheel at 0.
    ControlObject::getControl(ConfigKey(m_sGroup1, "wheel"))->set(0);
    for (int i = 0; i < 10; ++i) {
        ProcessBuffer();
    }

    // Stop the second deck.  This causes the leader beat distance to get
    // seeded with the beat distance from deck 1.
    ControlObject::getControl(ConfigKey(m_sGroup2, "play"))->set(0.0);

    // Play a buffer, which is enough to see if the beat distances align.
    ProcessBuffer();

    EXPECT_NEAR(
            ControlObject::getControl(ConfigKey(m_sGroup1, "beat_distance"))->get(),
            ControlObject::getControl(ConfigKey(m_sInternalClockGroup, "beat_distance"))->get(),
            .00001);

    EXPECT_DOUBLE_EQ(0.0,
            m_pChannel1->getEngineBuffer()
                    ->m_pBpmControl->getUserOffset());
}

TEST_F(EngineSyncTest, UserTweakPreservedInSeek) {
    // Ensure that when we do a seek during sync lock, the user offset is maintained.

    // This is about 128 bpm, but results in nice round numbers of samples.
    constexpr double kDivisibleBpm = 44100.0 / 344.0;
    mixxx::BeatsPointer pBeats1 = mixxx::Beats::fromConstTempo(
            m_pTrack1->getSampleRate(), mixxx::audio::kStartFramePos, mixxx::Bpm(kDivisibleBpm));
    m_pTrack1->trySetBeats(pBeats1);
    ProcessBuffer();
    mixxx::BeatsPointer pBeats2 = mixxx::Beats::fromConstTempo(
            m_pTrack2->getSampleRate(), mixxx::audio::kStartFramePos, mixxx::Bpm(130));
    m_pTrack2->trySetBeats(pBeats2);
    ProcessBuffer();

    ControlObject::getControl(ConfigKey(m_sGroup2, "sync_enabled"))->set(1);
    ControlObject::getControl(ConfigKey(m_sGroup1, "sync_enabled"))->set(1);
    ControlObject::set(ConfigKey(m_sGroup1, "quantize"), 1.0);
    ControlObject::set(ConfigKey(m_sGroup2, "quantize"), 1.0);
    ControlObject::set(ConfigKey(m_sGroup1, "play"), 1.0);
    ControlObject::set(ConfigKey(m_sGroup2, "play"), 1.0);

    ProcessBuffer();
    EXPECT_DOUBLE_EQ(kDivisibleBpm,
            ControlObject::getControl(ConfigKey(m_sGroup1, "bpm"))->get());
    EXPECT_DOUBLE_EQ(kDivisibleBpm,
            ControlObject::getControl(ConfigKey(m_sGroup2, "bpm"))->get());

    EXPECT_NEAR(0.024806201,
            ControlObject::get(ConfigKey(m_sGroup1, "beat_distance")),
            kMaxFloatingPointErrorLowPrecision);
    EXPECT_NEAR(0.0023219956,
            ControlObject::get(ConfigKey(m_sGroup1, "playposition")),
            kMaxFloatingPointErrorLowPrecision);

    ControlObject::set(ConfigKey(m_sGroup2, "playposition"), 0.2);
    ProcessBuffer();
    ControlObject::set(ConfigKey(m_sGroup1, "playposition"), 0.2);
    ProcessBuffer();

    // We are a few buffers past the seeked position.  It's less than 0.2 because
    // of quantizing.  The playpositions are different because the tracks are at
    // different bpms.
    EXPECT_NEAR(0.19417687,
            ControlObject::get(ConfigKey(m_sGroup1, "playposition")),
            kMaxFloatingPointErrorLowPrecision);
    EXPECT_NEAR(0.19148479,
            ControlObject::get(ConfigKey(m_sGroup2, "playposition")),
            kMaxFloatingPointErrorLowPrecision);
    // The beat distances are identical though.
    EXPECT_NEAR(0.074418604,
            ControlObject::get(ConfigKey(m_sGroup1, "beat_distance")),
            kMaxFloatingPointErrorLowPrecision);
    EXPECT_NEAR(0.074418604,
            ControlObject::get(ConfigKey(m_sGroup2, "beat_distance")),
            kMaxFloatingPointErrorLowPrecision);

    // Apply user tweak offset.
    m_pChannel1->getEngineBuffer()
            ->m_pBpmControl->m_dUserOffset.setValue(0.3);

    // Seek back to 0.2
    ControlObject::set(ConfigKey(m_sGroup2, "playposition"), 0.2);
    ProcessBuffer();
    ControlObject::set(ConfigKey(m_sGroup1, "playposition"), 0.2);
    ProcessBuffer();

    // Now, the locations are much more different - but the beat distance appears
    // to be the same because beat distance CO hides the user offset.
    EXPECT_NEAR(0.1990531,
            ControlObject::get(ConfigKey(m_sGroup1, "playposition")),
            kMaxFloatingPointErrorLowPrecision);
    EXPECT_NEAR(0.1686011,
            ControlObject::get(ConfigKey(m_sGroup2, "playposition")),
            kMaxFloatingPointErrorLowPrecision);
    EXPECT_NEAR(0.82651163,
            ControlObject::get(ConfigKey(m_sGroup1, "beat_distance")),
            kMaxFloatingPointErrorLowPrecision);
    EXPECT_NEAR(0.82651163,
            ControlObject::get(ConfigKey(m_sGroup2, "beat_distance")),
            kMaxFloatingPointErrorLowPrecision);
}

TEST_F(EngineSyncTest, FollowerUserTweakPreservedInLeaderChange) {
    // Ensure that when the leader deck changes, the user offset is accounted for when
    // reinitializing the leader parameters..

    // This is about 128 bpm, but results in nice round numbers of samples.
    constexpr double kDivisibleBpm = 44100.0 / 344.0;
    mixxx::BeatsPointer pBeats1 = mixxx::Beats::fromConstTempo(
            m_pTrack1->getSampleRate(), mixxx::audio::kStartFramePos, mixxx::Bpm(130));
    m_pTrack1->trySetBeats(pBeats1);
    ProcessBuffer();
    mixxx::BeatsPointer pBeats2 = mixxx::Beats::fromConstTempo(
            m_pTrack2->getSampleRate(), mixxx::audio::kStartFramePos, mixxx::Bpm(kDivisibleBpm));
    m_pTrack2->trySetBeats(pBeats2);
    ProcessBuffer();

    ControlObject::getControl(ConfigKey(m_sGroup1, "sync_enabled"))->set(1);
    ControlObject::getControl(ConfigKey(m_sGroup2, "sync_enabled"))->set(1);
    ControlObject::set(ConfigKey(m_sGroup1, "quantize"), 1.0);
    ControlObject::set(ConfigKey(m_sGroup2, "quantize"), 1.0);
    // Apply user tweak offset to follower
    m_pChannel2->getEngineBuffer()
            ->m_pBpmControl->m_dUserOffset.setValue(0.3);
    ControlObject::set(ConfigKey(m_sGroup1, "play"), 1.0);
    ControlObject::set(ConfigKey(m_sGroup2, "play"), 1.0);

    EXPECT_TRUE(isSoftLeader(m_sGroup1));
    EXPECT_TRUE(isFollower(m_sGroup2));

    ProcessBuffer();
    EXPECT_DOUBLE_EQ(kDivisibleBpm,
            ControlObject::getControl(ConfigKey(m_sGroup1, "bpm"))->get());
    EXPECT_DOUBLE_EQ(kDivisibleBpm,
            ControlObject::getControl(ConfigKey(m_sGroup2, "bpm"))->get());

    for (int i = 0; i < 5; ++i) {
        ProcessBuffer();
        EXPECT_NEAR(ControlObject::get(ConfigKey(m_sGroup1, "beat_distance")),
                ControlObject::get(ConfigKey(m_sGroup2, "beat_distance")),
                kMaxFloatingPointErrorLowPrecision);
    }

    // Switch leader
    ControlObject::getControl(ConfigKey(m_sGroup2, "sync_leader"))->set(1);
    ProcessBuffer();
    EXPECT_TRUE(isFollower(m_sGroup1));
    EXPECT_TRUE(isSoftLeader(m_sGroup2));

    for (int i = 0; i < 10; ++i) {
        ProcessBuffer();
        EXPECT_NEAR(ControlObject::get(ConfigKey(m_sGroup1, "beat_distance")),
                ControlObject::get(ConfigKey(m_sGroup2, "beat_distance")),
                kMaxFloatingPointErrorLowPrecision);
    }
}

TEST_F(EngineSyncTest, FollowerUserTweakPreservedInSyncDisable) {
    // Ensure that when a follower disables sync, a phase seek is not performed.
    // This is about 128 bpm, but results in nice round numbers of samples.
    constexpr double kDivisibleBpm = 44100.0 / 344.0;
    mixxx::BeatsPointer pBeats1 = mixxx::Beats::fromConstTempo(
            m_pTrack1->getSampleRate(), mixxx::audio::kStartFramePos, mixxx::Bpm(kDivisibleBpm));
    m_pTrack1->trySetBeats(pBeats1);
    ProcessBuffer();
    mixxx::BeatsPointer pBeats2 = mixxx::Beats::fromConstTempo(
            m_pTrack2->getSampleRate(), mixxx::audio::kStartFramePos, mixxx::Bpm(130));
    m_pTrack2->trySetBeats(pBeats2);
    ProcessBuffer();

    ControlObject::getControl(ConfigKey(m_sGroup1, "sync_leader"))->set(1);
    ControlObject::getControl(ConfigKey(m_sGroup2, "sync_enabled"))->set(1);
    ControlObject::set(ConfigKey(m_sGroup1, "quantize"), 1.0);
    ControlObject::set(ConfigKey(m_sGroup2, "quantize"), 1.0);
    ControlObject::set(ConfigKey(m_sGroup1, "play"), 1.0);
    ControlObject::set(ConfigKey(m_sGroup2, "play"), 1.0);
    ProcessBuffer();

    // Apply user tweak offset to follower
    m_pChannel2->getEngineBuffer()
            ->m_pBpmControl->m_dUserOffset.setValue(0.3);

    EXPECT_TRUE(isSoftLeader(m_sGroup1));
    EXPECT_TRUE(isFollower(m_sGroup2));

    ProcessBuffer();

    // Disable sync, no seek should occur
    ControlObject::getControl(ConfigKey(m_sGroup2, "sync_enabled"))->set(0);
    ProcessBuffer();
    EXPECT_FALSE(m_pChannel2->getEngineBuffer()->previousBufferSeek());
}

TEST_F(EngineSyncTest, LeaderUserTweakPreservedInLeaderChange) {
    // Ensure that when the leader deck changes, the user offset is accounted for when
    // reinitializing the leader parameters..

    // This is about 128 bpm, but results in nice round numbers of samples.
    constexpr double kDivisibleBpm = 44100.0 / 344.0;
    mixxx::BeatsPointer pBeats1 = mixxx::Beats::fromConstTempo(
            m_pTrack1->getSampleRate(), mixxx::audio::kStartFramePos, mixxx::Bpm(kDivisibleBpm));
    m_pTrack1->trySetBeats(pBeats1);
    ProcessBuffer();
    mixxx::BeatsPointer pBeats2 = mixxx::Beats::fromConstTempo(
            m_pTrack2->getSampleRate(), mixxx::audio::kStartFramePos, mixxx::Bpm(130));
    m_pTrack2->trySetBeats(pBeats2);
    ProcessBuffer();

    ControlObject::getControl(ConfigKey(m_sGroup2, "sync_enabled"))->set(1);
    ControlObject::getControl(ConfigKey(m_sGroup1, "sync_leader"))->set(1);
    ControlObject::set(ConfigKey(m_sGroup1, "quantize"), 1.0);
    ControlObject::set(ConfigKey(m_sGroup2, "quantize"), 1.0);
    ControlObject::set(ConfigKey(m_sGroup1, "play"), 1.0);
    ControlObject::set(ConfigKey(m_sGroup2, "play"), 1.0);

    EXPECT_TRUE(isSoftLeader(m_sGroup1));
    EXPECT_TRUE(isFollower(m_sGroup2));

    ProcessBuffer();
    EXPECT_DOUBLE_EQ(kDivisibleBpm,
            ControlObject::getControl(ConfigKey(m_sGroup1, "bpm"))->get());
    EXPECT_DOUBLE_EQ(kDivisibleBpm,
            ControlObject::getControl(ConfigKey(m_sGroup2, "bpm"))->get());

    // Apply user tweak offset to leader -- to test the bug we found, we need
    // to apply it indirectly.
    ControlObject::set(ConfigKey(m_sGroup1, "rate_temp_up"), 1);
    for (int i = 0; i < 5; ++i) {
        ProcessBuffer();
        EXPECT_NEAR(ControlObject::get(ConfigKey(m_sGroup1, "beat_distance")),
                ControlObject::get(ConfigKey(m_sGroup2, "beat_distance")),
                kMaxFloatingPointErrorLowPrecision);
    }
    ControlObject::set(ConfigKey(m_sGroup1, "rate_temp_up"), 0);

    // Switch leader
    ControlObject::getControl(ConfigKey(m_sGroup2, "sync_leader"))->set(1);
    ProcessBuffer();
    EXPECT_TRUE(isFollower(m_sGroup1));
    EXPECT_TRUE(isSoftLeader(m_sGroup2));

    for (int i = 0; i < 10; ++i) {
        ProcessBuffer();
        EXPECT_NEAR(ControlObject::get(ConfigKey(m_sGroup1, "beat_distance")),
                ControlObject::get(ConfigKey(m_sGroup2, "beat_distance")),
                kMaxFloatingPointErrorLowPrecision);
    }
}

TEST_F(EngineSyncTest, LeaderBpmNeverZero) {
    mixxx::BeatsPointer pBeats1 = mixxx::Beats::fromConstTempo(
            m_pTrack1->getSampleRate(), mixxx::audio::kStartFramePos, mixxx::Bpm(128));
    m_pTrack1->trySetBeats(pBeats1);
    ProcessBuffer();

    auto pButtonSyncEnabled1 = std::make_unique<ControlProxy>(m_sGroup1, "sync_enabled");
    pButtonSyncEnabled1->set(1.0);

    m_pTrack1->trySetBeats(mixxx::BeatsPointer());
    ProcessBuffer();
    EXPECT_EQ(128.0,
              ControlObject::getControl(ConfigKey(m_sInternalClockGroup, "bpm"))->get());
}

TEST_F(EngineSyncTest, ZeroBpmNaturalRate) {
    // If a track has a zero bpm and a bad beatgrid, make sure the rate
    // doesn't end up something crazy when sync is enabled..
    // Maybe the beatgrid ended up at zero also.
    m_pTrack1->trySetBeats(nullptr);

    auto pButtonSyncEnabled1 = std::make_unique<ControlProxy>(m_sGroup1, "sync_enabled");
    pButtonSyncEnabled1->set(1.0);

    ProcessBuffer();

    // 0 bpm is what we want, the sync code will play the track back at rate
    // 1.0.
    EXPECT_EQ(0.0,
              ControlObject::getControl(ConfigKey(m_sGroup1, "local_bpm"))->get());
}

TEST_F(EngineSyncTest, QuantizeImpliesSyncPhase) {
    auto pButtonSyncEnabled1 = std::make_unique<ControlProxy>(m_sGroup1, "sync_enabled");
    auto pButtonBeatsync1 = std::make_unique<ControlProxy>(m_sGroup1, "beatsync");
    auto pButtonBeatsyncPhase1 = std::make_unique<ControlProxy>(m_sGroup1, "beatsync_phase");

    mixxx::BeatsPointer pBeats1 = mixxx::Beats::fromConstTempo(
            m_pTrack1->getSampleRate(), mixxx::audio::kStartFramePos, mixxx::Bpm(130));
    m_pTrack1->trySetBeats(pBeats1);

    ControlObject::set(ConfigKey(m_sGroup2, "rate"), getRateSliderValue(1.0));
    mixxx::BeatsPointer pBeats2 = mixxx::Beats::fromConstTempo(
            m_pTrack2->getSampleRate(), mixxx::audio::kStartFramePos, mixxx::Bpm(100));
    m_pTrack2->trySetBeats(pBeats2);

    ProcessBuffer();

    ControlObject::set(ConfigKey(m_sGroup1, "play"), 1.0);
    ControlObject::set(ConfigKey(m_sGroup2, "play"), 1.0);
    ProcessBuffer();

    // Beat length: 40707.692307692; Buffer size is 1024
    // Expected beat_distance = 1024/40707 = 0.025155
    EXPECT_DOUBLE_EQ(130, ControlObject::get(ConfigKey(m_sGroup1, "bpm")));
    EXPECT_DOUBLE_EQ(0.025154950869236584, ControlObject::get(ConfigKey(m_sGroup1, "beat_distance")));

    // Beat length: 52920; Buffer size is 1024
    // Expected beat_distance = 0.01935
    EXPECT_DOUBLE_EQ(100, ControlObject::get(ConfigKey(m_sGroup2, "bpm")));
    EXPECT_DOUBLE_EQ(0.019349962207105064, ControlObject::get(ConfigKey(m_sGroup2, "beat_distance")));

    // first test without quantization
    pButtonSyncEnabled1->set(1.0);
    ProcessBuffer();

    // Deck 1 continues with 100 bpm
    EXPECT_DOUBLE_EQ(100, ControlObject::get(ConfigKey(m_sInternalClockGroup, "bpm")));
    EXPECT_DOUBLE_EQ(100, ControlObject::get(ConfigKey(m_sGroup1, "bpm")));
    EXPECT_DOUBLE_EQ(0.044504913076341648, ControlObject::get(ConfigKey(m_sGroup1, "beat_distance")));

    // Deck 2 continues normally
    EXPECT_DOUBLE_EQ(100, ControlObject::get(ConfigKey(m_sGroup2, "bpm")));
    EXPECT_DOUBLE_EQ(0.038699924414210128, ControlObject::get(ConfigKey(m_sGroup2, "beat_distance")));

    pButtonSyncEnabled1->set(0.0);

    ControlObject::set(ConfigKey(m_sGroup1, "quantize"), 1.0);
    ProcessBuffer();

    // Quantize only has no effect here, both decks are running freely.

    EXPECT_DOUBLE_EQ(100, ControlObject::get(ConfigKey(m_sGroup1, "bpm")));
    EXPECT_DOUBLE_EQ(0.063854875283446716, ControlObject::get(ConfigKey(m_sGroup1, "beat_distance")));

    // Deck 2 continues normally
    EXPECT_DOUBLE_EQ(100, ControlObject::get(ConfigKey(m_sGroup2, "bpm")));
    EXPECT_DOUBLE_EQ(0.058049886621315196, ControlObject::get(ConfigKey(m_sGroup2, "beat_distance")));
    pButtonSyncEnabled1->set(1.0);
    ProcessBuffer();

    // normally the deck would be at 0.083204837, due to quantize it has been seeked
    // in phase of deck 2
    EXPECT_DOUBLE_EQ(100, ControlObject::get(ConfigKey(m_sGroup1, "bpm")));
    EXPECT_DOUBLE_EQ(100, ControlObject::get(ConfigKey(m_sGroup2, "bpm")));

    EXPECT_NEAR(
            ControlObject::get(ConfigKey(m_sGroup1, "beat_distance")),
            ControlObject::get(ConfigKey(m_sGroup2, "beat_distance")),
            1e-15);

    // Reset Deck 1 Tempo
    pButtonSyncEnabled1->set(0.0);
    ControlObject::set(ConfigKey(m_sGroup1, "quantize"), 0.0);
    ControlObject::set(ConfigKey(m_sGroup1, "rate_ratio"), 1.0);
    ProcessBuffer();

    // Now the deck should be running normally
    EXPECT_DOUBLE_EQ(130, ControlObject::get(ConfigKey(m_sGroup1, "bpm")));
    EXPECT_NEAR(
            0.10255479969765675,
            ControlObject::get(ConfigKey(m_sGroup1, "beat_distance")),
            1e-15);

    EXPECT_DOUBLE_EQ(100, ControlObject::get(ConfigKey(m_sGroup2, "bpm")));
    EXPECT_DOUBLE_EQ(0.096749811035525324, ControlObject::get(ConfigKey(m_sGroup2, "beat_distance")));

    pButtonBeatsyncPhase1->set(1.0);
    ProcessBuffer();

    // check if the next beat will be aligned:

    EXPECT_DOUBLE_EQ(130, ControlObject::get(ConfigKey(m_sGroup1, "bpm")));
    EXPECT_DOUBLE_EQ(100, ControlObject::get(ConfigKey(m_sGroup2, "bpm")));

    // we align here to the past beat, because beat_distance < 1.0/8
    EXPECT_NEAR(
            ControlObject::get(ConfigKey(m_sGroup1, "beat_distance")) / 130 * 100,
            ControlObject::get(ConfigKey(m_sGroup2, "beat_distance")),
            1e-15);
}

TEST_F(EngineSyncTest, SeekStayInPhase) {
    ControlObject::set(ConfigKey(m_sGroup1, "quantize"), 1.0);

    mixxx::BeatsPointer pBeats1 = mixxx::Beats::fromConstTempo(
            m_pTrack1->getSampleRate(), mixxx::audio::kStartFramePos, mixxx::Bpm(130));
    m_pTrack1->trySetBeats(pBeats1);

    ControlObject::set(ConfigKey(m_sGroup1, "play"), 1.0);
    ProcessBuffer();

    EXPECT_DOUBLE_EQ(0.025154950869236584, ControlObject::get(ConfigKey(m_sGroup1, "beat_distance")));
    EXPECT_DOUBLE_EQ(0.0023219954648526077, ControlObject::get(ConfigKey(m_sGroup1, "playposition")));

    ControlObject::set(ConfigKey(m_sGroup1, "playposition"), 0.2);
    ProcessBuffer();

    // We expect to be two buffers ahead in a beat near 0.2
    EXPECT_DOUBLE_EQ(0.050309901738473162,
            ControlObject::get(ConfigKey(m_sGroup1, "beat_distance")));
    EXPECT_DOUBLE_EQ(0.18925937554508981, ControlObject::get(ConfigKey(m_sGroup1, "playposition")));

    // The same again with a stopped track loaded in Channel 2
    ControlObject::set(ConfigKey(m_sGroup1, "playposition"), 0.0);
    ControlObject::set(ConfigKey(m_sGroup1, "play"), 0.0);
    ProcessBuffer();

    mixxx::BeatsPointer pBeats2 = mixxx::Beats::fromConstTempo(
            m_pTrack1->getSampleRate(), mixxx::audio::kStartFramePos, mixxx::Bpm(130));
    m_pTrack2->trySetBeats(pBeats2);

    ControlObject::set(ConfigKey(m_sGroup1, "play"), 1.0);
    ProcessBuffer();

    EXPECT_DOUBLE_EQ(0.025154950869236584,
            ControlObject::get(ConfigKey(m_sGroup1, "beat_distance")));
    EXPECT_DOUBLE_EQ(0.0023219954648526077,
            ControlObject::get(ConfigKey(m_sGroup1, "playposition")));

    ControlObject::set(ConfigKey(m_sGroup1, "playposition"), 0.2);
    ProcessBuffer();

    // We expect to be two buffers ahead in a beat near 0.2
    EXPECT_DOUBLE_EQ(0.050309901738473162,
            ControlObject::get(ConfigKey(m_sGroup1, "beat_distance")));
    EXPECT_DOUBLE_EQ(0.18925937554508981, ControlObject::get(ConfigKey(m_sGroup1, "playposition")));
}

TEST_F(EngineSyncTest, ScratchEndOtherStoppedTrackStayInPhase) {
    // After scratching, confirm that we are still in phase.
    // This version tests with a stopped other track.
    mixxx::BeatsPointer pBeats1 =
            mixxx::Beats::fromConstTempo(m_pTrack1->getSampleRate(),
                    mixxx::audio::kStartFramePos,
                    mixxx::Bpm(130));
    m_pTrack1->trySetBeats(pBeats1);
    ControlObject::set(ConfigKey(m_sGroup1, "quantize"), 1.0);
    ControlObject::set(ConfigKey(m_sGroup1, "sync_enabled"), 1);

    mixxx::BeatsPointer pBeats2 =
            mixxx::Beats::fromConstTempo(m_pTrack2->getSampleRate(),
                    mixxx::audio::kStartFramePos,
                    mixxx::Bpm(125));
    m_pTrack2->trySetBeats(pBeats2);
    ControlObject::set(ConfigKey(m_sGroup2, "quantize"), 1.0);
    ControlObject::set(ConfigKey(m_sGroup2, "sync_enabled"), 1);

    ControlObject::set(ConfigKey(m_sGroup1, "play"), 1.0);
    ProcessBuffer();
    ProcessBuffer();
    ControlObject::set(ConfigKey(m_sGroup1, "scratch2_enable"), 1.0);
    ControlObject::set(ConfigKey(m_sGroup1, "scratch2"), 20.0);
    ProcessBuffer();
    ControlObject::set(ConfigKey(m_sGroup1, "scratch2_enable"), 0.0);
    ProcessBuffer();
    ProcessBuffer();

    EXPECT_NEAR(ControlObject::get(ConfigKey(m_sInternalClockGroup, "beat_distance")),
            ControlObject::get(ConfigKey(m_sGroup1, "beat_distance")),
            1e-8);
}

TEST_F(EngineSyncTest, ScratchEndOtherPlayingTrackStayInPhase) {
    // After scratching, confirm that we are still in phase.
    // This version tests with a playing other track.
    mixxx::BeatsPointer pBeats1 =
            mixxx::Beats::fromConstTempo(m_pTrack1->getSampleRate(),
                    mixxx::audio::kStartFramePos,
                    mixxx::Bpm(130));
    m_pTrack1->trySetBeats(pBeats1);
    ControlObject::set(ConfigKey(m_sGroup1, "quantize"), 1.0);
    ControlObject::set(ConfigKey(m_sGroup1, "sync_enabled"), 1);

    mixxx::BeatsPointer pBeats2 =
            mixxx::Beats::fromConstTempo(m_pTrack2->getSampleRate(),
                    mixxx::audio::kStartFramePos,
                    mixxx::Bpm(125));
    m_pTrack2->trySetBeats(pBeats2);
    ControlObject::set(ConfigKey(m_sGroup2, "quantize"), 1.0);
    ControlObject::set(ConfigKey(m_sGroup2, "sync_enabled"), 1);

    ControlObject::set(ConfigKey(m_sGroup1, "play"), 1.0);
    ControlObject::set(ConfigKey(m_sGroup2, "play"), 1.0);
    ProcessBuffer();
    ProcessBuffer();
    ControlObject::set(ConfigKey(m_sGroup1, "scratch2_enable"), 1.0);
    ControlObject::set(ConfigKey(m_sGroup1, "scratch2"), 20.0);
    ProcessBuffer();
    ControlObject::set(ConfigKey(m_sGroup1, "scratch2_enable"), 0.0);
    ProcessBuffer();
    ProcessBuffer();

    EXPECT_NEAR(ControlObject::get(ConfigKey(m_sInternalClockGroup, "beat_distance")),
            ControlObject::get(ConfigKey(m_sGroup1, "beat_distance")),
            1e-8);
}

TEST_F(EngineSyncTest, SyncWithoutBeatgrid) {
    // this tests issue #9391, notresetting rate when other deck has no beatgrid
    mixxx::BeatsPointer pBeats1 = mixxx::Beats::fromConstTempo(
            m_pTrack1->getSampleRate(), mixxx::audio::kStartFramePos, mixxx::Bpm(128));
    m_pTrack1->trySetBeats(pBeats1);
    m_pTrack2->trySetBeats(mixxx::BeatsPointer());

    ControlObject::set(ConfigKey(m_sGroup1, "rate"), 0.5);

    // Play a buffer, which is enough to see if the beat distances align.
    ProcessBuffer();

    ControlObject::set(ConfigKey(m_sGroup1, "sync_enabled"), 1);

    ProcessBuffer();

    EXPECT_DOUBLE_EQ(0.5,
            ControlObject::get(ConfigKey(m_sGroup1, "rate")));
}

TEST_F(EngineSyncTest, QuantizeHotCueActivate) {
    mixxx::BeatsPointer pBeats1 = mixxx::Beats::fromConstTempo(
            m_pTrack1->getSampleRate(), mixxx::audio::kStartFramePos, mixxx::Bpm(130));
    m_pTrack1->trySetBeats(pBeats1);

    auto pHotCue2Activate = std::make_unique<ControlProxy>(m_sGroup2, "hotcue_1_activate");
    mixxx::BeatsPointer pBeats2 = mixxx::Beats::fromConstTempo(
            m_pTrack2->getSampleRate(), mixxx::audio::kStartFramePos, mixxx::Bpm(100));

    m_pTrack2->trySetBeats(pBeats2);

    ProcessBuffer();

    ControlObject::set(ConfigKey(m_sGroup1, "play"), 1.0);

    // store a hot cue at 0:00
    pHotCue2Activate->set(1.0);
    ProcessBuffer();
    pHotCue2Activate->set(0.0);
    ProcessBuffer();

    ControlObject::set(ConfigKey(m_sGroup2, "quantize"), 0.0);
    // preview a hot cue without quantize
    pHotCue2Activate->set(1.0);
    ProcessBuffer();

    // Expect that the track has advanced by one buffer starting from the hot cue at 0:00
    EXPECT_DOUBLE_EQ(0.019349962207105064, ControlObject::get(ConfigKey(m_sGroup2, "beat_distance")));

    pHotCue2Activate->set(0.0);
    ProcessBuffer();

    // Expect that the track was set back to 0:00
    EXPECT_DOUBLE_EQ(0, ControlObject::get(ConfigKey(m_sGroup2, "beat_distance")));

    // Expect that the first track has advanced 4 buffers in the mean time
    EXPECT_DOUBLE_EQ(0.10061980347694634, ControlObject::get(ConfigKey(m_sGroup1, "beat_distance")));

    ControlObject::set(ConfigKey(m_sGroup2, "quantize"), 1.0);
    // preview a hot cue with quantize seeks back to 0:00 and adjust beat distance to match the first track
    pHotCue2Activate->set(1.0);
    ProcessBuffer();

    // Beat_distance is the distance to the previous beat which has already passed.
    // We compare here the distance to the next beat (1 - beat_distance) and
    // scale it by the different tempos.
    EXPECT_NEAR(
            (1 - ControlObject::get(ConfigKey(m_sGroup1, "beat_distance"))) / 130 * 100,
            (1 - ControlObject::get(ConfigKey(m_sGroup2, "beat_distance"))),
            1e-15);

    pHotCue2Activate->set(0.0);
    ProcessBuffer();

    // Expect that the track is back to 0:00
    EXPECT_DOUBLE_EQ(0, ControlObject::get(ConfigKey(m_sGroup2, "beat_distance")));

    // Expect that the first track has advanced 6 buffers in the mean time
    EXPECT_DOUBLE_EQ(0.15092970521541951, ControlObject::get(ConfigKey(m_sGroup1, "beat_distance")));
}

TEST_F(EngineSyncTest, ChangeBeatGrid) {
    // https://github.com/mixxxdj/mixxx/issues/9550

    auto pButtonSyncEnabled1 = std::make_unique<ControlProxy>(m_sGroup1, "sync_enabled");
    auto pButtonSyncEnabled2 = std::make_unique<ControlProxy>(m_sGroup2, "sync_enabled");
    ProcessBuffer();

    // set beatgrid for deck 1
    mixxx::BeatsPointer pBeats1 = mixxx::Beats::fromConstTempo(
            m_pTrack1->getSampleRate(), mixxx::audio::kStartFramePos, mixxx::Bpm(130));
    m_pTrack1->trySetBeats(pBeats1);
    ProcessBuffer();

    pButtonSyncEnabled1->set(1.0);
    ControlObject::set(ConfigKey(m_sGroup1, "play"), 1.0);

    ProcessBuffer();

    EXPECT_TRUE(isSoftLeader(m_sGroup1));
    EXPECT_DOUBLE_EQ(130.0, ControlObject::get(ConfigKey(m_sGroup1, "bpm")));
    EXPECT_DOUBLE_EQ(130.0, ControlObject::get(ConfigKey(m_sInternalClockGroup, "bpm")));

    // deck 2 has not beats, so it will sync 0 bpm track to the first one
    pButtonSyncEnabled2->set(1.0);
    ControlObject::set(ConfigKey(m_sGroup2, "play"), 1.0);

    ProcessBuffer();

    // expect no change in Deck 1
    EXPECT_TRUE(isSoftLeader(m_sGroup1));
    EXPECT_TRUE(isFollower(m_sGroup2));
    EXPECT_DOUBLE_EQ(130.0, ControlObject::get(ConfigKey(m_sGroup1, "bpm")));
    EXPECT_DOUBLE_EQ(0, ControlObject::get(ConfigKey(m_sGroup2, "bpm")));
    EXPECT_DOUBLE_EQ(130.0, ControlObject::get(ConfigKey(m_sInternalClockGroup, "bpm")));

    // deck 1 is stopped.
    ControlObject::set(ConfigKey(m_sGroup1, "play"), 0.0);

    ProcessBuffer();
    // Group1 remains leader because it is the only one with a tempo.
    EXPECT_TRUE(isSoftLeader(m_sGroup1));
    EXPECT_TRUE(isFollower(m_sGroup2));

    // Load a new beatgrid during playing, this happens when the analyser is finished.
    mixxx::BeatsPointer pBeats2 = mixxx::Beats::fromConstTempo(
            m_pTrack2->getSampleRate(), mixxx::audio::kStartFramePos, mixxx::Bpm(140));
    m_pTrack2->trySetBeats(pBeats2);

    ProcessBuffer();

    // Since deck 1 is not playing, deck 2 doesn't change its speed to match theirs. Deck 2
    // should be leader now, and playing at our own rate.  The other deck should have
    // changed rate to match us.
    EXPECT_TRUE(isSoftLeader(m_sGroup2));
    EXPECT_TRUE(isFollower(m_sGroup1));
    EXPECT_TRUE(isFollower(m_sInternalClockGroup));
    EXPECT_DOUBLE_EQ(140.0, ControlObject::get(ConfigKey(m_sGroup1, "bpm")));
    EXPECT_DOUBLE_EQ(140.0, ControlObject::get(ConfigKey(m_sGroup2, "bpm")));
    EXPECT_DOUBLE_EQ(140.0, ControlObject::get(ConfigKey(m_sInternalClockGroup, "bpm")));

    // Play Both Tracks.
    ControlObject::set(ConfigKey(m_sGroup1, "play"), 1.0);
    ProcessBuffer();

    EXPECT_TRUE(isFollower(m_sInternalClockGroup));
    EXPECT_TRUE(isFollower(m_sGroup1));
    EXPECT_TRUE(isSoftLeader(m_sGroup2));

    // Load a new beatgrid again, this happens when the user adjusts the beatgrid
    mixxx::BeatsPointer pBeats2n = mixxx::Beats::fromConstTempo(
            m_pTrack2->getSampleRate(), mixxx::audio::kStartFramePos, mixxx::Bpm(75));
    m_pTrack2->trySetBeats(pBeats2n);

    ProcessBuffer();

    // We expect that the second deck is still playing at unity -- it was the leader
    // and it should not change speed just because it reloaded and the bpm changed.
    EXPECT_DOUBLE_EQ(75.0, ControlObject::get(ConfigKey(m_sGroup1, "bpm")));
    // Expect to sync on half beats
    EXPECT_DOUBLE_EQ(75.0, ControlObject::get(ConfigKey(m_sGroup2, "bpm")));
    EXPECT_DOUBLE_EQ(75.0, ControlObject::get(ConfigKey(m_sInternalClockGroup, "bpm")));
}

TEST_F(EngineSyncTest, BeatMapQuantizePlay) {
    // This test demonstrates https://github.com/mixxxdj/mixxx/issues/9938
    mixxx::BeatsPointer pBeats1 = mixxx::Beats::fromConstTempo(
            m_pTrack1->getSampleRate(), mixxx::audio::kStartFramePos, mixxx::Bpm(120));
    m_pTrack1->trySetBeats(pBeats1);

    constexpr auto kSampleRate = mixxx::audio::SampleRate(44100);

    auto pBeats2 = mixxx::Beats::fromBeatPositions(kSampleRate,
            // Add two beats at 120 Bpm
            QVector<mixxx::audio::FramePos>(
                    {mixxx::audio::FramePos(
                             static_cast<double>(kSampleRate) / 2),
                            mixxx::audio::FramePos(
                                    static_cast<double>(kSampleRate))}));
    m_pTrack2->trySetBeats(pBeats2);

    ControlObject::set(ConfigKey(m_sGroup1, "quantize"), 1.0);
    ControlObject::set(ConfigKey(m_sGroup2, "quantize"), 1.0);

    ControlObject::getControl(ConfigKey(m_sGroup1, "sync_mode"))
            ->set(static_cast<double>(SyncMode::LeaderExplicit));
    ControlObject::getControl(ConfigKey(m_sGroup2, "sync_mode"))
            ->set(static_cast<double>(SyncMode::Follower));

    ControlObject::getControl(ConfigKey(m_sGroup1, "play"))->set(1.0);
    ControlObject::getControl(ConfigKey(m_sGroup2, "play"))->set(1.0);

    // Beat Distance shall be still 0, because we are before the first beat.
    // This was fixed in https://github.com/mixxxdj/mixxx/issues/10358
    EXPECT_DOUBLE_EQ(m_pChannel2->getEngineBuffer()->m_pSyncControl->getBeatDistance(), 0);
    EXPECT_DOUBLE_EQ(
            ControlObject::get(ConfigKey(m_sGroup1, "playposition")),
            ControlObject::get(ConfigKey(m_sGroup2, "playposition")));
}

TEST_F(EngineSyncTest, BpmAdjustFactor) {
    // Failing test case from https://github.com/mixxxdj/mixxx/pull/2376

    m_pMixerDeck1->loadFakeTrack(false, 40.0);
    m_pMixerDeck2->loadFakeTrack(false, 150.0);
    ProcessBuffer();

    EXPECT_DOUBLE_EQ(40.0, ControlObject::get(ConfigKey(m_sGroup1, "bpm")));
    EXPECT_DOUBLE_EQ(150.0, ControlObject::get(ConfigKey(m_sGroup2, "bpm")));

    ControlObject::set(ConfigKey(m_sGroup1, "play"), 1.0);
    ProcessBuffer();

    ControlObject::set(ConfigKey(m_sGroup2, "sync_enabled"), 1.0);
    ProcessBuffer();

    // group 2 should be synced to the first playing deck and becomes leader
    EXPECT_DOUBLE_EQ(40.0, ControlObject::get(ConfigKey(m_sGroup1, "bpm")));
    EXPECT_DOUBLE_EQ(80.0, ControlObject::get(ConfigKey(m_sGroup2, "bpm")));
    EXPECT_TRUE(isSoftLeader(m_sGroup2));
    assertSyncOff(m_sGroup1);

    ControlObject::set(ConfigKey(m_sGroup2, "play"), 1.0);
    ProcessBuffer();
    EXPECT_TRUE(isSoftLeader(m_sGroup2));
    // Pretend a changing beatgrid
    static_cast<SyncControl*>(m_pEngineSync->getLeaderSyncable())->setLocalBpm(mixxx::Bpm{152});
    ProcessBuffer();

    ControlObject::set(ConfigKey(m_sGroup1, "sync_enabled"), 1.0);
    ProcessBuffer();

    // Group 1 should be already in sync, it must not change due to sync
    // and Group 2 must also remain.
    EXPECT_DOUBLE_EQ(40.0, ControlObject::get(ConfigKey(m_sGroup1, "bpm")));
    EXPECT_DOUBLE_EQ(80.0, ControlObject::get(ConfigKey(m_sGroup2, "bpm")));
    EXPECT_TRUE(isFollower(m_sGroup1));
    EXPECT_TRUE(isSoftLeader(m_sGroup2));
    EXPECT_TRUE(isFollower(m_sInternalClockGroup));
}

TEST_F(EngineSyncTest, ImplicitLeaderToInternalClock) {
    m_pMixerDeck1->loadFakeTrack(false, 100.0);
    m_pMixerDeck2->loadFakeTrack(false, 125.0);
    ProcessBuffer();

    EXPECT_DOUBLE_EQ(100.0, ControlObject::get(ConfigKey(m_sGroup1, "bpm")));
    EXPECT_DOUBLE_EQ(125.0, ControlObject::get(ConfigKey(m_sGroup2, "bpm")));

    // During cue-ing volume is 0.0
    ControlObject::set(ConfigKey(m_sGroup1, "volume"), 0.0);
    ControlObject::set(ConfigKey(m_sGroup1, "play"), 1.0);
    ControlObject::set(ConfigKey(m_sGroup2, "play"), 1.0);
    ProcessBuffer();

    ControlObject::set(ConfigKey(m_sGroup1, "sync_enabled"), 1.0);
    ControlObject::set(ConfigKey(m_sGroup2, "sync_enabled"), 1.0);
    ProcessBuffer();

    // group 2 should be synced to the first playing deck and becomes leader
    EXPECT_DOUBLE_EQ(125.0, ControlObject::get(ConfigKey(m_sGroup1, "bpm")));
    EXPECT_DOUBLE_EQ(125.0, ControlObject::get(ConfigKey(m_sGroup2, "bpm")));
    ASSERT_FALSE(isSoftLeader(m_sGroup1));
    ASSERT_TRUE(isSoftLeader(m_sGroup2));
    ASSERT_FALSE(isSoftLeader(m_sInternalClockGroup));
    ProcessBuffer();

    // Drop Track, no change
    ControlObject::set(ConfigKey(m_sGroup1, "volume"), 1.0);
    ASSERT_FALSE(isSoftLeader(m_sGroup1));
    ASSERT_TRUE(isSoftLeader(m_sGroup2));
    ASSERT_FALSE(isSoftLeader(m_sInternalClockGroup));

    // Other track stops, leader switches to deck 1
    ControlObject::set(ConfigKey(m_sGroup2, "volume"), 0.0);
    ProcessBuffer();
    ProcessBuffer();

    ASSERT_TRUE(isSoftLeader(m_sGroup1));
    ASSERT_FALSE(isSoftLeader(m_sGroup2));
    ASSERT_FALSE(isSoftLeader(m_sInternalClockGroup));
}

TEST_F(EngineSyncTest, BeatContextRounding) {
    ControlObject::getControl(ConfigKey(m_sGroup1, "quantize"))->set(1.0);
    mixxx::BeatsPointer pBeats1 = mixxx::Beats::fromConstTempo(
            m_pTrack1->getSampleRate(), mixxx::audio::FramePos(11166), mixxx::Bpm(162));
    m_pTrack1->trySetBeats(pBeats1);

    // This playposition is was able to fail a DEBUG_ASSERT before
    // https://github.com/mixxxdj/mixxx/pull/11263 It was not possible to pass
    // it as one double literal
    ControlObject::set(ConfigKey(m_sGroup1, "playposition"),
            (-5167.0 - 0.33333333333393966313) / 220500);
    ProcessBuffer();
    ControlObject::getControl(ConfigKey(m_sGroup1, "play"))->set(1.0);
    // this must not abort due to the assertion in Beats::iteratorFrom()
    ProcessBuffer();

    EXPECT_NEAR(-0.021112622826908536,
            ControlObject::get(ConfigKey(m_sGroup1, "playposition")),
            kMaxFloatingPointErrorHighPrecision);
}

TEST_F(EngineSyncTest, KeepCorrectFactorUponResync) {
    /* Usecase
        - load track @ 174bpm in deck 1
        - load track @ 87 bpm in deck 2
        - Sync deck 2 to 1, keep 87 BPM on deck 2
        - adjust rate slider on deck 1 to 184bpm
        - Sync deck 2 to 1, get 92 BPM on deck 2
    */
    m_pMixerDeck1->loadFakeTrack(false, 174.0);
    m_pMixerDeck2->loadFakeTrack(false, 87.0);
    ProcessBuffer();

    EXPECT_DOUBLE_EQ(174.0, ControlObject::get(ConfigKey(m_sGroup1, "bpm")));
    EXPECT_DOUBLE_EQ(87.0, ControlObject::get(ConfigKey(m_sGroup2, "bpm")));

    ControlObject::set(ConfigKey(m_sGroup2, "sync_enabled"), 1.0);
    ControlObject::set(ConfigKey(m_sGroup1, "sync_leader"), 1.0);
    ProcessBuffer();

    EXPECT_DOUBLE_EQ(174.0, ControlObject::get(ConfigKey(m_sGroup1, "bpm")));
    EXPECT_DOUBLE_EQ(87.0, ControlObject::get(ConfigKey(m_sGroup2, "bpm")));

    ControlObject::set(ConfigKey(m_sGroup1, "rate"), getRateSliderValue(184.0 / 174));
    ProcessBuffer();
    EXPECT_NEAR(184.0, ControlObject::get(ConfigKey(m_sGroup1, "bpm")), 0.001);
    EXPECT_NEAR(92.0, ControlObject::get(ConfigKey(m_sGroup2, "bpm")), 0.001);
    EXPECT_NEAR(getRateSliderValue(1.0574),
            ControlObject::get(ConfigKey(m_sGroup2, "rate")),
            0.005);
}

// There is a race condition preventing this usecase to work.
// https://github.com/mixxxdj/mixxx/issues/13689
TEST_F(EngineSyncTest, DISABLED_KeepCorrectFactorOnLoad) {
    /* Usecase
        - load track @ 174bpm in deck 1 and enable sync leader
        - enable sync follower on the empty second track
        - load track @ 87 bpm in deck 2
        - deck 2 should remain at 87 BPM and rate of 1.0
    */
    m_pMixerDeck1->loadFakeTrack(false, 174.0);
    ProcessBuffer();

    EXPECT_DOUBLE_EQ(174.0, ControlObject::get(ConfigKey(m_sGroup1, "bpm")));
    EXPECT_DOUBLE_EQ(0.0, ControlObject::get(ConfigKey(m_sGroup2, "bpm")));

    ControlObject::set(ConfigKey(m_sGroup2, "sync_enabled"), 1.0);
    ControlObject::set(ConfigKey(m_sGroup1, "sync_leader"), 1.0);
    ProcessBuffer();

    m_pMixerDeck2->loadFakeTrack(false, 87.0);
    ProcessBuffer();

    EXPECT_DOUBLE_EQ(174.0, ControlObject::get(ConfigKey(m_sGroup1, "bpm")));
    EXPECT_DOUBLE_EQ(87.0, ControlObject::get(ConfigKey(m_sGroup2, "bpm")));

    ControlObject::set(ConfigKey(m_sGroup1, "rate"), getRateSliderValue(184.0 / 174));
    ProcessBuffer();
    EXPECT_NEAR(184.0, ControlObject::get(ConfigKey(m_sGroup1, "bpm")), 0.001);
    EXPECT_NEAR(92.0, ControlObject::get(ConfigKey(m_sGroup2, "bpm")), 0.001);
    EXPECT_NEAR(getRateSliderValue(1.0574),
            ControlObject::get(ConfigKey(m_sGroup2, "rate")),
            0.005);
}
