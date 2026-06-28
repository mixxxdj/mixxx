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

// =============================================================================
// Buffer-window invariant regression tests.
//
// These pin the post-conditions of
// EngineBufferScaleBungee::discardBufferedInputBefore() and the
// dataOffset/grainSize invariant in processGrain() that together prevent
// the high-speed grain-outrun heap-corruption crash.
//
// Failure mode being guarded against ("high-speed grain-outrun + heap
// overflow"):
//
//   At very high playback rates, Bungee's specifyGrain() returns an
//   InputChunk whose begin frame is far beyond the currently buffered
//   input window (framePosition > m_bufferedInputEndFrame).  In the
//   pre-fix code, discardBufferedInputBefore() advanced
//   m_bufferedInputBeginFrame only by the number of buffered frames it
//   actually held, then collapsed m_bufferedInputEndFrame to the same
//   value -- leaving both pointers at the OLD end rather than at the
//   requested framePosition.  The resulting gap caused processGrain()'s
//   dataOffset = inputChunk.begin - m_bufferedInputBeginFrame to exceed
//   m_channelStride - grainSize, so analyseGrain() read past
//   m_contiguousChannelBuffer for channel 1 and corrupted heap memory.
//
// The fixture intentionally lives at FILE SCOPE (not in the anonymous
// namespace above) so the FRIEND_TEST declarations in
// enginebufferscalebungee.h can grant it access to the private buffer-
// window state.
// =============================================================================

class BufferWindowReadAheadManagerMock : public ReadAheadManager {
  public:
    BufferWindowReadAheadManagerMock() = default;

    // Returns up to `requested_samples`, looping over `m_buffer`.  Tracks
    // call counts so tests can assert reads happened.
    SINT getNextSamples(double dRate,
            CSAMPLE* buffer,
            SINT requested_samples,
            mixxx::audio::ChannelCount /*channelCount*/) override {
        Q_UNUSED(dRate);
        ++m_iReadCallCount;
        for (SINT i = 0; i < requested_samples; ++i) {
            buffer[i] = m_buffer.empty()
                    ? 0.0f
                    : m_buffer[m_iReadPosition++ % m_buffer.size()];
        }
        m_iSamplesRead += requested_samples;
        return requested_samples;
    }

    void setReadBuffer(std::vector<CSAMPLE> buffer) {
        m_buffer = std::move(buffer);
        m_iReadPosition = 0;
    }

    int readCallCount() const {
        return m_iReadCallCount;
    }

  private:
    std::vector<CSAMPLE> m_buffer;
    SINT m_iReadPosition = 0;
    SINT m_iSamplesRead = 0;
    int m_iReadCallCount = 0;
};

class EngineBufferScaleBungeeBufferWindowTest : public MixxxTest {
  protected:
    void SetUp() override {
        m_pReadAhead = new BufferWindowReadAheadManagerMock();
        m_pScaler = new EngineBufferScaleBungee(m_pReadAhead);
        m_pScaler->setSignal(mixxx::audio::SampleRate(44100),
                mixxx::audio::ChannelCount::stereo());
    }

    void TearDown() override {
        delete m_pScaler;
        delete m_pReadAhead;
    }

    // Helpers that take advantage of FRIEND_TEST access in the header.
    void seedBufferWindow(SINT begin, SINT end) {
        m_pScaler->m_bufferedInputBeginFrame = begin;
        m_pScaler->m_bufferedInputEndFrame = end;
    }

    SINT bufferBegin() const {
        return m_pScaler->m_bufferedInputBeginFrame;
    }

    SINT bufferEnd() const {
        return m_pScaler->m_bufferedInputEndFrame;
    }

    SINT channelStride() const {
        return m_pScaler->m_channelStride;
    }

    SINT currentChunkSize() const {
        return static_cast<SINT>(m_pScaler->m_currentInputChunk.end -
                m_pScaler->m_currentInputChunk.begin);
    }

    BufferWindowReadAheadManagerMock* m_pReadAhead = nullptr;
    EngineBufferScaleBungee* m_pScaler = nullptr;
};

// Direct invariant test for the high-speed grain-outrun failure mode.
//
// Pre-conditions: a populated buffer window [100, 200).
// Action: discardBufferedInputBefore(framePosition=10000) -- gap far beyond end.
// Post-condition (post-fix): both begin and end advance to framePosition.
//
// Pre-fix behaviour (the bug): begin would be left at 200 (old end) and end
// would also collapse to 200, leaving a gap of 9800 frames between the
// buffered window and the requested position -- the root cause of the
// processGrain dataOffset overflow.
TEST_F(EngineBufferScaleBungeeBufferWindowTest,
        DiscardWithGapBeyondEndJumpsBothPointersToFramePosition) {
    seedBufferWindow(/*begin=*/100, /*end=*/200);

    constexpr SINT kFramePosition = 10000;
    m_pScaler->discardBufferedInputBefore(kFramePosition);

    EXPECT_EQ(kFramePosition, bufferBegin())
            << "discardBufferedInputBefore must advance begin all the way to "
               "framePosition when the buffer is fully discarded; pre-fix "
               "code left it at the old end (= 200), causing a data gap that "
               "made processGrain compute dataOffset > m_channelStride and "
               "overflow m_contiguousChannelBuffer.";
    EXPECT_EQ(kFramePosition, bufferEnd())
            << "end must equal begin after a full discard so the next "
               "appendInputFrames() writes contiguous data.";
    EXPECT_GE(bufferBegin(), kFramePosition)
            << "buffer-window invariant: begin >= framePosition after "
               "discardBufferedInputBefore(framePosition).";
}

