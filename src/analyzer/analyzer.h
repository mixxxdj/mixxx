#ifndef ANALYZER_ANALYZER_H
#define ANALYZER_ANALYZER_H

#include "util/types.h"

/*
 * An Analyzer is an object which wants to process an entire song to
 * calculate some kind of metadata about it. This could be bpm, the
 * summary, key or something else crazy. This is to help consolidate the
 * many different threads currently processing the whole track in Mixxx on load.
 *   -- Adam
 */

#include "track/track.h"

class Analyzer {
  public:
    virtual bool initialize(TrackPointer tio, int sampleRate, int totalSamples) = 0;
    virtual bool isDisabledOrLoadStoredSuccess(TrackPointer tio) const = 0;
    virtual void process(const CSAMPLE* pIn, const int iLen) = 0;
    virtual void cleanup(TrackPointer tio) = 0;
    virtual void finalize(TrackPointer tio) = 0;
    virtual ~Analyzer() = default;
};

#endif
