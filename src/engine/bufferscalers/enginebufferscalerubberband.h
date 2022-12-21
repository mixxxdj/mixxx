#pragma once

#include <rubberband/RubberBandStretcher.h>

#include <array>

#include "engine/bufferscalers/enginebufferscale.h"
#include "util/memory.h"
#include "util/samplebuffer.h"

class ReadAheadManager;

// Uses librubberband to scale audio.  This class is not thread safe.
class EngineBufferScaleRubberBand final : public EngineBufferScale {
    Q_OBJECT
  public:
    explicit EngineBufferScaleRubberBand(
            ReadAheadManager* pReadAheadManager);

    EngineBufferScaleRubberBand(const EngineBufferScaleRubberBand&) = delete;
    EngineBufferScaleRubberBand& operator=(const EngineBufferScaleRubberBand&) = delete;

    EngineBufferScaleRubberBand(EngineBufferScaleRubberBand&&) = delete;
    EngineBufferScaleRubberBand& operator=(EngineBufferScaleRubberBand&&) = delete;

    // Let EngineBuffer know if engine v3 is available
    static bool isEngineFinerAvailable();

    // Enable engine v3 if available
    void useEngineFiner(bool enable);

    void setScaleParameters(double base_rate,
                            double* pTempoRatio,
                            double* pPitchRatio) override;

    double scaleBuffer(
            CSAMPLE* pOutputBuffer,
            SINT iOutputBufferSize) override;

    // Flush buffer.
    void clear() override;

  private:
    // Reset RubberBand library with new audio signal
    void onSampleRateChanged() override;

    /// Calls `m_pRubberBand->getPreferredStartPad()`, with backwards
    /// compatibility for older librubberband versions.
    size_t getPreferredStartPad() const;
    /// Calls `m_pRubberBand->getStartDelay()`, with backwards compatibility for
    /// older librubberband versions.
    size_t getStartDelay() const;
    int runningEngineVersion();
    /// Reset the rubberband instance and run the prerequisite amount of padding
    /// through it. This should be used instead of calling
    /// `m_pRubberBand->reset()` directly.
    void reset();

    void deinterleaveAndProcess(const CSAMPLE* pBuffer, SINT frames, bool flush);
    SINT retrieveAndDeinterleave(CSAMPLE* pBuffer, SINT frames);

    // The read-ahead manager that we use to fetch samples
    ReadAheadManager* m_pReadAheadManager;

    std::unique_ptr<RubberBand::RubberBandStretcher> m_pRubberBand;

    /// The audio buffers samples used to send audio to Rubber Band and to
    /// receive processed audio from Rubber Band. This is needed because Mixxx
    /// uses interleaved buffers in most other places.
    std::array<mixxx::SampleBuffer, 2> m_buffers;
    /// These point to the buffers in `m_buffers`. They can be defined here
    /// since this object cannot be moved or copied.
    std::array<float*, 2> m_bufferPtrs;

    /// Contains interleaved samples read from `m_pReadAheadManager`. These need
    /// to be deinterleaved before they can be passed to Rubber Band.
    mixxx::SampleBuffer m_interleavedReadBuffer;

    // Holds the playback direction
    bool m_bBackwards;
    /// The amount of silence padding that still needs to be dropped from the
    /// retrieve samples in `retrieveAndDeinterleave()`. See the `reset()`
    /// function for an explanation.
    SINT m_remainingPaddingInOutput = 0;

    bool m_useEngineFiner;
};
