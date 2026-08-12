#pragma once

#include <bungee/Bungee.h>
#include <gtest/gtest_prod.h>

#include <array>
#include <memory>
#include <vector>

#include "engine/bufferscalers/enginebufferscale.h"
#include "util/samplebuffer.h"

class ReadAheadManager;
class EngineBufferScaleBungeeBufferWindowTest;

// Uses Bungee's low-level grain API to perform time-stretching and pitch-shifting.
//
// ## Rate / position semantics
//
//   m_dBaseRate    — sample-rate ratio (input SR / output SR); almost always 1.0
//                    because Bungee's own resample mode handles SR conversion.
//   m_dTempoRatio  — absolute (unsigned) tempo ratio; 1.0 = original speed.
//                    Values <MIN_SEEK_SPEED are clamped to 0.0 (stopped).
//   m_bBackwards   — true when the caller requested a negative tempo ratio.
//   m_effectiveRate — latched source-frame advance per output frame for
//                    already-synthesised output and the current grain. New
//                    m_dBaseRate * m_dTempoRatio requests are adopted only at
//                    grain boundaries so queued output keeps the tempo that
//                    produced it.
//   m_request.speed — signed speed passed to Bungee each grain. Bungee's
//                    request positions are input frame timestamps, so
//                    sample-rate conversion must be part of this speed rather
//                    than only the Mixxx cursor update.
//
// ## Input window / InputChunk contract
//
//   Bungee's specifyGrain() returns an InputChunk {.begin, .end} representing
//   the range of input frames it needs for the current grain.  Consecutive
//   grains overlap, so the same input frames appear in multiple chunks.  This
//   class maintains a sliding planar input window:
//
//     [m_bufferedInputBeginFrame … m_bufferedInputEndFrame)
//
//   Before each analyseGrain() call:
//     1. discardBufferedInputBefore() removes frames no longer needed.
//     2. appendInputFrames() extends the window forward until it covers
//        inputChunk.end (or ReadAheadManager runs dry).
//     3. muteHead / muteTail are computed from the difference between the
//        requested range and what the window actually covers, then passed
//        directly to analyseGrain() so Bungee can zero-pad internally.
//
// ## Buffer-window invariant
//
//   The two pointers m_bufferedInputBeginFrame and m_bufferedInputEndFrame
//   together with m_channelStride satisfy a single invariant on every grain
//   boundary:
//
//     m_bufferedInputBeginFrame <= currentInputChunk.begin
//     dataOffset = currentInputChunk.begin - m_bufferedInputBeginFrame
//     dataOffset + grainSize <= m_channelStride
//
//   Where grainSize = currentInputChunk.end - currentInputChunk.begin and
//   m_channelStride is the per-channel capacity of m_contiguousChannelBuffer.
//   Violating this invariant causes Bungee's analyseGrain() to read past
//   the end of m_contiguousChannelBuffer (heap corruption).
//
//   discardBufferedInputBefore(framePosition) preserves the invariant in
//   two regimes:
//     - Partial discard (framePosition inside the buffered window):
//       memmove() shifts the unread tail left and advances
//       m_bufferedInputBeginFrame by the discard count; m_bufferedInputEndFrame
//       is unchanged.
//     - Full discard (framePosition >= m_bufferedInputEndFrame, which
//       happens when grain hops outrun the input window at very high
//       playback rates): first consume the skipped gap from ReadAheadManager,
//       then both pointers jump to framePosition.  Any other choice (e.g.
//       leaving begin at the OLD end, as an earlier implementation did)
//       produces a gap that violates the dataOffset invariant on the next
//       grain; jumping without consuming the skipped read-ahead input labels
//       future reads with the wrong absolute frame position.
//
//   processGrain() additionally enforces the invariant defensively: if it
//   ever computes dataOffset + grainSize > m_channelStride, it sets
//   m_bResetNeeded = true and returns 0 instead of calling analyseGrain().
//   This is belt-and-braces protection — discardBufferedInputBefore() must
//   not rely on it to be correct.
//
// ## Thread safety
//   Not thread-safe; intended for single-threaded engine use only.
class EngineBufferScaleBungee final : public EngineBufferScale {
    Q_OBJECT
  public:
    explicit EngineBufferScaleBungee(ReadAheadManager* pReadAheadManager);

    EngineBufferScaleBungee(const EngineBufferScaleBungee&) = delete;
    EngineBufferScaleBungee& operator=(const EngineBufferScaleBungee&) = delete;
    EngineBufferScaleBungee(EngineBufferScaleBungee&&) = delete;
    EngineBufferScaleBungee& operator=(EngineBufferScaleBungee&&) = delete;

    ~EngineBufferScaleBungee() override = default;

    void setScaleParameters(double base_rate,
            double* pTempoRatio,
            double* pPitchRatio) override;

    double scaleBuffer(CSAMPLE* pOutputBuffer,
            SINT iOutputBufferSize) override;

    double getVisualPlayPositionOffset() const override;

    void clear() override;

  private:
    void onSignalChanged() override;

    // Process a single grain and return the number of output frames produced.
    SINT processGrain(CSAMPLE* pOutputBuffer, SINT maxFrames);