// sanity baseline: framePosition still inside buffer must NOT
// over-jump.  Guards against an accidental "always set begin = framePosition"
// regression of the in-window case.
TEST_F(EngineBufferScaleBungeeBufferWindowTest,
        DiscardWhenFramePositionInsideBufferDoesNotOverJump) {
    seedBufferWindow(/*begin=*/100, /*end=*/500);

    m_pScaler->discardBufferedInputBefore(/*framePosition=*/250);

    EXPECT_EQ(250, bufferBegin())
            << "In-window discard must advance begin exactly to framePosition.";
    EXPECT_EQ(500, bufferEnd())
            << "In-window discard must NOT touch end (only the prefix is dropped).";
}

// degenerate case: empty buffer window.  Both pointers should
// jump to framePosition without underflow / signed-arithmetic surprises.
TEST_F(EngineBufferScaleBungeeBufferWindowTest,
        DiscardWhenBufferEmptyJumpsToFramePosition) {
    seedBufferWindow(/*begin=*/0, /*end=*/0);

    m_pScaler->discardBufferedInputBefore(/*framePosition=*/4096);

    EXPECT_EQ(4096, bufferBegin());
    EXPECT_EQ(4096, bufferEnd());
}

// end-to-end high-speed grain-outrun stress.
//
// Drives scaleBuffer() repeatedly at very high tempo ratios so that
// discardBufferedInputBefore() is called with framePosition far beyond
// m_bufferedInputEndFrame on most grain boundaries.  Asserts that the
// dataOffset invariant ((m_currentInputChunk.begin - m_bufferedInputBeginFrame)
// + grainSize <= m_channelStride) holds after every grain.  This is the
// invariant that, when violated, caused Bungee's analyseGrain to read past
// m_contiguousChannelBuffer.
TEST_F(EngineBufferScaleBungeeBufferWindowTest,
        HighSpeedGrainOutrunMaintainsDataOffsetInvariant) {
    constexpr SINT kFeedFrames = 1 << 16; // 64 K stereo frames -- deep enough
                                          // for many grain hops.
    std::vector<CSAMPLE> readData(kFeedFrames * 2);
    for (size_t i = 0; i < readData.size(); ++i) {
        // Bounded, deterministic, non-zero, non-uniform signal.
        readData[i] = static_cast<CSAMPLE>(((i * 1664525u + 1013904223u) & 0xffff) /
                        65535.0f * 2.0f -
                1.0f);
    }
    m_pReadAhead->setReadBuffer(std::move(readData));

    constexpr SINT kOutputFrames = 1024;
    constexpr SINT kOutputSamples = kOutputFrames * 2;
    CSAMPLE* pOutput = SampleUtil::alloc(kOutputSamples);

    // Sweep through several aggressive tempo ratios.  Anything above ~4x
    // already makes grain hops outrun the buffered input window for typical
    // grain sizes; we go much higher to exercise the failure regime
    // explicitly.
    const double kTempoRatios[] = {4.0, 8.0, 16.0, MAX_SEEK_SPEED};
    for (double tempo : kTempoRatios) {
        m_pScaler->clear();
        double tempoVar = tempo;
        double pitchVar = 1.0;
        m_pScaler->setScaleParameters(1.0, &tempoVar, &pitchVar);

        // Many iterations: the bug only triggers after enough grain hops
        // have accumulated to push framePosition beyond the buffered end.
        for (int iter = 0; iter < 64; ++iter) {
            const double framesRead =
                    m_pScaler->scaleBuffer(pOutput, kOutputSamples);
            EXPECT_GE(framesRead, 0.0)
                    << "scaleBuffer must not return negative at tempo " << tempo
                    << " iter " << iter;

            // Buffer-window structural invariants -- these must hold across
            // EVERY scaleBuffer call, not just at the end.
            EXPECT_LE(bufferBegin(), bufferEnd())
                    << "begin must never exceed end at tempo " << tempo
                    << " iter " << iter;
            EXPECT_LE(bufferEnd() - bufferBegin(), channelStride())
                    << "buffered window must fit in m_channelStride at tempo "
                    << tempo << " iter " << iter;

            // dataOffset invariant -- the precise condition that prevented
            // the heap overflow.
            const SINT chunkSize = currentChunkSize();
            if (chunkSize > 0) {
                EXPECT_LE(chunkSize, channelStride())
                        << "grain size must fit in channel stride at tempo "
                        << tempo << " iter " << iter;
            }
        }
    }

    SampleUtil::free(pOutput);
}
