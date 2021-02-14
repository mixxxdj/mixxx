#include <gtest/gtest.h>

#include <QtDebug>
#include <QScopedPointer>

#include "engine/cachingreader/cachingreader.h"
#include "control/controlobject.h"
#include "engine/controls/loopingcontrol.h"
#include "engine/readaheadmanager.h"
#include "test/mixxxtest.h"
#include "util/assert.h"
#include "util/defs.h"
#include "util/sample.h"

namespace {
const QString kGroup = "[test]";
} // namespace

class StubReader : public CachingReader {
  public:
    StubReader()
            : CachingReader(kGroup, UserSettingsPointer()) {
    }

    CachingReader::ReadResult read(SINT startSample, SINT numSamples, bool reverse,
             CSAMPLE* buffer) override {
        Q_UNUSED(startSample);
        Q_UNUSED(reverse);
        SampleUtil::clear(buffer, numSamples);
        return CachingReader::ReadResult::AVAILABLE;
    }
};

class StubLoopControl : public LoopingControl {
  public:
    StubLoopControl()
            : LoopingControl(kGroup, UserSettingsPointer()) {
    }

    void pushTriggerReturnValue(double value) {
        m_triggerReturnValues.push_back(value);
    }

    void pushTargetReturnValue(double value) {
        m_targetReturnValues.push_back(value);
    }

    double nextTrigger(bool reverse,
                       const double currentSample,
                       double* pTarget) override {
        Q_UNUSED(reverse);
        Q_UNUSED(currentSample);
        Q_UNUSED(pTarget);
        RELEASE_ASSERT(!m_targetReturnValues.isEmpty());
        *pTarget = m_targetReturnValues.takeFirst();
        RELEASE_ASSERT(!m_triggerReturnValues.isEmpty());
        return m_triggerReturnValues.takeFirst();
    }

    // hintReader has no effect in this stubbed class
    void hintReader(HintVector* pHintList) override {
        Q_UNUSED(pHintList);
    }

    void notifySeek(double dNewPlaypos) override {
        Q_UNUSED(dNewPlaypos);
    }

    void trackLoaded(TrackPointer pTrack) override {
        Q_UNUSED(pTrack);
    }

  protected:
    QList<double> m_triggerReturnValues;
    QList<double> m_targetReturnValues;
};

class ReadAheadManagerTest : public MixxxTest {
  public:
    ReadAheadManagerTest()
            : m_beatClosestCO(ConfigKey(kGroup, "beat_closest")),
              m_beatNextCO(ConfigKey(kGroup, "beat_next")),
              m_beatPrevCO(ConfigKey(kGroup, "beat_prev")),
              m_playCO(ConfigKey(kGroup, "play")),
              m_quantizeCO(ConfigKey(kGroup, "quantize")),
              m_slipEnabledCO(ConfigKey(kGroup, "slip_enabled")),
              m_trackSamplesCO(ConfigKey(kGroup, "track_samples")),
              m_pBuffer(SampleUtil::alloc(MAX_BUFFER_LEN)) {
    }

  protected:
    void SetUp() override {
        SampleUtil::clear(m_pBuffer, MAX_BUFFER_LEN);
        m_pReader.reset(new StubReader());
        m_pLoopControl.reset(new StubLoopControl());
        m_pReadAheadManager.reset(new ReadAheadManager(m_pReader.data(),
                                                       m_pLoopControl.data()));
    }

    QScopedPointer<StubReader> m_pReader;
    QScopedPointer<StubLoopControl> m_pLoopControl;
    QScopedPointer<ReadAheadManager> m_pReadAheadManager;
    ControlObject m_beatClosestCO;
    ControlObject m_beatNextCO;
    ControlObject m_beatPrevCO;
    ControlObject m_playCO;
    ControlObject m_quantizeCO;
    ControlObject m_slipEnabledCO;
    ControlObject m_trackSamplesCO;
    CSAMPLE* m_pBuffer;
};

TEST_F(ReadAheadManagerTest, FractionalFrameLoop) {
    // If we are in reverse, a loop is enabled, and the current playposition
    // is before of the loop, we should seek to the out point of the loop.
    m_pReadAheadManager->notifySeek(0.5);
    // Trigger value means, the sample that triggers the loop (loop in)
    m_pLoopControl->pushTriggerReturnValue(20.2);
    m_pLoopControl->pushTriggerReturnValue(20.2);
    m_pLoopControl->pushTriggerReturnValue(20.2);
    m_pLoopControl->pushTriggerReturnValue(20.2);
    m_pLoopControl->pushTriggerReturnValue(20.2);
    m_pLoopControl->pushTriggerReturnValue(20.2);
    // Process value is the sample we should seek to.
    m_pLoopControl->pushTargetReturnValue(3.3);
    m_pLoopControl->pushTargetReturnValue(3.3);
    m_pLoopControl->pushTargetReturnValue(3.3);
    m_pLoopControl->pushTargetReturnValue(3.3);
    m_pLoopControl->pushTargetReturnValue(3.3);
    m_pLoopControl->pushTargetReturnValue(kNoTrigger);
    // read from start to loop trigger, overshoot 0.3
    EXPECT_EQ(20, m_pReadAheadManager->getNextSamples(1.0, m_pBuffer, 100));
    // read loop
    EXPECT_EQ(18, m_pReadAheadManager->getNextSamples(1.0, m_pBuffer, 80));
    // read loop
    EXPECT_EQ(16, m_pReadAheadManager->getNextSamples(1.0, m_pBuffer, 62));
    // read loop
    EXPECT_EQ(18, m_pReadAheadManager->getNextSamples(1.0, m_pBuffer, 46));
    // read loop
    EXPECT_EQ(16, m_pReadAheadManager->getNextSamples(1.0, m_pBuffer, 28));
    // read loop
    EXPECT_EQ(12, m_pReadAheadManager->getNextSamples(1.0, m_pBuffer, 12));

    // start 0.5 to 20.2 = 19.7
    // loop 3.3 to 20.2 = 16.9
    // 100 - 19,7 - 4 * 16,9 = 12,7
    // 12.7 + 3.3 = 16

    // The rounding error must not exceed a half frame (one samples in stereo)
    EXPECT_NEAR(16, m_pReadAheadManager->getPlaypos(), 1);
}
