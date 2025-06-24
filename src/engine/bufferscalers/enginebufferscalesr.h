/**
 * FILE: enginebufferscalesr.h
 * -----------------------------
 * Scaler class that uses the libsamplerate API
 */

#pragma once

#include <samplerate.h>

#include <memory>

#include "engine/bufferscalers/enginebufferscale.h"
#include "util/samplebuffer.h"

class ReadAheadManager;

class EngineBufferScaleSR : public EngineBufferScale {
    Q_OBJECT
  public:
    explicit EngineBufferScaleSR(
            ReadAheadManager* pReadAheadManager, double eIndex);
    ~EngineBufferScaleSR() override;

    void setScaleParameters(double base_rate,
            double* pTempoRatio,
            double* pPitchRatio) override;

    void setQuality(double engine_quality);

    // Main scaler method
    double scaleBuffer(
            CSAMPLE* pOutputBuffer,
            SINT iOutputBufferSize) override;

    void clear() override;

  private:
    void onSignalChanged() override;

    ReadAheadManager* m_pReadAheadManager;
    bool m_bBackwards;
    mixxx::audio::ChannelCount m_dChannels;

    SRC_STATE* m_pResampler;
    mixxx::SampleBuffer m_bufferBack; // to hold samples from RAMAN
};
