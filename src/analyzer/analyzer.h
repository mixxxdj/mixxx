#pragma once

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
    virtual ~Analyzer() = default;

    // This method is supposed to:
    //  1. Check if the track needs to be analyzed, otherwise return false.
    //  2. Perform the initialization and return true on success.
    //  3. If the initialization failed log the internal error and return false.
    virtual bool initialize(TrackPointer tio, int sampleRate, int totalSamples) = 0;

    /////////////////////////////////////////////////////////////////////////
    // All following methods will only be invoked after initialize()
    // returned true!
    /////////////////////////////////////////////////////////////////////////

    // Analyze the next chunk of audio samples.
    virtual void process(const CSAMPLE* pIn, const int iLen) = 0;

    // Update the track object with the analysis results after
    // processing finished, i.e. all available audio samples have
    // been processed.
    virtual void finalize(TrackPointer tio) = 0;

    // Discard any temporary results or free allocated memory after
    // finalization.
    virtual void cleanup(TrackPointer tio) = 0;
};
