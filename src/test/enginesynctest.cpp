#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <string>

#include "control/controlobject.h"
#include "engine/controls/bpmcontrol.h"
#include "engine/sync/synccontrol.h"
#include "mixer/basetrackplayer.h"
#include "preferences/usersettings.h"
#include "test/mixxxtest.h"
#include "test/mockedenginebackendtest.h"
#include "track/beatfactory.h"
#include "track/beatmap.h"
#include "util/memory.h"

namespace {
constexpr double kMaxFloatingPointErrorLowPrecision = 0.005;
constexpr double kMaxFloatingPointErrorHighPrecision = 0.0000000000000005;
constexpr double kMaxBeatDistanceEpsilon = 1e-9;
} // namespace

/// Tests for Master Sync.
/// The following manual tests should probably be performed:
/// * Quantize mode nudges tracks in sync, whether internal or deck master.
/// * Flinging tracks with the waveform should work.
/// * vinyl??
class EngineSyncTest : public MockedEngineBackendTest {
  public:
    QString getMasterGroup() {
        Syncable* pMasterSyncable = m_pEngineSync->getMasterSyncable();
        if (pMasterSyncable) {
            return pMasterSyncable->getGroup();
        }
        return QString();
    }
    bool isExplicitMaster(const QString& group) {
        return isMaster(group, true);
    }
    bool isSoftMaster(const QString& group) {
        return isMaster(group, false);
    }

    bool isFollower(const QString& group) {
        if (group == m_sInternalClockGroup) {
            return !ControlObject::getControl(ConfigKey(m_sInternalClockGroup,
                                                      "sync_master"))
                            ->toBool();
        }
        if (ControlObject::getControl(ConfigKey(group, "sync_mode"))->get() != SYNC_FOLLOWER) {
            return false;
        }
        if (!ControlObject::getControl(ConfigKey(group, "sync_enabled"))->toBool()) {
            return false;
        }
        if (ControlObject::getControl(ConfigKey(group, "sync_master"))->toBool()) {
            return false;
        }
        return true;
    }

    void assertSyncOff(const QString& group) {
        if (group == m_sInternalClockGroup) {
            ASSERT_EQ(0,
                    ControlObject::getControl(
                            ConfigKey(m_sInternalClockGroup, "sync_master"))
                            ->get());
        } else {
            ASSERT_EQ(SYNC_NONE,
                    ControlObject::getControl(ConfigKey(group, "sync_mode"))
                            ->get());
            ASSERT_EQ(0,
                    ControlObject::getControl(ConfigKey(group, "sync_enabled"))
                            ->get());
            ASSERT_EQ(0,
                    ControlObject::getControl(ConfigKey(group, "sync_master"))
                            ->get());
        }
    }

    void assertNoMaster() {
        ASSERT_EQ(NULL, m_pEngineSync->getMaster());
        ASSERT_EQ(NULL, m_pEngineSync->getMasterSyncable());
    }

  private:
    bool isMaster(const QString& group, bool explicitMaster) {
        if (group == m_sInternalClockGroup) {
            if (!ControlObject::getControl(ConfigKey(m_sInternalClockGroup,
                                                   "sync_master"))
                            ->toBool()) {
                return false;
            }
            if (m_pEngineSync->getMaster()) {
                return false;
            }
            if (m_sInternalClockGroup != getMasterGroup()) {
                return false;
            }
            // Internal Clock doesn't have explicit mode
            if (explicitMaster) {
                qWarning() << "test error, internal clock can never be explicit master";
                return false;
            }
            return true;
        }
        if (group == m_sGroup1) {
            if (m_pEngineSync->getMaster() != m_pChannel1) {
                return false;
            }
        } else if (group == m_sGroup2) {
            if (m_pEngineSync->getMaster() != m_pChannel2) {
                return false;
            }
        }
        if (getMasterGroup() != group) {
            return false;
        }

        if (explicitMaster) {
            if (ControlObject::getControl(ConfigKey(group, "sync_mode"))
                            ->get() != SYNC_MASTER_EXPLICIT) {
                return false;
            }
        } else {
            if (ControlObject::getControl(ConfigKey(group, "sync_mode"))
                            ->get() != SYNC_MASTER_SOFT) {
                return false;
            }
        }
        if (!ControlObject::getControl(ConfigKey(group, "sync_enabled"))->toBool()) {
            return false;
        }
        if (!ControlObject::getControl(ConfigKey(group, "sync_master"))->toBool()) {
            return false;
        }
        return true;
    }
};

TEST_F(EngineSyncTest, ControlObjectsExist) {
    // This isn't exhaustive, but certain COs have a habit of not being set up properly.
    ASSERT_TRUE(ControlObject::getControl(ConfigKey(m_sGroup1, "file_bpm")) !=
            NULL);
}

TEST_F(EngineSyncTest, SetMasterSuccess) {
    // If we set the first channel to master, EngineSync should get that message.

    auto pButtonMasterSync1 =
            std::make_unique<ControlProxy>(m_sGroup1, "sync_mode");
    pButtonMasterSync1->slotSet(SYNC_MASTER_EXPLICIT);
    ProcessBuffer();

    // No tracks are playing and we have no beats, SYNC_MASTER_EXPLICIT state is in stand-by
    EXPECT_DOUBLE_EQ(
            0.0, ControlObject::getControl(ConfigKey(m_sGroup1, "bpm"))->get());
    // The master sync should now be channel 1.
    ASSERT_TRUE(isExplicitMaster(m_sGroup1));

    auto pButtonMasterSync2 =
            std::make_unique<ControlProxy>(m_sGroup2, "sync_mode");
    pButtonMasterSync2->set(SYNC_FOLLOWER);
    ProcessBuffer();

    ASSERT_TRUE(isFollower(m_sGroup2));

    // Now set channel 2 to be master.
    pButtonMasterSync2->set(SYNC_MASTER_EXPLICIT);
    ProcessBuffer();

    // Now channel 2 should be master, and channel 1 should be a follower.
    ASSERT_TRUE(isExplicitMaster(m_sGroup2));
    ASSERT_TRUE(isFollower(m_sGroup1));

    // Now back again.
    pButtonMasterSync1->set(SYNC_MASTER_EXPLICIT);
    ProcessBuffer();

    // Now channel 1 should be master, and channel 2 should be a follower.
    ASSERT_TRUE(isExplicitMaster(m_sGroup1));
    ASSERT_TRUE(isFollower(m_sGroup2));

    // Now set channel 1 to follower, no all are followers, waiting for a tempo to adopt.
    pButtonMasterSync1->slotSet(SYNC_FOLLOWER);
    ProcessBuffer();

    ASSERT_TRUE(isFollower(m_sInternalClockGroup));
    ASSERT_TRUE(isFollower(m_sGroup1));
    ASSERT_TRUE(isFollower(m_sGroup2));
}

TEST_F(EngineSyncTest, ExplicitMasterPersists) {
    // If we set an explicit master, enabling sync or pressing play on other decks
    // doesn't cause the master to move around.
    mixxx::BeatsPointer pBeats1 = BeatFactory::makeBeatGrid(*m_pTrack1, 120, 0.0);
    m_pTrack1->setBeats(pBeats1);
    mixxx::BeatsPointer pBeats2 = BeatFactory::makeBeatGrid(*m_pTrack2, 124, 0.0);
    m_pTrack2->setBeats(pBeats2);

    auto pButtonMasterSync1 =
            std::make_unique<ControlProxy>(m_sGroup1, "sync_mode");
    ControlObject::getControl(ConfigKey(m_sGroup1, "play"))->set(1.0);
    pButtonMasterSync1->slotSet(SYNC_MASTER_EXPLICIT);
    ProcessBuffer();
    // The master sync should now be channel 1.
    ASSERT_TRUE(isExplicitMaster(m_sGroup1));

    auto pButtonMasterSync2 =
            std::make_unique<ControlProxy>(m_sGroup2, "sync_enabled");
    ControlObject::getControl(ConfigKey(m_sGroup2, "play"))->set(1.0);
    pButtonMasterSync2->set(1.0);
    ProcessBuffer();
    ASSERT_TRUE(isExplicitMaster(m_sGroup1));
    ASSERT_TRUE(isFollower(m_sGroup2));

    // Stop deck 2, and restart it, no change.
    ControlObject::getControl(ConfigKey(m_sGroup2, "play"))->set(0.0);
    ProcessBuffer();
    ControlObject::getControl(ConfigKey(m_sGroup2, "play"))->set(1.0);
    ProcessBuffer();
    ASSERT_TRUE(isExplicitMaster(m_sGroup1));
    ASSERT_TRUE(isFollower(m_sGroup2));
}

TEST_F(EngineSyncTest, SetMasterWhilePlaying) {
    // Make sure we don't get two master lights if we change masters while playing.
    mixxx::BeatsPointer pBeats1 = BeatFactory::makeBeatGrid(*m_pTrack1, 120, 0.0);
    m_pTrack1->setBeats(pBeats1);
    mixxx::BeatsPointer pBeats2 = BeatFactory::makeBeatGrid(*m_pTrack2, 124, 0.0);
    m_pTrack2->setBeats(pBeats2);
    mixxx::BeatsPointer pBeats3 = BeatFactory::makeBeatGrid(*m_pTrack3, 128, 0.0);
    m_pTrack3->setBeats(pBeats3);

    auto pButtonMasterSync1 =
            std::make_unique<ControlProxy>(m_sGroup1, "sync_mode");
    pButtonMasterSync1->set(SYNC_MASTER_EXPLICIT);
    auto pButtonMasterSync2 =
            std::make_unique<ControlProxy>(m_sGroup2, "sync_mode");
    pButtonMasterSync2->slotSet(SYNC_FOLLOWER);
    auto pButtonMasterSync3 =
            std::make_unique<ControlProxy>(m_sGroup3, "sync_mode");
    pButtonMasterSync3->slotSet(SYNC_FOLLOWER);

    ControlObject::getControl(ConfigKey(m_sGroup1, "play"))->set(1.0);
    ControlObject::getControl(ConfigKey(m_sGroup2, "play"))->set(1.0);
    ControlObject::getControl(ConfigKey(m_sGroup3, "play"))->set(1.0);

    ProcessBuffer();

    pButtonMasterSync3->slotSet(SYNC_MASTER_EXPLICIT);

    ProcessBuffer();

    ASSERT_TRUE(isFollower(m_sGroup1));
    ASSERT_TRUE(isFollower(m_sGroup2));
    ASSERT_TRUE(isExplicitMaster(m_sGroup3));
}

TEST_F(EngineSyncTest, SetEnabledBecomesMaster) {
    // If we set the first channel with a valid tempo to follower, it should be master.
    mixxx::BeatsPointer pBeats1 = BeatFactory::makeBeatGrid(*m_pTrack1, 80, 0.0);
    m_pTrack1->setBeats(pBeats1);
    auto pButtonMasterSync1 =
            std::make_unique<ControlProxy>(m_sGroup1, "sync_mode");
    pButtonMasterSync1->slotSet(SYNC_FOLLOWER);
    ProcessBuffer();

    ASSERT_TRUE(isSoftMaster(m_sGroup1));
    ASSERT_TRUE(isFollower(m_sInternalClockGroup));
}

