/***************************************************************************
 *      enginefilter.cpp - Wrapper for FidLib Filter Library	           *
 *			----------------------                             *
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

#include "enginefilter.h"

EngineFilter::EngineFilter(char *conf)
{
    ff = fid_design(conf, 44100., -1., -1., 1, NULL);
    qDebug("Filter %s Setup: 0x%X", conf, ff);
    run = fid_run_new(ff, &funcp);
    fbuf1 = fid_run_newbuf(run);
    fbuf2 = fid_run_newbuf(run);
}

EngineFilter::~EngineFilter()
{
    fid_run_freebuf(fbuf2);
    fid_run_freebuf(fbuf1);
    fid_run_free(run);
    free(ff);
}

void EngineFilter::process(const CSAMPLE *pIn, const CSAMPLE *ppOut, const int iBufferSize)
{
    CSAMPLE *pOut = (CSAMPLE*) ppOut;	//overrides the const, I don't like it but the parent class
					//forced this on us
    int i;
    for(i=0; i < iBufferSize; i+=2)
    {
	pOut[i] = (CSAMPLE) processSample((double) pIn[i], fbuf1);
	pOut[i+1] = (CSAMPLE) processSample((double) pIn[i+1], fbuf2);
    }
}

inline double EngineFilter::processSample(const double sample, void *buf)
{
    return funcp(buf, sample);
}
