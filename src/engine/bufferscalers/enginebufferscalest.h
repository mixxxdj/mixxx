#pragma once

#include <memory>

#include "engine/bufferscalers/enginebufferscale.h"
#include "util/samplebuffer.h"

class ReadAheadManager;

namespace soundtouch {
class SoundTouch;
}  // namespace soundtouch

// Uses libsoundtouch to scale audio.
class EngineBufferScaleST : public EngineBufferScale {
    Q_OBJECT
  public:
    explicit EngineBufferScaleST(
            ReadAheadManager* pReadAheadManager);
    ~EngineBufferScaleST() override;

    void setScaleParameters(double base_rate,
                            double* pTempoRatio,
                            double* pPitchRatio) override;

    // Scale buffer.
    double scaleBuffer(
            CSAMPLE* pOutputBuffer,
            SINT iOutputBufferSize) override;

    // Flush buffer.
    void clear() override;

  private:
    void onSignalChanged() override;

    // The read-ahead manager that we use to fetch samples
    ReadAheadManager* m_pReadAheadManager;

    // SoundTouch time/pitch scaling lib
    std::unique_ptr<soundtouch::SoundTouch> m_pSoundTouch;

    // Temporary buffer for reading from the RAMAN.
    mixxx::SampleBuffer m_bufferBack;

    // Holds the playback direction.
    bool m_bBackwards;

    // SoundTouch runs its anti-alias FIR filter even when it does not
    // resample (rate 1.0, e.g. keylock at the original key on a file with the
    // engine sample rate). Its multichannel path is plain C: with an 8-channel
    // stem deck it took 40% of the audio thread on ARM64 and caused underruns.
    // Enabled only when the rate is not 1.0.
    void updateAntiAliasFilter();
    bool m_bAntiAliasFilter;
};
