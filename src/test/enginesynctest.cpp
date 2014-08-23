// Tests for Master Sync.
// There are no tests for actual deck playback, since I don't know how to mock that out yet.
// The following manual tests should probably be performed:
// * Quantize mode nudges tracks in sync, whether internal or deck master.
// * Flinging tracks with the waveform should work.
// * vinyl??

#include <string>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <QtDebug>

#include "configobject.h"
#include "controlobject.h"
#include "test/mockedenginebackendtest.h"
#include "test/mixxxtest.h"


class EngineSyncTest : public MockedEngineBackendTest {
  public:
    std::string getMasterGroup() {
        Syncable* pMasterSyncable = m_pEngineSync->getMasterSyncable();
        if (pMasterSyncable) {
            return pMasterSyncable->getGroup().toStdString();
        }
        return "";
    }
    void assertIsMaster(QString group) {
        if (group == m_sInternalClockGroup){
            ASSERT_EQ(1,
                      ControlObject::getControl(ConfigKey(m_sInternalClockGroup,
                                                          "sync_master"))->get());
            ASSERT_EQ(NULL, m_pEngineSync->getMaster());
            ASSERT_EQ(m_sInternalClockGroup, getMasterGroup());
        } else {
            if (group == m_sGroup1) {
                ASSERT_EQ(m_pChannel1, m_pEngineSync->getMaster());
            } else if (group == m_sGroup2) {
                ASSERT_EQ(m_pChannel2, m_pEngineSync->getMaster());
            }
            ASSERT_EQ(group.toStdString(), getMasterGroup());
            ASSERT_EQ(SYNC_MASTER, ControlObject::getControl(ConfigKey(group, "sync_mode"))->get());
            ASSERT_EQ(1, ControlObject::getControl(ConfigKey(group, "sync_enabled"))->get());
            ASSERT_EQ(1, ControlObject::getControl(ConfigKey(group, "sync_master"))->get());
        }
    }

    void assertIsFollower(QString group) {
        ASSERT_EQ(SYNC_FOLLOWER, ControlObject::getControl(ConfigKey(group, "sync_mode"))->get());
        ASSERT_EQ(1, ControlObject::getControl(ConfigKey(group, "sync_enabled"))->get());
        ASSERT_EQ(0, ControlObject::getControl(ConfigKey(group, "sync_master"))->get());
    }

    void assertSyncOff(QString group) {
        if (group == m_sInternalClockGroup) {
            ASSERT_EQ(0,
                      ControlObject::getControl(ConfigKey(m_sInternalClockGroup,
                                                          "sync_master"))->get());
        } else {
            ASSERT_EQ(SYNC_NONE, ControlObject::getControl(ConfigKey(group, "sync_mode"))->get());
            ASSERT_EQ(0, ControlObject::getControl(ConfigKey(group, "sync_enabled"))->get());
            ASSERT_EQ(0, ControlObject::getControl(ConfigKey(group, "sync_master"))->get());
        }
    }

    void assertNoMaster() {
        ASSERT_EQ(NULL, m_pEngineSync->getMaster());
        ASSERT_EQ(NULL, m_pEngineSync->getMasterSyncable());
    }
};

TEST_F(EngineSyncTest, ControlObjectsExist) {
    // This isn't exhaustive, but certain COs have a habit of not being set up properly.
    ASSERT_TRUE(ControlObject::getControl(ConfigKey(m_sGroup1, "file_bpm")) != NULL);
}

TEST_F(EngineSyncTest, SetMasterSuccess) {
    // If we set the first channel to master, EngineSync should get that message.

    // Throughout these tests we use ControlObjectThreads so that we can trigger ValueChanged,
    // and not just ValueChangedFromEngine.
    QScopedPointer<ControlObjectThread> pButtonMasterSync1(getControlObjectThread(
            ConfigKey(m_sGroup1, "sync_mode")));
    pButtonMasterSync1->slotSet(SYNC_MASTER);
    ProcessBuffer();

    // The master sync should now be channel 1.
    assertIsMaster(m_sGroup1);

    QScopedPointer<ControlObjectThread> pButtonMasterSync2(getControlObjectThread(
            ConfigKey(m_sGroup2, "sync_mode")));
    pButtonMasterSync2->slotSet(SYNC_FOLLOWER);
    ProcessBuffer();

    assertIsFollower(m_sGroup2);

    // Now set channel 2 to be master.
    pButtonMasterSync2->slotSet(SYNC_MASTER);
    ProcessBuffer();

    // Now channel 2 should be master, and channel 1 should be a slave.
    assertIsMaster(m_sGroup2);
    assertIsFollower(m_sGroup1);

    // Now back again.
    pButtonMasterSync1->slotSet(SYNC_MASTER);
    ProcessBuffer();

    // Now channel 1 should be master, and channel 2 should be a slave.
    assertIsMaster(m_sGroup1);
    assertIsFollower(m_sGroup2);

    // Now set channel 1 to slave, internal will be master because no track loaded.
    pButtonMasterSync1->slotSet(SYNC_FOLLOWER);
    ProcessBuffer();

    assertIsMaster(m_sInternalClockGroup);
    assertIsFollower(m_sGroup1);
    assertIsFollower(m_sGroup2);
}

TEST_F(EngineSyncTest, SetMasterWhilePlaying) {
    // Make sure we don't get two master lights if we change masters while playing.

    QScopedPointer<ControlObjectThread> pFileBpm1(getControlObjectThread(
        ConfigKey(m_sGroup1, "file_bpm")));
    QScopedPointer<ControlObjectThread> pFileBpm2(getControlObjectThread(
        ConfigKey(m_sGroup2, "file_bpm")));
    QScopedPointer<ControlObjectThread> pFileBpm3(getControlObjectThread(
        ConfigKey(m_sGroup3, "file_bpm")));

    pFileBpm1->set(120.0);
    pFileBpm2->set(124.0);
    pFileBpm3->set(128.0);

    QScopedPointer<ControlObjectThread> pButtonMasterSync1(getControlObjectThread(
            ConfigKey(m_sGroup1, "sync_mode")));
    pButtonMasterSync1->slotSet(SYNC_MASTER);
    QScopedPointer<ControlObjectThread> pButtonMasterSync2(getControlObjectThread(
            ConfigKey(m_sGroup2, "sync_mode")));
    pButtonMasterSync2->slotSet(SYNC_FOLLOWER);
    QScopedPointer<ControlObjectThread> pButtonMasterSync3(getControlObjectThread(
            ConfigKey(m_sGroup3, "sync_mode")));
    pButtonMasterSync3->slotSet(SYNC_FOLLOWER);

    ControlObject::getControl(ConfigKey(m_sGroup1, "play"))->set(1.0);
    ControlObject::getControl(ConfigKey(m_sGroup2, "play"))->set(1.0);
    ControlObject::getControl(ConfigKey(m_sGroup3, "play"))->set(1.0);

    ProcessBuffer();

    pButtonMasterSync3->slotSet(SYNC_MASTER);

    ProcessBuffer();

    assertIsFollower(m_sGroup1);
    assertIsFollower(m_sGroup2);
    assertIsMaster(m_sGroup3);
}

