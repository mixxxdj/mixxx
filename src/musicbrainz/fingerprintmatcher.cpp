#include "musicbrainz/fingerprintmatcher.h"

#include <QtDebug>
#include <algorithm>
#include <cmath>

namespace mixxx {

namespace {

const bool sDebugFingerprintMatcher = true;

// SimHash pre-filter: how many bits may differ before we still bother
// reading both .chroma files.

constexpr int kMaxOffsetSearchItems = 120;

constexpr int kMatchBits = 14;
constexpr int kMatchMask = (1 << kMatchBits) - 1;

inline quint32 matchStrip(quint32 x) {
    return x >> (32 - kMatchBits);
}

inline quint32 uniqStrip(quint32 x) {
    return x >> (32 - kMatchBits);
}

} // anonymous namespace

// static
bool FingerprintMatcher::simHashCandidatesMatch(
        quint32 hashA, quint32 hashB, int maxHammingBits) {
    const int distance = popcount(static_cast<quint64>(hashA ^ hashB));
    if (sDebugFingerprintMatcher) {
        qDebug() << "FingerprintMatcher -> [simHashCandidatesMatch] ->"
                 << "hashA:" << hashA << "hashB:" << hashB
                 << "hammingDistance:" << distance
                 << "maxAllowed:" << maxHammingBits;
    }
    return distance <= maxHammingBits;
}

// check PR #15585

// static
FingerprintMatcher::MatchResult FingerprintMatcher::compare(
        const QVector<quint32>& a, const QVector<quint32>& b, float matchThreshold) {
    MatchResult result;
    if (a.isEmpty() || b.isEmpty()) {
        qDebug() << "FingerprintMatcher -> [compare] -> "
                    "aborting: one or both fingerprints are empty";
        return result;
    }

    float offset = 0.0f;
    result.score = scoreFingerprints(
            a.constData(), a.size(), b.constData(), b.size(), kMaxOffsetSearchItems, &offset);
    result.offsetItems = offset;
    result.isMatch = result.score >= matchThreshold;

    if (sDebugFingerprintMatcher) {
        qDebug() << "FingerprintMatcher -> [compare] -> result"
                 << "score:" << result.score
                 << "offsetItems:" << result.offsetItems
                 << "isMatch:" << result.isMatch
                 << "threshold:" << matchThreshold;
    }
    return result;
}

// static
int FingerprintMatcher::findTopOffset(
        const quint32* a, int aSize, const quint32* b, int bSize, int maxOffset, int* outTopCount) {
    QVector<quint16> aOffsets(kMatchMask + 1, 0);
    QVector<quint16> bOffsets(kMatchMask + 1, 0);
    for (int i = 0; i < aSize; i++) {
        aOffsets[matchStrip(a[i])] = static_cast<quint16>(i);
    }
    for (int i = 0; i < bSize; i++) {
        bOffsets[matchStrip(b[i])] = static_cast<quint16>(i);
    }

    QVector<quint16> counts(aSize + bSize + 1, 0);
    int topCount = 0;
    int topOffset = 0;

    for (int i = 0; i < kMatchMask; i++) {
        if (aOffsets[i] && bOffsets[i]) {
            int offset = aOffsets[i] - bOffsets[i];
            if (maxOffset == 0 || (-maxOffset <= offset && offset <= maxOffset)) {
                offset += bSize;
                counts[offset]++;
                if (counts[offset] > topCount) {
                    topCount = counts[offset];
                    topOffset = offset;
                }
            }
        }
    }
    topOffset -= bSize;

    if (sDebugFingerprintMatcher) {
        constexpr double kSecondsPerFrame = 1365.3333 / 11025.0;
        qDebug() << "FingerprintMatcher -> [findTopOffset] ->"
                 << "File 1 frames:" << aSize << "File 2 frames:" << bSize;
        qDebug() << "FingerprintMatcher -> [findTopOffset] ->"
                 << "Coarse offset (frames):" << topOffset
                 << "≈" << topOffset * kSecondsPerFrame << "seconds";
        qDebug() << "FingerprintMatcher -> [findTopOffset] ->"
                 << "Top count (matching masks):" << topCount;
    }

    if (outTopCount) {
        *outTopCount = topCount;
    }
    return topOffset;
}

// static
float FingerprintMatcher::scoreFingerprints(const quint32* a,
        int aSize,
        const quint32* b,
        int bSize,
        int maxOffset,
        float* outOffset) {
    int topCount = 0;
    int topOffset = findTopOffset(a, aSize, b, bSize, maxOffset, &topCount);

    int countR1 = 0, countC1 = 0, countL1 = 0;
    int countR2 = 0, countC2 = 0, countL2 = 0;
    const int corrSize = std::min(aSize, bSize);

    if (topOffset > 0) {
        int i = 1 + topOffset;
        for (; i < (corrSize - (1 + topOffset)) / 2; i++) {
            for (int k = 0; k < 32; k++) {
                if (compareBit(a[i - (-1 - topOffset)], b[i], k))
                    countR1++;
                if (compareBit(a[i - (0 - topOffset)], b[i], k))
                    countC1++;
                if (compareBit(a[i - (1 - topOffset)], b[i], k))
                    countL1++;
            }
        }
        for (; i < corrSize - (1 + topOffset); i++) {
            for (int k = 0; k < 32; k++) {
                if (compareBit(a[i - (-1 - topOffset)], b[i], k))
                    countR2++;
                if (compareBit(a[i - (0 - topOffset)], b[i], k))
                    countC2++;
                if (compareBit(a[i - (1 - topOffset)], b[i], k))
                    countL2++;
            }
        }
    } else {
        int i = 1 - topOffset;
        for (; i < (corrSize - (1 - topOffset)) / 2; i++) {
            for (int k = 0; k < 32; k++) {
                if (compareBit(a[i], b[i - (1 + topOffset)], k))
                    countR1++;
                if (compareBit(a[i], b[i - (0 + topOffset)], k))
                    countC1++;
                if (compareBit(a[i], b[i - (-1 + topOffset)], k))
                    countL1++;
            }
        }
        for (; i < corrSize - (1 - topOffset); i++) {
            for (int k = 0; k < 32; k++) {
                if (compareBit(a[i], b[i - (1 + topOffset)], k))
                    countR2++;
                if (compareBit(a[i], b[i - (0 + topOffset)], k))
                    countC2++;
                if (compareBit(a[i], b[i - (-1 + topOffset)], k))
                    countL2++;
            }
        }
    }

    const int countMin1 = std::min(std::min(countR1, countC1), countL1);
    countR1 -= countMin1;
    countC1 -= countMin1;
    countL1 -= countMin1;
    const float fract1 = (static_cast<float>(countR1) - static_cast<float>(countL1)) /
            static_cast<float>(countR1 + countC1 + countL1);

    const int countMin2 = std::min(std::min(countR2, countC2), countL2);
    countR2 -= countMin2;
    countC2 -= countMin2;
    countL2 -= countMin2;
    const float fract2 = (static_cast<float>(countR2) - static_cast<float>(countL2)) /
            static_cast<float>(countR2 + countC2 + countL2);

    // See PR #15585.
    constexpr double kFractionCorrectionAmplitude = 0.06;
    const float correction = static_cast<float>(kFractionCorrectionAmplitude *
            std::sin(2.0 * M_PI * (fract1 + fract2) / 2.0));
    *outOffset = static_cast<float>(topOffset) + (fract1 + fract2) / 2.0f - correction;

    if (sDebugFingerprintMatcher) {
        constexpr double kSecondsPerFrame = 1365.3333 / 11025.0;
        float coarseOffsetItems = static_cast<float>(topOffset);
        float fineOffsetItems = *outOffset;
        qDebug() << "FingerprintMatcher -> [scoreFingerprints] ->"
                 << "Refined Sub-frame Offset:" << fineOffsetItems << "frames";
        qDebug() << "FingerprintMatcher -> [scoreFingerprints] ->"
                 << "Estimated Coarse Time Offset:"
                 << coarseOffsetItems * kSecondsPerFrame << "seconds";
        qDebug() << "FingerprintMatcher -> [scoreFingerprints] ->"
                 << "Estimated Fine Time Offset:"
                 << fineOffsetItems * kSecondsPerFrame << "seconds";
    }

    const int minSize = std::min(aSize, bSize) & ~1;
    if (topOffset < 0) {
        b -= topOffset;
        bSize = std::max(0, bSize + topOffset);
    } else {
        a += topOffset;
        aSize = std::max(0, aSize - topOffset);
    }

    const int size = std::min(aSize, bSize) / 2;
    if (!size || !minSize) {
        if (sDebugFingerprintMatcher) {
            qDebug() << "FingerprintMatcher -> [scoreFingerprints] -> "
                        "empty overlap after alignment, score 0.0";
        }
        return 0.0f;
    }

    QVector<quint16> seen(kMatchMask + 1, 0);
    int aUniq = 0;
    for (int i = 0; i < aSize; i++) {
        const quint32 key = uniqStrip(a[i]);
        if (!seen[key]) {
            aUniq++;
            seen[key] = 1;
        }
    }
    seen.fill(0);
    int bUniq = 0;
    for (int i = 0; i < bSize; i++) {
        const quint32 key = uniqStrip(b[i]);
        if (!seen[key]) {
            bUniq++;
            seen[key] = 1;
        }
    }
    const float diversity = std::min(
            std::min(1.0f, static_cast<float>(aUniq + 10) / aSize + 0.5f),
            std::min(1.0f, static_cast<float>(bUniq + 10) / bSize + 0.5f));

    if (sDebugFingerprintMatcher) {
        qDebug() << "FingerprintMatcher -> [scoreFingerprints] ->"
                 << "aligned sizes: aSize" << aSize << "bSize" << bSize;
        qDebug() << "FingerprintMatcher -> [scoreFingerprints] ->"
                 << "minSize" << minSize << "size" << (minSize / 2) << "(64‑bit chunks)";
        qDebug() << "FingerprintMatcher -> [scoreFingerprints] ->"
                 << "unique masks: aUniq" << aUniq << "bUniq" << bUniq;
        qDebug() << "FingerprintMatcher -> [scoreFingerprints] ->"
                 << "diversity:" << diversity;
        qDebug() << "FingerprintMatcher -> [scoreFingerprints] ->"
                 << "topcount:" << topCount << "threshold (2% of max unique):"
                 << std::max(aUniq, bUniq) * 0.02;
    }

    if (topCount < std::max(aUniq, bUniq) * 0.02) {
        if (sDebugFingerprintMatcher) {
            qDebug() << "FingerprintMatcher -> [scoreFingerprints] -> "
                        "top offset vote count below 2% of unique size, score 0.0";
        }
        return 0.0f;
    }

    const quint64* aData = reinterpret_cast<const quint64*>(a);
    const quint64* bData = reinterpret_cast<const quint64*>(b);
    int biterror = 0;
    for (int i = 0; i < size; i++, aData++, bData++) {
        biterror += popcount(*aData ^ *bData);
    }

    float score = (static_cast<float>(size) * 2.0f / static_cast<float>(minSize)) *
            (1.0f - 2.0f * static_cast<float>(biterror) / (64.0f * static_cast<float>(size)));

    if (sDebugFingerprintMatcher) {
        qDebug() << "FingerprintMatcher -> [scoreFingerprints] ->"
                 << "biterror:" << biterror << "raw score:" << score;
    }

    if (score < 0.0f) {
        score = 0.0f;
    }
    if (diversity < 1.0f) {
        score = static_cast<float>(std::pow(score, 8.0f - 7.0f * diversity));
    }

    if (sDebugFingerprintMatcher) {
        qDebug() << "FingerprintMatcher -> [scoreFingerprints] ->"
                 << "final score:" << score
                 << "(raw was" << (diversity < 1.0f ? " scaled" : " unchanged") << ")";
    }
    return score;
}

// static
int FingerprintMatcher::popcount(quint64 x) {
    // PR #15585's popcount_3()
    const quint64 m1 = 0x5555555555555555ULL;
    const quint64 m2 = 0x3333333333333333ULL;
    const quint64 m4 = 0x0f0f0f0f0f0f0f0fULL;
    const quint64 h01 = 0x0101010101010101ULL;
    x -= (x >> 1) & m1;
    x = (x & m2) + ((x >> 2) & m2);
    x = (x + (x >> 4)) & m4;
    return static_cast<int>((x * h01) >> 56);
}

// static
bool FingerprintMatcher::compareBit(quint32 a, quint32 b, int bit) {
    return ((a >> bit) & 1) == ((b >> bit) & 1);
}

} // namespace mixxx
