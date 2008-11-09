#ifndef ENGINEANALYSER_H
#define ENGINEANALYSER_H

#include "defs.h"

/*
 * An EngineAnalyser is an object which wants to process an entire song to
 * calculate some kind of metadata about it. This could be bpm, the
 * summary, key or something else crazy. This is to help consolidate the
 * many different threads currently processing the whole track in Mixxx on load.
 *   -- Adam
 */

class TrackInfoObject;

class EngineAnalyser {

public:
	virtual void initialise(TrackInfoObject* tio) { }
	virtual void process(const CSAMPLE* pIn, const int iLen) = 0;
	virtual void finalise(TrackInfoObject* tio) { }

};

#endif