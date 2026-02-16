#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QVector>
#include <QtDebug>

#include "engine/bufferscalers/enginebufferscalebungee.h"
#include "engine/readaheadmanager.h"
#include "test/mixxxtest.h"
#include "util/math.h"
#include "util/sample.h"
#include "util/types.h"

using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;
using ::testing::StrictMock;

namespace {

class ReadAheadManagerMock : public ReadAheadManager {
  public:
    ReadAheadManagerMock()
            : ReadAheadManager(),
              m_pBuffer(nullptr),
              m_iBufferSize(0),
              m_iReadPosition(0),
              m_iSamplesRead(0) {
    }

    SINT getNextSamplesFake(double dRate,
            CSAMPLE* buffer,
            SINT requested_samples,
            mixxx::audio::ChannelCount channelCount) {
        Q_UNUSED(dRate);
        Q_UNUSED(channelCount);
        bool hasBuffer = m_pBuffer != nullptr;
        // You forgot to set the mock read buffer.
        EXPECT_TRUE(hasBuffer);

        for (SINT i = 0; i < requested_samples; ++i) {
            buffer[i] = hasBuffer ? m_pBuffer[m_iReadPosition++ % m_iBufferSize] : 0;
        }
        m_iSamplesRead += requested_samples;
        return requested_samples;
    }

    void setReadBuffer(CSAMPLE* pBuffer, SINT iBufferSize) {
        m_pBuffer = pBuffer;
        m_iBufferSize = iBufferSize;
        m_iReadPosition = 0;
    }

    int getSamplesRead() {
        return m_iSamplesRead;
    }

    void resetReadCount() {
        m_iSamplesRead = 0;
    }

    MOCK_METHOD4(getNextSamples,
            SINT(double dRate,
                    CSAMPLE* buffer,
                    SINT requested_samples,
                    mixxx::audio::ChannelCount channelCount));

    CSAMPLE* m_pBuffer;
    SINT m_iBufferSize;
    SINT m_iReadPosition;
    SINT m_iSamplesRead;
};

class EngineBufferScaleBungeeTest : public MixxxTest {
  protected:
    void SetUp() override {
        m_pReadAheadMock = new StrictMock<ReadAheadManagerMock>();
        m_pScaler = new EngineBufferScaleBungee(m_pReadAheadMock);
        m_pScaler->setSignal(mixxx::audio::SampleRate(44100),
                mixxx::audio::ChannelCount::stereo());
    }

    void TearDown() override {
        delete m_pScaler;
        delete m_pReadAheadMock;
    }

    void SetRate(double tempoRatio, double pitchRatio = 1.0, double baseRate = 1.0) {
        double tempo = tempoRatio;
        double pitch = pitchRatio;
        m_pScaler->setScaleParameters(baseRate, &tempo, &pitch);
    }

    void ClearBuffer(CSAMPLE* pBuffer, int length) {
        SampleUtil::clear(pBuffer, length);
    }

    void FillBuffer(CSAMPLE* pBuffer, CSAMPLE value, int length) {
        SampleUtil::fill(pBuffer, value, length);
    }

    void AssertWholeBufferEquals(const CSAMPLE* pBuffer, CSAMPLE value, int iBufferLen) {
        for (int i = 0; i < iBufferLen; ++i) {
            EXPECT_FLOAT_EQ(value, pBuffer[i]) << "Mismatch at index " << i;
        }
    }

