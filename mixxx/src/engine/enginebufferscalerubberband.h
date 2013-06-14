#ifndef ENGINEBUFFERSCALERUBBERBAND_H
#define ENGINEBUFFERSCALERUBBERBAND_H

#include <QMutex>

#include "engine/enginebufferscale.h"

namespace RubberBand {
class RubberBandStretcher;
}  // namespace RubberBand

class ReadAheadManager;

class EngineBufferScaleRubberBand : public EngineBufferScale {
    Q_OBJECT
  public:
    EngineBufferScaleRubberBand(ReadAheadManager* pReadAheadManager);
    virtual ~EngineBufferScaleRubberBand();

    void setScaleParameters(double* rate_adjust,
                            double* tempo_adjust,
                            double* pitch_adjust);

    /** Scale buffer */
    CSAMPLE* getScaled(unsigned long buf_size);

    /** Flush buffer */
    void clear();

  public slots:
    void slotSetSamplerate(double dSampleRate);

  private:
    void initializeRubberBand(int iSampleRate);
    void deinterleaveAndProcess(const CSAMPLE* pBuffer, size_t frames, bool flush);
    size_t retrieveAndDeinterleave(CSAMPLE* pBuffer, size_t frames);

    // Holds the playback direction
    bool m_bBackwards;

    CSAMPLE* m_retrieve_buffer[2];
    CSAMPLE* m_buffer_back;

    // Used when clear is called
    bool m_bClear;

    // Used to protect RubberBand calls
    QMutex m_qMutex;

    RubberBand::RubberBandStretcher* m_pRubberBand;

    // The read-ahead manager that we use to fetch samples
    ReadAheadManager* m_pReadAheadManager;
};


#endif /* ENGINEBUFFERSCALERUBBERBAND_H */
