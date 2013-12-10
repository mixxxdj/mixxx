#ifndef MOCKENGINEBACKENDTEST_H_
#define MOCKENGINEBACKENDTEST_H_

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
    MockScaler() : EngineBufferScale() {
        SampleUtil::applyGain(m_buffer, 0, MAX_BUFFER_LEN);
    }
    void setScaleParameters(int iSampleRate,
                            double* rate_adjust,
                            double* tempo_adjust,
                            double* pitch_adjust) {
        m_iSampleRate = iSampleRate;
        m_dRateAdjust = *rate_adjust;
        m_dTempoAdjust = *tempo_adjust;
        m_dPitchAdjust = *pitch_adjust;
    }
    void clear() { }
    CSAMPLE *getScaled(unsigned long buf_size) {
        m_samplesRead += buf_size;
        return m_buffer;
    }
};


class MockedEngineBackendTest : public MixxxTest {
  protected:
    virtual void SetUp() {
        m_pBuffer = new CSAMPLE[MAX_BUFFER_LEN];
        m_pNumDecks = new ControlObject(ConfigKey("[Master]", "num_decks"));
        m_pNumDecks->set(3);

        m_pEngineMaster = new EngineMaster(m_pConfig.data(), "[Master]", false, false);
        m_pChannel1 = new EngineDeck(m_sGroup1, m_pConfig.data(), m_pEngineMaster, EngineChannel::CENTER);
        ControlObject::getControl(ConfigKey(m_sGroup1, "master"))->set(1);
        m_pChannel2 = new EngineDeck(m_sGroup2, m_pConfig.data(), m_pEngineMaster, EngineChannel::CENTER);
        ControlObject::getControl(ConfigKey(m_sGroup2, "master"))->set(1);
        m_pChannel3 = new EngineDeck(m_sGroup3, m_pConfig.data(), m_pEngineMaster, EngineChannel::CENTER);
        ControlObject::getControl(ConfigKey(m_sGroup3, "master"))->set(1);
        m_pEngineMaster->addChannel(m_pChannel1);
        m_pEngineMaster->addChannel(m_pChannel2);
        m_pEngineMaster->addChannel(m_pChannel3);
        m_pEngineSync = m_pEngineMaster->getEngineSync();
        m_pRateControl1 = m_pEngineSync->getRateControlForGroup(m_sGroup1);
        m_pRateControl2 = m_pEngineSync->getRateControlForGroup(m_sGroup2);
        m_pRateControl3 = m_pEngineSync->getRateControlForGroup(m_sGroup3);

        m_pMockScaler1 = new MockScaler();
        m_pMockScaler2 = new MockScaler();
        m_pMockScaler3 = new MockScaler();
        m_pChannel1->getEngineBuffer()->setScalerForTest(m_pMockScaler1);
        m_pChannel2->getEngineBuffer()->setScalerForTest(m_pMockScaler2);
        m_pChannel3->getEngineBuffer()->setScalerForTest(m_pMockScaler3);
        m_pChannel1->getEngineBuffer()->loadFakeTrack();
        m_pChannel2->getEngineBuffer()->loadFakeTrack();
        m_pChannel3->getEngineBuffer()->loadFakeTrack();

        m_pEngineSync->addChannel(m_pChannel1);
        m_pEngineSync->addChannel(m_pChannel2);
        m_pEngineSync->addChannel(m_pChannel3);
    }

    virtual void TearDown() {
        // I get crashes if I delete this.  Better to just leak like a sieve.
        //delete m_pEngineMaster;
        delete m_pNumDecks;

        delete m_pChannel1;
        delete m_pChannel2;
        delete m_pChannel3;

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
        delete m_pMockScaler3;
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
    RateControl *m_pRateControl1, *m_pRateControl2, *m_pRateControl3;
    EngineDeck *m_pChannel1, *m_pChannel2, *m_pChannel3;
    MockScaler *m_pMockScaler1, *m_pMockScaler2, *m_pMockScaler3;

    CSAMPLE *m_pBuffer;

    static const char* m_sGroup1;
    static const char* m_sGroup2;
    static const char* m_sGroup3;
    static const char* m_sMasterGroup;
    static const char* m_sInternalClockGroup;
    static const double kRateRangeDivisor;
};

const char* MockedEngineBackendTest::m_sMasterGroup = "[Master]";
const char* MockedEngineBackendTest::m_sInternalClockGroup = "[InternalClock]";
const char* MockedEngineBackendTest::m_sGroup1 = "[Test1]";
const char* MockedEngineBackendTest::m_sGroup2 = "[Test2]";
const char* MockedEngineBackendTest::m_sGroup3 = "[Test3]";
// This value is 2x the default rate range set in ratecontrol.cpp.
const double MockedEngineBackendTest::kRateRangeDivisor = 4.0;


#endif /* MOCKEDENGINEBACKENDTEST_H_ */
