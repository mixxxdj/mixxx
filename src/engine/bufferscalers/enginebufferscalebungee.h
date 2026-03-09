#pragma once

#include <bungee/Bungee.h>

#include <array>
#include <memory>
#include <vector>

#include "engine/bufferscalers/enginebufferscale.h"
#include "util/samplebuffer.h"

class ReadAheadManager;

// Uses Bungee library to scale audio. This class is not thread safe.
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

    void clear() override;

  private:
    void onSignalChanged() override;

    // Process a single grain and return the number of output frames produced
    SINT processGrain(CSAMPLE* pOutputBuffer, SINT maxFrames);

    // Deinterleave input data into channel buffers for Bungee
    void deinterleaveInput(const CSAMPLE* pBuffer, SINT frames);

    // The read-ahead manager that we use to fetch samples
    ReadAheadManager* m_pReadAheadManager;

    // Bungee stretcher instance (using Basic edition)
    std::unique_ptr<Bungee::Stretcher<Bungee::Basic>> m_pStretcher;

    // Current Bungee request for grain processing
    Bungee::Request m_request;

    // Output chunk for synthesiseGrain
    Bungee::OutputChunk m_outputChunk;

    // Deinterleaved channel buffers for Bungee input/output
    std::vector<mixxx::SampleBuffer> m_channelBuffers;
    std::vector<float*> m_channelBufferPtrs;

    // Single contiguous buffer for all channels (for Bungee's planar format)
    mixxx::SampleBuffer m_contiguousChannelBuffer;

    // Interleaved read buffer from ReadAheadManager
    mixxx::SampleBuffer m_interleavedReadBuffer;

    // Playback direction
    bool m_bBackwards;

    // Current output channel stride
    SINT m_channelStride;

    // Current grain's input chunk
    Bungee::InputChunk m_currentInputChunk;

    // Position tracking for continuous grain processing
    double m_grainPosition;

    // Whether we need to reset on next process
    bool m_bResetNeeded;

    // Output frames remaining from previous grain
    SINT m_remainingOutputFrames;
    SINT m_outputChunkConsumed;

    // Static maximum buffer size for grain processing
    static constexpr SINT kMaxGrainFrames = 4096;

    // Buffer size for input (set dynamically based on Bungee's requirements)
    SINT m_inputBufferFrames;
};