TEST_F(EngineSyncTest, DisableInternalMasterWhilePlaying) {
    auto pButtonMasterSync = std::make_unique<ControlProxy>(
            m_sInternalClockGroup, "sync_master");
    pButtonMasterSync->slotSet(1.0);
    auto pButtonSyncMode1 =
            std::make_unique<ControlProxy>(m_sGroup1, "sync_mode");
    pButtonSyncMode1->slotSet(SYNC_FOLLOWER);
    auto pButtonSyncMode2 =
            std::make_unique<ControlProxy>(m_sGroup2, "sync_mode");
    pButtonSyncMode2->slotSet(SYNC_FOLLOWER);
    ProcessBuffer();

    // The master sync should now be Internal.
    ASSERT_TRUE(isSoftMaster(m_sInternalClockGroup));

    // Make sure both decks are playing.
    mixxx::BeatsPointer pBeats1 = BeatFactory::makeBeatGrid(*m_pTrack1, 80, 0.0);
    m_pTrack1->setBeats(pBeats1);
    ControlObject::getControl(ConfigKey(m_sGroup1, "play"))->set(1.0);
    mixxx::BeatsPointer pBeats2 = BeatFactory::makeBeatGrid(*m_pTrack2, 80, 0.0);
    m_pTrack2->setBeats(pBeats2);
    ControlObject::getControl(ConfigKey(m_sGroup2, "play"))->set(1.0);
    ProcessBuffer();

    // Now unset Internal master.
    pButtonMasterSync->slotSet(0.0);
    ProcessBuffer();

    // This is not allowed, Internal should still be master.
    ASSERT_TRUE(isSoftMaster(m_sInternalClockGroup));
    ASSERT_EQ(1, pButtonMasterSync->get());
}

TEST_F(EngineSyncTest, DisableSyncOnMaster) {
    // Channel 1 follower, channel 2 master.
    mixxx::BeatsPointer pBeats1 = BeatFactory::makeBeatGrid(*m_pTrack1, 130, 0.0);
    m_pTrack1->setBeats(pBeats1);
    auto pButtonSyncMode1 =
            std::make_unique<ControlProxy>(m_sGroup1, "sync_mode");
    pButtonSyncMode1->slotSet(SYNC_FOLLOWER);

    mixxx::BeatsPointer pBeats2 = BeatFactory::makeBeatGrid(*m_pTrack2, 130, 0.0);
    m_pTrack2->setBeats(pBeats2);
    auto pButtonSyncMaster2 =
            std::make_unique<ControlProxy>(m_sGroup2, "sync_master");
    pButtonSyncMaster2->slotSet(1.0);

    ASSERT_TRUE(isFollower(m_sGroup1));
    ASSERT_TRUE(isExplicitMaster(m_sGroup2));

    // Unset enabled on channel2, it should work.
    auto pButtonSyncEnabled2 =
            std::make_unique<ControlProxy>(m_sGroup2, "sync_enabled");
    pButtonSyncEnabled2->slotSet(0.0);

    ASSERT_TRUE(isSoftMaster(m_sGroup1));
    ASSERT_EQ(0, ControlObject::getControl(ConfigKey(m_sGroup2, "sync_enabled"))->get());
    ASSERT_EQ(0, ControlObject::getControl(ConfigKey(m_sGroup2, "sync_master"))->get());
}

TEST_F(EngineSyncTest, InternalMasterSetFollowerSliderMoves) {
    // If internal is master, and we turn on a follower, the slider should move.
    auto pButtonMasterSyncInternal = std::make_unique<ControlProxy>(
            m_sInternalClockGroup, "sync_master");
    pButtonMasterSyncInternal->slotSet(1);
    auto pMasterSyncSlider =
            std::make_unique<ControlProxy>(m_sInternalClockGroup, "bpm");
    pMasterSyncSlider->set(100.0);

    // Set the file bpm of channel 1 to 80 bpm.
    mixxx::BeatsPointer pBeats1 = BeatFactory::makeBeatGrid(*m_pTrack1, 80, 0.0);
    m_pTrack1->setBeats(pBeats1);

    auto pButtonMasterSync1 =
            std::make_unique<ControlProxy>(m_sGroup1, "sync_mode");
    pButtonMasterSync1->slotSet(SYNC_FOLLOWER);
    ProcessBuffer();

    EXPECT_DOUBLE_EQ(getRateSliderValue(1.25),
            ControlObject::getControl(ConfigKey(m_sGroup1, "rate"))->get());
    EXPECT_DOUBLE_EQ(100.0,
            ControlObject::getControl(ConfigKey(m_sGroup1, "bpm"))->get());
}

TEST_F(EngineSyncTest, AnySyncDeckSliderStays) {
    // If there exists a sync deck, even if it's not playing, don't change the
    // master BPM if a new deck enables sync.

    mixxx::BeatsPointer pBeats1 = BeatFactory::makeBeatGrid(*m_pTrack1, 80, 0.0);
    m_pTrack1->setBeats(pBeats1);
    auto pButtonSyncEnabled1 =
            std::make_unique<ControlProxy>(m_sGroup1, "sync_enabled");
    pButtonSyncEnabled1->set(1.0);

    // After setting up the first deck, the internal BPM should be 80.
    EXPECT_DOUBLE_EQ(80.0,
            ControlObject::getControl(ConfigKey(m_sInternalClockGroup, "bpm"))
                    ->get());

    mixxx::BeatsPointer pBeats2 = BeatFactory::makeBeatGrid(*m_pTrack2, 100, 0.0);
    m_pTrack2->setBeats(pBeats2);
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
    auto pButtonMasterSync1 =
            std::make_unique<ControlProxy>(m_sGroup1, "sync_mode");
    auto pButtonMasterSync2 =
            std::make_unique<ControlProxy>(m_sGroup2, "sync_mode");
    auto pButtonSyncEnabled1 =
            std::make_unique<ControlProxy>(m_sGroup1, "sync_enabled");
    auto pButtonSyncEnabled2 =
            std::make_unique<ControlProxy>(m_sGroup2, "sync_enabled");

    // Set up decks so they can be playing, and start deck 1.
    mixxx::BeatsPointer pBeats1 = BeatFactory::makeBeatGrid(*m_pTrack1, 100, 0.0);
    m_pTrack1->setBeats(pBeats1);
    ControlObject::set(ConfigKey(m_sGroup1, "rate"), getRateSliderValue(1.0));
    ControlObject::set(ConfigKey(m_sGroup1, "play"), 1.0);
    mixxx::BeatsPointer pBeats2 = BeatFactory::makeBeatGrid(*m_pTrack2, 130, 0.0);
    m_pTrack2->setBeats(pBeats2);
    ControlObject::set(ConfigKey(m_sGroup2, "rate"), getRateSliderValue(1.0));
    ControlObject::set(ConfigKey(m_sGroup2, "play"), 0.0);
    ProcessBuffer();

    // Set channel 1 to be enabled
    pButtonSyncEnabled1->set(1.0);
    ProcessBuffer();

    // The master sync should now be deck 1.
    ASSERT_TRUE(isSoftMaster(m_sGroup1));
    EXPECT_DOUBLE_EQ(100.0,
            ControlObject::get(ConfigKey(m_sInternalClockGroup, "bpm")));

    // Set channel 2 to be enabled.
    pButtonSyncEnabled2->set(1);
    ProcessBuffer();

    // channel 1 still master while 2 is not playing
    ASSERT_TRUE(isSoftMaster(m_sGroup1));
    ASSERT_TRUE(isFollower(m_sInternalClockGroup));
    ASSERT_TRUE(isFollower(m_sGroup2));

    // The rate should not have changed -- deck 1 still matches deck 2.
    EXPECT_DOUBLE_EQ(getRateSliderValue(1.0),
            ControlObject::getControl(ConfigKey(m_sGroup1, "rate"))->get());

    // Reset channel 2 rate, set channel 2 to play, and process a buffer.
    ControlObject::set(ConfigKey(m_sGroup2, "rate"), getRateSliderValue(1.0));
    ControlObject::set(ConfigKey(m_sGroup2, "play"), 1.0);
    ProcessBuffer();
    // Now internal clock is master
    ASSERT_TRUE(isSoftMaster(m_sInternalClockGroup));
    ASSERT_TRUE(isFollower(m_sGroup1));
    ASSERT_TRUE(isFollower(m_sGroup2));

    // Now disable sync on channel 1.
    pButtonSyncEnabled1->set(0);
    ProcessBuffer();

    // Master flips to deck 2
    ASSERT_TRUE(isSoftMaster(m_sGroup2));
    ASSERT_TRUE(isFollower(m_sInternalClockGroup));

    // Rate should now match channel 2.
    EXPECT_DOUBLE_EQ(
            130.0, ControlObject::get(ConfigKey(m_sInternalClockGroup, "bpm")));
}

TEST_F(EngineSyncTest, SetExplicitMasterByLights) {
    // Same as above, except we use the midi lights to change state.
    auto pButtonMasterSync1 =
            std::make_unique<ControlProxy>(m_sGroup1, "sync_mode");
    auto pButtonMasterSync2 =
            std::make_unique<ControlProxy>(m_sGroup2, "sync_mode");
    auto pButtonSyncEnabled1 =
            std::make_unique<ControlProxy>(m_sGroup1, "sync_enabled");
    auto pButtonSyncEnabled2 =
            std::make_unique<ControlProxy>(m_sGroup2, "sync_enabled");
    auto pButtonSyncMaster1 =
            std::make_unique<ControlProxy>(m_sGroup1, "sync_master");
    auto pButtonSyncMaster2 =
            std::make_unique<ControlProxy>(m_sGroup2, "sync_master");

    // Set the file bpm of channel 1 to 160bpm.
    mixxx::BeatsPointer pBeats1 = BeatFactory::makeBeatGrid(*m_pTrack1, 160, 0.0);
    m_pTrack1->setBeats(pBeats1);
    // Set the file bpm of channel 2 to 150bpm.
    mixxx::BeatsPointer pBeats2 = BeatFactory::makeBeatGrid(*m_pTrack1, 150, 0.0);
    m_pTrack2->setBeats(pBeats2);

    // Set channel 1 to be explicit master.
    pButtonSyncMaster1->slotSet(1.0);
    ProcessBuffer();

    // The master sync should now be channel 1.
    ASSERT_TRUE(isExplicitMaster(m_sGroup1));

    // Set channel 2 to be follower.
    pButtonSyncEnabled2->slotSet(1);

    ASSERT_TRUE(isFollower(m_sGroup2));

    // Now set channel 2 to be master.
    pButtonSyncMaster2->slotSet(1);

    // Now channel 2 should be master, and channel 1 should be a follower.
    ASSERT_TRUE(isFollower(m_sGroup1));
    ASSERT_TRUE(isExplicitMaster(m_sGroup2));

    // Now back again.
    pButtonSyncMaster1->slotSet(1);

    // Now channel 1 should be master, and channel 2 should be a follower.
    ASSERT_TRUE(isExplicitMaster(m_sGroup1));
    ASSERT_TRUE(isFollower(m_sGroup2));

    // Now set channel 1 to not-master, all will become follower.
    // handing over master to the internal clock
    pButtonSyncMaster1->slotSet(0);

    ASSERT_TRUE(isSoftMaster(m_sInternalClockGroup));
    ASSERT_TRUE(isFollower(m_sGroup1));
    ASSERT_TRUE(isFollower(m_sGroup2));
}

