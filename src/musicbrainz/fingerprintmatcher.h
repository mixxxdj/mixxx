#pragma once

#include <QVector>
#include <cstdint>

namespace mixxx {

/// Compares two raw Chromaprint fingerprints (uint32 arrays) and reports
/// whether they are the same mastering, plus the timing offset between them.
///
/// This is the implementation of the POC from PR #15585 (Daniel/Eve's
/// chroma-correlation research) -- matchFingerprints() there was a debug
/// prototype gated behind DEBUG_PRINT_FINGERPRINT. This class extracts the
/// real comparison logic only; none of the qDebug()/old-fingerprint-caching
/// scaffolding from the POC belongs in implementation code.
///

class FingerprintMatcher {
  public:
    static constexpr double kItemDurationSeconds = (512.0 * 8 / 3) / 11025.0; // ~ 0.1238

    static constexpr int kSimHashMaxHammingBits = 8;

    /// Fallback threshold for callers that don't supply one to compare().
    /// CmrtGroupingService overrides this with kCmrtMatchThresholdConfigKey;
    // Full-fingerprint match threshold. ">80% overlap"
    static constexpr float kDefaultMatchThreshold = 0.80f;

    struct MatchResult {
        bool isMatch = false;     // score >= matchThreshold
        float score = 0.0f;       // 0.0-1.0, see scoreFingerprints()
        float offsetItems = 0.0f; // fine offset, fractional Chromaprint items
    };

    /// SimHash candidate filter -- O(1) integer comparison. Called before
    /// any .chroma file is read.
    static bool simHashCandidatesMatch(quint32 hashA, quint32 hashB, int maxHammingBits);

    /// Full comparison -- requires both fingerprints already loaded from disk.
    /// This is the expensive path: only call after simHashCandidatesMatch().
    /// matchThreshold sets MatchResult::isMatch's cutoff; defaults to
    /// kDefaultMatchThreshold for callers with no preference of their own.
    static MatchResult compare(const QVector<quint32>& a,
            const QVector<quint32>& b,
            float matchThreshold = kDefaultMatchThreshold);

  private:
    static int findTopOffset(const quint32* a,
            int aSize,
            const quint32* b,
            int bSize,
            int maxOffset,
            int* outTopCount);

    static float scoreFingerprints(const quint32* a,
            int aSize,
            const quint32* b,
            int bSize,
            int maxOffset,
            float* outOffset);

    static int popcount(quint64 x);
    static bool compareBit(quint32 a, quint32 b, int bit);
};

} // namespace mixxx
