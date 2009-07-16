/***************************************************************************
                          probabilityvector.h  -  description
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

#ifndef PROBABILITYVECTOR_H
#define PROBABILITYVECTOR_H

#ifdef FILEOUTPUT
    #include <qfile.h>
    #include <qstring.h>
#endif

/** Width of gauss/2 used in histogram updates */
const int kiGaussWidth = 8;
/** Hysterisis factor. */
const float kfHysterisis = 2.0f;

/**
  *@author Tue & Ken Haste Andersen
  */


class ProbabilityVector {
public: 
    /** Constructs the histogram with intervals ranging from fMinInterval to
      * fMaxInterval seconds, using iBins number of bins */
    ProbabilityVector(float fMinInterval, float fMaxInterval, int iBins);
    ~ProbabilityVector();
    /** Initialize the BPV with a given BPM at a given confidence */
    void setBpm(float fBpm, float fBpmConfidence);
    /** Add an interval to the histogram with a given value*/
    void add(float fInterval, float fValue);
    /** Returns the current maximum interval of the histogram in seconds */
    float getCurrMaxInterval();
    /** Down write the histogram with the given factor, ie. newval = oldval*fFactor */
    void downWrite(float fFactor);
    /** Reset the probability vector */
    void reset();
    /** New file */
    void newsource(QString qFilename);
    /** Returns best BPM value (BPM with highest value in histogram) for this track */
    float getBestBpmValue();
    /** Returns histogram value for the best BPM */
    float getBestBpmConfidence();
private:
    /** Number of bins in histogram */
    int m_iBins;
    /** Number of seconds covered by each bin */
    float m_fSecPerBin;
    /** Minimum and maximum interval in seconds considered in the histogram */
    float m_fMinInterval, m_fMaxInterval;
    /** Pointer to histogram */
    float *m_pHist;
    /** Current maximum interval of the histogram in seconds */
    float m_fCurrMaxInterval;
    /** Current bin corresponding to m_fCurrMaxInterval */
    int m_iCurrMaxBin;
    /** Bin corresponding to last max interval */
    int m_iLastMaxBin;
    /** BPM value with highest value in histogram, and the histogram value */
    float m_fBestBpmValue, m_fBestBpmConfidence;

#ifdef FILEOUTPUT
    QFile texthist;
#endif

};

#endif