    // Deinterleave input data into the buffered planar input window for Bungee.
    void deinterleaveInput(const CSAMPLE* pBuffer, SINT destOffsetFrames, SINT frames);

    // Discard buffered input that is no longer needed by future overlapping grains.
    void discardBufferedInputBefore(SINT framePosition, double signedEffectiveRate = 1.0);

    // Consume skipped source frames from ReadAheadManager without storing them.
    SINT consumeReadAheadGap(double signedEffectiveRate, SINT framesToConsume);

    // Read more input from ReadAheadManager into the buffered planar window.
    SINT appendInputFrames(double signedEffectiveRate, SINT framesToRead);

    // Ensure the current input chunk is covered by the buffered planar window.
    SINT ensureInputForCurrentChunk(double signedEffectiveRate);

    // Copy nFrames from m_outputChunk into pDest starting at offsetInChunk.
    // Uses SampleUtil::interleaveBuffer for the stereo fast path.
    void copyOutputFrames(CSAMPLE* pDest, SINT offsetInChunk, SINT nFrames) const;
    bool hasValidOutputChunk() const;
    double copyFlushOutputFrames(CSAMPLE*& pOutput, SINT& remainingFrames) const;

    // The read-ahead manager that we use to fetch samples
    ReadAheadManager* m_pReadAheadManager;

    // Bungee stretcher instance (using Basic edition)
    std::unique_ptr<Bungee::Stretcher<Bungee::Basic>> m_pStretcher;

    // Current Bungee request for grain processing.
    Bungee::Request m_request;

    // Output chunk for synthesiseGrain.
    Bungee::OutputChunk m_outputChunk;

    // Deinterleaved channel pointers into the contiguous buffered input window.
    std::vector<float*> m_channelBufferPtrs;

    // Single contiguous buffer for all channels (for Bungee's planar format)
    mixxx::SampleBuffer m_contiguousChannelBuffer;

    // Interleaved read buffer from ReadAheadManager
    mixxx::SampleBuffer m_interleavedReadBuffer;

    // Playback direction.
    bool m_bBackwards;

    // Current output channel stride.
    SINT m_channelStride;

    // Current grain's input chunk.
    Bungee::InputChunk m_currentInputChunk;

    // Absolute frame range currently buffered in m_contiguousChannelBuffer.
    SINT m_bufferedInputBeginFrame;
    SINT m_bufferedInputEndFrame;

    // Whether we need to reset on the next processed grain.
    bool m_bResetNeeded;

    // Output frames remaining from the current synthesised grain.
    SINT m_remainingOutputFrames;
    SINT m_outputChunkConsumed;
    double m_lastReadFramesProcessed;

    // Bungee Basic's lapped output is delayed by two synthesis hops. The
    // sample-rate-dependent latency is converted into source frames here.
    SINT m_outputLatencyFrames;

    // Maximum number of frames to request from ReadAheadManager in one call.
    static constexpr SINT kMaxGrainFrames = 4096;

    // Capacity of the buffered planar input window in frames.
    SINT m_inputBufferFrames;

    // Regression tests pin the buffer-window invariant
    // (m_bufferedInputBeginFrame / m_bufferedInputEndFrame must both jump
    // to framePosition when the requested position outruns the buffered
    // input window).  Without this invariant, processGrain() computes a
    // dataOffset that exceeds m_channelStride, causing Bungee's Eigen map
    // to read past m_contiguousChannelBuffer (heap corruption observed at
    // very high playback rates).
    // The fixture itself is friended so its protected accessor helpers
    // (bufferBegin / bufferEnd / channelStride / currentChunkSize) compile.
    // FRIEND_TEST alone only grants access to the generated test classes,
    // not to the inherited fixture methods they call.
    friend class ::EngineBufferScaleBungeeBufferWindowTest;
    FRIEND_TEST(EngineBufferScaleBungeeBufferWindowTest,
            DiscardWithGapBeyondEndJumpsBothPointersToFramePosition);
    FRIEND_TEST(EngineBufferScaleBungeeBufferWindowTest,
            DiscardWithGapBeyondEndConsumesSkippedReadAheadFrames);
    FRIEND_TEST(EngineBufferScaleBungeeBufferWindowTest,
            DiscardWithGapBeyondEndRetriesAfterTransientZeroRead);
    FRIEND_TEST(EngineBufferScaleBungeeBufferWindowTest,
            DiscardWithGapBeyondEndConsumesPartialReadsBeforeCompleting);
    FRIEND_TEST(EngineBufferScaleBungeeBufferWindowTest,
            DiscardWhenFramePositionInsideBufferDoesNotOverJump);
    FRIEND_TEST(EngineBufferScaleBungeeBufferWindowTest,
            DiscardWhenBufferEmptyJumpsToFramePosition);
    FRIEND_TEST(EngineBufferScaleBungeeBufferWindowTest,
            HighSpeedGrainOutrunMaintainsDataOffsetInvariant);
    FRIEND_TEST(EngineBufferScaleBungeeFlushAccountingTest,
            CopyFlushOutputFramesReportsEffectiveRateAdvance);
};
