#ifndef MOCKENGINEBACKENDTEST_H_
#define MOCKENGINEBACKENDTEST_H_

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <QtDebug>

#include "util/types.h"
#include "util/defs.h"
#include "configobject.h"
#include "controlobject.h"
#include "deck.h"
#include "effects/effectsmanager.h"
#include "engine/enginebuffer.h"
#include "engine/enginebufferscale.h"
#include "engine/enginechannel.h"
#include "engine/enginedeck.h"
#include "engine/enginemaster.h"
#include "engine/sync/enginesync.h"
#include "engine/ratecontrol.h"
#include "previewdeck.h"
#include "sampler.h"
#include "sampleutil.h"

#include "mixxxtest.h"

using ::testing::Return;
using ::testing::_;

class MockScaler : public EngineBufferScale {
  public:
    MockScaler() : EngineBufferScale() {
        SampleUtil::clear(m_buffer, MAX_BUFFER_LEN);
    }
    void clear() { }
    CSAMPLE *getScaled(unsigned long buf_size) {
        m_samplesRead = round(buf_size * m_dSpeedAdjust);
        if (static_cast<int>(m_samplesRead) % 2) { m_samplesRead--; }
        return m_buffer;
    }
};


class MockedEngineBackendTest : public MixxxTest {
  protected:
    virtual void SetUp() {
        m_pNumDecks = new ControlObject(ConfigKey("[Master]", "num_decks"));
        m_pEffectsManager = new EffectsManager(NULL, config());
        m_pEngineMaster = new EngineMaster(m_pConfig.data(), "[Master]",
                                           m_pEffectsManager, false, false);

        m_pChannel1 = new EngineDeck(m_sGroup1, m_pConfig.data(),
                                     m_pEngineMaster, m_pEffectsManager,
                                     EngineChannel::CENTER);
        m_pChannel2 = new EngineDeck(m_sGroup2, m_pConfig.data(),
                                     m_pEngineMaster, m_pEffectsManager,
                                     EngineChannel::CENTER);
        m_pChannel3 = new EngineDeck(m_sGroup3, m_pConfig.data(),
                                     m_pEngineMaster, m_pEffectsManager,
                                     EngineChannel::CENTER);
        m_pPreview1 = new PreviewDeck(NULL, m_pConfig.data(),
                                     m_pEngineMaster, m_pEffectsManager,
                                     EngineChannel::CENTER, m_sPreviewGroup);
        ControlObject::getControl(ConfigKey(m_sPreviewGroup, "file_bpm"))->set(2.0);
        // TODO(owilliams) Tests fail with this turned on because EngineSync is syncing
        // to this sampler.  FIX IT!
//        m_pSampler1 = new Sampler(NULL, m_pConfig.data(),
//                                  m_pEngineMaster, m_pEffectsManager,
//                                  EngineChannel::CENTER, m_sSamplerGroup);
//        ControlObject::getControl(ConfigKey(m_sSamplerGroup, "file_bpm"))->set(2.0);

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
        m_pTrack1 = m_pChannel1->getEngineBuffer()->loadFakeTrack();
        m_pTrack2 = m_pChannel2->getEngineBuffer()->loadFakeTrack();
        m_pTrack3 = m_pChannel3->getEngineBuffer()->loadFakeTrack();
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

        // Deletes all EngineChannels added to it.
        delete m_pEngineMaster;
        delete m_pEffectsManager;
        delete m_pMockScaler1;
        delete m_pMockScaler2;
        delete m_pMockScaler3;
        delete m_pNumDecks;
    }

    double getRateSliderValue(double rate) const {
        return (rate - 1.0) / kRateRangeDivisor;
    }

    void ProcessBuffer() {
        m_pEngineMaster->process(1024);
    }

    ControlObject* m_pNumDecks;

    EffectsManager* m_pEffectsManager;
    EngineSync* m_pEngineSync;
    EngineMaster* m_pEngineMaster;
    EngineDeck *m_pChannel1, *m_pChannel2, *m_pChannel3;
    MockScaler *m_pMockScaler1, *m_pMockScaler2, *m_pMockScaler3;
    TrackPointer m_pTrack1, m_pTrack2, m_pTrack3;
    PreviewDeck *m_pPreview1;
    Sampler *m_pSampler1;

    static const char* m_sGroup1;
    static const char* m_sGroup2;
    static const char* m_sGroup3;
    static const char* m_sMasterGroup;
    static const char* m_sInternalClockGroup;
    static const char* m_sPreviewGroup;
    static const char* m_sSamplerGroup;
    static const double kDefaultRateRange;
    static const double kDefaultRateDir;
    static const double kRateRangeDivisor;
};

#endif /* MOCKEDENGINEBACKENDTEST_H_ */