TEST_F(EngineSyncTest, SetEnabledBecomesMaster) {
    // If we set the first channel to slave, it should be master.
    QScopedPointer<ControlObjectThread> pButtonMasterSync1(getControlObjectThread(
            ConfigKey(m_sGroup1, "sync_mode")));
    pButtonMasterSync1->slotSet(SYNC_FOLLOWER);
    ProcessBuffer();

    // The master sync should now be internal.
    assertIsMaster(m_sInternalClockGroup);
}

TEST_F(EngineSyncTest, DisableInternalMasterWhilePlaying) {
    QScopedPointer<ControlObjectThread> pButtonMasterSync(getControlObjectThread(
            ConfigKey(m_sInternalClockGroup, "sync_master")));
    pButtonMasterSync->slotSet(1.0);
    QScopedPointer<ControlObjectThread> pButtonSyncMode1(getControlObjectThread(
            ConfigKey(m_sGroup1, "sync_mode")));
    pButtonSyncMode1->slotSet(SYNC_FOLLOWER);
    ProcessBuffer();

    // The master sync should now be Internal.
    assertIsMaster(m_sInternalClockGroup);

    // Make sure deck 1 is playing.
    QScopedPointer<ControlObjectThread> pFileBpm1(getControlObjectThread(
        ConfigKey(m_sGroup1, "file_bpm")));
    pFileBpm1->set(80.0);
    ControlObject::getControl(ConfigKey(m_sGroup1, "play"))->set(1.0);
    ProcessBuffer();

    // Now unset Internal master.
    pButtonMasterSync->slotSet(0.0);
    ProcessBuffer();

    // This is not allowed, Internal should still be master.
    assertIsMaster(m_sInternalClockGroup);
    ASSERT_EQ(1, pButtonMasterSync->get());
}

TEST_F(EngineSyncTest, DisableSyncOnMaster) {
    // Channel 1 follower, channel 2 master.
    QScopedPointer<ControlObjectThread> pButtonSyncMode1(getControlObjectThread(
            ConfigKey(m_sGroup1, "sync_mode")));
    pButtonSyncMode1->slotSet(SYNC_FOLLOWER);

    QScopedPointer<ControlObjectThread> pButtonSyncMaster2(getControlObjectThread(
            ConfigKey(m_sGroup2, "sync_master")));
    pButtonSyncMaster2->slotSet(1.0);

    assertIsFollower(m_sGroup1);
    assertIsMaster(m_sGroup2);

    // Unset enabled on channel2, it should work.
    QScopedPointer<ControlObjectThread> pButtonSyncEnabled2(getControlObjectThread(
            ConfigKey(m_sGroup2, "sync_enabled")));
    pButtonSyncEnabled2->slotSet(0.0);

    assertIsFollower(m_sGroup1);
    ASSERT_EQ(0, ControlObject::getControl(ConfigKey(m_sGroup2, "sync_enabled"))->get());
    ASSERT_EQ(0, ControlObject::getControl(ConfigKey(m_sGroup2, "sync_master"))->get());
}

TEST_F(EngineSyncTest, InternalMasterSetSlaveSliderMoves) {
    // If internal is master, and we turn on a slave, the slider should move.
    QScopedPointer<ControlObjectThread> pButtonMasterSyncInternal(getControlObjectThread(
            ConfigKey(m_sInternalClockGroup, "sync_master")));
    pButtonMasterSyncInternal->slotSet(1);
    QScopedPointer<ControlObjectThread> pMasterSyncSlider(getControlObjectThread(
            ConfigKey(m_sInternalClockGroup, "bpm")));
    pMasterSyncSlider->set(100.0);

    // Set the file bpm of channel 1 to 160bpm.
    QScopedPointer<ControlObjectThread> pFileBpm1(getControlObjectThread(
        ConfigKey(m_sGroup1, "file_bpm")));
    pFileBpm1->set(80.0);

    QScopedPointer<ControlObjectThread> pButtonMasterSync1(getControlObjectThread(
            ConfigKey(m_sGroup1, "sync_mode")));
    pButtonMasterSync1->slotSet(SYNC_FOLLOWER);
    ProcessBuffer();

    ASSERT_FLOAT_EQ(getRateSliderValue(1.25),
                    ControlObject::getControl(ConfigKey(m_sGroup1, "rate"))->get());
    ASSERT_FLOAT_EQ(100.0, ControlObject::getControl(ConfigKey(m_sGroup1, "bpm"))->get());
}

TEST_F(EngineSyncTest, AnySyncDeckSliderStays) {
    // If there exists a sync deck, even if it's not playing, don't change the
    // master BPM if a new deck enables sync.

    QScopedPointer<ControlObjectThread> pFileBpm1(getControlObjectThread(
        ConfigKey(m_sGroup1, "file_bpm")));
    pFileBpm1->set(80.0);
    QScopedPointer<ControlObjectThread> pButtonSyncEnabled1(getControlObjectThread(
            ConfigKey(m_sGroup1, "sync_enabled")));
    pButtonSyncEnabled1->set(1.0);

    // After setting up the first deck, the internal BPM should be 80.
    ASSERT_FLOAT_EQ(80.0,
                    ControlObject::getControl(ConfigKey(m_sInternalClockGroup, "bpm"))->get());

    QScopedPointer<ControlObjectThread> pFileBpm2(getControlObjectThread(
        ConfigKey(m_sGroup2, "file_bpm")));
    pFileBpm2->set(100.0);
    QScopedPointer<ControlObjectThread> pButtonSyncEnabled2(getControlObjectThread(
            ConfigKey(m_sGroup2, "sync_enabled")));
    pButtonSyncEnabled2->set(1.0);

    // After the second one, though, the internal BPM should still be 80.
    ASSERT_FLOAT_EQ(80.0,
                    ControlObject::getControl(ConfigKey(m_sInternalClockGroup, "bpm"))->get());
}

