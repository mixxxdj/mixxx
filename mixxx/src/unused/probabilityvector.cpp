/***************************************************************************
                          probabilityvector.cpp  -  description
                             -------------------
    begin                : Fri Feb 14 2003
    copyright            : (C) 2003 by Tue & Ken Haste Andersen
    email                : haste@diku.dk
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "qapplication.h"
#include "probabilityvector.h"
#include "mathstuff.h"

ProbabilityVector::ProbabilityVector(float fMinInterval, float fMaxInterval, int iBins)
{
    m_fMinInterval = fMinInterval;
    m_fMaxInterval = fMaxInterval;
    m_iBins = iBins;
    m_fSecPerBin = (fMaxInterval-fMinInterval)/m_iBins;

    m_pHist = new float[m_iBins];

    reset();
}

ProbabilityVector::~ProbabilityVector()
{
    delete [] m_pHist;
}

void ProbabilityVector::setBpm(float fBpm, float fBpmConfidence)
{
    // Find hist position from fBpm and insert fBpmConfidence.
    m_fCurrMaxInterval = (1./fBpm)*60.;
    m_iCurrMaxBin = math_max(0, (int)((m_fCurrMaxInterval-m_fMinInterval)/m_fSecPerBin) );
    m_pHist[m_iCurrMaxBin] = fBpmConfidence;
}

void ProbabilityVector::add(float fInterval, float fValue)
{
    if (fInterval>=m_fMinInterval && fInterval<=m_fMaxInterval)
    {
        // Histogram is updated with a gauss function centered at the found interval
        float fCenter =  (fInterval-m_fMinInterval)/m_fSecPerBin;
        float fStart  = math_min( (float)m_iBins-1 - fCenter, -math_min(kiGaussWidth, fCenter) );
        float fEnd    =  math_min(kiGaussWidth, (float)(m_iBins-1)-fCenter);

        for (float j=fStart; j<fEnd; j++)
        {
            int idx = (int)round((fCenter+j));
            m_pHist[idx] += exp((-0.5*j*j)/(0.5*(CSAMPLE)kiGaussWidth))*fValue; //*fHysterisisFactor;
            if (m_pHist[idx]>m_pHist[m_iCurrMaxBin])
            {
                m_iCurrMaxBin = idx;

                // Only use the newly found max if the distance to the last max is small, or it is
                // bigger than the old max times the hysterisis factor
                if (m_iLastMaxBin<0 || abs(m_iLastMaxBin-m_iCurrMaxBin)<kiGaussWidth || m_pHist[m_iCurrMaxBin]>m_pHist[m_iLastMaxBin]*kfHysterisis)
                {
                    m_iLastMaxBin = m_iCurrMaxBin;

                    // Interpolate maximum
                    float fCorr = 0.;
                    if (m_iCurrMaxBin>1 && m_iCurrMaxBin<m_iBins-2)
                    {
                        float t  = m_pHist[m_iCurrMaxBin];
                        float t1 = m_pHist[m_iCurrMaxBin-1];
                        float t2 = m_pHist[m_iCurrMaxBin+1];

                        if ((t1-2.0*t+t2) != 0.)
                            fCorr = (0.5*(t1-t2))/(t1-2.*t+t2);
                    }

                    // Interval in seconds
                    m_fCurrMaxInterval = ((float)m_iCurrMaxBin+fCorr)*m_fSecPerBin+m_fMinInterval;

                    // Check if the current interval has a higher histogram value than the one in m_fBestBpmValue
                    if (m_pHist[m_iCurrMaxBin]>m_fBestBpmConfidence)
                    {
                        m_fBestBpmConfidence = m_pHist[m_iCurrMaxBin];
                        m_fBestBpmValue = 60./m_fCurrMaxInterval;
                    }

//                    qDebug() << "hist idx " << m_iCurrMaxBin << ", int " << m_fCurrMaxInterval << ", corr " << fCorr << ", bpm " << 60./m_fCurrMaxInterval;
                }
            }
        }
    }


/*
 #ifdef __GNUPLOT__
    //
    // Plot Histogram
    //
    setLineType(gnuplot_hist,"lines");
    plotData(hist, histSize, gnuplot_hist, plotFloats);

    setLineType(gnuplot_hist,"points");
    float _maxidx = (float)histMaxIdx+histMaxCorr;
    replotxy(&_maxidx, &hist[histMaxIdx], 1, gnuplot_hist);

    //savePlot(gnuplot_hist, "hist.png", "png");
 #endif
 */
}

float ProbabilityVector::getCurrMaxInterval()
{
    return m_fCurrMaxInterval;
}

void ProbabilityVector::downWrite(float fFactor)
{
#ifdef FILEOUTPUT
    QTextStream streamhist(&texthist);
#endif

    for (int i=0; i<m_iBins; ++i)
    {
        m_pHist[i] = fFactor*m_pHist[i];
#ifdef FILEOUTPUT
//        streamhist << m_pHist[i] << " ";
#endif
    }
#ifdef FILEOUTPUT
//    streamhist << "\n";
#endif
}

void ProbabilityVector::reset()
{
    for (int i=0; i<m_iBins; ++i)
        m_pHist[i] = 0;

    m_iCurrMaxBin = -1;
    m_iLastMaxBin = -1;
    m_fCurrMaxInterval = 0.;

    m_fBestBpmValue = 0.;
    m_fBestBpmConfidence = 0.;
}

void ProbabilityVector::newsource(QString qFilename)
{
#ifdef FILEOUTPUT
    texthist.close();
    texthist.setName(QString(qFilename).append(".hist"));
    texthist.open(QIODevice::WriteOnly);
#endif
}

float ProbabilityVector::getBestBpmValue()
{
    return m_fBestBpmValue;
}

float ProbabilityVector::getBestBpmConfidence()
{
    return m_fBestBpmConfidence;
}
