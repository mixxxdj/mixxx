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
#include "engineobject.h"
#include "../lib/fidlib-0.9.9/fidlib.h"
#include "defs.h"

#define BUF_SIZE
class EngineFilter : public EngineObject
{
public:
    EngineFilter( char *conf);
    ~EngineFilter();
    void process(const CSAMPLE *pIn, const CSAMPLE *ppOut, const int iBufferSize);
protected:
    const double *coefs;
    double iir;
    double fir;
    double tmp;
    double buf[BUF_SIZE];
private:
    double processSample(const double sample, void *buf);
    FidFilter *ff;
    FidFunc *funcp;
    FidRun *run;
    void *fbuf1;
    void *fbuf2;
};

#endif