TEST_F(EngineSyncTest, InternalClockFollowsFirstPlayingDeck) {
    // Same as above, except we use the midi lights to change state.
    QScopedPointer<ControlObjectThread> pButtonMasterSync1(getControlObjectThread(
            ConfigKey(m_sGroup1, "sync_mode")));
    QScopedPointer<ControlObjectThread> pButtonMasterSync2(getControlObjectThread(
            ConfigKey(m_sGroup2, "sync_mode")));
    QScopedPointer<ControlObjectThread> pButtonSyncEnabled1(getControlObjectThread(
            ConfigKey(m_sGroup1, "sync_enabled")));
    QScopedPointer<ControlObjectThread> pButtonSyncEnabled2(getControlObjectThread(
            ConfigKey(m_sGroup2, "sync_enabled")));
    QScopedPointer<ControlObjectThread> pButtonSyncMaster1(getControlObjectThread(
            ConfigKey(m_sGroup1, "sync_master")));
    QScopedPointer<ControlObjectThread> pButtonSyncMaster2(getControlObjectThread(
            ConfigKey(m_sGroup2, "sync_master")));

    // Set up decks so they can be playing, and start deck 1.
    QScopedPointer<ControlObjectThread> pFileBpm1(getControlObjectThread(
        ConfigKey(m_sGroup1, "file_bpm")));
    pFileBpm1->set(100.0);
    ControlObject::getControl(ConfigKey(m_sGroup1, "rate"))->set(getRateSliderValue(1.0));
    ControlObject::getControl(ConfigKey(m_sGroup1, "play"))->set(1.0);
    QScopedPointer<ControlObjectThread> pFileBpm2(getControlObjectThread(
        ConfigKey(m_sGroup2, "file_bpm")));
    pFileBpm2->set(130.0);
    ControlObject::getControl(ConfigKey(m_sGroup2, "rate"))->set(getRateSliderValue(1.0));
    ControlObject::getControl(ConfigKey(m_sGroup2, "play"))->set(0.0);
    ProcessBuffer();

    // Set channel 1 to be enabled
    pButtonSyncEnabled1->slotSet(1.0);
    ProcessBuffer();

    // The master sync should now be internal and the speed should match deck 2.
    assertIsMaster(m_sInternalClockGroup);
    assertIsFollower(m_sGroup1);
    ASSERT_FLOAT_EQ(130.0,
                    ControlObject::getControl(ConfigKey(m_sInternalClockGroup, "bpm"))->get());

    // Set channel 2 to be enabled.
    pButtonSyncEnabled2->slotSet(1);
    ProcessBuffer();

    assertIsFollower(m_sGroup2);

    // Internal should still be master.
    assertIsMaster(m_sInternalClockGroup);

    // The rate should not have changed -- deck 1 still matches deck 2.
    ASSERT_FLOAT_EQ(getRateSliderValue(1.3),
                    ControlObject::getControl(ConfigKey(m_sGroup1, "rate"))->get());

    // Reset channel 2 rate, set channel 2 to play, and process a buffer.
    ControlObject::getControl(ConfigKey(m_sGroup2, "rate"))->set(getRateSliderValue(1.0));
    ControlObject::getControl(ConfigKey(m_sGroup2, "play"))->set(1.0);
    ProcessBuffer();

    // Keep double checking these.
    assertIsMaster(m_sInternalClockGroup);
    assertIsFollower(m_sGroup1);
    assertIsFollower(m_sGroup2);

    // Now disable sync on channel 1.
    pButtonSyncEnabled1->slotSet(0);
    ProcessBuffer();

    // Rate should now match channel 2.
    ASSERT_FLOAT_EQ(130.0,
                    ControlObject::getControl(ConfigKey(m_sInternalClockGroup, "bpm"))->get());
}


TEST_F(EngineSyncTest, SetExplicitMasterByLights) {
    // Same as above, except we use the midi lights to change state.
    QScopedPointer<ControlObjectThread> pButtonMasterSync1(getControlObjectThread(
            ConfigKey(m_sGroup1, "sync_mode")));
    QScopedPointer<ControlObjectThread> pButtonMasterSync2(getControlObjectThread(
            ConfigKey(m_sGroup2, "sync_mode")));
    QScopedPointer<ControlObjectThread> pButtonSyncEnabled1(getControlObjectThread(
            ConfigKey(m_sGroup1, "sync_enabled")));
    QScopedPointer<ControlObjectThread> pButtonSyncEnabled2(getControlObjectThread(
            ConfigKey(m_sGroup2, "sync_enabled")));
    QScopedPointer<ControlObjectThread> pButtonSyncMaster1(getControlObjectThread(
            ConfigKey(m_sGroup1, "sync_master")));
    QScopedPointer<ControlObjectThread> pButtonSyncMaster2(getControlObjectThread(
            ConfigKey(m_sGroup2, "sync_master")));

    // Set channel 1 to be master.
    pButtonSyncMaster1->slotSet(1.0);
    ProcessBuffer();

    // The master sync should now be channel 1.
    assertIsMaster(m_sGroup1);

    // Set channel 2 to be slave.
    pButtonSyncEnabled2->slotSet(1);

    assertIsFollower(m_sGroup2);

    // Now set channel 2 to be master.
    pButtonSyncMaster2->slotSet(1);

    // Now channel 2 should be master, and channel 1 should be a slave.
    assertIsFollower(m_sGroup1);
    assertIsMaster(m_sGroup2);

    // Now back again.
    pButtonSyncMaster1->slotSet(1);

    // Now channel 1 should be master, and channel 2 should be a slave.
    assertIsMaster(m_sGroup1);
    assertIsFollower(m_sGroup2);

    // Now set channel 1 to not-master, internal will be master.
    pButtonSyncMaster1->slotSet(0);

    assertIsMaster(m_sInternalClockGroup);
    assertIsFollower(m_sGroup1);
    assertIsFollower(m_sGroup2);
}

