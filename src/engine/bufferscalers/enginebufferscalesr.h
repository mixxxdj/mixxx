/**
 * FILE: enginebufferscalesr.h
 * -----------------------------
 * Scaler class that uses the libsamplerate API
 */

#pragma once

extern "C" {
#include <samplerate.h>
}

#include <memory>

#include "engine/bufferscalers/enginebufferscale.h"
#include "util/samplebuffer.h"

class ReadAheadManager;

class EngineBufferScaleSR : public EngineBufferScale {
    Q_OBJECT
  public:
    explicit EngineBufferScaleSR(
            ReadAheadManager* pReadAheadManager, double e_Index);
    ~EngineBufferScaleSR() override;

    void setScaleParameters(double base_rate,
            double* pTempoRatio,
            double* pPitchRatio) override;

    void setQuality(double engine_quality);
    long getInputFrames(float** ppAudio);

    // Main scaler method
    double scaleBuffer(
            CSAMPLE* pOutputBuffer,
            SINT iOutputBufferSize) override;

    void clear() override;

  private:
    void onSignalChanged() override;
    long do_scale(CSAMPLE* pOutput, SINT outFrames);

    ReadAheadManager* m_pReadAheadManager;
    bool m_bBackwards;
    mixxx::audio::ChannelCount m_dChannels;
    double m_inputFramesRead;
    double m_lastPositionOld;
    double m_savedFramesOld;

    SRC_STATE* m_pResampler;
    int m_dQuality;                   // resampler quality for the this enginebuffer(scaler)
    mixxx::SampleBuffer m_bufferBack; // to hold samples from RAMAN
};
