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
#include "engine/sync/enginesync.h"
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
        m_pBuffer = SampleUtil::alloc(MAX_BUFFER_LEN);
        m_pNumDecks = new ControlObject(ConfigKey("[Master]", "num_decks"));
        m_pEngineMaster = new EngineMaster(m_pConfig.data(), "[Master]", false, false);

        m_pChannel1 = new EngineDeck(m_sGroup1, m_pConfig.data(),
                                     m_pEngineMaster, EngineChannel::CENTER);
        m_pChannel2 = new EngineDeck(m_sGroup2, m_pConfig.data(),
                                     m_pEngineMaster, EngineChannel::CENTER);
        m_pChannel3 = new EngineDeck(m_sGroup3, m_pConfig.data(),
                                     m_pEngineMaster, EngineChannel::CENTER);

        addDeck(m_pChannel1);
        addDeck(m_pChannel2);
        addDeck(m_pChannel3);

        m_pEngineSync = m_pEngineMaster->getEngineSync();

        m_pMockScaler1 = new MockScaler();
        m_pMockScaler2 = new MockScaler();
        m_pMockScaler3 = new MockScaler();
        m_pChannel1->getEngineBuffer()->setScalerForTest(m_pMockScaler1);
        m_pChannel2->getEngineBuffer()->setScalerForTest(m_pMockScaler2);
        m_pChannel3->getEngineBuffer()->setScalerForTest(m_pMockScaler3);
        m_pChannel1->getEngineBuffer()->loadFakeTrack();
        m_pChannel2->getEngineBuffer()->loadFakeTrack();
        m_pChannel3->getEngineBuffer()->loadFakeTrack();
    }

    void addDeck(EngineDeck* pDeck) {
        m_pEngineMaster->addChannel(pDeck);
        ControlObject::getControl(ConfigKey(pDeck->getGroup(), "master"))
                ->set(1.0);
        ControlObject::getControl(ConfigKey(pDeck->getGroup(), "rate_dir"))
                ->set(kDefaultRateDir);
        ControlObject::getControl(ConfigKey(pDeck->getGroup(), "rateRange"))
                ->set(kDefaultRateRange);
        m_pNumDecks->set(m_pNumDecks->get() + 1);
    }

    virtual void TearDown() {
        m_pChannel1 = NULL;
        m_pChannel2 = NULL;
        m_pChannel3 = NULL;
        m_pEngineSync = NULL;
        SampleUtil::free(m_pBuffer);

        // Deletes all EngineChannels added to it.
        delete m_pEngineMaster;
        delete m_pMockScaler1;
        delete m_pMockScaler2;
        delete m_pMockScaler3;
        delete m_pNumDecks;
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
    EngineDeck *m_pChannel1, *m_pChannel2, *m_pChannel3;
    MockScaler *m_pMockScaler1, *m_pMockScaler2, *m_pMockScaler3;

    CSAMPLE *m_pBuffer;

    static const char* m_sGroup1;
    static const char* m_sGroup2;
    static const char* m_sGroup3;
    static const char* m_sMasterGroup;
    static const char* m_sInternalClockGroup;
    static const double kDefaultRateRange;
    static const double kDefaultRateDir;
    static const double kRateRangeDivisor;
};

#endif /* MOCKEDENGINEBACKENDTEST_H_ */