TEST_F(EngineSyncTest, RateChangeTest) {
    QScopedPointer<ControlObjectThread> pButtonMasterSync1(getControlObjectThread(
            ConfigKey(m_sGroup1, "sync_mode")));
    pButtonMasterSync1->slotSet(SYNC_MASTER);
    QScopedPointer<ControlObjectThread> pButtonMasterSync2(getControlObjectThread(
            ConfigKey(m_sGroup2, "sync_mode")));
    pButtonMasterSync2->slotSet(SYNC_FOLLOWER);
    ProcessBuffer();

    // Set the file bpm of channel 1 to 160bpm.
    QScopedPointer<ControlObjectThread> pFileBpm1(getControlObjectThread(
        ConfigKey(m_sGroup1, "file_bpm")));
    pFileBpm1->set(160.0);
    ASSERT_FLOAT_EQ(160.0, ControlObject::getControl(ConfigKey(m_sGroup1, "file_bpm"))->get());
    ASSERT_FLOAT_EQ(160.0, ControlObject::getControl(ConfigKey(m_sInternalClockGroup, "bpm"))->get());

    // Set the rate of channel 1 to 1.2.
    ControlObject::getControl(ConfigKey(m_sGroup1, "rate"))->set(getRateSliderValue(1.2));
    ASSERT_FLOAT_EQ(getRateSliderValue(1.2),
                    ControlObject::getControl(ConfigKey(m_sGroup1, "rate"))->get());
    ASSERT_FLOAT_EQ(192.0, ControlObject::getControl(ConfigKey(m_sGroup1, "bpm"))->get());

    // Set the file bpm of channel 2 to 120bpm.
    QScopedPointer<ControlObjectThread> pFileBpm2(getControlObjectThread(
        ConfigKey(m_sGroup2, "file_bpm")));
    pFileBpm2->set(120.0);
    ASSERT_FLOAT_EQ(120.0, ControlObject::getControl(ConfigKey(m_sGroup2, "file_bpm"))->get());

    // rate slider for channel 2 should now be 1.6 = 160 * 1.2 / 120.
    ASSERT_FLOAT_EQ(getRateSliderValue(1.6),
                    ControlObject::getControl(ConfigKey(m_sGroup2, "rate"))->get());
    ASSERT_FLOAT_EQ(192.0, ControlObject::getControl(ConfigKey(m_sGroup2, "bpm"))->get());

    // Internal master should also be 192.
    ASSERT_FLOAT_EQ(192.0, ControlObject::getControl(ConfigKey(m_sInternalClockGroup, "bpm"))->get());
}

TEST_F(EngineSyncTest, RateChangeTestWeirdOrder) {
    // This is like the test above, but the user loads the track after the slider has been tweaked.
    QScopedPointer<ControlObjectThread> pButtonMasterSync1(getControlObjectThread(
            ConfigKey(m_sGroup1, "sync_mode")));
    pButtonMasterSync1->slotSet(SYNC_MASTER);
    QScopedPointer<ControlObjectThread> pButtonMasterSync2(getControlObjectThread(
            ConfigKey(m_sGroup2, "sync_mode")));
    pButtonMasterSync2->slotSet(SYNC_FOLLOWER);
    ProcessBuffer();

    // Set the file bpm of channel 1 to 160bpm.
    QScopedPointer<ControlObjectThread> pFileBpm1(getControlObjectThread(
        ConfigKey(m_sGroup1, "file_bpm")));
    pFileBpm1->set(160.0);
    ASSERT_FLOAT_EQ(160.0, ControlObject::getControl(ConfigKey(m_sInternalClockGroup, "bpm"))->get());

    // Set the file bpm of channel 2 to 120bpm.
    QScopedPointer<ControlObjectThread> pFileBpm2(getControlObjectThread(
        ConfigKey(m_sGroup2, "file_bpm")));
    pFileBpm2->set(120.0);

    // Set the rate slider of channel 1 to 1.2.
    ControlObject::getControl(ConfigKey(m_sGroup1, "rate"))->set(getRateSliderValue(1.2));

    // Rate slider for channel 2 should now be 1.6 = (160 * 1.2 / 120) - 1.0.
    ASSERT_FLOAT_EQ(getRateSliderValue(1.6),
                    ControlObject::getControl(ConfigKey(m_sGroup2, "rate"))->get());
    ASSERT_FLOAT_EQ(192.0, ControlObject::getControl(ConfigKey(m_sGroup2, "bpm"))->get());

    // Internal Master BPM should read the same.
    ASSERT_FLOAT_EQ(192.0, ControlObject::getControl(ConfigKey(m_sInternalClockGroup, "bpm"))->get());
}

TEST_F(EngineSyncTest, RateChangeTestOrder3) {
    // Set the file bpm of channel 1 to 160bpm.
    QScopedPointer<ControlObjectThread> pFileBpm1(getControlObjectThread(
        ConfigKey(m_sGroup1, "file_bpm")));
    pFileBpm1->set(160.0);
    ASSERT_FLOAT_EQ(160.0, ControlObject::getControl(ConfigKey(m_sGroup1, "file_bpm"))->get());

    // Set the file bpm of channel 2 to 120bpm.
    QScopedPointer<ControlObjectThread> pFileBpm2(getControlObjectThread(
        ConfigKey(m_sGroup2, "file_bpm")));
    pFileBpm2->set(120.0);
    ASSERT_FLOAT_EQ(120.0, ControlObject::getControl(ConfigKey(m_sGroup2, "file_bpm"))->get());

    // Turn on Master and Slave.
    QScopedPointer<ControlObjectThread> pButtonMasterSync1(getControlObjectThread(
            ConfigKey(m_sGroup1, "sync_mode")));
    pButtonMasterSync1->slotSet(SYNC_MASTER);
    ProcessBuffer();

    assertIsMaster(m_sGroup1);

    QScopedPointer<ControlObjectThread> pButtonMasterSync2(getControlObjectThread(
            ConfigKey(m_sGroup2, "sync_mode")));
    pButtonMasterSync2->slotSet(SYNC_FOLLOWER);
    ProcessBuffer();

    // Slave should immediately set its slider.
    ASSERT_FLOAT_EQ(getRateSliderValue(1.3333333333),
                    ControlObject::getControl(ConfigKey(m_sGroup2, "rate"))->get());
    ASSERT_FLOAT_EQ(160.0, ControlObject::getControl(ConfigKey(m_sGroup2, "bpm"))->get());
    ASSERT_FLOAT_EQ(160.0,
                    ControlObject::getControl(ConfigKey(m_sInternalClockGroup, "bpm"))->get());
}

