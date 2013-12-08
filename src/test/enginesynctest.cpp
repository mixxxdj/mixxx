// Tests for Master Sync.
// There are no tests for actual deck playback, since I don't know how to mock that out yet.
// The following manual tests should probably be performed:
// * Quantize mode nudges tracks in sync, whether internal or deck master.
// * Flinging tracks with the waveform should work.
// * vinyl??


#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <QtDebug>

#include "configobject.h"
#include "controlobject.h"
#include "test/mockedenginebackendtest.h"
#include "test/mixxxtest.h"


namespace {

class EngineSyncTest : public MockedEngineBackendTest {
};

TEST_F(EngineSyncTest, ControlObjectsExist) {
    // This isn't exhaustive, but certain COs have a habit of not being set up properly.
    ASSERT_TRUE(m_pRateControl1 != NULL);
    ASSERT_TRUE(ControlObject::getControl(ConfigKey(m_sGroup1, "file_bpm")) != NULL);
    ASSERT_TRUE(ControlObject::getControl(ConfigKey(m_sGroup1, "rateEngine")) != NULL);
}

TEST_F(EngineSyncTest, SetMasterSuccess) {
    // If we set the first channel to master, EngineSync should get that message.

    // Throughout these tests we use ControlObjectThreads so that we can trigger ValueChanged,
    // and not just ValueChangedFromEngine.
    QScopedPointer<ControlObjectThread> pButtonMasterSync1(getControlObjectThread(
            ConfigKey(m_sGroup1, "sync_mode")));
    pButtonMasterSync1->slotSet(SYNC_MASTER);

    // The master sync should now be channel 1.
    ASSERT_EQ(m_pChannel1, m_pEngineSync->getMaster());
    EXPECT_EQ(0, ControlObject::getControl(ConfigKey(m_sGroup1, "sync_slave"))->get());
    EXPECT_EQ(1, ControlObject::getControl(ConfigKey(m_sGroup1, "sync_master"))->get());

    QScopedPointer<ControlObjectThread> pButtonMasterSync2(getControlObjectThread(
            ConfigKey(m_sGroup2, "sync_mode")));
    pButtonMasterSync2->slotSet(SYNC_SLAVE);

    ASSERT_EQ(SYNC_SLAVE, pButtonMasterSync2->get());
    EXPECT_EQ(1, ControlObject::getControl(ConfigKey(m_sGroup2, "sync_slave"))->get());
    EXPECT_EQ(0, ControlObject::getControl(ConfigKey(m_sGroup2, "sync_master"))->get());

    // Now set channel 2 to be master.
    pButtonMasterSync2->slotSet(SYNC_MASTER);

    // Now channel 2 should be master, and channel 1 should be a slave.
    ASSERT_EQ(m_pChannel2, m_pEngineSync->getMaster());
    ASSERT_EQ(SYNC_MASTER, pButtonMasterSync2->get());
    EXPECT_EQ(0, ControlObject::getControl(ConfigKey(m_sGroup2, "sync_slave"))->get());
    EXPECT_EQ(1, ControlObject::getControl(ConfigKey(m_sGroup2, "sync_master"))->get());
    ASSERT_EQ(SYNC_SLAVE, pButtonMasterSync1->get());
    EXPECT_EQ(1, ControlObject::getControl(ConfigKey(m_sGroup1, "sync_slave"))->get());
    EXPECT_EQ(0, ControlObject::getControl(ConfigKey(m_sGroup1, "sync_master"))->get());
    ASSERT_EQ(m_sGroup2, m_pEngineSync->getSyncSource().toStdString());

    // Now back again.
    pButtonMasterSync1->slotSet(SYNC_MASTER);

    // Now channel 1 should be master, and channel 2 should be a slave.
    ASSERT_EQ(m_pChannel1, m_pEngineSync->getMaster());
    ASSERT_EQ(SYNC_MASTER, pButtonMasterSync1->get());
    EXPECT_EQ(0, ControlObject::getControl(ConfigKey(m_sGroup1, "sync_slave"))->get());
    EXPECT_EQ(1, ControlObject::getControl(ConfigKey(m_sGroup1, "sync_master"))->get());
    ASSERT_EQ(SYNC_SLAVE, pButtonMasterSync2->get());
    EXPECT_EQ(1, ControlObject::getControl(ConfigKey(m_sGroup2, "sync_slave"))->get());
    EXPECT_EQ(0, ControlObject::getControl(ConfigKey(m_sGroup2, "sync_master"))->get());
    ASSERT_EQ(m_sGroup1, m_pEngineSync->getSyncSource().toStdString());

    // Now set channel 1 to slave, internal will be master because no track loaded.
    pButtonMasterSync1->slotSet(SYNC_SLAVE);

    ASSERT_EQ(NULL, m_pEngineSync->getMaster());
    ASSERT_EQ(SYNC_SLAVE, pButtonMasterSync1->get());
    EXPECT_EQ(1, ControlObject::getControl(ConfigKey(m_sGroup1, "sync_slave"))->get());
    EXPECT_EQ(0, ControlObject::getControl(ConfigKey(m_sGroup1, "sync_master"))->get());
    ASSERT_EQ(SYNC_SLAVE, pButtonMasterSync2->get());
    EXPECT_EQ(1, ControlObject::getControl(ConfigKey(m_sGroup2, "sync_slave"))->get());
    EXPECT_EQ(0, ControlObject::getControl(ConfigKey(m_sGroup2, "sync_master"))->get());
    EXPECT_EQ(1, ControlObject::getControl(ConfigKey("[Master]", "sync_master"))->get());
    ASSERT_EQ("[Master]", m_pEngineSync->getSyncSource().toStdString());
}

TEST_F(EngineSyncTest, SetMasterWhilePlaying) {
    // Make sure we don't get two master lights if we change masters while playing.

    ControlObject::getControl(ConfigKey(m_sGroup1, "file_bpm"))->set(120.0);
    ControlObject::getControl(ConfigKey(m_sGroup2, "file_bpm"))->set(124.0);
    ControlObject::getControl(ConfigKey(m_sGroup3, "file_bpm"))->set(128.0);

    QScopedPointer<ControlObjectThread> pButtonMasterSync1(getControlObjectThread(
            ConfigKey(m_sGroup1, "sync_mode")));
    pButtonMasterSync1->slotSet(SYNC_MASTER);
    QScopedPointer<ControlObjectThread> pButtonMasterSync2(getControlObjectThread(
            ConfigKey(m_sGroup2, "sync_mode")));
    pButtonMasterSync2->slotSet(SYNC_SLAVE);
    QScopedPointer<ControlObjectThread> pButtonMasterSync3(getControlObjectThread(
            ConfigKey(m_sGroup3, "sync_mode")));
    pButtonMasterSync3->slotSet(SYNC_SLAVE);

    ControlObject::getControl(ConfigKey(m_sGroup1, "play"))->set(1.0);
    ControlObject::getControl(ConfigKey(m_sGroup2, "play"))->set(1.0);
    ControlObject::getControl(ConfigKey(m_sGroup3, "play"))->set(1.0);

    ProcessBuffer();

    pButtonMasterSync3->slotSet(SYNC_MASTER);

    ProcessBuffer();

    EXPECT_EQ(1, ControlObject::getControl(ConfigKey(m_sGroup1, "sync_slave"))->get());
    EXPECT_EQ(0, ControlObject::getControl(ConfigKey(m_sGroup1, "sync_master"))->get());
    EXPECT_EQ(1, ControlObject::getControl(ConfigKey(m_sGroup2, "sync_slave"))->get());
    EXPECT_EQ(0, ControlObject::getControl(ConfigKey(m_sGroup2, "sync_master"))->get());
    EXPECT_EQ(0, ControlObject::getControl(ConfigKey(m_sGroup3, "sync_slave"))->get());
    EXPECT_EQ(1, ControlObject::getControl(ConfigKey(m_sGroup3, "sync_master"))->get());
}


TEST_F(EngineSyncTest, SetSlaveNoMaster) {
    // If we set the first channel to slave, Internal should become master.
    QScopedPointer<ControlObjectThread> pButtonMasterSync1(getControlObjectThread(
            ConfigKey(m_sGroup1, "sync_mode")));
    pButtonMasterSync1->slotSet(SYNC_SLAVE);

    // The master sync should now be Internal.
    ASSERT_EQ(NULL, m_pEngineSync->getMaster());
    EXPECT_EQ(1, ControlObject::getControl(ConfigKey(m_sGroup1, "sync_slave"))->get());
    EXPECT_EQ(0, ControlObject::getControl(ConfigKey(m_sGroup1, "sync_master"))->get());
    EXPECT_EQ(1, ControlObject::getControl(ConfigKey("[Master]", "sync_master"))->get());
    ASSERT_EQ("[Master]", m_pEngineSync->getSyncSource().toStdString());
}

TEST_F(EngineSyncTest, DisableInternalMasterWhilePlaying) {
    // If we set the first channel to slave, Internal should become master.
    QScopedPointer<ControlObjectThread> pButtonMasterSync(getControlObjectThread(
            ConfigKey("[Master]", "sync_master")));
    pButtonMasterSync->slotSet(1.0);
    QScopedPointer<ControlObjectThread> pButtonSyncMode1(getControlObjectThread(
            ConfigKey(m_sGroup1, "sync_mode")));
    pButtonSyncMode1->slotSet(SYNC_SLAVE);

    // The master sync should now be Internal.
    ASSERT_EQ(NULL, m_pEngineSync->getMaster());
    EXPECT_EQ(1, ControlObject::getControl(ConfigKey(m_sGroup1, "sync_slave"))->get());
    EXPECT_EQ(0, ControlObject::getControl(ConfigKey(m_sGroup1, "sync_master"))->get());
    EXPECT_EQ(1, ControlObject::getControl(ConfigKey("[Master]", "sync_master"))->get());
    ASSERT_EQ("[Master]", m_pEngineSync->getSyncSource().toStdString());

    // Make sure deck 1 is playing.
    ControlObject::getControl(ConfigKey(m_sGroup1, "file_bpm"))->set(80.0);
    ControlObject::getControl(ConfigKey(m_sGroup1, "play"))->set(1.0);
    ProcessBuffer();

    // Now unset Internal master.
    pButtonMasterSync->slotSet(0.0);

    // Master sync should be the channel again.
    ASSERT_EQ(m_pChannel1, m_pEngineSync->getMaster());
    EXPECT_EQ(0, ControlObject::getControl(ConfigKey(m_sGroup1, "sync_slave"))->get());
    EXPECT_EQ(1, ControlObject::getControl(ConfigKey(m_sGroup1, "sync_master"))->get());
    EXPECT_EQ(0, ControlObject::getControl(ConfigKey("[Master]", "sync_master"))->get());
    ASSERT_EQ(m_sGroup1, m_pEngineSync->getSyncSource().toStdString());
}

TEST_F(EngineSyncTest, InternalMasterSetSlaveSliderMoves) {
    // If internal is master, and we turn on a slave, the slider should move.
    QScopedPointer<ControlObjectThread> pButtonMasterSyncInternal(getControlObjectThread(
            ConfigKey("[Master]", "sync_master")));
    pButtonMasterSyncInternal->slotSet(1);
    ControlObject::getControl(ConfigKey("[Master]", "sync_bpm"))->set(100.0);
    ControlObject::getControl(ConfigKey("[Master]", "sync_slider"))->set(100.0);

	// Set the file bpm of channel 1 to 160bpm.
    ControlObject::getControl(ConfigKey(m_sGroup1, "file_bpm"))->set(80.0);

    QScopedPointer<ControlObjectThread> pButtonMasterSync1(getControlObjectThread(
            ConfigKey(m_sGroup1, "sync_mode")));
    pButtonMasterSync1->slotSet(SYNC_SLAVE);

    ASSERT_FLOAT_EQ(getRateSliderValue(1.25),
                    ControlObject::getControl(ConfigKey(m_sGroup1, "rate"))->get());
    ASSERT_FLOAT_EQ(100.0, ControlObject::getControl(ConfigKey(m_sGroup1, "bpm"))->get());
}

TEST_F(EngineSyncTest, SetMasterByLights) {
    // Same as above, except we use the midi lights to change state.
    QScopedPointer<ControlObjectThread> pButtonMasterSync1(getControlObjectThread(
            ConfigKey(m_sGroup1, "sync_mode")));
    QScopedPointer<ControlObjectThread> pButtonMasterSync2(getControlObjectThread(
            ConfigKey(m_sGroup2, "sync_mode")));
    QScopedPointer<ControlObjectThread> pButtonSyncSlave1(getControlObjectThread(
            ConfigKey(m_sGroup1, "sync_slave")));
    QScopedPointer<ControlObjectThread> pButtonSyncSlave2(getControlObjectThread(
            ConfigKey(m_sGroup2, "sync_slave")));
    QScopedPointer<ControlObjectThread> pButtonSyncMaster1(getControlObjectThread(
            ConfigKey(m_sGroup1, "sync_master")));
    QScopedPointer<ControlObjectThread> pButtonSyncMaster2(getControlObjectThread(
            ConfigKey(m_sGroup2, "sync_master")));

    // Set channel 1 to be master.
    pButtonSyncMaster1->slotSet(1.0);

    // The master sync should now be channel 1.
    ASSERT_EQ(m_pChannel1, m_pEngineSync->getMaster());
    EXPECT_EQ(0, pButtonSyncSlave1->get());
    EXPECT_EQ(1, pButtonSyncMaster1->get());

    // Set channel 2 to be slave.
    pButtonSyncSlave2->slotSet(1);

    ASSERT_EQ(SYNC_SLAVE, pButtonMasterSync2->get());
    EXPECT_EQ(1, pButtonSyncSlave2->get());
    EXPECT_EQ(0, pButtonSyncMaster2->get());

    // Now set channel 2 to be master.
    pButtonSyncMaster2->slotSet(1);

    // Now channel 2 should be master, and channel 1 should be a slave.
    ASSERT_EQ(m_pChannel2, m_pEngineSync->getMaster());
    ASSERT_EQ(SYNC_MASTER, pButtonMasterSync2->get());
    EXPECT_EQ(0, pButtonSyncSlave2->get());
    EXPECT_EQ(1, pButtonSyncMaster2->get());
    ASSERT_EQ(SYNC_SLAVE, pButtonMasterSync1->get());
    EXPECT_EQ(1, pButtonSyncSlave1->get());
    EXPECT_EQ(0, pButtonSyncMaster1->get());
    ASSERT_EQ(m_sGroup2, m_pEngineSync->getSyncSource().toStdString());

    // Now back again.
    pButtonSyncMaster1->slotSet(1);

    // Now channel 1 should be master, and channel 2 should be a slave.
    ASSERT_EQ(m_pChannel1, m_pEngineSync->getMaster());
    ASSERT_EQ(SYNC_MASTER, pButtonMasterSync1->get());
    EXPECT_EQ(0, pButtonSyncSlave1->get());
    EXPECT_EQ(1, pButtonSyncMaster1->get());
    ASSERT_EQ(SYNC_SLAVE, pButtonMasterSync2->get());
    EXPECT_EQ(1, pButtonSyncSlave2->get());
    EXPECT_EQ(0, pButtonSyncMaster2->get());
    ASSERT_EQ(m_sGroup1, m_pEngineSync->getSyncSource().toStdString());

    // Now set channel 1 to slave, internal will be master because no track loaded.
    pButtonSyncSlave1->slotSet(1);

    ASSERT_EQ(NULL, m_pEngineSync->getMaster());
    ASSERT_EQ(SYNC_SLAVE, pButtonMasterSync1->get());
    EXPECT_EQ(1, ControlObject::getControl(ConfigKey(m_sGroup1, "sync_slave"))->get());
    EXPECT_EQ(0, ControlObject::getControl(ConfigKey(m_sGroup1, "sync_master"))->get());
    ASSERT_EQ(SYNC_SLAVE, pButtonMasterSync2->get());
    EXPECT_EQ(1, ControlObject::getControl(ConfigKey(m_sGroup2, "sync_slave"))->get());
    EXPECT_EQ(0, ControlObject::getControl(ConfigKey(m_sGroup2, "sync_master"))->get());
    EXPECT_EQ(1, ControlObject::getControl(ConfigKey("[Master]", "sync_master"))->get());
    ASSERT_EQ("[Master]", m_pEngineSync->getSyncSource().toStdString());
}

TEST_F(EngineSyncTest, RateChangeTest) {
    QScopedPointer<ControlObjectThread> pButtonMasterSync1(getControlObjectThread(
            ConfigKey(m_sGroup1, "sync_mode")));
    pButtonMasterSync1->slotSet(SYNC_MASTER);
    QScopedPointer<ControlObjectThread> pButtonMasterSync2(getControlObjectThread(
            ConfigKey(m_sGroup2, "sync_mode")));
    pButtonMasterSync2->slotSet(SYNC_SLAVE);

    // Set the file bpm of channel 1 to 160bpm.
    ControlObject::getControl(ConfigKey(m_sGroup1, "file_bpm"))->set(160.0);
    ASSERT_FLOAT_EQ(160.0, ControlObject::getControl(ConfigKey(m_sGroup1, "file_bpm"))->get());
    ASSERT_FLOAT_EQ(160.0, ControlObject::getControl(ConfigKey("[Master]", "sync_bpm"))->get());

    // Set the rate of channel 1 to 1.2.
    ControlObject::getControl(ConfigKey(m_sGroup1, "rate"))->set(getRateSliderValue(1.2));
    ASSERT_FLOAT_EQ(getRateSliderValue(1.2),
                    ControlObject::getControl(ConfigKey(m_sGroup1, "rate"))->get());
    ASSERT_FLOAT_EQ(192.0, ControlObject::getControl(ConfigKey(m_sGroup1, "bpm"))->get());

    // Set the file bpm of channel 2 to 120bpm.
    ControlObject::getControl(ConfigKey(m_sGroup2, "file_bpm"))->set(120.0);
    ASSERT_FLOAT_EQ(120.0, ControlObject::getControl(ConfigKey(m_sGroup2, "file_bpm"))->get());

    // rate slider for channel 2 should now be 1.6 = 160 * 1.2 / 120.
    ASSERT_FLOAT_EQ(getRateSliderValue(1.6),
                    ControlObject::getControl(ConfigKey(m_sGroup2, "rate"))->get());
    ASSERT_FLOAT_EQ(192.0, ControlObject::getControl(ConfigKey(m_sGroup2, "bpm"))->get());

    // Internal master should also be 192.
    ASSERT_FLOAT_EQ(192.0, ControlObject::getControl(ConfigKey("[Master]", "sync_bpm"))->get());
}

TEST_F(EngineSyncTest, RateChangeTestWeirdOrder) {
    // This is like the test above, but the user loads the track after the slider has been tweaked.
    QScopedPointer<ControlObjectThread> pButtonMasterSync1(getControlObjectThread(
            ConfigKey(m_sGroup1, "sync_mode")));
    pButtonMasterSync1->slotSet(SYNC_MASTER);
    QScopedPointer<ControlObjectThread> pButtonMasterSync2(getControlObjectThread(
            ConfigKey(m_sGroup2, "sync_mode")));
    pButtonMasterSync2->slotSet(SYNC_SLAVE);

    // Set the file bpm of channel 1 to 160bpm.
    ControlObject::getControl(ConfigKey(m_sGroup1, "file_bpm"))->set(160.0);
    ASSERT_FLOAT_EQ(160.0, ControlObject::getControl(ConfigKey("[Master]", "sync_bpm"))->get());

    // Set the file bpm of channel 2 to 120bpm.
    ControlObject::getControl(ConfigKey(m_sGroup2, "file_bpm"))->set(120.0);

    // Set the rate slider of channel 1 to 1.2.
    ControlObject::getControl(ConfigKey(m_sGroup1, "rate"))->set(getRateSliderValue(1.2));

    // Rate slider for channel 2 should now be 1.6 = (160 * 1.2 / 120) - 1.0.
    ASSERT_FLOAT_EQ(getRateSliderValue(1.6),
                    ControlObject::getControl(ConfigKey(m_sGroup2, "rate"))->get());
    ASSERT_FLOAT_EQ(192.0, ControlObject::getControl(ConfigKey(m_sGroup2, "bpm"))->get());

    // Internal Master BPM should read the same.
    ASSERT_FLOAT_EQ(192.0, ControlObject::getControl(ConfigKey("[Master]", "sync_bpm"))->get());
}

TEST_F(EngineSyncTest, RateChangeTestOrder3) {
    // Set the file bpm of channel 1 to 160bpm.
    ControlObject::getControl(ConfigKey(m_sGroup1, "file_bpm"))->set(160.0);
    ASSERT_FLOAT_EQ(160.0, ControlObject::getControl(ConfigKey(m_sGroup1, "file_bpm"))->get());

    // Set the file bpm of channel 2 to 120bpm.
    ControlObject::getControl(ConfigKey(m_sGroup2, "file_bpm"))->set(120.0);
    ASSERT_FLOAT_EQ(120.0, ControlObject::getControl(ConfigKey(m_sGroup2, "file_bpm"))->get());

    // Turn on Master and Slave.
    QScopedPointer<ControlObjectThread> pButtonMasterSync1(getControlObjectThread(
            ConfigKey(m_sGroup1, "sync_mode")));
    pButtonMasterSync1->slotSet(SYNC_MASTER);
    QScopedPointer<ControlObjectThread> pButtonMasterSync2(getControlObjectThread(
            ConfigKey(m_sGroup2, "sync_mode")));
    pButtonMasterSync2->slotSet(SYNC_SLAVE);

    // Slave should immediately set its slider.
    ASSERT_FLOAT_EQ(getRateSliderValue(1.3333333333),
                    ControlObject::getControl(ConfigKey(m_sGroup2, "rate"))->get());
    ASSERT_FLOAT_EQ(160.0, ControlObject::getControl(ConfigKey(m_sGroup2, "bpm"))->get());
    ASSERT_FLOAT_EQ(160.0, ControlObject::getControl(ConfigKey("[Master]", "sync_slider"))->get());
}


TEST_F(EngineSyncTest, SlaveRateChange) {
    // Confirm that slaves can change master sync rate as well.
    QScopedPointer<ControlObjectThread> pButtonMasterSync1(getControlObjectThread(
            ConfigKey(m_sGroup1, "sync_mode")));
    pButtonMasterSync1->slotSet(SYNC_MASTER);
    QScopedPointer<ControlObjectThread> pButtonMasterSync2(getControlObjectThread(
            ConfigKey(m_sGroup2, "sync_mode")));
    pButtonMasterSync2->slotSet(SYNC_SLAVE);

    // Set the file bpm of channel 1 to 160bpm.
    ControlObject::getControl(ConfigKey(m_sGroup1, "file_bpm"))->set(160.0);

    // Set the file bpm of channel 2 to 120bpm.
    ControlObject::getControl(ConfigKey(m_sGroup2, "file_bpm"))->set(120.0);

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
            ConfigKey("[Master]", "sync_master")));
    pButtonMasterSyncInternal->slotSet(SYNC_MASTER);
    QScopedPointer<ControlObjectThread> pButtonMasterSync1(getControlObjectThread(
            ConfigKey(m_sGroup1, "sync_mode")));
    pButtonMasterSync1->slotSet(SYNC_SLAVE);
    QScopedPointer<ControlObjectThread> pButtonMasterSync2(getControlObjectThread(
            ConfigKey(m_sGroup2, "sync_mode")));
    pButtonMasterSync2->slotSet(SYNC_SLAVE);

    // Set the file bpm of channel 1 to 160bpm.
    ControlObject::getControl(ConfigKey(m_sGroup1, "file_bpm"))->set(160.0);
    ASSERT_FLOAT_EQ(160.0, ControlObject::getControl(ConfigKey(m_sGroup1, "file_bpm"))->get());

    // Set the file bpm of channel 2 to 120bpm.
    ControlObject::getControl(ConfigKey(m_sGroup2, "file_bpm"))->set(120.0);
    ASSERT_FLOAT_EQ(120.0, ControlObject::getControl(ConfigKey(m_sGroup2, "file_bpm"))->get());

    // Set the internal rate to 150.
    ControlObject::getControl(ConfigKey("[Master]", "sync_slider"))->set(150.0);
    ASSERT_FLOAT_EQ(150.0, ControlObject::getControl(ConfigKey("[Master]", "sync_bpm"))->get());

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
    ASSERT_FLOAT_EQ(0.9375, ControlObject::getControl(ConfigKey(m_sGroup1, "rateEngine"))->get());
    ASSERT_FLOAT_EQ(1.25, ControlObject::getControl(ConfigKey(m_sGroup2, "rateEngine"))->get());

    // Set the internal rate to 80.
    ControlObject::getControl(ConfigKey("[Master]", "sync_slider"))->set(80.0);

    // Update COs again.
    ProcessBuffer();

    ASSERT_FLOAT_EQ(80.0, ControlObject::getControl(ConfigKey("[Master]", "sync_bpm"))->get());

    // Rate sliders for channels 1 and 2 should change appropriately.
    ASSERT_FLOAT_EQ(getRateSliderValue(0.5),
                    ControlObject::getControl(ConfigKey(m_sGroup1, "rate"))->get());
    ASSERT_FLOAT_EQ(80.0, ControlObject::getControl(ConfigKey(m_sGroup1, "bpm"))->get());
    ASSERT_FLOAT_EQ(getRateSliderValue(0.6666667),
                    ControlObject::getControl(ConfigKey(m_sGroup2, "rate"))->get());
    ASSERT_FLOAT_EQ(80.0, ControlObject::getControl(ConfigKey(m_sGroup2, "bpm"))->get());
    ASSERT_FLOAT_EQ(0.5, ControlObject::getControl(ConfigKey(m_sGroup1, "rateEngine"))->get());
    ASSERT_FLOAT_EQ(0.6666667, ControlObject::getControl(ConfigKey(m_sGroup2, "rateEngine"))->get());
}

