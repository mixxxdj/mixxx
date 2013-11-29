#ifndef MOCKENGINEBACKEND_H_
#define MOCKENGINEBACKEND_H_

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <QtDebug>

#include "defs.h"
#include "configobject.h"
#include "controlobject.h"
#include "deck.h"
#include "engine/enginebuffer.h"
#include "engine/enginebufferscale.h"
#include "engine/enginechannel.h"
#include "engine/enginedeck.h"
#include "engine/enginemaster.h"
#include "engine/enginesync.h"
#include "engine/ratecontrol.h"
#include "sampleutil.h"

#include "mixxxtest.h"

using ::testing::Return;
using ::testing::_;

class MockScaler : public EngineBufferScale {
  public:
	MockScaler() :
			EngineBufferScale(),
			m_dSamplesRead(0) {
		SampleUtil::applyGain(m_buffer, 0, MAX_BUFFER_LEN);

	}
    void setBaseRate(double dBaseRate) { m_dBaseRate = dBaseRate; }
    double setTempo(double dTempo) { return m_dTempo = dTempo; }
    double getSamplesRead() { return m_dSamplesRead; }
    void clear() { }
    CSAMPLE *getScaled(unsigned long buf_size) {
    	m_dSamplesRead += buf_size;
    	return m_buffer;
    }

  private:
    double m_dSamplesRead;
};


class MockedEngineBackendTest : public MixxxTest {
  protected:
    virtual void SetUp() {
        m_pBuffer = new CSAMPLE[MAX_BUFFER_LEN];
        m_pNumDecks = new ControlObject(ConfigKey("[Master]", "num_decks"));
        m_pNumDecks->set(2);

        m_pEngineMaster = new EngineMaster(m_pConfig.data(), "[Master]", false, false);
        m_pChannel1 = new EngineDeck(m_sGroup1, m_pConfig.data(), m_pEngineMaster, EngineChannel::CENTER);
        ControlObject::getControl(ConfigKey(m_sGroup1, "master"))->set(1);
        m_pChannel2 = new EngineDeck(m_sGroup2, m_pConfig.data(), m_pEngineMaster, EngineChannel::CENTER);
        ControlObject::getControl(ConfigKey(m_sGroup2, "master"))->set(1);
        m_pEngineMaster->addChannel(m_pChannel1);
        m_pEngineMaster->addChannel(m_pChannel2);
        m_pEngineSync = m_pEngineMaster->getEngineSync();
        m_pRateControl1 = m_pEngineSync->getRateControlForGroup(m_sGroup1);
        m_pRateControl2 = m_pEngineSync->getRateControlForGroup(m_sGroup2);

        m_pMockScaler1 = new MockScaler();
        m_pMockScaler2 = new MockScaler();
        m_pChannel1->getEngineBuffer()->setScaler(m_pMockScaler1);
        m_pChannel2->getEngineBuffer()->setScaler(m_pMockScaler2);
        m_pChannel1->getEngineBuffer()->loadFakeTrack();
        m_pChannel2->getEngineBuffer()->loadFakeTrack();

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

        delete m_pMockScaler1;
        delete m_pMockScaler2;
    }

    double getRateSliderValue(double rate) const {
        return (rate - 1.0) / kRateRangeDivisor;
    }

    void ProcessBuffer() {
        m_pEngineMaster->process(NULL, m_pBuffer, 1024);
    }

    ControlObject* m_pNumDecks;

    EngineSync* m_pEngineSync;
    EngineMaster* m_pEngineMaster;
    RateControl *m_pRateControl1, *m_pRateControl2;
    EngineDeck *m_pChannel1, *m_pChannel2;
    MockScaler *m_pMockScaler1, *m_pMockScaler2;

    CSAMPLE *m_pBuffer;

    static const char* m_sGroup1;
    static const char* m_sGroup2;
    static const double kRateRangeDivisor;
};


const char* MockedEngineBackendTest::m_sGroup1 = "[Test1]";
const char* MockedEngineBackendTest::m_sGroup2 = "[Test2]";
const double MockedEngineBackendTest::kRateRangeDivisor = 4.0;


#endif /* MOCKEDDECK_H_ */
