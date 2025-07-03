#include "engine/bufferscalers/enginebufferscalesr.h"

#include <QDebug>

#include "engine/readaheadmanager.h"
#include "moc_enginebufferscalesr.cpp"
#include "util/math.h"
#include "util/sample.h"

EngineBufferScaleSR::EngineBufferScaleSR(ReadAheadManager* pReadAheadManager)
        : m_pReadAheadManager(pReadAheadManager),
          m_bBackwards(false) {
    qDebug() << "constructed enginebufferscaleSR";
}

EngineBufferScaleSR::~EngineBufferScaleSR() {
}

void EngineBufferScaleSR::setQuality(double engine_quality) {
    m_pEngineQuality = engine_quality;
}

// these parameters describe the "request" that
// needs to be handled by libsamplerate.
// using soundtouch as template.
void EngineBufferScaleSR::setScaleParameters(double base_rate,
        double* pTempoRatio,
        double* pPitchRatio) {
    (void)base_rate;
    (void)pTempoRatio;
    (void)pPitchRatio;
}

void EngineBufferScaleSR::onSignalChanged() {
    qDebug() << "onSignalChanged called";
}

void EngineBufferScaleSR::clear() {
    qDebug() << "clear called";
}

// not called during regular playback
double EngineBufferScaleSR::scaleBuffer(
        CSAMPLE* pOutputBuffer,
        SINT iOutputBufferSize) {
    qDebug() << "scaling using libsamplerate";
    (void)pOutputBuffer;
    (void)iOutputBufferSize;
    return 1.0;
}
