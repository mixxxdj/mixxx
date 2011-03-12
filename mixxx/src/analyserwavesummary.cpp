#include <QtDebug>

#include "trackinfoobject.h"
#include "analyserwavesummary.h"

// Taken from the old wavesummary.cpp code
#ifndef WAVESUMMARYCONSTANTS
const int kiBlockSize = 2048;
const int kiBeatBlockNo = 1000;
const int kiBeatBins = 100;
const int kiSummaryBufferSize = 2100;
const float kfFeatureStepSize = 0.01f;
#define WAVESUMMARYCONSTANTS
#endif

AnalyserWavesummary::AnalyserWavesummary() {
    m_pData = NULL;
}

void AnalyserWavesummary::initialise(TrackPointer tio, int sampleRate, int totalSamples) {
    Q_UNUSED(sampleRate);
    // Check if the preview has already been generated
    const QByteArray* p = tio->getWaveSummary();
    if(p != NULL && p->size() > 0) {
        return;
    }

    // Initialize kiSummaryBufferSize bytes to 0
    m_pData = new QByteArray(kiSummaryBufferSize, 0);

    // The stride length is the number of samples that correspond
    // to one "line" (3 entries) in the data buffer.
    m_iStrideLength = (int)ceilf((float)totalSamples/((float)kiSummaryBufferSize/3.));
    if(m_iStrideLength%2 != 0)
        m_iStrideLength--;

    m_iCurPos = 0;
    m_iBufferPos = 0;
    m_fMax = -1.0f;
    m_fMin = 1.0f;
}

void AnalyserWavesummary::process(const CSAMPLE *pIn, const int iLen) {
    // Check if processing is disabled.
    if(m_pData == NULL)
        return;

    //qDebug() << "AnalyserWavesummary::process() processing " << iLen << " samples";

    for(int i=0; i<iLen && m_iCurPos+2 < m_pData->size() ; i++) {
        if(m_iBufferPos >= m_iStrideLength) {
            (*m_pData)[m_iCurPos] = (char)(m_fMin*127);
            (*m_pData)[m_iCurPos+1] = (char)(m_fMax*127);
            (*m_pData)[m_iCurPos+2] = 0;
            m_iCurPos += 3;

            m_iBufferPos = 0;
            m_fMax = -1.0f;
            m_fMin = 1.0f;
        }

        if(m_iBufferPos <= kiBlockSize) {
            if(pIn[i] > m_fMax)
                m_fMax = pIn[i];
            if(pIn[i] < m_fMin)
                m_fMin = pIn[i];
        }
        m_iBufferPos++;
    }
}

void AnalyserWavesummary::finalise(TrackPointer tio) {
    if(m_pData == NULL)
        return;
    tio->setWaveSummary(m_pData, true);
    //setWaveSummary copies the waveform from the pointer we pass it, so it's safe
    //to delete the pointer.
    delete m_pData;
    m_pData = NULL;
    //qDebug() << "AnalyserWavesummary generation successful for " << tio->getFilename();
}
