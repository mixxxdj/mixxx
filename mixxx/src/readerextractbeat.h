/***************************************************************************
                          readerextractbeat.h  -  description
                             -------------------
    begin                : Tue Mar 18 2003
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

#ifndef READEREXTRACTBEAT_H
#define READEREXTRACTBEAT_H

#include "readerextract.h"
#include "defs.h"
#include <qvaluelist.h>

#ifdef __GNUPLOT__
extern "C" {
    #include "gplot.h"
}
#endif

//#define FILEOUTPUT

#ifdef FILEOUTPUT
    #include <qfile.h>
#endif

class PeakList;
class ProbabilityVector;

/** Minimum acceptable BPM */
const CSAMPLE histMinBPM = 60.f;
/** Maximum acceptable BPM */
const CSAMPLE histMaxBPM = 200.f;
/** Down write factor of the histogram, between 0 and 1. */
const float kfHistDownWrite = 0.99f;
/** Range around predicted beat position in which it is allowed to place a beat
  * (given in seconds times two, ie. 0.05 = 0.1 seconds total range). */
const float kfBeatRange = 0.05f;
/** Range the predicted beat position in which is allowed to place a beat
  * when forcing a beat. The search is done for the nearest maximum placed
  * ahead of the predicted position in this range */
const float kfBeatRangeForce = 0.05f;
/** Confidence threashold. When confidence value goes below this value, try
  * to resyncronize beat marks, and set confidence to zero on success */
const float kfBeatConfThreshold = -0.1f;
/** Filter coefficient for confidence measure */
const float kfBeatConfFilter = 0.995f;

/**
  * Extracts beat information based on peaks in the HFC and a beat probability vector.
  * Based on algorithm in work by Kristoffer Jensen and Tue Haste Andersen. More info
  * comming at http://www.diku.dk/musinf/
  *
  *@author Tue Haste Andersen
  */

class ReaderExtractBeat : public ReaderExtract
{
public: 
    ReaderExtractBeat(ReaderExtract *input, int frameSize, int frameStep, int _histSize);
    ~ReaderExtractBeat();
    /** Reset all buffers */
    void reset();
    /** Reset buffers except histogram (BPM value) */
    void softreset();
    /** Used only when writing output to text files */
    void newsource(QString filename);
    void *getBasePtr();
    CSAMPLE *getBpmPtr();
    int getRate();
    int getChannels();
    int getBufferSize();
    void *processChunk(const int idx, const int start_idx, const int end_idx, bool backwards);
private:
    /** Updates the confidence variable. */
    void updateConfidence(int curBeatIdx, int lastBeatIdx);
    /** HFC peak list */
    PeakList *peaks;
    /** Beat probability vector histogram */
    ProbabilityVector *bpv;
        
    /** Buffer indicating if a beat has occoured or not. */
    float *beatBuffer;
    /** Buffer holding the interpolated beat position for the beats marked in beatBuffer */
    float *beatCorr;
    /** Last updated index of beatBuffer containing a beat */
    int beatBufferLastIdx;
    /** Buffer holding bpm values */
    CSAMPLE *bpmBuffer;
    /** Pointer to histogram */
    //CSAMPLE *hist;
    /** Size of histogram */
    //int histSize;
    /** Histogram interval size, and min and max interval in seconds */
    //CSAMPLE histInterval, histMinInterval, histMaxInterval;
    /** Index of maximum histogram value */
    //int histMaxIdx;
    /** Correction (second order interpolation) relative to histMaxIdx */
    //float histMaxCorr;
    /** Total number of frames */
    int frameNo;
    int framePerChunk, framePerFrameSize;
    /** Pointer to HFC array */
    CSAMPLE *hfc;
    /** Confidence measure */
    CSAMPLE confidence;

#ifdef __GNUPLOT__
    /** Pointer to gnuplot interface */
    plot_t *gnuplot_bpm;
    plot_t *gnuplot_hfc;    
#endif

#ifdef FILEOUTPUT
    QFile textbpm, textconf, textbeat, texthfc;   
#endif
};

#endif
