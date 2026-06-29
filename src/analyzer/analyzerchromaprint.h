#pragma once

#include <chromaprint.h>

#include <QDateTime>
#include <QSqlDatabase>
#include <vector>

#include "analyzer/analyzer.h"
#include "library/dao/trackfingerprintdao.h"
#include "preferences/usersettings.h"
#include "util/sample.h"

// Analyzer that computes a full-track Chromaprint fingerprint and persists
// it to disk as a raw .chroma binary file plus a fingerprint_metadata row.
//
// Unlike the existing ChromaPrinter (which reads only the first 120 seconds
// for AcoustID submission), this analyzer processes the entire track so that
// the complete fingerprint can be used for CMRT duplicate detection and
// mastering-level offset calculation.
//
// Storage layout :
//   ~/.mixxx/fingerprints/track_{id}.chroma  — raw uint32[] array (on disk)
//   fingerprint_metadata                      — SimHash + SHA-256 + linkage (DB)
//
// The Base64-encoded fingerprint for AcoustID is generated on demand by the
// background worker at submission time and is never stored permanently.
class AnalyzerChromaprint : public Analyzer {
  public:
    AnalyzerChromaprint(
            UserSettingsPointer pConfig,
            const QSqlDatabase& dbConnection);
    ~AnalyzerChromaprint() override;

    // Returns true if analysis should proceed for this track.
    // Returns false (skip) if a valid, non-stale fingerprint already exists.
    bool initialize(const AnalyzerTrack& track,
            mixxx::audio::SampleRate sampleRate,
            mixxx::audio::ChannelCount channelCount,
            SINT frameLength) override;

    // Converts floating-point samples to int16 and feeds them to Chromaprint.
    bool processSamples(const CSAMPLE* buffer, SINT count) override;

    // Finalizes the fingerprint, computes SimHash + SHA-256, saves the
    // .chroma file, writes fingerprint_metadata, and enqueues for AcoustID.
    void storeResults(TrackPointer pTrack) override;

    // Frees the Chromaprint context and clears the conversion buffer.
    // Always safe to call — guards against null context.
    void cleanup() override;

  private:
    // Returns true if the track already has a valid fingerprint that does
    // not need regeneration. Also checks if the source file has been modified
    // since the fingerprint was computed — if so, marks it stale and returns false.
    bool hasValidFingerprint(TrackPointer pTrack) const;

    // Computes a 32-bit SimHash from the raw Chromaprint uint32 array using
    // bit-voting: for each of the 32 bit positions, increments a counter if
    // the bit is set in a fingerprint value, decrements if unset. The final
    // SimHash sets the bit if the counter is positive.
    //
    // This is a locality-sensitive hash — similar fingerprints produce similar
    // SimHash values — used as the fast candidate pre-filter
    // It is NOT a unique key; full XOR/popcount
    // comparison of .chroma files is always required to confirm a match.
    static quint32 computeSimHash(const uint32_t* fprint, int size);

    TrackFingerprintDao m_fingerprintDao;
    ChromaprintContext* m_pChromaprintCtx;

    // Reusable int16 conversion buffer — avoids per-chunk heap allocation
    std::vector<SAMPLE> m_int16Buffer;

    // Stored in initialize() for use in storeResults()
    mixxx::audio::SampleRate m_sampleRate;
    mixxx::audio::ChannelCount m_channelCount;
    SINT m_frameLength;
    TrackId m_trackId;
};