TEST_F(EngineSyncTest, MasterStopSliderCheck) {
	// If the master is playing, and stop is pushed, the sliders should stay the same.
	ControlObject::getControl(ConfigKey(m_sGroup1, "file_bpm"))->set(120.0);
	ControlObject::getControl(ConfigKey(m_sGroup2, "file_bpm"))->set(128.0);

	QScopedPointer<ControlObjectThread> pButtonMasterSync1(getControlObjectThread(
            ConfigKey(m_sGroup1, "sync_mode")));
    pButtonMasterSync1->slotSet(SYNC_MASTER);
    QScopedPointer<ControlObjectThread> pButtonMasterSync2(getControlObjectThread(
            ConfigKey(m_sGroup2, "sync_mode")));
    pButtonMasterSync2->slotSet(SYNC_SLAVE);

	QScopedPointer<ControlObjectThread> pChannel1Play(getControlObjectThread(
            ConfigKey(m_sGroup1, "play")));
	pChannel1Play->set(1.0);
	QScopedPointer<ControlObjectThread> pChannel2Play(getControlObjectThread(
            ConfigKey(m_sGroup2, "play")));
    pChannel2Play->set(1.0);

    ProcessBuffer();

    ASSERT_FLOAT_EQ(0.9375, ControlObject::getControl(ConfigKey(m_sGroup2, "rateEngine"))->get());
    ASSERT_FLOAT_EQ(120.0, ControlObject::getControl(ConfigKey(m_sGroup2, "bpm"))->get());
    ASSERT_FLOAT_EQ(getRateSliderValue(0.9375),
                    ControlObject::getControl(ConfigKey(m_sGroup2, "rate"))->get());

    pChannel1Play->set(0.0);

    ProcessBuffer();

    ASSERT_FLOAT_EQ(0.0, ControlObject::getControl(ConfigKey(m_sGroup2, "rateEngine"))->get());
    EXPECT_FLOAT_EQ(120.0, ControlObject::getControl(ConfigKey(m_sGroup2, "bpm"))->get());
    EXPECT_FLOAT_EQ(getRateSliderValue(0.9375),
                    ControlObject::getControl(ConfigKey(m_sGroup2, "rate"))->get());
}

}  // namespace
