#pragma once

#include <qtmetamacros.h>

#include "engine/bufferscalers/enginebufferscale.h"
#include "signalsmith-stretch.h"
#include "util/samplebuffer.h"

class ReadAheadManager;

class EngineBufferScaleSignalSmith final : public EngineBufferScale {
    Q_OBJECT
  public:
    enum class Preset {
        Default,
        Cheaper
    };
    Q_ENUM(Preset);

    explicit EngineBufferScaleSignalSmith(ReadAheadManager* pReadAheadManager);
    ~EngineBufferScaleSignalSmith() override = default;

    void setScaleParameters(double base_rate, double* pTempoRatio, double* pPitchRatio) override;
    void setPreset(Preset preset) {
        m_currentPreset = preset;
    }
    void clear() override;
    double scaleBuffer(CSAMPLE* pOutputBuffer, SINT iOutputBufferSize) override;

  private:
    void onSignalChanged() override;
    SINT fetchAndDeinterleave(SINT frames, SINT offset = 0);

    ReadAheadManager* m_pReadAheadManager;
    signalsmith::stretch::SignalsmithStretch<float> m_stretch;

    /// The audio buffers samples used to send audio to Rubber Band and to
    /// receive processed audio from Rubber Band. This is needed because Mixxx
    /// uses interleaved buffers in most other places.
    std::vector<mixxx::SampleBuffer> m_buffers;
    /// These point to the buffers in `m_buffers`. They can be defined here
    /// since this object cannot be moved or copied.
    std::vector<float*> m_bufferPtrs;

    /// Contains interleaved samples read from `m_pReadAheadManager`. These need
    /// to be deinterleaved before they can be passed to Rubber Band.
    mixxx::SampleBuffer m_interleavedBuffer;

    mixxx::SampleBuffer m_buffer;
    SINT m_currentFrameOffset;
    // Holds the playback direction
    bool m_bBackwards;
    Preset m_currentPreset;
};
