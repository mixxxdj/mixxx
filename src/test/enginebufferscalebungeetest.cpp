#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <vector>

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
              m_iSamplesRead(0),
              m_iReadCallCount(0) {
    }

    SINT getNextSamplesFake(double dRate,
            CSAMPLE* buffer,
            SINT requested_samples,
            mixxx::audio::ChannelCount channelCount) {
        Q_UNUSED(dRate);
        Q_UNUSED(channelCount);
        ++m_iReadCallCount;

        const bool hasBuffer = m_pBuffer != nullptr;
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

    int getSamplesRead() const {
        return m_iSamplesRead;
    }

    int getReadCallCount() const {
        return m_iReadCallCount;
    }

    void resetReadStats() {
        m_iSamplesRead = 0;
        m_iReadCallCount = 0;
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
    SINT m_iReadCallCount;
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

TEST_F(EngineBufferScaleBungeeTest, BasicPlayback) {
    SetRate(1.0);

    constexpr SINT kBufferSize = 4096;
    CSAMPLE readBuffer[kBufferSize];
    for (SINT i = 0; i < kBufferSize; ++i) {
        readBuffer[i] = static_cast<CSAMPLE>(i % 2 == 0 ? 0.5f : -0.5f);
    }
    m_pReadAheadMock->setReadBuffer(readBuffer, kBufferSize);

    EXPECT_CALL(*m_pReadAheadMock, getNextSamples(_, _, _, _))
            .WillRepeatedly(Invoke(m_pReadAheadMock, &ReadAheadManagerMock::getNextSamplesFake));

    constexpr SINT kOutputBufferSize = 2048;
    CSAMPLE* pOutput = SampleUtil::alloc(kOutputBufferSize);
    ClearBuffer(pOutput, kOutputBufferSize);

    const double framesRead = m_pScaler->scaleBuffer(pOutput, kOutputBufferSize);
    EXPECT_GT(framesRead, 0.0);

    SampleUtil::free(pOutput);
}

TEST_F(EngineBufferScaleBungeeTest, VariableSpeeds) {
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

    const double framesRead = m_pScaler->scaleBuffer(pOutput, kOutputBufferSize);
    EXPECT_GT(framesRead, 0.0);

    m_pReadAheadMock->resetReadStats();
    SetRate(2.0);
    m_pScaler->clear();

    const double framesRead2x = m_pScaler->scaleBuffer(pOutput, kOutputBufferSize);
    EXPECT_GT(framesRead2x, 0.0);
    EXPECT_GT(framesRead2x, framesRead);

    SampleUtil::free(pOutput);
}

TEST_F(EngineBufferScaleBungeeTest, KeylockMode) {
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

    const double framesRead = m_pScaler->scaleBuffer(pOutput, kOutputBufferSize);
    EXPECT_GT(framesRead, 0.0);

    SampleUtil::free(pOutput);
}

TEST_F(EngineBufferScaleBungeeTest, PitchShifting) {
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

    const double framesRead = m_pScaler->scaleBuffer(pOutput, kOutputBufferSize);
    EXPECT_GT(framesRead, 0.0);

    SampleUtil::free(pOutput);
}

TEST_F(EngineBufferScaleBungeeTest, ReversePlayback) {
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

    const double framesRead = m_pScaler->scaleBuffer(pOutput, kOutputBufferSize);
    EXPECT_GT(framesRead, 0.0);

    SampleUtil::free(pOutput);
}

TEST_F(EngineBufferScaleBungeeTest, ExtremeSpeeds) {
    SetRate(MIN_SEEK_SPEED * 2);

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

    const double framesRead = m_pScaler->scaleBuffer(pOutput, kOutputBufferSize);
    EXPECT_GE(framesRead, 0.0);

    m_pReadAheadMock->resetReadStats();
    SetRate(MAX_SEEK_SPEED);
    m_pScaler->clear();

    const double framesReadMax = m_pScaler->scaleBuffer(pOutput, kOutputBufferSize);
    EXPECT_GE(framesReadMax, 0.0);

    SampleUtil::free(pOutput);
}

TEST_F(EngineBufferScaleBungeeTest, ZeroSpeed) {
    SetRate(0.0);

    constexpr SINT kOutputBufferSize = 2048;
    CSAMPLE* pOutput = SampleUtil::alloc(kOutputBufferSize);
    FillBuffer(pOutput, 1.0f, kOutputBufferSize);

    const double framesRead = m_pScaler->scaleBuffer(pOutput, kOutputBufferSize);

    EXPECT_EQ(framesRead, 0.0);
    AssertWholeBufferEquals(pOutput, 0.0f, kOutputBufferSize);

    SampleUtil::free(pOutput);
}

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

    m_pScaler->scaleBuffer(pOutput, kOutputBufferSize);
    m_pScaler->clear();

    m_pReadAheadMock->resetReadStats();
    const double framesRead = m_pScaler->scaleBuffer(pOutput, kOutputBufferSize);
    EXPECT_GT(framesRead, 0.0);
    EXPECT_GT(m_pReadAheadMock->getReadCallCount(), 0);

    SampleUtil::free(pOutput);
}

TEST_F(EngineBufferScaleBungeeTest, SignalFormatChanges) {
    SetRate(1.0);

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

    const double framesRead = m_pScaler->scaleBuffer(pOutput, kOutputBufferSize);
    EXPECT_GT(framesRead, 0.0);

    SampleUtil::free(pOutput);
}

TEST_F(EngineBufferScaleBungeeTest, ReadFailureHandling) {
    SetRate(1.0);

    constexpr SINT kBufferSize = 1024;
    CSAMPLE readBuffer[kBufferSize];
    for (SINT i = 0; i < kBufferSize; ++i) {
        readBuffer[i] = static_cast<CSAMPLE>(0.9f);
    }
    m_pReadAheadMock->setReadBuffer(readBuffer, kBufferSize);

    int callCount = 0;
    EXPECT_CALL(*m_pReadAheadMock, getNextSamples(_, _, _, _))
            .WillRepeatedly(Invoke([&](double dRate,
                                           CSAMPLE* buffer,
                                           SINT requested_samples,
                                           mixxx::audio::ChannelCount channelCount) {
                ++callCount;
                if (callCount == 1) {
                    return m_pReadAheadMock->getNextSamplesFake(
                            dRate, buffer, requested_samples, channelCount);
                }
                return static_cast<SINT>(0);
            }));

    constexpr SINT kOutputBufferSize = 4096;
    CSAMPLE* pOutput = SampleUtil::alloc(kOutputBufferSize);

    const double framesRead = m_pScaler->scaleBuffer(pOutput, kOutputBufferSize);
    EXPECT_GE(framesRead, 0.0);
    EXPECT_LE(callCount, 4);

    SampleUtil::free(pOutput);
}

TEST_F(EngineBufferScaleBungeeTest, ReusesBufferedInputAcrossOverlappingGrains) {
    SetRate(1.0);

    constexpr SINT kBufferSize = 65536;
    std::vector<CSAMPLE> readBuffer(kBufferSize);
    for (SINT i = 0; i < kBufferSize; ++i) {
        readBuffer[i] = static_cast<CSAMPLE>((i % 97) / 97.0f);
    }
    m_pReadAheadMock->setReadBuffer(readBuffer.data(), kBufferSize);

    EXPECT_CALL(*m_pReadAheadMock, getNextSamples(_, _, _, _))
            .WillRepeatedly(Invoke(m_pReadAheadMock, &ReadAheadManagerMock::getNextSamplesFake));

    constexpr SINT kOutputBufferSize = 512;
    CSAMPLE* pOutput = SampleUtil::alloc(kOutputBufferSize);

    double totalFramesRead = 0.0;
    for (int i = 0; i < 8; ++i) {
        totalFramesRead += m_pScaler->scaleBuffer(pOutput, kOutputBufferSize);
    }

    EXPECT_GT(totalFramesRead, 0.0);
    EXPECT_LT(m_pReadAheadMock->getReadCallCount(), 8);
    EXPECT_LT(m_pReadAheadMock->getSamplesRead(), 8 * kOutputBufferSize * 4);

    SampleUtil::free(pOutput);
}

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

    for (int i = 0; i < 5; ++i) {
        SetRate(0.5 + i * 0.25);
        const double framesRead = m_pScaler->scaleBuffer(pOutput, kOutputBufferSize);
        EXPECT_GT(framesRead, 0.0) << "Failed at iteration " << i;
    }

    SampleUtil::free(pOutput);
}

} // namespace
