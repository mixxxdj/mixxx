#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <QtDebug>

#include "defs.h"
#include "configobject.h"
#include "deck.h"
#include "engine/enginebuffer.h"
#include "engine/enginechannel.h"
#include "engine/enginemaster.h"
#include "engine/enginesync.h"
#include "engine/ratecontrol.h"

#include "mixxxtest.h"

using ::testing::Return;
using ::testing::_;

namespace {

class EngineSyncTest : public MixxxTest {
  protected:
    virtual void SetUp() {
        m_pNumDecks = new ControlObject(ConfigKey("[Master]", "num_decks"));
        m_pNumDecks->set(2);

        m_pEngineMaster = new EngineMaster(m_pConfig.data(), "[Master]", false, false);
        m_pChannel1 = new EngineDeck(m_sGroup1, m_pConfig.data(), m_pEngineMaster, EngineChannel::CENTER);
        m_pChannel2 = new EngineDeck(m_sGroup2, m_pConfig.data(), m_pEngineMaster, EngineChannel::CENTER);
        m_pEngineSync = m_pEngineMaster->getEngineSync();
        m_pRateControl1 = m_pEngineSync->addDeck(m_sGroup1);
        m_pRateControl2 = m_pEngineSync->addDeck(m_sGroup2);

        m_pEngineSync->addChannel(m_pChannel1);
        m_pEngineSync->addChannel(m_pChannel2);
    }

    virtual void TearDown() {
        // I get crashes if I delete this.  Better to just leak like a sieve.
        //delete m_pEngineMaster;
        delete m_pNumDecks;

        delete m_pChannel1;
        delete m_pChannel2;

        // Clean up the rest of the controls.
        QList<ControlDoublePrivate*> leakedControls;
        QList<ConfigKey> leakedConfigKeys;

        ControlDoublePrivate::getControls(&leakedControls);
        int count = leakedControls.size();
        while (leakedControls.size() > 0) {
            foreach (ControlDoublePrivate* pCOP, leakedControls) {
                ConfigKey key = pCOP->getKey();
                leakedConfigKeys.append(key);
            }

            foreach (ConfigKey key, leakedConfigKeys) {
                ControlObject* pCo = ControlObject::getControl(key, false);
                if (pCo) {
                    delete pCo;
                }
            }

            ControlDoublePrivate::getControls(&leakedControls);
            // Sometimes we can't delete all of the controls.  Give up.
            if (leakedControls.size() == count) {
                break;
            }
            count = leakedControls.size();
        }
    }

    ControlObject* m_pNumDecks;

    EngineSync* m_pEngineSync;
    EngineMaster* m_pEngineMaster;
    RateControl *m_pRateControl1, *m_pRateControl2;
    EngineDeck *m_pChannel1, *m_pChannel2;

