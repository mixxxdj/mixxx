#pragma once

#include "engine/bufferscalers/enginebufferscale.h"
#include "util/samplebuffer.h"

class ReadAheadManager;

/** Number of samples to read ahead */
constexpr int kiLinearScaleReadAheadLength = 10240;

class EngineBufferScaleLinear : public EngineBufferScale  {
    Q_OBJECT
  public:
    explicit EngineBufferScaleLinear(
            ReadAheadManager* pReadAheadManager);
    ~EngineBufferScaleLinear() override;

    double scaleBuffer(
            CSAMPLE* pOutputBuffer,
            SINT iOutputBufferSize) override;
    void clear() override;

    void setScaleParameters(double base_rate,
                            double* pTempoRatio,
                             double* pPitchRatio) override;

  private:
    void onSignalChanged() override;

    double do_scale(CSAMPLE* buf, SINT buf_size);
    SINT do_copy(CSAMPLE* buf, SINT buf_size);
    double do_direction_change(CSAMPLE* buf, SINT bufSize, double rateOld, double rateNew);
    void do_refill_buffer(double rate,
            SINT* pUnscaledFramesNeeded,
            double* pStartFrame,
            SINT* pCurrentFrameFloor);
    SINT do_fetch_samples(double rate,
            SINT* pUnscaledFramesNeeded,
            double* pStartFrame);

    // The read-ahead manager that we use to fetch samples
    ReadAheadManager* m_pReadAheadManager;

    // Buffer for handling calls to ReadAheadManager
    CSAMPLE* m_bufferInt;
    SINT m_bufferIntSize;

    mixxx::SampleBuffer m_floorSampleOld;
    mixxx::SampleBuffer m_floorSample;
    mixxx::SampleBuffer m_ceilSample;

    bool m_bClear;
    double m_dRate;
    double m_dOldRate;

    double m_dCurrentFrame;
    double m_dNextFrame;
};