TEST_F(EngineSyncTest, SetExplicitMasterByLightsNoTracks) {
    // Same as above, except we use the midi lights to change state.
    auto pButtonSyncEnabled2 =
            std::make_unique<ControlProxy>(m_sGroup2, "sync_enabled");
    auto pButtonSyncMaster1 =
            std::make_unique<ControlProxy>(m_sGroup1, "sync_master");

    // Now back again.
    pButtonSyncMaster1->slotSet(1);

    // Set channel 2 to be follower.
    pButtonSyncEnabled2->slotSet(1);

    // Now channel 1 should be master, and channel 2 should be a follower.
    ASSERT_TRUE(isExplicitMaster(m_sGroup1));

    // Now set channel 1 to not-master, all will be follower waiting for a valid bpm.
    pButtonSyncMaster1->slotSet(0);

    ASSERT_TRUE(isFollower(m_sGroup1));
    ASSERT_TRUE(isFollower(m_sGroup2));
    ASSERT_TRUE(isFollower(m_sInternalClockGroup));
}

TEST_F(EngineSyncTest, RateChangeTest) {
    auto pButtonMasterSync1 =
            std::make_unique<ControlProxy>(m_sGroup1, "sync_mode");
    pButtonMasterSync1->set(SYNC_MASTER_EXPLICIT);
    auto pButtonMasterSync2 =
            std::make_unique<ControlProxy>(m_sGroup2, "sync_mode");
    pButtonMasterSync2->set(SYNC_FOLLOWER);
    ProcessBuffer();

    // Set the file bpm of channel 1 to 160bpm.
    mixxx::BeatsPointer pBeats1 = BeatFactory::makeBeatGrid(*m_pTrack1, 160, 0.0);
    m_pTrack1->setBeats(pBeats1);
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

    // Internal master should also be 192.
    EXPECT_DOUBLE_EQ(
            192.0, ControlObject::get(ConfigKey(m_sInternalClockGroup, "bpm")));

    // Set the file bpm of channel 2 to 120bpm.
    mixxx::BeatsPointer pBeats2 = BeatFactory::makeBeatGrid(*m_pTrack2, 120, 0.0);
    m_pTrack2->setBeats(pBeats2);
    EXPECT_DOUBLE_EQ(
            120.0, ControlObject::get(ConfigKey(m_sGroup2, "file_bpm")));

    // rate slider for channel 2 should now be 1.6 = 160 * 1.2 / 120.
    EXPECT_DOUBLE_EQ(getRateSliderValue(1.6),
            ControlObject::get(ConfigKey(m_sGroup2, "rate")));
    EXPECT_DOUBLE_EQ(192.0, ControlObject::get(ConfigKey(m_sGroup2, "bpm")));
}

TEST_F(EngineSyncTest, RateChangeTestWeirdOrder) {
    // This is like the test above, but the user loads the track after the slider has been tweaked.
    auto pButtonMasterSync1 =
            std::make_unique<ControlProxy>(m_sGroup1, "sync_mode");
    pButtonMasterSync1->slotSet(SYNC_MASTER_EXPLICIT);
    auto pButtonMasterSync2 =
            std::make_unique<ControlProxy>(m_sGroup2, "sync_mode");
    pButtonMasterSync2->slotSet(SYNC_FOLLOWER);
    ProcessBuffer();

    // Set the file bpm of channel 1 to 160bpm.
    mixxx::BeatsPointer pBeats1 = BeatFactory::makeBeatGrid(*m_pTrack1, 160, 0.0);
    m_pTrack1->setBeats(pBeats1);
    EXPECT_DOUBLE_EQ(
            160.0, ControlObject::get(ConfigKey(m_sInternalClockGroup, "bpm")));

    // Set the file bpm of channel 2 to 120bpm.
    mixxx::BeatsPointer pBeats2 = BeatFactory::makeBeatGrid(*m_pTrack2, 120, 0.0);
    m_pTrack2->setBeats(pBeats2);

    // Set the rate slider of channel 1 to 1.2.
    ControlObject::set(ConfigKey(m_sGroup1, "rate"), getRateSliderValue(1.2));

    // Rate slider for channel 2 should now be 1.6 = (160 * 1.2 / 120) - 1.0.
    EXPECT_DOUBLE_EQ(getRateSliderValue(1.6),
            ControlObject::get(ConfigKey(m_sGroup2, "rate")));
    EXPECT_DOUBLE_EQ(192.0, ControlObject::get(ConfigKey(m_sGroup2, "bpm")));

    // Internal Master BPM should read the same.
    EXPECT_DOUBLE_EQ(
            192.0, ControlObject::get(ConfigKey(m_sInternalClockGroup, "bpm")));
}

TEST_F(EngineSyncTest, RateChangeTestOrder3) {
    // Set the file bpm of channel 1 to 160bpm.
    mixxx::BeatsPointer pBeats1 = BeatFactory::makeBeatGrid(*m_pTrack1, 160, 0.0);
    m_pTrack1->setBeats(pBeats1);
    EXPECT_DOUBLE_EQ(
            160.0, ControlObject::get(ConfigKey(m_sGroup1, "file_bpm")));

    // Set the file bpm of channel 2 to 120bpm.
    mixxx::BeatsPointer pBeats2 = BeatFactory::makeBeatGrid(*m_pTrack2, 120, 0.0);
    m_pTrack2->setBeats(pBeats2);
    EXPECT_DOUBLE_EQ(
            120.0, ControlObject::get(ConfigKey(m_sGroup2, "file_bpm")));

    // Turn on Master and Follower.
    auto pButtonMasterSync1 =
            std::make_unique<ControlProxy>(m_sGroup1, "sync_mode");
    pButtonMasterSync1->set(SYNC_MASTER_EXPLICIT);
    ProcessBuffer();

    ASSERT_TRUE(isExplicitMaster(m_sGroup1));

    auto pButtonMasterSync2 =
            std::make_unique<ControlProxy>(m_sGroup2, "sync_mode");
    pButtonMasterSync2->set(SYNC_FOLLOWER);
    ProcessBuffer();

    // Follower should immediately set its slider.
    EXPECT_NEAR(getRateSliderValue(1.3333333333),
            ControlObject::get(ConfigKey(m_sGroup2, "rate")),
            kMaxFloatingPointErrorLowPrecision);
    EXPECT_DOUBLE_EQ(160.0, ControlObject::get(ConfigKey(m_sGroup2, "bpm")));
    EXPECT_DOUBLE_EQ(
            160.0, ControlObject::get(ConfigKey(m_sInternalClockGroup, "bpm")));
}