TEST_F(EngineSyncTest, SlaveRateChange) {
    // Confirm that slaves can change master sync rate as well.
    QScopedPointer<ControlObjectThread> pButtonMasterSync1(getControlObjectThread(
            ConfigKey(m_sGroup1, "sync_mode")));
    pButtonMasterSync1->slotSet(SYNC_MASTER);
    QScopedPointer<ControlObjectThread> pButtonMasterSync2(getControlObjectThread(
            ConfigKey(m_sGroup2, "sync_mode")));
    pButtonMasterSync2->slotSet(SYNC_FOLLOWER);
    ProcessBuffer();

    // Set the file bpm of channel 1 to 160bpm.
    QScopedPointer<ControlObjectThread> pFileBpm1(getControlObjectThread(
        ConfigKey(m_sGroup1, "file_bpm")));
    pFileBpm1->set(160.0);

    // Set the file bpm of channel 2 to 120bpm.
    QScopedPointer<ControlObjectThread> pFileBpm2(getControlObjectThread(
        ConfigKey(m_sGroup2, "file_bpm")));
    pFileBpm2->set(120.0);

    // Set the rate slider of channel 1 to 1.2.
    ControlObject::getControl(ConfigKey(m_sGroup1, "rate"))->set(getRateSliderValue(1.2));

    // Rate slider for channel 2 should now be 1.6 = (160 * 1.2 / 120).
    ASSERT_FLOAT_EQ(getRateSliderValue(1.6),
                    ControlObject::getControl(ConfigKey(m_sGroup2, "rate"))->get());
    ASSERT_FLOAT_EQ(192.0, ControlObject::getControl(ConfigKey(m_sGroup2, "bpm"))->get());

    // Try to twiddle the rate slider on channel 2.
    QScopedPointer<ControlObjectThread> pSlider2(getControlObjectThread(
            ConfigKey(m_sGroup2, "rate")));
    pSlider2->slotSet(getRateSliderValue(0.8));
    ProcessBuffer();

    // Rates should still be changed even though it's a slave.
    ASSERT_FLOAT_EQ(getRateSliderValue(0.8),
                    ControlObject::getControl(ConfigKey(m_sGroup2, "rate"))->get());
    ASSERT_FLOAT_EQ(96.0, ControlObject::getControl(ConfigKey(m_sGroup2, "bpm"))->get());
    ASSERT_FLOAT_EQ(getRateSliderValue(0.6),
                    ControlObject::getControl(ConfigKey(m_sGroup1, "rate"))->get());
    ASSERT_FLOAT_EQ(96.0, ControlObject::getControl(ConfigKey(m_sGroup1, "bpm"))->get());
}

TEST_F(EngineSyncTest, InternalRateChangeTest) {
    QScopedPointer<ControlObjectThread> pButtonMasterSyncInternal(getControlObjectThread(
            ConfigKey(m_sInternalClockGroup, "sync_master")));
    pButtonMasterSyncInternal->slotSet(SYNC_MASTER);
    QScopedPointer<ControlObjectThread> pButtonMasterSync1(getControlObjectThread(
            ConfigKey(m_sGroup1, "sync_mode")));
    pButtonMasterSync1->slotSet(SYNC_FOLLOWER);
    QScopedPointer<ControlObjectThread> pButtonMasterSync2(getControlObjectThread(
            ConfigKey(m_sGroup2, "sync_mode")));
    pButtonMasterSync2->slotSet(SYNC_FOLLOWER);
    ProcessBuffer();

    assertIsMaster(m_sInternalClockGroup);
    assertIsFollower(m_sGroup1);
    assertIsFollower(m_sGroup2);

    // Set the file bpm of channel 1 to 160bpm.
    QScopedPointer<ControlObjectThread> pFileBpm1(getControlObjectThread(
        ConfigKey(m_sGroup1, "file_bpm")));
    pFileBpm1->set(160.0);
    ASSERT_FLOAT_EQ(160.0, ControlObject::getControl(ConfigKey(m_sGroup1, "file_bpm"))->get());

    // Set the file bpm of channel 2 to 120bpm.
    QScopedPointer<ControlObjectThread> pFileBpm2(getControlObjectThread(
        ConfigKey(m_sGroup2, "file_bpm")));
    pFileBpm2->set(120.0);
    ASSERT_FLOAT_EQ(120.0, ControlObject::getControl(ConfigKey(m_sGroup2, "file_bpm"))->get());

    // Set the internal rate to 150.
    QScopedPointer<ControlObjectThread> pMasterSyncSlider(getControlObjectThread(
            ConfigKey(m_sInternalClockGroup, "bpm")));
    pMasterSyncSlider->set(150.0);
    ASSERT_FLOAT_EQ(150.0, ControlObject::getControl(ConfigKey(m_sInternalClockGroup, "bpm"))->get());
    // Set decks playing, and process a buffer to update all the COs.
    ControlObject::getControl(ConfigKey(m_sGroup1, "play"))->set(1.0);
    ControlObject::getControl(ConfigKey(m_sGroup2, "play"))->set(1.0);

    ProcessBuffer();

    // Rate sliders for channels 1 and 2 should change appropriately.
    ASSERT_FLOAT_EQ(getRateSliderValue(0.9375),
                    ControlObject::getControl(ConfigKey(m_sGroup1, "rate"))->get());
    ASSERT_FLOAT_EQ(150.0, ControlObject::getControl(ConfigKey(m_sGroup1, "bpm"))->get());
    ASSERT_FLOAT_EQ(getRateSliderValue(1.25),
                    ControlObject::getControl(ConfigKey(m_sGroup2, "rate"))->get());
    ASSERT_FLOAT_EQ(150.0, ControlObject::getControl(ConfigKey(m_sGroup2, "bpm"))->get());
    // TODO(rryan): Re-enable with a true mock of SyncableFollower.
    //ASSERT_FLOAT_EQ(0.9375, ControlObject::getControl(ConfigKey(m_sGroup1, "rateEngine"))->get());
    //ASSERT_FLOAT_EQ(1.25, ControlObject::getControl(ConfigKey(m_sGroup2, "rateEngine"))->get());

    // Set the internal rate to 80.
    pMasterSyncSlider->set(80.0);

    // Update COs again.
    ProcessBuffer();

    // Rate sliders for channels 1 and 2 should change appropriately.
    ASSERT_FLOAT_EQ(getRateSliderValue(0.5),
                    ControlObject::getControl(ConfigKey(m_sGroup1, "rate"))->get());
    ASSERT_FLOAT_EQ(80.0, ControlObject::getControl(ConfigKey(m_sGroup1, "bpm"))->get());
    ASSERT_FLOAT_EQ(getRateSliderValue(0.6666667),
                    ControlObject::getControl(ConfigKey(m_sGroup2, "rate"))->get());
    ASSERT_FLOAT_EQ(80.0, ControlObject::getControl(ConfigKey(m_sGroup2, "bpm"))->get());
    // TODO(rryan): Re-enable with a true mock of SyncableFollower.
    //ASSERT_FLOAT_EQ(0.5, ControlObject::getControl(ConfigKey(m_sGroup1, "rateEngine"))->get());
    //ASSERT_FLOAT_EQ(0.6666667, ControlObject::getControl(ConfigKey(m_sGroup2, "rateEngine"))->get());

}

