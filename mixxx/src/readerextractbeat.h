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
#include <q3valuelist.h>

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
const CSAMPLE histMinBPM = 70.f;
/** Maximum acceptable BPM */
const CSAMPLE histMaxBPM = 170.f;
/** Down write factor of the histogram, between 0 and 1. */
const float kfHistDownWrite = 0.991f;
/** Range around predicted beat position in which it is allowed to place a beat
  * (given in seconds times two, ie. 0.05 = 0.1 seconds total range). */
const float kfBeatRange = 0.02f; // 0.04f;
/** Range the predicted beat position in which is allowed to place a beat
  * when forcing a beat. The search is done for the nearest maximum placed
  * ahead of the predicted position in this range */
const float kfBeatRangeForce = 0.1f; // 0.04f;
/** Confidence threashold. When confidence value goes below this value, try
  * to resyncronize beat marks, and set confidence to zero on success */
const float kfBeatConfThreshold = -0.3f;
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
    ReaderExtractBeat(ReaderExtract *input, EngineBuffer *pEngineBuffer, int frameSize, int frameStep, int _histSize);
    ~ReaderExtractBeat();
    void requestNewTrack(TrackInfoObject *pTrack);
    /** Reset buffer without cleaning the histogram (BPM value) */
    void reset();
    /** Open a new sound source */
    void newSource(TrackInfoObject *pTrack);
    /** Close a sound source */
    void closeSource();
    void *getBasePtr();
    CSAMPLE *getBpmPtr();
    int getRate();
    int getChannels();
    int getBufferSize();
    double getFirstBeat();
    double getBeatInterval();
    void *processChunk(const int idx, const int start_idx, const int end_idx, bool backwards, const long signed int filepos_start);

private:
    /** Updates the confidence variable. */
    void updateConfidence(int curBeatIdx, int lastBeatIdx);
    /** Mark beat at the given index */
    void markBeat(int i);
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
    /** Total number of frames */
    int frameNo;
    int framePerChunk, framePerFrameSize;
    /** Pointer to HFC array */
    CSAMPLE *hfc;
    /** Confidence measure */
    CSAMPLE confidence;
    /** Pointer to TrackInfoObject of open file */
    TrackInfoObject *m_pTrack;
    /** First beat mark. -1 if not defined */
    double m_dBeatFirst;
    /** Beat interval in seconds. Only used if m_dBeatFirst>0 */
    double m_dBeatInterval;
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