TEST_F(EngineSyncTest, FollowerRateChange) {
    // Confirm that followers can change master sync rate as well.
    auto pButtonMasterSync1 =
            std::make_unique<ControlProxy>(m_sGroup1, "sync_mode");
    pButtonMasterSync1->set(SYNC_MASTER_EXPLICIT);
    auto pButtonMasterSync2 =
            std::make_unique<ControlProxy>(m_sGroup2, "sync_mode");
    pButtonMasterSync2->set(SYNC_FOLLOWER);
    ProcessBuffer();

    // Set the file bpm of channel 1 to 160bpm.
    mixxx::BeatsPointer pBeats1 = BeatFactory::makeBeatGrid(*m_pTrack1, 160, 0.0);
    m_pTrack1->setBeats(pBeats1);

    // Set the file bpm of channel 2 to 120bpm.
    mixxx::BeatsPointer pBeats2 = BeatFactory::makeBeatGrid(*m_pTrack2, 120, 0.0);
    m_pTrack2->setBeats(pBeats2);

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
    auto pButtonMasterSyncInternal = std::make_unique<ControlProxy>(
            m_sInternalClockGroup, "sync_master");
    pButtonMasterSyncInternal->set(SYNC_MASTER_SOFT);
    auto pButtonMasterSync1 =
            std::make_unique<ControlProxy>(m_sGroup1, "sync_mode");
    pButtonMasterSync1->set(SYNC_FOLLOWER);
    auto pButtonMasterSync2 =
            std::make_unique<ControlProxy>(m_sGroup2, "sync_mode");
    pButtonMasterSync2->set(SYNC_FOLLOWER);
    ProcessBuffer();

    ASSERT_TRUE(isSoftMaster(m_sInternalClockGroup));
    ASSERT_TRUE(isFollower(m_sGroup1));
    ASSERT_TRUE(isFollower(m_sGroup2));

    // Set the file bpm of channel 1 to 160bpm.
    mixxx::BeatsPointer pBeats1 = BeatFactory::makeBeatGrid(*m_pTrack1, 160, 0.0);
    m_pTrack1->setBeats(pBeats1);
    EXPECT_DOUBLE_EQ(160.0,
            ControlObject::getControl(ConfigKey(m_sGroup1, "file_bpm"))->get());

    // Set the file bpm of channel 2 to 120bpm.
    mixxx::BeatsPointer pBeats2 = BeatFactory::makeBeatGrid(*m_pTrack2, 120, 0.0);
    m_pTrack2->setBeats(pBeats2);
    EXPECT_DOUBLE_EQ(120.0,
            ControlObject::getControl(ConfigKey(m_sGroup2, "file_bpm"))->get());

    // Set the internal rate to 150.
    auto pMasterSyncSlider =
            std::make_unique<ControlProxy>(m_sInternalClockGroup, "bpm");
    pMasterSyncSlider->set(150.0);
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
    pMasterSyncSlider->set(140.0);

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

TEST_F(EngineSyncTest, MasterStopSliderCheck) {
    // If the master is playing, and stop is pushed, the sliders should stay the same.
    mixxx::BeatsPointer pBeats1 = BeatFactory::makeBeatGrid(*m_pTrack1, 120, 0.0);
    m_pTrack1->setBeats(pBeats1);
    mixxx::BeatsPointer pBeats2 = BeatFactory::makeBeatGrid(*m_pTrack2, 128, 0.0);
    m_pTrack2->setBeats(pBeats2);

    auto pButtonMasterSync1 =
            std::make_unique<ControlProxy>(m_sGroup1, "sync_mode");
    pButtonMasterSync1->slotSet(SYNC_MASTER_EXPLICIT);
    auto pButtonMasterSync2 =
            std::make_unique<ControlProxy>(m_sGroup2, "sync_mode");
    pButtonMasterSync2->slotSet(SYNC_FOLLOWER);
    ProcessBuffer();

    //ASSERT_TRUE(isExplicitMaster(m_sGroup1));
    ASSERT_TRUE(isFollower(m_sGroup2));

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

TEST_F(EngineSyncTest, EnableOneDeckInitsMaster) {
    // If Internal is master, and we turn sync on a playing deck, the playing deck sets the
    // internal master and the beat distances are now aligned.
    ControlObject::set(ConfigKey(m_sInternalClockGroup, "bpm"), 124.0);
    ControlObject::set(ConfigKey(m_sInternalClockGroup, "beat_distance"), 0.5);
    ProcessBuffer();

    // Set up the deck to play.
    mixxx::BeatsPointer pBeats1 = BeatFactory::makeBeatGrid(*m_pTrack1, 130, 0.0);
    m_pTrack1->setBeats(pBeats1);
    ControlObject::getControl(ConfigKey(m_sGroup1, "rate"))
            ->set(getRateSliderValue(1.0));
    ControlObject::getControl(ConfigKey(m_sGroup1, "beat_distance"))->set(0.2);
    ControlObject::getControl(ConfigKey(m_sGroup1, "play"))->set(1.0);

    // Enable Sync.  We have to call requestEnableSync directly
    // because calling ProcessBuffer() tries to advance the beat_distance values.
    m_pEngineSync->requestEnableSync(
            m_pEngineSync->getSyncableForGroup(m_sGroup1), true);

    // Internal is no longer master because there is exactly one playing deck.
    ASSERT_TRUE(isSoftMaster(m_sGroup1));
    ASSERT_TRUE(isFollower(m_sInternalClockGroup));

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
    mixxx::BeatsPointer pBeats2 = BeatFactory::makeBeatGrid(*m_pTrack2, 140, 0.0);
    m_pTrack2->setBeats(pBeats2);
    ControlObject::getControl(ConfigKey(m_sGroup2, "rate"))
            ->set(getRateSliderValue(1.0));
    ControlObject::getControl(ConfigKey(m_sGroup2, "beat_distance"))->set(0.2);
    ControlObject::getControl(ConfigKey(m_sGroup2, "play"))->set(1.0);

    m_pEngineSync->requestEnableSync(
            m_pEngineSync->getSyncableForGroup(m_sGroup2), true);
    // Now master should be Internal Clock because we have two playing decks.
    ASSERT_TRUE(isSoftMaster(m_sInternalClockGroup));
    ASSERT_TRUE(isFollower(m_sGroup1));
    ASSERT_TRUE(isFollower(m_sGroup2));

    EXPECT_DOUBLE_EQ(
            130.0, ControlObject::get(ConfigKey(m_sInternalClockGroup, "bpm")));
    EXPECT_DOUBLE_EQ(130.0, ControlObject::get(ConfigKey(m_sGroup2, "bpm")));
    EXPECT_DOUBLE_EQ(
            0.2, ControlObject::get(ConfigKey(m_sGroup2, "beat_distance")));
    EXPECT_DOUBLE_EQ(0.2,
            ControlObject::get(
                    ConfigKey(m_sInternalClockGroup, "beat_distance")));
}

TEST_F(EngineSyncTest, EnableOneDeckInitializesMaster) {
    // Enabling sync on a deck causes it to be master, and sets bpm and clock.
    // Set the deck to play.
    mixxx::BeatsPointer pBeats1 = BeatFactory::makeBeatGrid(*m_pTrack1, 130, 0.0);
    m_pTrack1->setBeats(pBeats1);
    ControlObject::getControl(ConfigKey(m_sGroup1, "rate"))
            ->set(getRateSliderValue(1.0));
    ControlObject::getControl(ConfigKey(m_sGroup1, "beat_distance"))->set(0.2);
    ControlObject::getControl(ConfigKey(m_sGroup1, "play"))->set(1.0);

    // Set the deck to follower.
    // As above, use direct call to avoid advancing beat distance.
    m_pEngineSync->requestEnableSync(
            m_pEngineSync->getSyncableForGroup(m_sGroup1), true);

    // That first deck is now master
    ASSERT_TRUE(isSoftMaster(m_sGroup1));

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

TEST_F(EngineSyncTest, LoadTrackInitializesMaster) {
    // First eject the fake tracks that come with the testing framework.
    m_pChannel1->getEngineBuffer()->slotEjectTrack(1.0);
    m_pChannel2->getEngineBuffer()->slotEjectTrack(1.0);
    m_pChannel3->getEngineBuffer()->slotEjectTrack(1.0);

    auto pButtonSyncEnabled1 =
            std::make_unique<ControlProxy>(m_sGroup1, "sync_enabled");
    pButtonSyncEnabled1->slotSet(1.0);

    // No master because this deck has no track.
    EXPECT_EQ(NULL, m_pEngineSync->getMaster());
    EXPECT_DOUBLE_EQ(
            0.0, ControlObject::getControl(ConfigKey(m_sGroup1, "bpm"))->get());

    // The track load trigger a master change.
    m_pMixerDeck1->loadFakeTrack(false, 140.0);
    ASSERT_TRUE(isSoftMaster(m_sGroup1));
    EXPECT_DOUBLE_EQ(140.0,
            ControlObject::getControl(ConfigKey(m_sGroup1, "bpm"))->get());

    // But as soon as we play, deck 1 is master
    ControlObject::getControl(ConfigKey(m_sGroup1, "play"))->set(1.0);
    ASSERT_TRUE(isSoftMaster(m_sGroup1));
    ControlObject::getControl(ConfigKey(m_sGroup1, "play"))->set(0.0);

    // If sync is on two decks and we load a track in only one of them, it will be
    // master.
    m_pChannel1->getEngineBuffer()->slotEjectTrack(1.0);
    ASSERT_TRUE(isFollower(m_sGroup1));
    // no relevant tempo available so internal clock is following
    ASSERT_TRUE(isFollower(m_sInternalClockGroup));

    auto pButtonSyncEnabled2 =
            std::make_unique<ControlProxy>(m_sGroup2, "sync_enabled");
    pButtonSyncEnabled2->slotSet(1.0);

    m_pMixerDeck1->loadFakeTrack(false, 128.0);

    // Deck 2 is still empty so Deck 1 becomes master again
    ASSERT_TRUE(isSoftMaster(m_sGroup1));
    EXPECT_DOUBLE_EQ(128.0,
            ControlObject::getControl(ConfigKey(m_sGroup1, "bpm"))->get());

    // If sync is on two decks and one deck is loaded but not playing, we should
    // initialize to that deck with internal clock master.
    m_pMixerDeck2->loadFakeTrack(false, 110.0);

    ASSERT_TRUE(isSoftMaster(m_sInternalClockGroup));
    EXPECT_DOUBLE_EQ(128.0,
            ControlObject::getControl(ConfigKey(m_sInternalClockGroup, "bpm"))
                    ->get());
    EXPECT_DOUBLE_EQ(128.0,
            ControlObject::getControl(ConfigKey(m_sGroup1, "bpm"))->get());
    EXPECT_DOUBLE_EQ(128.0,
            ControlObject::getControl(ConfigKey(m_sGroup2, "bpm"))->get());
}

TEST_F(EngineSyncTest, LoadTrackResetTempoOption) {
    // Make sure playing decks with master sync enabled do not change tempo when
    // the "Reset Speed/Tempo" preference is set and a track is loaded to another
    // deck with master sync enabled.
    m_pConfig->set(ConfigKey("[Controls]", "SpeedAutoReset"),
            ConfigValue(BaseTrackPlayer::RESET_SPEED));

    // Enable sync on two stopped decks
    auto pButtonSyncEnabled1 =
            std::make_unique<ControlProxy>(m_sGroup1, "sync_enabled");
    pButtonSyncEnabled1->set(1.0);

    auto pButtonSyncEnabled2 =
            std::make_unique<ControlProxy>(m_sGroup2, "sync_enabled");
    pButtonSyncEnabled2->set(1.0);

    // If sync is on and we load a track, that should initialize master.
    TrackPointer track1 = m_pMixerDeck1->loadFakeTrack(false, 140.0);

    EXPECT_DOUBLE_EQ(
            140.0, ControlObject::get(ConfigKey(m_sInternalClockGroup, "bpm")));
    EXPECT_DOUBLE_EQ(140.0, ControlObject::get(ConfigKey(m_sGroup1, "bpm")));

    // If sync is on two decks and we load a track while one is playing,
    // that should not change the playing deck.
    ControlObject::set(ConfigKey(m_sGroup1, "play"), 1.0);

    TrackPointer track2 = m_pMixerDeck2->loadFakeTrack(false, 128.0);

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
    ControlObject::getControl(ConfigKey(m_sGroup1, "play"))->set(1.0);
    track2 = m_pMixerDeck2->loadFakeTrack(false, 128.0);
    EXPECT_DOUBLE_EQ(
            140.0, ControlObject::get(ConfigKey(m_sInternalClockGroup, "bpm")));
    EXPECT_DOUBLE_EQ(140.0, ControlObject::get(ConfigKey(m_sGroup1, "bpm")));
    EXPECT_DOUBLE_EQ(140.0, ControlObject::get(ConfigKey(m_sGroup2, "bpm")));

    // Repeat with RESET_NONE
    m_pConfig->set(ConfigKey("[Controls]", "SpeedAutoReset"),
            ConfigValue(BaseTrackPlayer::RESET_NONE));
    ControlObject::set(ConfigKey(m_sGroup1, "play"), 0.0);
    ControlObject::set(ConfigKey(m_sGroup1, "rate"), getRateSliderValue(1.0));
    ControlObject::set(ConfigKey(m_sGroup2, "rate"), getRateSliderValue(1.0));
    track1 = m_pMixerDeck1->loadFakeTrack(false, 140.0);
    ControlObject::set(ConfigKey(m_sGroup1, "play"), 1.0);
    track2 = m_pMixerDeck2->loadFakeTrack(false, 128.0);
    EXPECT_DOUBLE_EQ(
            128.0, ControlObject::get(ConfigKey(m_sInternalClockGroup, "bpm")));
    EXPECT_DOUBLE_EQ(128.0, ControlObject::get(ConfigKey(m_sGroup1, "bpm")));
    EXPECT_DOUBLE_EQ(128.0, ControlObject::get(ConfigKey(m_sGroup2, "bpm")));

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
    ControlObject::set(ConfigKey(m_sGroup1, "play"), 1.0);
    track2 = m_pMixerDeck2->loadFakeTrack(false, 128.0);
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
    pButtonSyncEnabled1->slotSet(0.0);
    pButtonSyncEnabled2->slotSet(0.0);
    track1 = m_pMixerDeck1->loadFakeTrack(false, 140.0);
    ControlObject::getControl(ConfigKey(m_sGroup1, "play"))->set(1.0);
    track2 = m_pMixerDeck2->loadFakeTrack(false, 128.0);
    EXPECT_DOUBLE_EQ(140.0,
            ControlObject::getControl(ConfigKey(m_sGroup1, "bpm"))->get());
    EXPECT_DOUBLE_EQ(128.0,
            ControlObject::getControl(ConfigKey(m_sGroup2, "bpm"))->get());
}

TEST_F(EngineSyncTest, EnableOneDeckSliderUpdates) {
    // If we enable a deck to be master, the internal slider should immediately update.
    auto pButtonSyncEnabled1 =
            std::make_unique<ControlProxy>(m_sGroup1, "sync_enabled");

    mixxx::BeatsPointer pBeats1 = BeatFactory::makeBeatGrid(*m_pTrack1, 130, 0.0);
    m_pTrack1->setBeats(pBeats1);
    ControlObject::getControl(ConfigKey(m_sGroup1, "rate"))
            ->set(getRateSliderValue(1.0));

    // Set the deck to sync enabled.
    pButtonSyncEnabled1->slotSet(1.0);
    ProcessBuffer();

    // Group 1 should now be master (only one sync deck).
    ASSERT_TRUE(isSoftMaster(m_sGroup1));

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

    mixxx::BeatsPointer pBeats1 = BeatFactory::makeBeatGrid(*m_pTrack1, 130, 0.0);
    m_pTrack1->setBeats(pBeats1);
    ProcessBuffer();
    ControlObject::set(ConfigKey(m_sGroup1, "rate"), getRateSliderValue(1.0));
    mixxx::BeatsPointer pBeats2 = BeatFactory::makeBeatGrid(*m_pTrack2, 100, 0.0);
    m_pTrack2->setBeats(pBeats2);
    ControlObject::getControl(ConfigKey(m_sGroup2, "rate"))
            ->set(getRateSliderValue(1.0));

    ControlObject::set(ConfigKey(m_sGroup1, "play"), 1.0);
    ControlObject::set(ConfigKey(m_sGroup2, "play"), 1.0);

    pButtonSyncEnabled2->set(1.0);
    pButtonSyncEnabled2->set(0.0);
    ProcessBuffer();

    // There should be no master, and deck2 should match rate of deck1.  Sync slider should be
    // updated with the value, however.
    assertNoMaster();
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

    // There should be no master, and deck2 should match rate of deck1.
    EXPECT_EQ(0,
            ControlObject::get(
                    ConfigKey(m_sInternalClockGroup, "sync_master")));
    EXPECT_DOUBLE_EQ(
            100.0, ControlObject::get(ConfigKey(m_sInternalClockGroup, "bpm")));
    EXPECT_EQ(NULL, m_pEngineSync->getMaster());
    EXPECT_EQ(NULL, m_pEngineSync->getMasterSyncable());
    EXPECT_EQ(SYNC_NONE, ControlObject::get(ConfigKey(m_sGroup1, "sync_mode")));
    EXPECT_EQ(0, ControlObject::get(ConfigKey(m_sGroup1, "sync_enabled")));
    EXPECT_EQ(0, ControlObject::get(ConfigKey(m_sGroup1, "sync_master")));
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

    // There should be no master, and deck2 should match rate of deck1.
    EXPECT_EQ(0,
            ControlObject::get(
                    ConfigKey(m_sInternalClockGroup, "sync_master")));
    EXPECT_DOUBLE_EQ(
            100.0, ControlObject::get(ConfigKey(m_sInternalClockGroup, "bpm")));
    EXPECT_EQ(NULL, m_pEngineSync->getMaster());
    EXPECT_EQ(NULL, m_pEngineSync->getMasterSyncable());
    EXPECT_EQ(SYNC_NONE, ControlObject::get(ConfigKey(m_sGroup2, "sync_mode")));
    EXPECT_EQ(0, ControlObject::get(ConfigKey(m_sGroup2, "sync_enabled")));
    EXPECT_EQ(0, ControlObject::get(ConfigKey(m_sGroup2, "sync_master")));
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
    mixxx::BeatsPointer pBeats1 = BeatFactory::makeBeatGrid(*m_pTrack1, 100, 0.0);
    m_pTrack1->setBeats(pBeats1);
    ControlObject::set(ConfigKey(m_sGroup1, "rate"), getRateSliderValue(1.0));
    ControlObject::set(ConfigKey(m_sGroup1, "play"), 1.0);
    mixxx::BeatsPointer pBeats2 = BeatFactory::makeBeatGrid(*m_pTrack2, 130, 0.0);
    m_pTrack2->setBeats(pBeats2);
    ControlObject::set(ConfigKey(m_sGroup2, "rate"), getRateSliderValue(1.0));
    ControlObject::set(ConfigKey(m_sGroup2, "play"), 1.0);
    ProcessBuffer();

    // Set channel 1 to be enabled momentarily.
    pButtonSyncEnabled1->set(1.0);
    pButtonSyncEnabled1->set(0.0);
    ProcessBuffer();

    // The master sync should still be off and the speed should match deck 2.
    assertNoMaster();
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
    assertNoMaster();
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
    assertNoMaster();
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
    assertNoMaster();
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
    auto pButtonEject1 = std::make_unique<ControlProxy>(m_sGroup1, "eject");

    mixxx::BeatsPointer pBeats1 = BeatFactory::makeBeatGrid(*m_pTrack1, 120, 0.0);
    m_pTrack1->setBeats(pBeats1);
    pButtonSyncEnabled1->set(1.0);
    ProcessBuffer();

    // m_sGroup1 takes over
    ASSERT_TRUE(isFollower(m_sInternalClockGroup));
    ASSERT_TRUE(isSoftMaster(m_sGroup1));
    assertSyncOff(m_sGroup2);

    pButtonEject1->set(1.0);
    // When an eject happens, the bpm gets set to zero.
    ProcessBuffer();

    EXPECT_DOUBLE_EQ(
            0.0, ControlObject::getControl(ConfigKey(m_sGroup1, "bpm"))->get());

    // No valid tempo available all are waiting as follower
    ASSERT_TRUE(isFollower(m_sInternalClockGroup));
    ASSERT_TRUE(isFollower(m_sGroup1));
    assertSyncOff(m_sGroup2);

    m_pMixerDeck1->loadFakeTrack(false, 128.0);
    EXPECT_DOUBLE_EQ(128.0,
            ControlObject::getControl(ConfigKey(m_sGroup1, "bpm"))->get());
    mixxx::BeatsPointer pBeats2 = BeatFactory::makeBeatGrid(*m_pTrack2, 135, 0.0);
    m_pTrack2->setBeats(pBeats2);
    pButtonSyncEnabled2->set(1.0);
    ProcessBuffer();

    ASSERT_TRUE(isSoftMaster(m_sInternalClockGroup));
    ASSERT_TRUE(isFollower(m_sGroup1));
    ASSERT_TRUE(isFollower(m_sGroup2));

    pButtonEject1->set(1.0);
    m_pTrack1->setBeats(mixxx::BeatsPointer());
    ProcessBuffer();

    ASSERT_TRUE(isFollower(m_sInternalClockGroup));
    ASSERT_TRUE(isFollower(m_sGroup1));
    ASSERT_TRUE(isSoftMaster(m_sGroup2));
}

TEST_F(EngineSyncTest, FileBpmChangesDontAffectMaster) {
    // If filebpm changes, don't treat it like a rate change.
    mixxx::BeatsPointer pBeats1 = BeatFactory::makeBeatGrid(*m_pTrack1, 100, 0.0);
    m_pTrack1->setBeats(pBeats1);
    auto pButtonSyncEnabled1 =
            std::make_unique<ControlProxy>(m_sGroup1, "sync_enabled");
    pButtonSyncEnabled1->set(1.0);
    ProcessBuffer();

    mixxx::BeatsPointer pBeats2 = BeatFactory::makeBeatGrid(*m_pTrack2, 120, 0.0);
    m_pTrack2->setBeats(pBeats2);
    auto pButtonSyncEnabled2 =
            std::make_unique<ControlProxy>(m_sGroup2, "sync_enabled");
    pButtonSyncEnabled2->set(1.0);
    ProcessBuffer();

    pBeats1 = BeatFactory::makeBeatGrid(*m_pTrack1, 160, 0.0);
    m_pTrack1->setBeats(pBeats1);
    EXPECT_DOUBLE_EQ(
            100.0, ControlObject::get(ConfigKey(m_sInternalClockGroup, "bpm")));
}

TEST_F(EngineSyncTest, ExplicitMasterPostProcessed) {
    // Regression test thanks to a bug.  Make sure that an explicit master
    // channel gets post-processed.
    auto pButtonMasterSync1 =
            std::make_unique<ControlProxy>(m_sGroup1, "sync_mode");
    pButtonMasterSync1->slotSet(SYNC_MASTER_EXPLICIT);
    mixxx::BeatsPointer pBeats1 = BeatFactory::makeBeatGrid(*m_pTrack1, 160, 0.0);
    m_pTrack1->setBeats(pBeats1);
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
    mixxx::BeatsPointer pBeats1 = BeatFactory::makeBeatGrid(*m_pTrack1, 0, 0.0);
    m_pTrack1->setBeats(pBeats1);
    auto pButtonSyncEnabled1 =
            std::make_unique<ControlProxy>(m_sGroup1, "sync_enabled");
    pButtonSyncEnabled1->set(1.0);
    ControlObject::getControl(ConfigKey(m_sGroup1, "rate"))
            ->set(getRateSliderValue(1.0));
    ProcessBuffer();

    mixxx::BeatsPointer pBeats2 = BeatFactory::makeBeatGrid(*m_pTrack2, 120, 0.0);
    m_pTrack2->setBeats(pBeats2);
    auto pButtonSyncEnabled2 =
            std::make_unique<ControlProxy>(m_sGroup2, "sync_enabled");
    pButtonSyncEnabled2->set(1.0);
    ProcessBuffer();

    ControlObject::getControl(ConfigKey(m_sGroup1, "rate"))
            ->set(getRateSliderValue(1.3));

    EXPECT_DOUBLE_EQ(getRateSliderValue(1.0),
            ControlObject::getControl(ConfigKey(m_sGroup2, "rate"))->get());

    // Also try with explicit master/follower setting
    pButtonSyncEnabled1->set(0.0);
    ControlObject::getControl(ConfigKey(m_sGroup1, "sync_mode"))
            ->set(SYNC_MASTER_EXPLICIT);

    ControlObject::getControl(ConfigKey(m_sGroup1, "rate"))
            ->set(getRateSliderValue(1.4));
    EXPECT_DOUBLE_EQ(getRateSliderValue(1.0),
            ControlObject::getControl(ConfigKey(m_sGroup2, "rate"))->get());

    pButtonSyncEnabled1->set(0.0);
    ControlObject::getControl(ConfigKey(m_sGroup1, "sync_mode"))
            ->set(SYNC_FOLLOWER);

    ControlObject::getControl(ConfigKey(m_sGroup1, "rate"))
            ->set(getRateSliderValue(0.9));
    EXPECT_DOUBLE_EQ(getRateSliderValue(1.0),
            ControlObject::getControl(ConfigKey(m_sGroup2, "rate"))->get());
}

TEST_F(EngineSyncTest, ZeroLatencyRateChangeNoQuant) {
    // Confirm that a rate change in an explicit master is instantly communicated
    // to followers.
    mixxx::BeatsPointer pBeats1 = BeatFactory::makeBeatGrid(*m_pTrack1, 128, 0.0);
    m_pTrack1->setBeats(pBeats1);
    mixxx::BeatsPointer pBeats2 = BeatFactory::makeBeatGrid(*m_pTrack2, 160, 0.0);
    m_pTrack2->setBeats(pBeats2);

    // Make Channel2 master to weed out any channel ordering issues.
    ControlObject::getControl(ConfigKey(m_sGroup2, "sync_mode"))
            ->set(SYNC_FOLLOWER);
    ControlObject::getControl(ConfigKey(m_sGroup1, "sync_mode"))
            ->set(SYNC_FOLLOWER);
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
        ASSERT_NEAR(
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
    // Confirm that a rate change in an explicit master is instantly communicated
    // to followers.
    mixxx::BeatsPointer pBeats1 = BeatFactory::makeBeatGrid(*m_pTrack1, 128, 0.0);
    m_pTrack1->setBeats(pBeats1);
    mixxx::BeatsPointer pBeats2 = BeatFactory::makeBeatGrid(*m_pTrack2, 160, 0.0);
    m_pTrack2->setBeats(pBeats2);

    ControlObject::getControl(ConfigKey(m_sGroup1, "quantize"))->set(1.0);
    ControlObject::getControl(ConfigKey(m_sGroup2, "quantize"))->set(1.0);

    // Make Channel2 master to weed out any channel ordering issues.
    ControlObject::getControl(ConfigKey(m_sGroup2, "sync_mode"))
            ->set(SYNC_FOLLOWER);
    ControlObject::getControl(ConfigKey(m_sGroup1, "sync_mode"))
            ->set(SYNC_FOLLOWER);
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
        ASSERT_NEAR(
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
    // Confirm that a rate change in an explicit master is instantly communicated
    // to followers.
    mixxx::BeatsPointer pBeats1 = BeatFactory::makeBeatGrid(*m_pTrack1, 128, 0.0);
    m_pTrack1->setBeats(pBeats1);
    mixxx::BeatsPointer pBeats2 = BeatFactory::makeBeatGrid(*m_pTrack2, 160, 0.0);
    m_pTrack2->setBeats(pBeats2);

    ControlObject::getControl(ConfigKey(m_sGroup2, "quantize"))->set(1.0);

    // Make Channel2 master to weed out any channel ordering issues.
    ControlObject::getControl(ConfigKey(m_sGroup2, "sync_mode"))
            ->set(SYNC_FOLLOWER);
    ControlObject::getControl(ConfigKey(m_sGroup1, "sync_mode"))
            ->set(SYNC_FOLLOWER);
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
        ASSERT_NEAR(
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

// In this test, we set play *first* and then turn on master sync.
// This exercises a slightly different ordering of signals that we
// need to check. The Sync feature is unfortunately brittle.
// This test exercises https://bugs.launchpad.net/mixxx/+bug/1884324
TEST_F(EngineSyncTest, ActivatingSyncDoesNotCauseDrifting) {
    mixxx::BeatsPointer pBeats1 = BeatFactory::makeBeatGrid(*m_pTrack1, 150, 0.0);
    m_pTrack1->setBeats(pBeats1);
    mixxx::BeatsPointer pBeats2 = BeatFactory::makeBeatGrid(*m_pTrack2, 150, 0.0);
    m_pTrack2->setBeats(pBeats2);

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

    // engage first sync-master
    ControlObject::getControl(ConfigKey(m_sGroup1, "sync_mode"))
            ->set(SYNC_FOLLOWER);

    // engage second Sync-master
    ControlObject::getControl(ConfigKey(m_sGroup2, "sync_mode"))
            ->set(SYNC_FOLLOWER);

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
    mixxx::BeatsPointer pBeats1 = BeatFactory::makeBeatGrid(*m_pTrack1, 70, 0.0);
    m_pTrack1->setBeats(pBeats1);
    mixxx::BeatsPointer pBeats2 = BeatFactory::makeBeatGrid(*m_pTrack2, 140, 0.0);
    m_pTrack2->setBeats(pBeats2);

    ControlObject::getControl(ConfigKey(m_sGroup1, "quantize"))->set(1.0);
    ControlObject::getControl(ConfigKey(m_sGroup2, "quantize"))->set(1.0);
    ControlObject::getControl(ConfigKey(m_sGroup2, "sync_mode"))
            ->set(SYNC_FOLLOWER);
    ControlObject::getControl(ConfigKey(m_sGroup1, "sync_mode"))
            ->set(SYNC_FOLLOWER);

    // Mixxx will choose the first playing deck to be master.  Let's start deck 2 first.
    ControlObject::getControl(ConfigKey(m_sGroup2, "play"))->set(1.0);
    ControlObject::getControl(ConfigKey(m_sGroup1, "play"))->set(1.0);
    ProcessBuffer();

    EXPECT_EQ(0.5,
            m_pChannel1->getEngineBuffer()
                    ->m_pSyncControl->m_masterBpmAdjustFactor);
    EXPECT_EQ(1.0,
            m_pChannel2->getEngineBuffer()
                    ->m_pSyncControl->m_masterBpmAdjustFactor);
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
            ->set(SYNC_NONE);
    ControlObject::getControl(ConfigKey(m_sGroup2, "sync_mode"))
            ->set(SYNC_NONE);

    EXPECT_EQ(1.0,
            m_pChannel1->getEngineBuffer()
                    ->m_pSyncControl->m_masterBpmAdjustFactor);
    EXPECT_EQ(1.0,
            m_pChannel2->getEngineBuffer()
                    ->m_pSyncControl->m_masterBpmAdjustFactor);

    ControlObject::getControl(ConfigKey(m_sGroup1, "sync_mode"))
            ->set(SYNC_FOLLOWER);
    ControlObject::getControl(ConfigKey(m_sGroup2, "sync_mode"))
            ->set(SYNC_FOLLOWER);
    ControlObject::getControl(ConfigKey(m_sGroup1, "rate"))
            ->set(getRateSliderValue(1.0));

    // Now start deck 1 first and check again.
    ControlObject::getControl(ConfigKey(m_sGroup1, "play"))->set(1.0);
    ControlObject::getControl(ConfigKey(m_sGroup2, "play"))->set(1.0);

    ProcessBuffer();

    EXPECT_EQ(1.0,
            m_pChannel1->getEngineBuffer()
                    ->m_pSyncControl->m_masterBpmAdjustFactor);
    EXPECT_EQ(2.0,
            m_pChannel2->getEngineBuffer()
                    ->m_pSyncControl->m_masterBpmAdjustFactor);

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
    mixxx::BeatsPointer pBeats1 = BeatFactory::makeBeatGrid(*m_pTrack1, 80, 0.0);
    m_pTrack1->setBeats(pBeats1);
    mixxx::BeatsPointer pBeats2 = BeatFactory::makeBeatGrid(*m_pTrack2, 175, 0.0);
    m_pTrack2->setBeats(pBeats2);
    ControlObject::getControl(ConfigKey(m_sGroup1, "rate"))
            ->set(getRateSliderValue(1.0));

    auto pButtonSyncEnabled1 =
            std::make_unique<ControlProxy>(m_sGroup1, "sync_enabled");
    pButtonSyncEnabled1->slotSet(1.0);
    auto pButtonSyncEnabled2 =
            std::make_unique<ControlProxy>(m_sGroup2, "sync_enabled");
    pButtonSyncEnabled2->slotSet(1.0);
    ControlObject::getControl(ConfigKey(m_sGroup1, "quantize"))->set(1.0);
    ControlObject::getControl(ConfigKey(m_sGroup2, "quantize"))->set(1.0);

    // We Expect that m_sGroup1 has adjusted its own bpm to the second deck and becomes a single master.
    // When the second deck is synced the master bpm is adopted by the interna clock, which becomes now
    // the master
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
    EXPECT_DOUBLE_EQ(1,
            ControlObject::getControl(ConfigKey(m_sGroup2, "rate_ratio"))
                    ->get());
    EXPECT_DOUBLE_EQ(80,
            ControlObject::getControl(ConfigKey(m_sGroup1, "local_bpm"))
                    ->get());
    EXPECT_DOUBLE_EQ(175.0,
            ControlObject::getControl(ConfigKey(m_sGroup2, "local_bpm"))
                    ->get());

    ControlObject::getControl(ConfigKey(m_sGroup2, "play"))->set(1.0);
    ControlObject::getControl(ConfigKey(m_sGroup1, "play"))->set(1.0);

    EXPECT_DOUBLE_EQ(175.0,
            ControlObject::getControl(ConfigKey(m_sInternalClockGroup, "bpm"))
                    ->get());

    ProcessBuffer();
    ProcessBuffer();
    ProcessBuffer();

    EXPECT_DOUBLE_EQ(
            m_pChannel1->getEngineBuffer()->m_pSyncControl->getBeatDistance(),
            m_pChannel2->getEngineBuffer()->m_pSyncControl->getBeatDistance());

    // Now enable the other deck first.
    // Sync only cares about which deck plays first now, enable order is irrelevant.
    ControlObject::getControl(ConfigKey(m_sGroup1, "play"))->set(0.0);
    ControlObject::getControl(ConfigKey(m_sGroup2, "play"))->set(0.0);
    pButtonSyncEnabled1->slotSet(0.0);
    pButtonSyncEnabled2->slotSet(0.0);
    ProcessBuffer();
    pButtonSyncEnabled2->slotSet(1.0);
    pButtonSyncEnabled1->slotSet(1.0);
    ControlObject::getControl(ConfigKey(m_sGroup1, "play"))->set(1.0);
    ControlObject::getControl(ConfigKey(m_sGroup2, "play"))->set(1.0);

    EXPECT_DOUBLE_EQ(87.5 / 80,
            ControlObject::getControl(ConfigKey(m_sGroup1, "rate_ratio"))
                    ->get());
    EXPECT_DOUBLE_EQ(1,
            ControlObject::getControl(ConfigKey(m_sGroup2, "rate_ratio"))
                    ->get());

    ProcessBuffer();

    EXPECT_NEAR(
            m_pChannel1->getEngineBuffer()->m_pSyncControl->getBeatDistance(),
            m_pChannel2->getEngineBuffer()->m_pSyncControl->getBeatDistance(),
            kMaxFloatingPointErrorHighPrecision);

    ProcessBuffer();
    ProcessBuffer();

    EXPECT_DOUBLE_EQ(87.5,
            ControlObject::getControl(ConfigKey(m_sGroup1, "bpm"))->get());

    EXPECT_NEAR(
            m_pChannel1->getEngineBuffer()->m_pSyncControl->getBeatDistance(),
            m_pChannel2->getEngineBuffer()->m_pSyncControl->getBeatDistance(),
            kMaxFloatingPointErrorHighPrecision);

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
    mixxx::BeatsPointer pBeats1 = BeatFactory::makeBeatGrid(*m_pTrack1, 70, 0.0);
    m_pTrack1->setBeats(pBeats1);
    mixxx::BeatsPointer pBeats2 = BeatFactory::makeBeatGrid(*m_pTrack2, 140, 0.0);
    m_pTrack2->setBeats(pBeats2);

    ControlObject::getControl(ConfigKey(m_sGroup1, "quantize"))->set(1.0);
    ControlObject::getControl(ConfigKey(m_sGroup2, "quantize"))->set(1.0);
    // Make Channel2 master to weed out any channel ordering issues.
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
QVector<double> createBeatVector(
        double first_beat, unsigned int num_beats, double beat_length) {
    QVector<double> beats;
    for (unsigned int i = 0; i < num_beats; ++i) {
        beats.append(first_beat + i * beat_length);
    }
    return beats;
}
} // namespace

TEST_F(EngineSyncTest, HalfDoubleConsistency) {
    // half-double matching should be consistent
    double beatLengthFrames = 60.0 * 44100 / 90.0;
    double startOffsetFrames = 0;
    const int numBeats = 100;
    QVector<double> beats1 =
            createBeatVector(startOffsetFrames, numBeats, beatLengthFrames);
    auto pBeats1 = new mixxx::BeatMap(*m_pTrack1, 0, beats1);
    m_pTrack1->setBeats(mixxx::BeatsPointer(pBeats1));

    beatLengthFrames = 60.0 * 44100 / 145.0;
    QVector<double> beats2 =
            createBeatVector(startOffsetFrames, numBeats, beatLengthFrames);
    auto pBeats2 = new mixxx::BeatMap(*m_pTrack2, 0, beats2);
    m_pTrack2->setBeats(mixxx::BeatsPointer(pBeats2));

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

TEST_F(EngineSyncTest, SetFileBpmUpdatesLocalBpm) {
    ControlObject::getControl(ConfigKey(m_sGroup1, "beat_distance"))->set(0.2);
    mixxx::BeatsPointer pBeats1 = BeatFactory::makeBeatGrid(*m_pTrack1, 130, 0.0);
    m_pTrack1->setBeats(pBeats1);
    ASSERT_EQ(
            130.0, m_pEngineSync->getSyncableForGroup(m_sGroup1)->getBaseBpm());
}

TEST_F(EngineSyncTest, SyncPhaseToPlayingNonSyncDeck) {
    // If we press play on a sync deck, we will only sync phase to a non-sync
    // deck if there are no sync decks and the non-sync deck is playing.

    auto pButtonSyncEnabled1 =
            std::make_unique<ControlProxy>(m_sGroup1, "sync_enabled");
    mixxx::BeatsPointer pBeats1 = BeatFactory::makeBeatGrid(*m_pTrack1, 130, 0.0);
    m_pTrack1->setBeats(pBeats1);
    ControlObject::getControl(ConfigKey(m_sGroup1, "quantize"))->set(1.0);

    auto pButtonSyncEnabled2 =
            std::make_unique<ControlProxy>(m_sGroup2, "sync_enabled");
    ControlObject::set(ConfigKey(m_sGroup2, "rate_ratio"), 1.0);
    mixxx::BeatsPointer pBeats2 = BeatFactory::makeBeatGrid(*m_pTrack2, 100, 0.0);
    m_pTrack2->setBeats(pBeats2);

    // Set the sync deck playing with nothing else active.
    // Next Deck becomes master and the Master clock is set to 100 BPM
    // The 130 BPM Track should be played at 100 BPM, rate = 0,769230769
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

    // we expect that Deck 1 distance has not changed and the internal clock has continued
    // with the same rate
    EXPECT_NEAR(0.019349962,
            ControlObject::getControl(ConfigKey(m_sGroup1, "beat_distance"))->get(),
            kMaxFloatingPointErrorLowPrecision);
    EXPECT_NEAR(0.019349962,
            ControlObject::getControl(ConfigKey(m_sInternalClockGroup, "beat_distance"))->get(),
            kMaxFloatingPointErrorLowPrecision);

    // Now make the second deck playing and see if it works.
    ControlObject::getControl(ConfigKey(m_sGroup2, "play"))->set(1.0);
    ControlObject::getControl(ConfigKey(m_sGroup1, "play"))->set(1.0);

    // What seems to be happening is a seek is queued when a track is loaded, so all these tracks
    // seek to 0, thus resetting the beat distance.
    ProcessBuffer();

    // We expect that the second deck (master) has adjusted the "beat_distance" of the
    // internal clock and the fist deck is advanced slightly but slower to slowly get
    // rid of the additional buffer ahead
    // TODO: It does sounds odd to start the track 1 at a random position and adjust the
    // phase later. Seeking into phase is the best option even with quantize off.
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

    // But if there is a third deck that is sync-enabled, we match that.
    auto pButtonSyncEnabled3 =
            std::make_unique<ControlProxy>(m_sGroup3, "sync_enabled");
    ControlObject::set(ConfigKey(m_sGroup3, "beat_distance"), 0.6);
    ControlObject::set(ConfigKey(m_sGroup2, "rate_ratio"), 1.0);
    mixxx::BeatsPointer pBeats3 = BeatFactory::makeBeatGrid(*m_pTrack3, 140, 0.0);
    m_pTrack3->setBeats(pBeats3);
    // This will sync to the first deck here and not the second (lp1784185)
    pButtonSyncEnabled3->set(1.0);
    ProcessBuffer();
    EXPECT_DOUBLE_EQ(130.0, ControlObject::get(ConfigKey(m_sGroup3, "bpm")));
    // revert that
    ControlObject::set(ConfigKey(m_sGroup3, "rate_ratio"), 1.0);
    ProcessBuffer();
    EXPECT_DOUBLE_EQ(140.0, ControlObject::get(ConfigKey(m_sGroup3, "bpm")));
    // now we have Deck 3 with 140 bpm and sync enabled

    pButtonSyncEnabled1->set(1.0);
    ProcessBuffer();

    ControlObject::getControl(ConfigKey(m_sGroup3, "play"))->set(1.0);
    ControlObject::getControl(ConfigKey(m_sGroup2, "play"))->set(1.0);
    ControlObject::getControl(ConfigKey(m_sGroup1, "play"))->set(1.0);
    ProcessBuffer();

    // We expect Deck 1 is Deck 3 bpm
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
    // is used to reseed the master beat distance, make sure the user offset
    // is reset.
    mixxx::BeatsPointer pBeats1 = BeatFactory::makeBeatGrid(*m_pTrack1, 128, 0.0);
    m_pTrack1->setBeats(pBeats1);
    mixxx::BeatsPointer pBeats2 = BeatFactory::makeBeatGrid(*m_pTrack2, 128, 0.0);
    m_pTrack2->setBeats(pBeats2);

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

    // Stop the second deck.  This causes the master beat distance to get
    // seeded with the beat distance from deck 1.
    ControlObject::getControl(ConfigKey(m_sGroup2, "play"))->set(0.0);

    // Play a buffer, which is enough to see if the beat distances align.
    ProcessBuffer();

    EXPECT_NEAR(ControlObject::getControl(ConfigKey(m_sGroup1, "beat_distance"))
                        ->get(),
            ControlObject::getControl(
                    ConfigKey(m_sInternalClockGroup, "beat_distance"))
                    ->get(),
            .00001);

    EXPECT_DOUBLE_EQ(0.0,
            m_pChannel1->getEngineBuffer()
                    ->m_pBpmControl->m_dUserOffset.getValue());
}

TEST_F(EngineSyncTest, UserTweakPreservedInSeek) {
    // Ensure that when we do a seek during master sync, the user offset is maintained.

    // This is about 128 bpm, but results in nice round numbers of samples.
    const double kDivisibleBpm = 44100.0 / 344.0;
    mixxx::BeatsPointer pBeats1 = BeatFactory::makeBeatGrid(*m_pTrack1, kDivisibleBpm, 0.0);
    m_pTrack1->setBeats(pBeats1);
    mixxx::BeatsPointer pBeats2 = BeatFactory::makeBeatGrid(*m_pTrack2, 130, 0.0);
    m_pTrack2->setBeats(pBeats2);

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
    EXPECT_NEAR(0.2269025,
            ControlObject::get(ConfigKey(m_sGroup1, "playposition")),
            kMaxFloatingPointErrorLowPrecision);
    EXPECT_NEAR(0.1960644,
            ControlObject::get(ConfigKey(m_sGroup2, "playposition")),
            kMaxFloatingPointErrorLowPrecision);
    EXPECT_NEAR(0.12403101,
            ControlObject::get(ConfigKey(m_sGroup1, "beat_distance")),
            kMaxFloatingPointErrorLowPrecision);
    EXPECT_NEAR(0.12403101,
            ControlObject::get(ConfigKey(m_sGroup2, "beat_distance")),
            kMaxFloatingPointErrorLowPrecision);
}

TEST_F(EngineSyncTest, MasterBpmNeverZero) {
    mixxx::BeatsPointer pBeats1 = BeatFactory::makeBeatGrid(*m_pTrack1, 128, 0.0);
    m_pTrack1->setBeats(pBeats1);

    auto pButtonSyncEnabled1 = std::make_unique<ControlProxy>(m_sGroup1, "sync_enabled");
    pButtonSyncEnabled1->set(1.0);

    m_pTrack1->setBeats(mixxx::BeatsPointer());
    EXPECT_EQ(128.0,
              ControlObject::getControl(ConfigKey(m_sInternalClockGroup, "bpm"))->get());
}

TEST_F(EngineSyncTest, ZeroBpmNaturalRate) {
    // If a track has a zero bpm and a bad beatgrid, make sure the rate
    // doesn't end up something crazy when sync is enabled..
    // Maybe the beatgrid ended up at zero also.
    mixxx::BeatsPointer pBeats1 = BeatFactory::makeBeatGrid(*m_pTrack1, 0.0, 0.0);
    m_pTrack1->setBeats(pBeats1);

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

    mixxx::BeatsPointer pBeats1 = BeatFactory::makeBeatGrid(*m_pTrack1, 130, 0.0);
    m_pTrack1->setBeats(pBeats1);

    ControlObject::set(ConfigKey(m_sGroup2, "rate"), getRateSliderValue(1.0));
    mixxx::BeatsPointer pBeats2 = BeatFactory::makeBeatGrid(*m_pTrack2, 100, 0.0);
    m_pTrack2->setBeats(pBeats2);

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
    EXPECT_DOUBLE_EQ(
            ControlObject::get(ConfigKey(m_sGroup1, "beat_distance")) / 130 * 100,
            ControlObject::get(ConfigKey(m_sGroup2, "beat_distance")));
}

TEST_F(EngineSyncTest, SeekStayInPhase) {
    ControlObject::set(ConfigKey(m_sGroup1, "quantize"), 1.0);

    mixxx::BeatsPointer pBeats1 = BeatFactory::makeBeatGrid(*m_pTrack1, 130, 0.0);
    m_pTrack1->setBeats(pBeats1);

    ControlObject::set(ConfigKey(m_sGroup1, "play"), 1.0);
    ProcessBuffer();

    EXPECT_DOUBLE_EQ(0.025154950869236584, ControlObject::get(ConfigKey(m_sGroup1, "beat_distance")));
    EXPECT_DOUBLE_EQ(0.0023219954648526077, ControlObject::get(ConfigKey(m_sGroup1, "playposition")));

    ControlObject::set(ConfigKey(m_sGroup1, "playposition"), 0.2);
    ProcessBuffer();

    // We expect to be two buffers ahead in a beat near 0.2
    EXPECT_DOUBLE_EQ(0.050309901738473183, ControlObject::get(ConfigKey(m_sGroup1, "beat_distance")));
    EXPECT_DOUBLE_EQ(0.18925937554508981, ControlObject::get(ConfigKey(m_sGroup1, "playposition")));

    // The same again with a stopped track loaded in Channel 2
    ControlObject::set(ConfigKey(m_sGroup1, "playposition"), 0.0);
    ControlObject::set(ConfigKey(m_sGroup1, "play"), 0.0);
    ProcessBuffer();

    mixxx::BeatsPointer pBeats2 = BeatFactory::makeBeatGrid(*m_pTrack1, 130, 0.0);
    m_pTrack2->setBeats(pBeats2);

    ControlObject::set(ConfigKey(m_sGroup1, "play"), 1.0);
    ProcessBuffer();

    EXPECT_DOUBLE_EQ(0.025154950869236584,
            ControlObject::get(ConfigKey(m_sGroup1, "beat_distance")));
    EXPECT_DOUBLE_EQ(0.0023219954648526077,
            ControlObject::get(ConfigKey(m_sGroup1, "playposition")));

    ControlObject::set(ConfigKey(m_sGroup1, "playposition"), 0.2);
    ProcessBuffer();

    // We expect to be two buffers ahead in a beat near 0.2
    EXPECT_DOUBLE_EQ(0.050309901738473183,
            ControlObject::get(ConfigKey(m_sGroup1, "beat_distance")));
    EXPECT_DOUBLE_EQ(0.18925937554508981, ControlObject::get(ConfigKey(m_sGroup1, "playposition")));
}

TEST_F(EngineSyncTest, SyncWithoutBeatgrid) {
    // this tests bug lp1783020, notresetting rate when other deck has no beatgrid
    mixxx::BeatsPointer pBeats1 = BeatFactory::makeBeatGrid(*m_pTrack1, 128, 0.0);
    m_pTrack1->setBeats(pBeats1);
    m_pTrack2->setBeats(mixxx::BeatsPointer());

    ControlObject::set(ConfigKey(m_sGroup1, "rate"), 0.5);

    // Play a buffer, which is enough to see if the beat distances align.
    ProcessBuffer();

    ControlObject::set(ConfigKey(m_sGroup1, "sync_enabled"), 1);

    ProcessBuffer();

    ASSERT_DOUBLE_EQ(0.5,
              ControlObject::get(ConfigKey(m_sGroup1, "rate")));

}

TEST_F(EngineSyncTest, QuantizeHotCueActivate) {
    mixxx::BeatsPointer pBeats1 = BeatFactory::makeBeatGrid(*m_pTrack1, 130, 0.0);
    m_pTrack1->setBeats(pBeats1);

    auto pHotCue2Activate = std::make_unique<ControlProxy>(m_sGroup2, "hotcue_1_activate");
    mixxx::BeatsPointer pBeats2 = BeatFactory::makeBeatGrid(*m_pTrack2, 100, 0.0);

    m_pTrack2->setBeats(pBeats2);

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

    EXPECT_NEAR(
            ControlObject::get(ConfigKey(m_sGroup1, "beat_distance")) / 130 * 100,
            ControlObject::get(ConfigKey(m_sGroup2, "beat_distance")),
            1e-15);

    pHotCue2Activate->set(0.0);
    ProcessBuffer();

    // Expect that the track is back to 0:00
    EXPECT_DOUBLE_EQ(0, ControlObject::get(ConfigKey(m_sGroup2, "beat_distance")));

    // Expect that the first track has advanced 6 buffers in the mean time
    EXPECT_DOUBLE_EQ(0.15092970521541951, ControlObject::get(ConfigKey(m_sGroup1, "beat_distance")));
}

TEST_F(EngineSyncTest, ChangeBeatGrid) {
    // https://bugs.launchpad.net/mixxx/+bug/1808698

    auto pButtonSyncEnabled1 = std::make_unique<ControlProxy>(m_sGroup1, "sync_enabled");
    auto pButtonSyncEnabled2 = std::make_unique<ControlProxy>(m_sGroup2, "sync_enabled");

    // set beatgrid
    mixxx::BeatsPointer pBeats1 = BeatFactory::makeBeatGrid(*m_pTrack1, 130, 0.0);
    m_pTrack1->setBeats(pBeats1);
    pButtonSyncEnabled1->set(1.0);
    ControlObject::set(ConfigKey(m_sGroup1, "play"), 1.0);

    ProcessBuffer();

    ASSERT_TRUE(isSoftMaster(m_sGroup1));
    EXPECT_DOUBLE_EQ(130.0, ControlObject::get(ConfigKey(m_sGroup1, "bpm")));
    EXPECT_DOUBLE_EQ(130.0, ControlObject::get(ConfigKey(m_sInternalClockGroup, "bpm")));

    // sync 0 bpm track to the first one
    pButtonSyncEnabled2->set(1.0);
    ControlObject::set(ConfigKey(m_sGroup2, "play"), 1.0);

    ProcessBuffer();

    // expect no change in Deck 1
    ASSERT_TRUE(isSoftMaster(m_sGroup1));
    ASSERT_TRUE(isFollower(m_sGroup2));
    EXPECT_DOUBLE_EQ(130.0, ControlObject::get(ConfigKey(m_sGroup1, "bpm")));
    EXPECT_DOUBLE_EQ(0, ControlObject::get(ConfigKey(m_sGroup2, "bpm")));
    EXPECT_DOUBLE_EQ(130.0, ControlObject::get(ConfigKey(m_sInternalClockGroup, "bpm")));

    ControlObject::set(ConfigKey(m_sGroup1, "play"), 0.0);

    ProcessBuffer();
    // Group1 remains master because it is the only one with a tempo,
    ASSERT_TRUE(isSoftMaster(m_sGroup1));
    ASSERT_TRUE(isFollower(m_sGroup2));

    // Load a new beatgrid during playing, this happens when the analyser is finished
    mixxx::BeatsPointer pBeats2 = BeatFactory::makeBeatGrid(*m_pTrack2, 140, 0.0);
    m_pTrack2->setBeats(pBeats2);

    ProcessBuffer();

    // we expect that the new beatgrid is aligned to the other playing track
    ASSERT_TRUE(isSoftMaster(m_sGroup2));
    ASSERT_TRUE(isFollower(m_sGroup1));
    ASSERT_TRUE(isFollower(m_sInternalClockGroup));
    EXPECT_DOUBLE_EQ(130.0, ControlObject::get(ConfigKey(m_sGroup1, "bpm")));
    EXPECT_DOUBLE_EQ(130.0, ControlObject::get(ConfigKey(m_sGroup2, "bpm")));
    EXPECT_DOUBLE_EQ(130.0, ControlObject::get(ConfigKey(m_sInternalClockGroup, "bpm")));

    // Play Both Tracks.
    ControlObject::set(ConfigKey(m_sGroup1, "play"), 1.0);
    ProcessBuffer();

    ASSERT_TRUE(isSoftMaster(m_sInternalClockGroup));
    ASSERT_TRUE(isFollower(m_sGroup1));
    ASSERT_TRUE(isFollower(m_sGroup2));

    // Load a new beatgrid again, this happens when the user adjusts the beatgrid
    mixxx::BeatsPointer pBeats2n = BeatFactory::makeBeatGrid(*m_pTrack2, 75, 0.0);
    m_pTrack2->setBeats(pBeats2n);

    ProcessBuffer();

    // we expect that the new beatgrid is aligned to the other playing track
    // Not the case before fixing lp1808698
    EXPECT_DOUBLE_EQ(130.0, ControlObject::get(ConfigKey(m_sGroup1, "bpm")));
    // Expect to sync on half beats
    EXPECT_DOUBLE_EQ(65.0, ControlObject::get(ConfigKey(m_sGroup2, "bpm")));
    EXPECT_DOUBLE_EQ(130.0, ControlObject::get(ConfigKey(m_sInternalClockGroup, "bpm")));
}

TEST_F(EngineSyncTest, BeatMapQantizePlay) {
    // This test demonstates https://bugs.launchpad.net/mixxx/+bug/1874918
    mixxx::BeatsPointer pBeats1 = BeatFactory::makeBeatGrid(*m_pTrack1, 120, 0.0);
    m_pTrack1->setBeats(pBeats1);

    mixxx::BeatsPointer pBeats2 = mixxx::BeatsPointer(new mixxx::BeatMap(*m_pTrack2, 44100));
    // Add two beats at 120 Bpm
    pBeats2->addBeat(44100 / 2);
    pBeats2->addBeat(44100);
    m_pTrack2->setBeats(pBeats2);

    ControlObject::set(ConfigKey(m_sGroup1, "quantize"), 1.0);
    ControlObject::set(ConfigKey(m_sGroup2, "quantize"), 1.0);

    ControlObject::getControl(ConfigKey(m_sGroup1, "sync_mode"))->set(SYNC_MASTER_EXPLICIT);
    ControlObject::getControl(ConfigKey(m_sGroup2, "sync_mode"))->set(SYNC_FOLLOWER);

    ControlObject::getControl(ConfigKey(m_sGroup1, "play"))->set(1.0);

    ProcessBuffer();

    ControlObject::getControl(ConfigKey(m_sGroup2, "play"))->set(1.0);

    ProcessBuffer();

    EXPECT_DOUBLE_EQ(m_pChannel1->getEngineBuffer()->m_pSyncControl->getBeatDistance(),
            m_pChannel2->getEngineBuffer()->m_pSyncControl->getBeatDistance());
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

    // group 2 should be synced to the first playing deck and becomes master
    EXPECT_DOUBLE_EQ(40.0, ControlObject::get(ConfigKey(m_sGroup1, "bpm")));
    EXPECT_DOUBLE_EQ(80.0, ControlObject::get(ConfigKey(m_sGroup2, "bpm")));
    ASSERT_TRUE(isSoftMaster(m_sGroup2));
    assertSyncOff(m_sGroup1);

    ControlObject::set(ConfigKey(m_sGroup2, "play"), 1.0);
    ProcessBuffer();
    ASSERT_TRUE(isSoftMaster(m_sGroup2));
    // Pretend a changing beatgrid
    static_cast<SyncControl*>(m_pEngineSync->getMasterSyncable())->setLocalBpm(152);
    ProcessBuffer();

    ControlObject::set(ConfigKey(m_sGroup1, "sync_enabled"), 1.0);
    ProcessBuffer();

    // Group 1 should be already in sync, it must not change due to sync
    // and Group 2 must also remain.
    EXPECT_DOUBLE_EQ(40.0, ControlObject::get(ConfigKey(m_sGroup1, "bpm")));
    EXPECT_DOUBLE_EQ(80.0, ControlObject::get(ConfigKey(m_sGroup2, "bpm")));
    ASSERT_TRUE(isFollower(m_sGroup1));
    ASSERT_TRUE(isFollower(m_sGroup2));
    ASSERT_TRUE(isSoftMaster(m_sInternalClockGroup));
}
