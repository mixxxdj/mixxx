#include <gtest/gtest.h>

#include <QtDebug>
#include <QScopedPointer>

#include "engine/cachingreader.h"
#include "control/controlobject.h"
#include "engine/loopingcontrol.h"
#include "engine/readaheadmanager.h"
#include "test/mixxxtest.h"
#include "util/assert.h"
#include "util/defs.h"
#include "util/sample.h"

class StubReader : public CachingReader {
  public:
    StubReader()
            : CachingReader("[test]", UserSettingsPointer()) { }

    SINT read(SINT sample, bool reverse, SINT num_samples,
             CSAMPLE* buffer) override {
        Q_UNUSED(sample);
        Q_UNUSED(reverse);
        SampleUtil::clear(buffer, num_samples);
        return num_samples;
    }
};

class StubLoopControl : public LoopingControl {
  public:
    StubLoopControl()
            : LoopingControl("[test]", UserSettingsPointer()) { }

    void pushTriggerReturnValue(double value) {
        m_triggerReturnValues.push_back(value);
    }

    void pushProcessReturnValue(double value) {
        m_processReturnValues.push_back(value);
    }

    double nextTrigger(const double dRate,
                       const double currentSample,
                       const double totalSamples,
                       const int iBufferSize) override {
        Q_UNUSED(dRate);
        Q_UNUSED(currentSample);
        Q_UNUSED(totalSamples);
        Q_UNUSED(iBufferSize);
        RELEASE_ASSERT(!m_triggerReturnValues.isEmpty());
        return m_triggerReturnValues.takeFirst();
    }

    double process(const double dRate,
                   const double dCurrentSample,
                   const double dTotalSamples,
                   const int iBufferSize) override {
        Q_UNUSED(dRate);
        Q_UNUSED(dCurrentSample);
        Q_UNUSED(dTotalSamples);
        Q_UNUSED(iBufferSize);
        RELEASE_ASSERT(!m_processReturnValues.isEmpty());
        return m_processReturnValues.takeFirst();
    }

    // getTrigger returns the sample that the engine will next be triggered to
    // loop to, given the value of currentSample and dRate.
    double getTrigger(const double dRate,
                      const double currentSample,
                      const double totalSamples,
                      const int iBufferSize) override {
        Q_UNUSED(dRate);
        Q_UNUSED(currentSample);
        Q_UNUSED(totalSamples);
        Q_UNUSED(iBufferSize);
        return kNoTrigger;
    }

    // hintReader has no effect in this stubbed class
    void hintReader(HintVector* pHintList) override {
        Q_UNUSED(pHintList);
    }

    void notifySeek(double dNewPlaypos) {
        Q_UNUSED(dNewPlaypos);
    }

  public slots:
    void trackLoaded(TrackPointer pTrack, TrackPointer pOldTrack) override {
        Q_UNUSED(pTrack);
        Q_UNUSED(pOldTrack);
    }

  protected:
    QList<double> m_triggerReturnValues;
    QList<double> m_processReturnValues;
};

class ReadAheadManagerTest : public MixxxTest {
  public:
    ReadAheadManagerTest() : m_pBuffer(SampleUtil::alloc(MAX_BUFFER_LEN)) { }
  protected:
    virtual void SetUp() {
        SampleUtil::clear(m_pBuffer, MAX_BUFFER_LEN);
        m_pReader.reset(new StubReader());
        m_pLoopControl.reset(new StubLoopControl());
        m_pReadAheadManager.reset(new ReadAheadManager(m_pReader.data(),
                                                       m_pLoopControl.data()));
    }

    QScopedPointer<StubReader> m_pReader;
    QScopedPointer<StubLoopControl> m_pLoopControl;
    QScopedPointer<ReadAheadManager> m_pReadAheadManager;
    CSAMPLE* m_pBuffer;
};

TEST_F(ReadAheadManagerTest, LoopEnableSeekBackward) {
    // If a loop is enabled and the current playposition is ahead of the loop,
    // we should seek to the beginning of the loop.
    m_pReadAheadManager->notifySeek(110);
    // Trigger value means, the sample that triggers the loop (loop out)
    m_pLoopControl->pushTriggerReturnValue(100);
    // Process value is the sample we should seek to
    m_pLoopControl->pushProcessReturnValue(10);
    EXPECT_EQ(0, m_pReadAheadManager->getNextSamples(1.0, m_pBuffer, 100));
    EXPECT_EQ(10, m_pReadAheadManager->getPlaypos());
}

TEST_F(ReadAheadManagerTest, InReverseLoopEnableSeekForward) {
    // If we are in reverse, a loop is enabled, and the current playposition
    // is before of the loop, we should seek to the out point of the loop.
    m_pReadAheadManager->notifySeek(1);
    // Trigger value means, the sample that triggers the loop (loop in)
    m_pLoopControl->pushTriggerReturnValue(10);
    // Process value is the sample we should seek to.
    m_pLoopControl->pushProcessReturnValue(100);
    EXPECT_EQ(0, m_pReadAheadManager->getNextSamples(-1.0, m_pBuffer, 100));
    EXPECT_EQ(100, m_pReadAheadManager->getPlaypos());
}
