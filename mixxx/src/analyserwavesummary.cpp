#include <q3memarray.h>
#include <QtDebug>

#include "trackinfoobject.h"
#include "analyserwavesummary.h"

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

void AnalyserWavesummary::initialise(TrackInfoObject* tio, int sampleRate, int totalSamples) {
    Q3MemArray<char> *p = tio->getWaveSummary();
    if(p != NULL && p->size() > 0) {
        return;
    }
    
    m_pData = new Q3MemArray<char>(kiSummaryBufferSize);

    for (int i=0; i<m_pData->size(); i++)
        m_pData->at(i) = 0;

    m_iStrideLength = (int)ceilf((float)totalSamples/((float)kiSummaryBufferSize/3.));
    if(m_iStrideLength%2 != 0)
        m_iStrideLength--;

    m_iCurPos = 0;
    m_iBufferPos = 0;
    m_fMax = -1.0f;
    m_fMin = 1.0f;
}

void AnalyserWavesummary::process(const CSAMPLE *pIn, const int iLen) {
    if(m_pData == NULL)
        return;

    //qDebug() << "AnalyserWavesummary::process() processing " << iLen << " samples";
    for(int i=0; i<iLen; i++) {
        if(m_iBufferPos >= m_iStrideLength) {
            m_pData->at(m_iCurPos) = (int)(m_fMin*127);
            m_pData->at(m_iCurPos+1) = (int)(m_fMax*127);
            m_pData->at(m_iCurPos+2) = 0;
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

void AnalyserWavesummary::finalise(TrackInfoObject *tio) {
    if(m_pData == NULL)
        return;
    tio->setWaveSummary(m_pData, 0, true);
    m_pData = NULL;
    qDebug() << "AnalyserWavesummary generation successful for " << tio->getFilename();
}