TEST_F(EngineSyncTest, MasterStopSliderCheck) {
    // If the master is playing, and stop is pushed, the sliders should stay the same.
    QScopedPointer<ControlObjectThread> pFileBpm1(getControlObjectThread(
        ConfigKey(m_sGroup1, "file_bpm")));
    pFileBpm1->set(120.0);
    QScopedPointer<ControlObjectThread> pFileBpm2(getControlObjectThread(
        ConfigKey(m_sGroup2, "file_bpm")));
    pFileBpm2->set(128.0);

    QScopedPointer<ControlObjectThread> pButtonMasterSync1(getControlObjectThread(
            ConfigKey(m_sGroup1, "sync_mode")));
    pButtonMasterSync1->slotSet(SYNC_MASTER);
    QScopedPointer<ControlObjectThread> pButtonMasterSync2(getControlObjectThread(
            ConfigKey(m_sGroup2, "sync_mode")));
    pButtonMasterSync2->slotSet(SYNC_FOLLOWER);
    ProcessBuffer();

    assertIsMaster(m_sGroup1);
    assertIsFollower(m_sGroup2);

    QScopedPointer<ControlObjectThread> pChannel1Play(getControlObjectThread(
            ConfigKey(m_sGroup1, "play")));
    pChannel1Play->set(1.0);
    QScopedPointer<ControlObjectThread> pChannel2Play(getControlObjectThread(
            ConfigKey(m_sGroup2, "play")));
    pChannel2Play->set(1.0);

    ProcessBuffer();

    // TODO(rryan): Re-enable with a true mock of SyncableFollower.
    //ASSERT_FLOAT_EQ(0.9375, ControlObject::getControl(ConfigKey(m_sGroup2, "rateEngine"))->get());
    ASSERT_FLOAT_EQ(120.0, ControlObject::getControl(ConfigKey(m_sGroup2, "bpm"))->get());
    ASSERT_FLOAT_EQ(getRateSliderValue(0.9375),
                    ControlObject::getControl(ConfigKey(m_sGroup2, "rate"))->get());

    pChannel1Play->set(0.0);

    ProcessBuffer();

    // TODO(rryan): Re-enable with a true mock of SyncableFollower.
    //ASSERT_FLOAT_EQ(0.0, ControlObject::getControl(ConfigKey(m_sGroup2, "rateEngine"))->get());
    EXPECT_FLOAT_EQ(120.0, ControlObject::getControl(ConfigKey(m_sGroup2, "bpm"))->get());
    EXPECT_FLOAT_EQ(getRateSliderValue(0.9375),
                    ControlObject::getControl(ConfigKey(m_sGroup2, "rate"))->get());
}

TEST_F(EngineSyncTest, EnableOneDeckInitsMaster) {
    // If Internal is master, and we turn sync on a playing deck, the playing deck sets the
    // internal master and the beat distances are now aligned.

    QScopedPointer<ControlObjectThread> pButtonMasterSyncInternal(getControlObjectThread(
            ConfigKey(m_sInternalClockGroup, "sync_master")));
    QScopedPointer<ControlObjectThread> pFileBpm1(getControlObjectThread(
        ConfigKey(m_sGroup1, "file_bpm")));

    // Set internal to master and give it a beat distance.
    ControlObject::getControl(ConfigKey(m_sInternalClockGroup, "bpm"))->set(124.0);
    ControlObject::getControl(ConfigKey(m_sInternalClockGroup, "beat_distance"))->set(0.5);
    pButtonMasterSyncInternal->slotSet(SYNC_MASTER);
    ProcessBuffer();

    // Set up the deck to play.
    pFileBpm1->set(130.0);
    ControlObject::getControl(ConfigKey(m_sGroup1, "rate"))->set(getRateSliderValue(1.0));
    ControlObject::getControl(ConfigKey(m_sGroup1, "beat_distance"))->set(0.2);
    ControlObject::getControl(ConfigKey(m_sGroup1, "play"))->set(1.0);

    // Set the deck to follower.  We have to call requestEnableSync directly
    // because calling ProcessBuffer() tries to advance the beat_distance values.
    m_pEngineSync->requestEnableSync(m_pEngineSync->getSyncableForGroup(m_sGroup1), true);

    // Internal should still be master (only one playing deck).
    assertIsMaster(m_sInternalClockGroup);

    // Internal clock rate should match master but beat distance should match follower.
    EXPECT_FLOAT_EQ(130.0,
                    ControlObject::getControl(ConfigKey(m_sInternalClockGroup, "bpm"))->get());
    EXPECT_FLOAT_EQ(130.0, ControlObject::getControl(ConfigKey(m_sGroup1, "bpm"))->get());
    EXPECT_FLOAT_EQ(0.2, ControlObject::getControl(ConfigKey(m_sGroup1, "beat_distance"))->get());
    EXPECT_FLOAT_EQ(0.2,
                    ControlObject::getControl(ConfigKey(m_sInternalClockGroup,
                                                        "beat_distance"))->get());

    // Enable second deck, beat distance should still match original setting.
    ControlObject::getControl(ConfigKey(m_sGroup2, "file_bpm"))->set(140.0);
    ControlObject::getControl(ConfigKey(m_sGroup2, "rate"))->set(getRateSliderValue(1.0));
    ControlObject::getControl(ConfigKey(m_sGroup2, "beat_distance"))->set(0.2);
    ControlObject::getControl(ConfigKey(m_sGroup2, "play"))->set(1.0);

    //pButtonSyncEnabled2->slotSet(1.0);
    m_pEngineSync->requestEnableSync(m_pEngineSync->getSyncableForGroup(m_sGroup2), true);

    EXPECT_FLOAT_EQ(130.0,
                    ControlObject::getControl(ConfigKey(m_sInternalClockGroup, "bpm"))->get());
    EXPECT_FLOAT_EQ(130.0, ControlObject::getControl(ConfigKey(m_sGroup2, "bpm"))->get());
    EXPECT_FLOAT_EQ(0.2, ControlObject::getControl(ConfigKey(m_sGroup2, "beat_distance"))->get());
    EXPECT_FLOAT_EQ(0.2,
                    ControlObject::getControl(ConfigKey(m_sInternalClockGroup,
                                                        "beat_distance"))->get());
}