    StrictMock<ReadAheadManagerMock>* m_pReadAheadMock;
    EngineBufferScaleBungee* m_pScaler;
};

// Test basic playback at 1.0x speed
TEST_F(EngineBufferScaleBungeeTest, BasicPlayback) {
    SetRate(1.0);

    // Create a simple test signal (alternating samples)
    constexpr SINT kBufferSize = 4096;
    CSAMPLE readBuffer[kBufferSize];
    for (SINT i = 0; i < kBufferSize; ++i) {
        readBuffer[i] = static_cast<CSAMPLE>(i % 2 == 0 ? 0.5f : -0.5f);
    }
    m_pReadAheadMock->setReadBuffer(readBuffer, kBufferSize);

    // Tell the RAMAN mock to invoke getNextSamplesFake
    EXPECT_CALL(*m_pReadAheadMock, getNextSamples(_, _, _, _))
            .WillRepeatedly(Invoke(m_pReadAheadMock, &ReadAheadManagerMock::getNextSamplesFake));

    // Allocate output buffer
    constexpr SINT kOutputBufferSize = 2048;
    CSAMPLE* pOutput = SampleUtil::alloc(kOutputBufferSize);
    ClearBuffer(pOutput, kOutputBufferSize);

    // Scale the buffer
    double framesRead = m_pScaler->scaleBuffer(pOutput, kOutputBufferSize);

    // Verify we got some output
    EXPECT_GT(framesRead, 0.0);

    // Clean up
    SampleUtil::free(pOutput);
}

// Test variable speeds (0.5x and 2.0x)
TEST_F(EngineBufferScaleBungeeTest, VariableSpeeds) {
    // Test at 0.5x speed
    SetRate(0.5);

    constexpr SINT kBufferSize = 8192;
    CSAMPLE readBuffer[kBufferSize];
    for (SINT i = 0; i < kBufferSize; ++i) {
        readBuffer[i] = static_cast<CSAMPLE>(0.5f);
    }
    m_pReadAheadMock->setReadBuffer(readBuffer, kBufferSize);

    EXPECT_CALL(*m_pReadAheadMock, getNextSamples(_, _, _, _))
            .WillRepeatedly(Invoke(m_pReadAheadMock, &ReadAheadManagerMock::getNextSamplesFake));

    constexpr SINT kOutputBufferSize = 2048;
    CSAMPLE* pOutput = SampleUtil::alloc(kOutputBufferSize);

    // At 0.5x speed, we should need fewer input frames for the same output
    double framesRead = m_pScaler->scaleBuffer(pOutput, kOutputBufferSize);
    EXPECT_GT(framesRead, 0.0);

    // Test at 2.0x speed
    m_pReadAheadMock->resetReadCount();
    SetRate(2.0);
    m_pScaler->clear();

    double framesRead2x = m_pScaler->scaleBuffer(pOutput, kOutputBufferSize);
    EXPECT_GT(framesRead2x, 0.0);

    // At 2.0x, we should consume more input frames than at 0.5x
    EXPECT_GT(framesRead2x, framesRead);

    SampleUtil::free(pOutput);
}

// Test keylock mode (tempo changes with fixed pitch)
TEST_F(EngineBufferScaleBungeeTest, KeylockMode) {
    // Set tempo to 1.5x but keep pitch at 1.0 (keylock)
    SetRate(1.5, 1.0);

    constexpr SINT kBufferSize = 8192;
    CSAMPLE readBuffer[kBufferSize];
    for (SINT i = 0; i < kBufferSize; ++i) {
        readBuffer[i] = static_cast<CSAMPLE>(0.3f);
    }
    m_pReadAheadMock->setReadBuffer(readBuffer, kBufferSize);

    EXPECT_CALL(*m_pReadAheadMock, getNextSamples(_, _, _, _))
            .WillRepeatedly(Invoke(m_pReadAheadMock, &ReadAheadManagerMock::getNextSamplesFake));

    constexpr SINT kOutputBufferSize = 2048;
    CSAMPLE* pOutput = SampleUtil::alloc(kOutputBufferSize);

    double framesRead = m_pScaler->scaleBuffer(pOutput, kOutputBufferSize);
    EXPECT_GT(framesRead, 0.0);

    SampleUtil::free(pOutput);
}

// Test pitch shifting (pitch changes with fixed tempo)
TEST_F(EngineBufferScaleBungeeTest, PitchShifting) {
    // Keep tempo at 1.0 but change pitch to 1.5 (semitone shift up)
    SetRate(1.0, 1.5);

    constexpr SINT kBufferSize = 8192;
    CSAMPLE readBuffer[kBufferSize];
    for (SINT i = 0; i < kBufferSize; ++i) {
        readBuffer[i] = static_cast<CSAMPLE>(0.4f);
    }
    m_pReadAheadMock->setReadBuffer(readBuffer, kBufferSize);

    EXPECT_CALL(*m_pReadAheadMock, getNextSamples(_, _, _, _))
            .WillRepeatedly(Invoke(m_pReadAheadMock, &ReadAheadManagerMock::getNextSamplesFake));

    constexpr SINT kOutputBufferSize = 2048;
    CSAMPLE* pOutput = SampleUtil::alloc(kOutputBufferSize);

    double framesRead = m_pScaler->scaleBuffer(pOutput, kOutputBufferSize);
    EXPECT_GT(framesRead, 0.0);

    SampleUtil::free(pOutput);
}

// Test reverse playback (negative speed)
TEST_F(EngineBufferScaleBungeeTest, ReversePlayback) {
    // Set negative speed for reverse playback
    SetRate(-1.0);

    constexpr SINT kBufferSize = 8192;
    CSAMPLE readBuffer[kBufferSize];
    for (SINT i = 0; i < kBufferSize; ++i) {
        readBuffer[i] = static_cast<CSAMPLE>(0.6f);
    }
    m_pReadAheadMock->setReadBuffer(readBuffer, kBufferSize);

    EXPECT_CALL(*m_pReadAheadMock, getNextSamples(_, _, _, _))
            .WillRepeatedly(Invoke(m_pReadAheadMock, &ReadAheadManagerMock::getNextSamplesFake));

    constexpr SINT kOutputBufferSize = 2048;
    CSAMPLE* pOutput = SampleUtil::alloc(kOutputBufferSize);

    double framesRead = m_pScaler->scaleBuffer(pOutput, kOutputBufferSize);
    EXPECT_GT(framesRead, 0.0);

    SampleUtil::free(pOutput);
}

// Test extreme speeds (MIN and MAX seek speeds)
TEST_F(EngineBufferScaleBungeeTest, ExtremeSpeeds) {
    // Test at minimum seek speed
    SetRate(MIN_SEEK_SPEED * 2); // Slightly above minimum to avoid zeroing

    constexpr SINT kBufferSize = 16384;
    CSAMPLE readBuffer[kBufferSize];
    for (SINT i = 0; i < kBufferSize; ++i) {
        readBuffer[i] = static_cast<CSAMPLE>(0.7f);
    }
    m_pReadAheadMock->setReadBuffer(readBuffer, kBufferSize);

    EXPECT_CALL(*m_pReadAheadMock, getNextSamples(_, _, _, _))
            .WillRepeatedly(Invoke(m_pReadAheadMock, &ReadAheadManagerMock::getNextSamplesFake));

    constexpr SINT kOutputBufferSize = 2048;
    CSAMPLE* pOutput = SampleUtil::alloc(kOutputBufferSize);

    double framesRead = m_pScaler->scaleBuffer(pOutput, kOutputBufferSize);
    EXPECT_GE(framesRead, 0.0);

    // Test at maximum seek speed
    m_pReadAheadMock->resetReadCount();
    SetRate(MAX_SEEK_SPEED);
    m_pScaler->clear();

    double framesReadMax = m_pScaler->scaleBuffer(pOutput, kOutputBufferSize);
    EXPECT_GE(framesReadMax, 0.0);

    SampleUtil::free(pOutput);
}

// Test zero speed (pause behavior)
TEST_F(EngineBufferScaleBungeeTest, ZeroSpeed) {
    // Set zero speed
    SetRate(0.0);

    constexpr SINT kOutputBufferSize = 2048;
    CSAMPLE* pOutput = SampleUtil::alloc(kOutputBufferSize);
    FillBuffer(pOutput, 1.0f, kOutputBufferSize); // Fill with non-zero

    // With zero speed, we should get zeros and no frames read
    double framesRead = m_pScaler->scaleBuffer(pOutput, kOutputBufferSize);

    EXPECT_EQ(framesRead, 0.0);
    // Buffer should be cleared (zeros)
    AssertWholeBufferEquals(pOutput, 0.0f, kOutputBufferSize);

    SampleUtil::free(pOutput);
}

// Test buffer clearing on seek
TEST_F(EngineBufferScaleBungeeTest, BufferClearing) {
    SetRate(1.0);

    constexpr SINT kBufferSize = 8192;
    CSAMPLE readBuffer[kBufferSize];
    for (SINT i = 0; i < kBufferSize; ++i) {
        readBuffer[i] = static_cast<CSAMPLE>(0.8f);
    }
    m_pReadAheadMock->setReadBuffer(readBuffer, kBufferSize);

    EXPECT_CALL(*m_pReadAheadMock, getNextSamples(_, _, _, _))
            .WillRepeatedly(Invoke(m_pReadAheadMock, &ReadAheadManagerMock::getNextSamplesFake));

    constexpr SINT kOutputBufferSize = 2048;
    CSAMPLE* pOutput = SampleUtil::alloc(kOutputBufferSize);

    // Process some audio
    m_pScaler->scaleBuffer(pOutput, kOutputBufferSize);

    // Clear should reset internal state
    m_pScaler->clear();

    // Process again after clear
    m_pReadAheadMock->resetReadCount();
    double framesRead = m_pScaler->scaleBuffer(pOutput, kOutputBufferSize);
    EXPECT_GT(framesRead, 0.0);

    SampleUtil::free(pOutput);
}

// Test signal format changes (sample rate and channel count)
TEST_F(EngineBufferScaleBungeeTest, SignalFormatChanges) {
    SetRate(1.0);

    // Change sample rate
    m_pScaler->setSignal(mixxx::audio::SampleRate(48000),
            mixxx::audio::ChannelCount::stereo());

    constexpr SINT kBufferSize = 8192;
    CSAMPLE readBuffer[kBufferSize];
    for (SINT i = 0; i < kBufferSize; ++i) {
        readBuffer[i] = static_cast<CSAMPLE>(0.5f);
    }
    m_pReadAheadMock->setReadBuffer(readBuffer, kBufferSize);

    EXPECT_CALL(*m_pReadAheadMock, getNextSamples(_, _, _, _))
            .WillRepeatedly(Invoke(m_pReadAheadMock, &ReadAheadManagerMock::getNextSamplesFake));

    constexpr SINT kOutputBufferSize = 2048;
    CSAMPLE* pOutput = SampleUtil::alloc(kOutputBufferSize);

    double framesRead = m_pScaler->scaleBuffer(pOutput, kOutputBufferSize);
    EXPECT_GT(framesRead, 0.0);

    SampleUtil::free(pOutput);
}

// Test handling of read failures (no data available)
TEST_F(EngineBufferScaleBungeeTest, ReadFailureHandling) {
    SetRate(1.0);

    // First call returns data, subsequent calls return 0 (EOF)
    EXPECT_CALL(*m_pReadAheadMock, getNextSamples(_, _, _, _))
            .WillOnce(Invoke(m_pReadAheadMock, &ReadAheadManagerMock::getNextSamplesFake))
            .WillRepeatedly(Return(0));

    constexpr SINT kBufferSize = 1024;
    CSAMPLE readBuffer[kBufferSize];
    for (SINT i = 0; i < kBufferSize; ++i) {
        readBuffer[i] = static_cast<CSAMPLE>(0.9f);
    }
    m_pReadAheadMock->setReadBuffer(readBuffer, kBufferSize);

    constexpr SINT kOutputBufferSize = 4096; // Larger than available input
    CSAMPLE* pOutput = SampleUtil::alloc(kOutputBufferSize);

    // Should handle EOF gracefully
    double framesRead = m_pScaler->scaleBuffer(pOutput, kOutputBufferSize);

    // We may not get all the output we asked for, but shouldn't crash
    EXPECT_GE(framesRead, 0.0);

    SampleUtil::free(pOutput);
}

// Test grain boundary conditions with rapid parameter changes
TEST_F(EngineBufferScaleBungeeTest, RapidParameterChanges) {
    constexpr SINT kBufferSize = 16384;
    CSAMPLE readBuffer[kBufferSize];
    for (SINT i = 0; i < kBufferSize; ++i) {
        readBuffer[i] = static_cast<CSAMPLE>(0.5f);
    }
    m_pReadAheadMock->setReadBuffer(readBuffer, kBufferSize);

    EXPECT_CALL(*m_pReadAheadMock, getNextSamples(_, _, _, _))
            .WillRepeatedly(Invoke(m_pReadAheadMock, &ReadAheadManagerMock::getNextSamplesFake));

    constexpr SINT kOutputBufferSize = 1024;
    CSAMPLE* pOutput = SampleUtil::alloc(kOutputBufferSize);

    // Rapidly change parameters between buffer scales
    for (int i = 0; i < 5; ++i) {
        SetRate(0.5 + i * 0.25); // 0.5, 0.75, 1.0, 1.25, 1.5
        double framesRead = m_pScaler->scaleBuffer(pOutput, kOutputBufferSize);
        EXPECT_GT(framesRead, 0.0) << "Failed at iteration " << i;
    }

    SampleUtil::free(pOutput);
}

} // namespace
