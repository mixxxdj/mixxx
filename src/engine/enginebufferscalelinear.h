#ifndef ENGINEBUFFERSCALELINEAR_H
#define ENGINEBUFFERSCALELINEAR_H

#include "engine/enginebufferscale.h"
#include "engine/readaheadmanager.h"

/** Number of samples to read ahead */
const int kiLinearScaleReadAheadLength = 10240;


class EngineBufferScaleLinear : public EngineBufferScale  {
  public:
    explicit EngineBufferScaleLinear(
            ReadAheadManager *pReadAheadManager);
    ~EngineBufferScaleLinear() override;

    double getScaledSampleFrames(
            CSAMPLE* pOutputBuffer,
            SINT iOutputBufferSize) override;
    void clear() override;

    void setScaleParameters(double base_rate,
                            double* pTempoRatio,
                             double* pPitchRatio) override;

  private:
    int do_scale(CSAMPLE* buf, const int buf_size);
    int do_copy(CSAMPLE* buf, const int buf_size);

    // The read-ahead manager that we use to fetch samples
    ReadAheadManager* m_pReadAheadManager;

    // Buffer for handling calls to ReadAheadManager
    CSAMPLE* m_bufferInt;
    int m_bufferIntSize;

    CSAMPLE m_floorSampleOld[2];

    bool m_bClear;
    double m_dRate;
    double m_dOldRate;

    double m_dCurrentFrame;
    double m_dNextFrame;
};

#endif
