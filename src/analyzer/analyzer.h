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

    // Analyze the next chunk of audio samples and return true if successful.
    // If processing fails the analysis can be aborted early by returning
    // false. After aborting the analysis only cleanup() will be invoked,
    // but not finalize()!
    virtual bool process(const CSAMPLE* pIn, const int iLen) = 0;

    // Update the track object with the analysis results after
    // processing finished successfully, i.e. all available audio
    // samples have been processed.
    virtual void finalize(TrackPointer tio) = 0;

    // Discard any temporary results or free allocated memory.
    virtual void cleanup() = 0;
};

typedef std::unique_ptr<Analyzer> AnalyzerPtr;

class AnalyzerState {
  public:
    explicit AnalyzerState(AnalyzerPtr analyzer)
            : m_analyzer(std::move(analyzer)),
              m_processing(false) {
        DEBUG_ASSERT(m_analyzer);
    }
    AnalyzerState(const AnalyzerState&) = delete;
    AnalyzerState(AnalyzerState&&) = default;

    bool initialize(TrackPointer tio, int sampleRate, int totalSamples) {
        DEBUG_ASSERT(!m_processing);
        m_processing = m_analyzer->initialize(tio, sampleRate, totalSamples);
        return m_processing;
    }

    void process(const CSAMPLE* pIn, const int iLen) {
        if (m_processing) {
            m_processing = m_analyzer->process(pIn, iLen);
        }
    }

    void finalize(TrackPointer tio) {
        if (m_processing) {
            m_analyzer->finalize(tio);
            m_processing = false;
        }
    }

    void cleanup() {
        m_analyzer->cleanup();
        m_processing = false;
    }

  private:
    AnalyzerPtr m_analyzer;
    bool m_processing;
};