TEST_F(EngineSyncTest, EnableOneDeckInitializesMaster) {
    // If we turn sync on a playing deck, the playing deck initializes the internal clock master.

    QScopedPointer<ControlObjectThread> pButtonMasterSyncInternal(getControlObjectThread(
            ConfigKey(m_sInternalClockGroup, "sync_master")));
    QScopedPointer<ControlObjectThread> pButtonSyncMasterEnabled1(getControlObjectThread(
            ConfigKey(m_sGroup1, "sync_master")));

    QScopedPointer<ControlObjectThread> pFileBpm1(getControlObjectThread(
        ConfigKey(m_sGroup1, "file_bpm")));

    // Set the deck to play.
    pFileBpm1->set(130.0);
    ControlObject::getControl(ConfigKey(m_sGroup1, "rate"))->set(getRateSliderValue(1.0));
    ControlObject::getControl(ConfigKey(m_sGroup1, "beat_distance"))->set(0.2);
    ControlObject::getControl(ConfigKey(m_sGroup1, "play"))->set(1.0);

    // Set the deck to follower.
    m_pEngineSync->requestEnableSync(m_pEngineSync->getSyncableForGroup(m_sGroup1), true);

    // Internal should still be master.
    assertIsMaster(m_sInternalClockGroup);

    // Internal clock rate should be set and beat distances reset.
    ASSERT_FLOAT_EQ(130.0,
                    ControlObject::getControl(ConfigKey(m_sInternalClockGroup, "bpm"))->get());
    ASSERT_FLOAT_EQ(130.0, ControlObject::getControl(ConfigKey(m_sGroup1, "bpm"))->get());
    ASSERT_FLOAT_EQ(0.2, ControlObject::getControl(ConfigKey(m_sGroup1, "beat_distance"))->get());
    ASSERT_FLOAT_EQ(0.2,
                    ControlObject::getControl(ConfigKey(m_sInternalClockGroup,
                                                        "beat_distance"))->get());
}

TEST_F(EngineSyncTest, EnableOneDeckSliderUpdates) {
    // If we enable a deck to be master, the internal slider should immediately update.
    QScopedPointer<ControlObjectThread> pButtonSyncEnabled1(getControlObjectThread(
            ConfigKey(m_sGroup1, "sync_enabled")));

    QScopedPointer<ControlObjectThread> pFileBpm1(getControlObjectThread(
        ConfigKey(m_sGroup1, "file_bpm")));
    pFileBpm1->set(130.0);
    ControlObject::getControl(ConfigKey(m_sGroup1, "rate"))->set(getRateSliderValue(1.0));

    // Set the deck to sync enabled.
    pButtonSyncEnabled1->slotSet(1.0);
    ProcessBuffer();

    // Internal should now be master (only one sync deck).
    assertIsMaster(m_sInternalClockGroup);

    // Internal clock rate should be set.
    ASSERT_FLOAT_EQ(130.0,
                    ControlObject::getControl(ConfigKey(m_sInternalClockGroup, "bpm"))->get());
}