    const char* m_sGroup1 = "[Test1]";
    const char* m_sGroup2 = "[Test2]";
};

TEST_F(EngineSyncTest, ControlObjectsExist) {
    // This isn't exhaustive, but certain COs have a habit of not being set up properly.
    ASSERT_TRUE(m_pRateControl1 != NULL);
    ASSERT_TRUE(ControlObject::getControl(ConfigKey(m_sGroup1, "file_bpm")) != NULL);
    ASSERT_TRUE(m_pRateControl1->getRateEngineControl() != NULL);
}

TEST_F(EngineSyncTest, SetMasterSuccess) {
    // If we set the first channel to master, EngineSync should get that message.

    QScopedPointer<ControlObjectThread> pButtonMasterSync1(getControlObjectThread(
            ConfigKey(m_sGroup1, "sync_state")));
    pButtonMasterSync1->slotSet(SYNC_MASTER);

    // The master sync should now be channel 1.
    ASSERT_EQ(m_pChannel1, m_pEngineSync->getMaster());
    EXPECT_EQ(0, ControlObject::getControl(ConfigKey(m_sGroup1, "sync_slave"))->get());
    EXPECT_EQ(1, ControlObject::getControl(ConfigKey(m_sGroup1, "sync_master"))->get());

    QScopedPointer<ControlObjectThread> pButtonMasterSync2(getControlObjectThread(
            ConfigKey(m_sGroup2, "sync_state")));
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
}

TEST_F(EngineSyncTest, SetMasterByLights) {
    // Same as above, except we use the midi lights to change state.
    QScopedPointer<ControlObjectThread> pButtonMasterSync1(getControlObjectThread(
            ConfigKey(m_sGroup1, "sync_state")));
    QScopedPointer<ControlObjectThread> pButtonMasterSync2(getControlObjectThread(
            ConfigKey(m_sGroup2, "sync_state")));
    QScopedPointer<ControlObjectThread> pButtonSyncSlave1(getControlObjectThread(
            ConfigKey(m_sGroup1, "sync_slave")));
    QScopedPointer<ControlObjectThread> pButtonSyncSlave2(getControlObjectThread(
            ConfigKey(m_sGroup2, "sync_slave")));
    QScopedPointer<ControlObjectThread> pButtonSyncMaster1(getControlObjectThread(
            ConfigKey(m_sGroup1, "sync_master")));
    QScopedPointer<ControlObjectThread> pButtonSyncMaster2(getControlObjectThread(
            ConfigKey(m_sGroup2, "sync_master")));

    // Set channel 1 to be master.
    pButtonSyncMaster1->slotSet(1);

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
}


TEST_F(EngineSyncTest, RateChangeTest) {
    QScopedPointer<ControlObjectThread> pButtonMasterSync1(getControlObjectThread(
            ConfigKey(m_sGroup1, "sync_state")));
    pButtonMasterSync1->slotSet(SYNC_MASTER);
    QScopedPointer<ControlObjectThread> pButtonMasterSync2(getControlObjectThread(
            ConfigKey(m_sGroup2, "sync_state")));
    pButtonMasterSync2->slotSet(SYNC_SLAVE);

    // Set the file bpm of channel 1 to 160bpm.
    ControlObject::getControl(ConfigKey(m_sGroup1, "file_bpm"))->set(160.0);
    ASSERT_FLOAT_EQ(160.0, ControlObject::getControl(ConfigKey(m_sGroup1, "file_bpm"))->get());

    // Set the rate of channel 1 to 1.2 (which, because of how the slider works, is 0.2).
    ControlObject::getControl(ConfigKey(m_sGroup1, "rate"))->set(0.2);
    ASSERT_FLOAT_EQ(0.2, ControlObject::getControl(ConfigKey(m_sGroup1, "rate"))->get());

    // Set the file bpm of channel 2 to 120bpm.
    ControlObject::getControl(ConfigKey(m_sGroup2, "file_bpm"))->set(120.0);
    ASSERT_FLOAT_EQ(120.0, ControlObject::getControl(ConfigKey(m_sGroup2, "file_bpm"))->get());

    // rate slider for channel 2 should now be 1.6 = 160 * 1.2 / 120.
    ASSERT_FLOAT_EQ(0.6, ControlObject::getControl(ConfigKey(m_sGroup2, "rate"))->get());
}

TEST_F(EngineSyncTest, RateChangeTestWeirdOrder) {
    // This is like the test above, but the user loads the track after the slider has been tweaked.
    QScopedPointer<ControlObjectThread> pButtonMasterSync1(getControlObjectThread(
            ConfigKey(m_sGroup1, "sync_state")));
    pButtonMasterSync1->slotSet(SYNC_MASTER);
    QScopedPointer<ControlObjectThread> pButtonMasterSync2(getControlObjectThread(
            ConfigKey(m_sGroup2, "sync_state")));
    pButtonMasterSync2->slotSet(SYNC_SLAVE);

    // Set the file bpm of channel 1 to 160bpm.
    ControlObject::getControl(ConfigKey(m_sGroup1, "file_bpm"))->set(160.0);

    // Set the file bpm of channel 2 to 120bpm.
    ControlObject::getControl(ConfigKey(m_sGroup2, "file_bpm"))->set(120.0);

    // Set the rate slider of channel 1 to 1.2.
    ControlObject::getControl(ConfigKey(m_sGroup1, "rate"))->set(0.2);

    // rate slider for channel 2 should now be 1.6 = 160 * 1.2 / 120.
    ASSERT_FLOAT_EQ(0.6, ControlObject::getControl(ConfigKey(m_sGroup2, "rate"))->get());
}


}  // namespace
