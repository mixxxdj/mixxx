/***************************************************************************
 *      enginefilter.h - Wrapper for FidLib Filter Library                 *
 *                      ----------------------                             *
 *   copyright      : (C) 2007 by John Sully                               *
 *   email          : jsully@scs.ryerson.ca                                *
 *                                                                         *
 **************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef ENGINEFILTER_H
#define ENGINEFILTER_H

#define MIXXX
#include "engine/engineobject.h"
#include "../lib/fidlib-0.9.10/fidlib.h"
#include "defs.h"

enum filterType{
	FILTER_LOWPASS,
	FILTER_BANDPASS,
	FILTER_HIGHPASS
};

#define PREDEF_HP 1
#define PREDEF_BP 2
#define PREDEF_LP 3

class EngineFilter : public EngineObject
{
public:
    EngineFilter( char *conf, int predefinedType = 0);
    ~EngineFilter();
    void process(const CSAMPLE *pIn, const CSAMPLE *ppOut, const int iBufferSize);
protected:
    const double *coefs;
    double iir;
    double fir;
    double tmp;
#define FILTER_BUF_SIZE 16
    double buf1[FILTER_BUF_SIZE];
	double buf2[FILTER_BUF_SIZE];
private:
    double (*processSample)(void *buf, const double sample);

    FidFilter *ff;
    FidFunc *funcp;
    FidRun *run;
    void *fbuf1;
    void *fbuf2;
};

double processSampleDynamic(void *buf, const double sample);
double processSampleHp(void *buf, const double sample);
double processSampleBp(void *buf, const double sample);
double processSampleLp(void *buf, const double sample);

#endif
