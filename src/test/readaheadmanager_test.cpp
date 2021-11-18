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
const QString kChannelGroup = "[test]";
}

class StubReader : public CachingReader {
  public:
    StubReader()
            : CachingReader(kChannelGroup, UserSettingsPointer()) {
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
            : LoopingControl(kChannelGroup, UserSettingsPointer()) {
    }

    void pushTriggerReturnValue(double value) {
        m_triggerReturnValues.push_back(
                mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(value));
    }

    void pushTargetReturnValue(double value) {
        m_targetReturnValues.push_back(
                mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(value));
    }

    mixxx::audio::FramePos nextTrigger(bool reverse,
            mixxx::audio::FramePos currentPosition,
            mixxx::audio::FramePos* pTargetPosition) override {
        Q_UNUSED(reverse);
        Q_UNUSED(currentPosition);
        Q_UNUSED(pTargetPosition);
        RELEASE_ASSERT(!m_targetReturnValues.isEmpty());
        *pTargetPosition = m_targetReturnValues.takeFirst();
        RELEASE_ASSERT(!m_triggerReturnValues.isEmpty());
        return m_triggerReturnValues.takeFirst();
    }

  protected:
    QList<mixxx::audio::FramePos> m_triggerReturnValues;
    QList<mixxx::audio::FramePos> m_targetReturnValues;
};

class ReadAheadManagerTest : public MixxxTest {
  public:
    ReadAheadManagerTest()
            : m_beatClosestCO(ConfigKey(kChannelGroup, "beat_closest")),
              m_beatNextCO(ConfigKey(kChannelGroup, "beat_next")),
              m_beatPrevCO(ConfigKey(kChannelGroup, "beat_prev")),
              m_playCO(ConfigKey(kChannelGroup, "play")),
              m_quantizeCO(ConfigKey(kChannelGroup, "quantize")),
              m_slipEnabledCO(ConfigKey(kChannelGroup, "slip_enabled")),
              m_trackSamplesCO(ConfigKey(kChannelGroup, "track_samples")),
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