TEST_F(EngineSyncTest, SyncToNonSyncDeck) {
    // If deck 1 is playing, and deck 2 presses sync, deck 2 should sync to deck 1 even if
    // deck 1 is not a sync deck.

    QScopedPointer<ControlObjectThread> pButtonSyncEnabled1(getControlObjectThread(
            ConfigKey(m_sGroup1, "sync_enabled")));
    QScopedPointer<ControlObjectThread> pButtonSyncEnabled2(getControlObjectThread(
            ConfigKey(m_sGroup2, "sync_enabled")));

    QScopedPointer<ControlObjectThread> pFileBpm1(getControlObjectThread(
        ConfigKey(m_sGroup1, "file_bpm")));
    pFileBpm1->set(130.0);
    ProcessBuffer();
    ControlObject::getControl(ConfigKey(m_sGroup1, "rate"))->set(getRateSliderValue(1.0));
    QScopedPointer<ControlObjectThread> pFileBpm2(getControlObjectThread(
        ConfigKey(m_sGroup2, "file_bpm")));
    pFileBpm2->set(100.0);
    ControlObject::getControl(ConfigKey(m_sGroup2, "rate"))->set(getRateSliderValue(1.0));

    ControlObject::getControl(ConfigKey(m_sGroup1, "play"))->set(1.0);
    ControlObject::getControl(ConfigKey(m_sGroup2, "play"))->set(1.0);

    pButtonSyncEnabled2->set(1.0);
    pButtonSyncEnabled2->set(0.0);
    ProcessBuffer();

    // There should be no master, and deck2 should match rate of deck1.  Sync slider should be
    // updated with the value, however.
    assertNoMaster();
    EXPECT_FLOAT_EQ(130.0,
                    ControlObject::getControl(ConfigKey(m_sInternalClockGroup, "bpm"))->get());
    assertSyncOff(m_sGroup2);
    EXPECT_FLOAT_EQ(getRateSliderValue(1.3),
                    ControlObject::getControl(ConfigKey(m_sGroup2, "rate"))->get());

    // Reset the pitch of deck 2.
    ControlObject::getControl(ConfigKey(m_sGroup2, "rate"))->set(getRateSliderValue(1.0));

    // The same should work in reverse.
    pButtonSyncEnabled1->set(1.0);
    pButtonSyncEnabled1->set(0.0);
    ProcessBuffer();

    // There should be no master, and deck2 should match rate of deck1.
    EXPECT_EQ(0, ControlObject::getControl(ConfigKey(m_sInternalClockGroup, "sync_master"))->get());
    EXPECT_FLOAT_EQ(100.0,
                    ControlObject::getControl(ConfigKey(m_sInternalClockGroup, "bpm"))->get());
    EXPECT_EQ(NULL, m_pEngineSync->getMaster());
    EXPECT_EQ(NULL, m_pEngineSync->getMasterSyncable());
    EXPECT_EQ(SYNC_NONE, ControlObject::getControl(ConfigKey(m_sGroup1, "sync_mode"))->get());
    EXPECT_EQ(0, ControlObject::getControl(ConfigKey(m_sGroup1, "sync_enabled"))->get());
    EXPECT_EQ(0, ControlObject::getControl(ConfigKey(m_sGroup1, "sync_master"))->get());
    EXPECT_FLOAT_EQ(getRateSliderValue(100.0 / 130.0),
                    ControlObject::getControl(ConfigKey(m_sGroup1, "rate"))->get());

    // Reset again.
    ControlObject::getControl(ConfigKey(m_sGroup1, "rate"))->set(getRateSliderValue(1.0));

    // If deck 1 is not playing, however, deck 2 should stay at the same rate.
    ControlObject::getControl(ConfigKey(m_sGroup1, "play"))->set(0.0);

    // The same should work in reverse.
    pButtonSyncEnabled1->set(1.0);
    pButtonSyncEnabled1->set(0.0);
    ProcessBuffer();

    // There should be no master, and deck2 should match rate of deck1.
    EXPECT_EQ(0, ControlObject::getControl(ConfigKey(m_sInternalClockGroup, "sync_master"))->get());
    EXPECT_FLOAT_EQ(100.0,
                    ControlObject::getControl(ConfigKey(m_sInternalClockGroup, "bpm"))->get());
    EXPECT_EQ(NULL, m_pEngineSync->getMaster());
    EXPECT_EQ(NULL, m_pEngineSync->getMasterSyncable());
    EXPECT_EQ(SYNC_NONE, ControlObject::getControl(ConfigKey(m_sGroup2, "sync_mode"))->get());
    EXPECT_EQ(0, ControlObject::getControl(ConfigKey(m_sGroup2, "sync_enabled"))->get());
    EXPECT_EQ(0, ControlObject::getControl(ConfigKey(m_sGroup2, "sync_master"))->get());
    EXPECT_FLOAT_EQ(getRateSliderValue(1.0),
                    ControlObject::getControl(ConfigKey(m_sGroup2, "rate"))->get());
}

TEST_F(EngineSyncTest, EjectTrackSyncRemains) {
    QScopedPointer<ControlObjectThread> pButtonMasterSyncInternal(getControlObjectThread(
            ConfigKey(m_sInternalClockGroup, "sync_master")));
    QScopedPointer<ControlObjectThread> pButtonSyncEnabled1(getControlObjectThread(
            ConfigKey(m_sGroup1, "sync_enabled")));
    QScopedPointer<ControlObjectThread> pButtonEject1(getControlObjectThread(
            ConfigKey(m_sGroup1, "eject")));

    pButtonMasterSyncInternal->slotSet(1.0);

    QScopedPointer<ControlObjectThread> pFileBpm1(getControlObjectThread(
            ConfigKey(m_sGroup1, "file_bpm")));
    pFileBpm1->set(120.0);
    pButtonSyncEnabled1->slotSet(1.0);

    ProcessBuffer();
    pButtonEject1->slotSet(1.0);
    // When an eject happens, the bpm gets set to zero.
    pFileBpm1->set(0.0);
    ProcessBuffer();

    assertIsFollower(m_sGroup1);
}

TEST_F(EngineSyncTest, FileBpmChangesDontAffectMaster) {
    // If filebpm changes, don't treat it like a rate change.
    QScopedPointer<ControlObjectThread> pFileBpm1(getControlObjectThread(
        ConfigKey(m_sGroup2, "file_bpm")));
    pFileBpm1->set(100.0);
    QScopedPointer<ControlObjectThread> pButtonSyncEnabled1(getControlObjectThread(
            ConfigKey(m_sGroup1, "sync_enabled")));
    pButtonSyncEnabled1->set(1.0);
    ProcessBuffer();

    QScopedPointer<ControlObjectThread> pFileBpm2(getControlObjectThread(
        ConfigKey(m_sGroup2, "file_bpm")));
    pFileBpm2->set(120.0);
    QScopedPointer<ControlObjectThread> pButtonSyncEnabled2(getControlObjectThread(
            ConfigKey(m_sGroup2, "sync_enabled")));
    pButtonSyncEnabled2->set(1.0);
    ProcessBuffer();

    pFileBpm1->set(160.0);
    EXPECT_FLOAT_EQ(100.0,
                    ControlObject::getControl(ConfigKey(m_sInternalClockGroup,
                                                        "bpm"))->get());
}

TEST_F(EngineSyncTest, ExplicitMasterPostProcessed) {
    // Regression test thanks to a bug.  Make sure that an explicit master
    // channel gets post-processed.
    QScopedPointer<ControlObjectThread> pButtonMasterSync1(getControlObjectThread(
            ConfigKey(m_sGroup1, "sync_mode")));
    QScopedPointer<ControlObjectThread> pFileBpm1(getControlObjectThread(
        ConfigKey(m_sGroup1, "file_bpm")));
    pButtonMasterSync1->slotSet(SYNC_MASTER);
    pFileBpm1->set(160.0);
    ProcessBuffer();
    ControlObject::getControl(ConfigKey(m_sGroup1, "play"))->set(1.0);
    ProcessBuffer();

    EXPECT_FLOAT_EQ(0.0046439911, m_pChannel1->getEngineBuffer()->getVisualPlayPos());
}
