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

#include "probabilityvector.h"
#include "mathstuff.h"
#include "qapplication.h"

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

void ProbabilityVector::add(float fInterval, float fValue)
{
    if (fInterval>=m_fMinInterval && fInterval<=m_fMaxInterval)
    {
        // Histogram is updated with a gauss function centered at the found interval
        float fCenter =  (fInterval-m_fMinInterval)/m_fSecPerBin;
        float fStart  = -min(kiGaussWidth, fCenter);
        float fEnd    =  min(kiGaussWidth, (float)(m_iBins-1)-fCenter);

        // Set hysterisisFactor. If the gauss is within histMaxIdx, use a large hysterisisFactor
        float fHysterisisFactor = 1.;
        if ((int)fCenter>m_iCurrMaxBin-kiGaussWidth && (int)fCenter<m_iCurrMaxBin+kiGaussWidth)
            fHysterisisFactor = kfHysterisis;

        for (float j=fStart; j<fEnd; j++)
        {
            int idx = round((fCenter+j));
            m_pHist[idx] += exp((-0.5*j*j)/(0.5*(CSAMPLE)kiGaussWidth))*fValue*fHysterisisFactor;
            if (m_pHist[idx]>m_pHist[m_iCurrMaxBin])
            {
                m_iCurrMaxBin = idx;

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
//                qDebug("hist idx %i, int %f, corr %f, bpm %f",m_iCurrMaxBin, m_fCurrMaxInterval,fCorr, 60./m_fCurrMaxInterval);
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
        streamhist << m_pHist[i] << " ";
#endif
    }
#ifdef FILEOUTPUT
    streamhist << "\n";        
#endif
}

void ProbabilityVector::reset()
{
    for (int i=0; i<m_iBins; ++i)
        m_pHist[i] = 0;

    m_iCurrMaxBin = -1;
    m_fCurrMaxInterval = 0.;
}

void ProbabilityVector::newsource(QString qFilename)
{
#ifdef FILEOUTPUT
    texthist.close();
    texthist.setName(QString(qFilename).append(".hist"));
    texthist.open(IO_WriteOnly);
#endif
}
    
