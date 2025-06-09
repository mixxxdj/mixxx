#include "engine/bufferscalers/enginebufferscalesr.h"

#include <QDebug>

#include "engine/readaheadmanager.h"
#include "moc_enginebufferscalesr.cpp"
#include "util/math.h"
#include "util/sample.h"

EngineBufferScaleSR::EngineBufferScaleSR(ReadAheadManager* pReadAheadManager)
        : m_pReadAheadManager(pReadAheadManager) {
    qDebug() << "constructed enginebufferscaleSR";
}

EngineBufferScaleSR::~EngineBufferScaleSR() {
}

void EngineBufferScaleSR::onSignalChanged(){
        qDebug() << "onSignalChanged called;
}

void EngineBufferScaleSR::clear() {
    qDebug() << "clear called";
}

void EngineBufferScaleSR::setScaleParameters(double base_rate,
        double* pTempoRatio,
        double* pPitchRatio) {
    qDebug() << "setting scale params";
}

double EngineBufferScaleSR::scaleBuffer(
        CSAMPLE* pOutputBuffer,
        SINT iOutputBufferSize) {
    qDebug() << "scaling using libsamplerate";
    return 0.0;
}
