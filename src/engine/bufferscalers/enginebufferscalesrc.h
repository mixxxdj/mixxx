/**
 * FILE: enginebufferscalesrc.h
 * -----------------------------
 * Scaler class that uses the libsamplerate (Secret Rabbit Code) API
 */

#pragma once

#include <samplerate.h>

#include <memory>

#include "engine/bufferscalers/enginebufferscale.h"
#include "util/samplebuffer.h"

class ReadAheadManager;

class EngineBufferScaleSRC : public EngineBufferScale {
    Q_OBJECT
  public:
    explicit EngineBufferScaleSRC(); // input driven
    ~EngineBufferScaleSRC() override;

    // Main scaler method
    double scaleBuffer(
            CSAMPLE* pOutputBuffer,
            SINT iOutputBufferSize) override;

    double recScaleBuffer(const CSAMPLE* pInputBuffer,
            CSAMPLE* pOutputBuffer,
            SINT iInputBufferSize,
            double baseRate);

    void clear() override;

  private:
    void onSignalChanged() override;

    mixxx::audio::ChannelCount m_dChannels;
    SRC_STATE* m_pResampler;
};
