#pragma once

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <QtDebug>

#include "preferences/usersettings.h"
#include "control/controlobject.h"
#include "mixer/deck.h"
#include "effects/effectsmanager.h"
#include "engine/enginebuffer.h"
#include "engine/bufferscalers/enginebufferscale.h"
#include "engine/channels/enginechannel.h"
#include "engine/channels/enginedeck.h"
#include "engine/enginemaster.h"
#include "engine/controls/ratecontrol.h"
#include "engine/sync/enginesync.h"
#include "mixer/deck.h"
#include "mixer/previewdeck.h"
#include "mixer/sampler.h"
#include "test/signalpathtest.h"
#include "util/defs.h"
#include "util/memory.h"
#include "util/sample.h"
#include "util/types.h"
#include "waveform/guitick.h"

using ::testing::Return;
using ::testing::_;

class MockScaler : public EngineBufferScale {
  public:
    MockScaler()
            : EngineBufferScale(),
              m_processedTempo(-1),
              m_processedPitch(-1) {
    }
    void clear() override { }
    double scaleBuffer(CSAMPLE* pOutput, SINT buf_size) override {
        Q_UNUSED(pOutput);
        m_processedTempo = m_dTempoRatio;
        m_processedPitch = m_dPitchRatio;
        DEBUG_ASSERT((buf_size % 2) == 0); // 2 channels
        SINT numFrames = buf_size / 2;
        double framesRead = numFrames * m_dTempoRatio;
        return framesRead;
    }

    double getProcessedTempo() {
        return m_processedTempo;
    }

    double getProcessedPitch() {
        return m_processedPitch;
    }

  private:
    void onSampleRateChanged() override {}

    double m_processedTempo;
    double m_processedPitch;
};

class MockedEngineBackendTest : public BaseSignalPathTest {
  protected:
    MockedEngineBackendTest() {
        m_pMockScaleVinyl1 = new MockScaler();
        m_pMockScaleKeylock1 = new MockScaler();
        m_pMockScaleVinyl2 = new MockScaler();
        m_pMockScaleKeylock2 = new MockScaler();
        m_pMockScaleVinyl3 = new MockScaler();
        m_pMockScaleKeylock3 = new MockScaler();
        m_pChannel1->getEngineBuffer()->setScalerForTest(m_pMockScaleVinyl1,
                                                         m_pMockScaleKeylock1);
        m_pChannel2->getEngineBuffer()->setScalerForTest(m_pMockScaleVinyl2,
                                                         m_pMockScaleKeylock2);
        m_pChannel3->getEngineBuffer()->setScalerForTest(m_pMockScaleVinyl3,
                                                         m_pMockScaleKeylock3);
        m_pTrack1 = m_pMixerDeck1->loadFakeTrack(false, 0.0);
        m_pTrack2 = m_pMixerDeck2->loadFakeTrack(false, 0.0);
        m_pTrack3 = m_pMixerDeck3->loadFakeTrack(false, 0.0);
    }

    ~MockedEngineBackendTest() override {
        delete m_pMockScaleVinyl1;
        delete m_pMockScaleVinyl2;
        delete m_pMockScaleVinyl3;
        delete m_pMockScaleKeylock1;
        delete m_pMockScaleKeylock2;
        delete m_pMockScaleKeylock3;
    }

    MockScaler *m_pMockScaleVinyl1, *m_pMockScaleVinyl2, *m_pMockScaleVinyl3;
    MockScaler *m_pMockScaleKeylock1, *m_pMockScaleKeylock2, *m_pMockScaleKeylock3;
    TrackPointer m_pTrack1, m_pTrack2, m_pTrack3;
};
