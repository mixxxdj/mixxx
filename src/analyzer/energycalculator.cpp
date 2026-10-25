#include "analyzer/energycalculator.h"

#include <algorithm>
#include <cmath>
#include <utility>

namespace {

constexpr double kPi = 3.14159265358979323846;
constexpr double kTwoPi = 2.0 * kPi;
constexpr double kBlockSeconds = 0.02; // 20 ms analysis blocks
constexpr int kBlocksPerWindow = 50;   // 1 s loudness windows
constexpr double kWindowSeconds = kBlockSeconds * kBlocksPerWindow;
constexpr int kBodySpanWindows = 4;    // "sustained" = 4 s
// Body start: where the bass AND the overall level first reach the track's
// typical level (its 40th percentile, 1 s windows) and stay there for 4 s.
// "Loud" alone misses intros that are loud but have no drums; tuned on 5
// tracks with the DJ's own beat times (all within about 1 s).
constexpr double kBodyTypicalPercentile = 0.4;
constexpr double kBodyStartMarginDb = 2.0;
constexpr double kBodyStartFirstWindowMarginDb = 4.0;
// Then pinpoint it: the first 20 ms bass block (a kick) at least this far
// above the typical 1 s bass level, from 2 s before to 1.5 s after the
// 1 s window found. Within 0.02 s of the DJ's times on the same 5 tracks.
constexpr double kKickAboveTypicalDb = 6.0;
constexpr int kKickSearchBackBlocks = 2 * kBlocksPerWindow;
constexpr int kKickSearchAheadBlocks = kBlocksPerWindow + kBlocksPerWindow / 2;
constexpr double kLowCutoffHz = 150.0;
constexpr double kHighCutoffHz = 2500.0;
constexpr double kSilenceDb = -60.0;
constexpr double kMinSoundSeconds = 5.0;

// Onset detection: a block this much louder than the average of the
// previous blocks, above a floor, and not too soon after the last one.
constexpr int kOnsetHistoryBlocks = 8; // 160 ms
constexpr double kOnsetRiseDb = 3.0;
constexpr double kOnsetFloorDb = -45.0;
constexpr int kOnsetMinGapBlocks = 3; // 60 ms

// Ranges mapped to 0..1 (starting points, to be tuned by ear).
constexpr double kLoudnessMinDb = -30.0;
constexpr double kLoudnessMaxDb = -8.0;
constexpr double kBrightRatioMin = 0.01;
constexpr double kBrightRatioMax = 0.30;
constexpr double kOnsetsMin = 1.0; // per second
constexpr double kOnsetsMax = 6.0;

constexpr double kWeightLoudness = 0.45;
constexpr double kWeightBrightness = 0.30;
constexpr double kWeightBusyness = 0.25;

// Beat grid check (gridDriftBeats). A "hit" = a rise in a band's level from
// one 20 ms block to the next. Tested on 5 real tracks: correct grids
// drifted 0.04..0.13 beats; a grid 0.05% too fast/slow 0.15 and more.
constexpr double kGridFloorDb = -60.0;
constexpr int kGridWindowBeats = 16;
constexpr int kGridWindowsPerRegion = 4; // 64 beats per region
// A band whose hits line up with the beat less than this is ignored
// (0 = hits anywhere, 1 = every hit exactly on the same spot of the beat).
constexpr double kGridMinClarity = 0.05;

double onePoleCoefficient(double cutoffHz, double sampleRate) {
    return 1.0 - std::exp(-2.0 * kPi * cutoffHz / sampleRate);
}

double clamp01(double x) {
    return std::clamp(x, 0.0, 1.0);
}

double powerToDb(double meanSquare) {
    return meanSquare > 1e-12 ? 10.0 * std::log10(meanSquare) : -120.0;
}

} // namespace

EnergyCalculator::EnergyCalculator(double sampleRate, int channelCount)
        : m_sampleRate(sampleRate),
          m_channelCount(channelCount),
          m_blockFrames(static_cast<int>(sampleRate * kBlockSeconds)),
          m_low1{onePoleCoefficient(kLowCutoffHz, sampleRate), 0.0},
          m_low2{onePoleCoefficient(kLowCutoffHz, sampleRate), 0.0},
          m_high1{onePoleCoefficient(kHighCutoffHz, sampleRate), 0.0},
          m_high2{onePoleCoefficient(kHighCutoffHz, sampleRate), 0.0} {
    m_blockDb.reserve(5000);
    m_blockLowDb.reserve(5000);
    m_blockHighDb.reserve(5000);
}

void EnergyCalculator::process(const float* pInterleaved, std::int64_t sampleCount) {
    for (std::int64_t i = 0; i < sampleCount; i += m_channelCount) {
        // Sum channels to mono.
        double sum = 0.0;
        for (int c = 0; c < m_channelCount; ++c) {
            sum += pInterleaved[i + c];
        }
        const double mono = sum / m_channelCount;

        // Filter into bass and treble.
        const double low = m_low2.step(m_low1.step(mono));
        const double high = mono - m_high2.step(m_high1.step(mono));
        const double fullSq = mono * mono;
        const double lowSq = low * low;
        const double highSq = high * high;

        m_blockSumFull += fullSq;
        m_blockSumLow += lowSq;
        m_blockSumHigh += highSq;
        ++m_framesInBlock;

        if (m_framesInBlock == m_blockFrames) {
            endBlock();
        }
    }
}

void EnergyCalculator::endBlock() {
    const double fullDb = powerToDb(m_blockSumFull / m_blockFrames);
    const double lowDb = powerToDb(m_blockSumLow / m_blockFrames);
    const double highDb = powerToDb(m_blockSumHigh / m_blockFrames);

    m_blockDb.push_back(static_cast<float>(fullDb));
    m_blockLowDb.push_back(static_cast<float>(lowDb));
    m_blockHighDb.push_back(static_cast<float>(highDb));

    if (fullDb > kSilenceDb) {
        m_totalFull += m_blockSumFull;
        m_totalLow += m_blockSumLow;
        m_totalHigh += m_blockSumHigh;
    }

    m_framesInBlock = 0;
    m_blockSumFull = 0.0;
    m_blockSumLow = 0.0;
    m_blockSumHigh = 0.0;
}

bool EnergyCalculator::finish(Result* pResult) const {
    const int blockCount = static_cast<int>(m_blockDb.size());
    if (blockCount == 0) {
        return false;
    }
    const double soundSeconds = std::count_if(m_blockDb.begin(), m_blockDb.end(),
            [](float db) { return db > kSilenceDb; }) * kBlockSeconds;
    if (soundSeconds < kMinSoundSeconds) {
        return false;
    }

    // Loudness: 90th percentile of 1 s RMS (non-silent windows).
    std::vector<double> windowDb;
    std::vector<double> windowLowDb;
    windowDb.reserve(blockCount / kBlocksPerWindow);
    windowLowDb.reserve(blockCount / kBlocksPerWindow);
    for (int w = 0; w + kBlocksPerWindow <= blockCount; w += kBlocksPerWindow) {
        double sumFull = 0.0;
        double sumLow = 0.0;
        int nonSilent = 0;
        for (int b = w; b < w + kBlocksPerWindow; ++b) {
            if (m_blockDb[b] > kSilenceDb) {
                sumFull += std::pow(10.0, m_blockDb[b] / 10.0);
                sumLow += std::pow(10.0, m_blockLowDb[b] / 10.0);
                ++nonSilent;
            }
        }
        if (nonSilent > 0) {
            windowDb.push_back(10.0 * std::log10(sumFull / nonSilent));
            windowLowDb.push_back(10.0 * std::log10(sumLow / nonSilent));
        }
    }

    if (windowDb.empty()) {
        return false;
    }
    std::sort(windowDb.begin(), windowDb.end());
    const auto p90Index = static_cast<std::size_t>(0.9 * (windowDb.size() - 1));
    const double loudnessDb = windowDb[p90Index];

    // Busyness: count onsets.
    int onsets = 0;
    int lastOnset = -kOnsetMinGapBlocks;
    for (int i = kOnsetHistoryBlocks; i < blockCount; ++i) {
        const double db = m_blockDb[i];
        if (db < kOnsetFloorDb || i - lastOnset < kOnsetMinGapBlocks) {
            continue;
        }
        double history = 0.0;
        for (int h = i - kOnsetHistoryBlocks; h < i; ++h) {
            history += std::max<double>(m_blockDb[h], -90.0);
        }
        history /= kOnsetHistoryBlocks;
        if (db - history > kOnsetRiseDb) {
            ++onsets;
            lastOnset = i;
        }
    }
    const double onsetsPerSec = onsets / soundSeconds;

    // Body: the first/last 4-second stretch whose average level is close
    // enough to the loud level (see kBodyStartDropDb / kBodyEndDropDb). A short dip (a breakdown) inside the
    // track does not matter; only the ends are looked for.
    const double bodyStartThresholdDb = loudnessDb - kBodyStartDropDb;
    const double bodyEndThresholdDb = loudnessDb - kBodyEndDropDb;
    const int span = kBodySpanWindows;
    const int windows = static_cast<int>(windowDb.size());
    const auto spanMeanDb = [&windowDb](int first, int count) {
        double sum = 0.0;
        for (int w = first; w < first + count; ++w) {
            sum += windowDb[w];
        }
        return sum / count;
    };
    const auto percentileOf = [](std::vector<double> values, double fraction) {
        std::sort(values.begin(), values.end());
        return values[static_cast<std::size_t>(fraction * (values.size() - 1))];
    };
    const auto spanMedian = [](const std::vector<double>& values, int first, int count) {
        std::vector<double> part(values.begin() + first, values.begin() + first + count);
        std::sort(part.begin(), part.end());
        const int mid = count / 2;
        return count % 2 ? part[mid] : 0.5 * (part[mid - 1] + part[mid]);
    };
    const double typicalDb = percentileOf(windowDb, kBodyTypicalPercentile);
    const double typicalLowDb = percentileOf(windowLowDb, kBodyTypicalPercentile);
    double bodyStart = 0.0;
    double bodyEnd = windows * kWindowSeconds;
    if (windows >= span) {
        bool found = false;
        for (int w = 0; w + span <= windows; ++w) {
            if (windowLowDb[w] >= typicalLowDb - kBodyStartFirstWindowMarginDb &&
                    spanMedian(windowLowDb, w, span) >= typicalLowDb - kBodyStartMarginDb &&
                    spanMedian(windowDb, w, span) >= typicalDb - kBodyStartMarginDb) {
                bodyStart = w * kWindowSeconds;
                found = true;
                const int from = std::max(0, w * kBlocksPerWindow - kKickSearchBackBlocks);
                const int to = std::min(blockCount, w * kBlocksPerWindow + kKickSearchAheadBlocks);
                for (int b = from; b < to; ++b) {
                    if (m_blockLowDb[b] >= typicalLowDb + kKickAboveTypicalDb) {
                        bodyStart = b * kBlockSeconds;
                        break;
                    }
                }
                break;
            }
        }
        if (!found) {
            // No clear bass entry (e.g. no bass at all): use loudness only.
            for (int w = 0; w + span <= windows; ++w) {
                if (spanMeanDb(w, span) >= bodyStartThresholdDb) {
                    bodyStart = w * kWindowSeconds;
                    break;
                }
            }
        }
        for (int w = windows - span; w >= 0; --w) {
            if (spanMeanDb(w, span) >= bodyEndThresholdDb) {
                bodyEnd = (w + span) * kWindowSeconds;
                break;
            }
        }
    }

    Result r;
    r.loudnessDb = loudnessDb;
    r.bodyStartSec = bodyStart;
    r.bodyEndSec = std::max(bodyEnd, bodyStart);
    r.brightRatio = m_totalHigh / m_totalFull;
    r.bassRatio = m_totalLow / m_totalFull;
    r.onsetsPerSec = onsetsPerSec;
    r.loudness01 = clamp01((loudnessDb - kLoudnessMinDb) / (kLoudnessMaxDb - kLoudnessMinDb));
    r.brightness01 = r.brightRatio > 0.0
            ? clamp01((std::log10(r.brightRatio) - std::log10(kBrightRatioMin)) /
                      (std::log10(kBrightRatioMax) - std::log10(kBrightRatioMin)))
            : 0.0;
    r.busyness01 = clamp01((onsetsPerSec - kOnsetsMin) / (kOnsetsMax - kOnsetsMin));
    r.energy = combine(r.loudness01, r.brightness01, r.busyness01);
    *pResult = r;
    return true;
}

double EnergyCalculator::gridDriftBeats(const phrasealign::Grid& grid,
        double bodyStartSec,
        double bodyEndSec) const {
    const int blockCount = static_cast<int>(m_blockDb.size());
    if (blockCount < 10 || bodyEndSec <= bodyStartSec || !grid.isValid()) {
        return -1.0;
    }

    // Convert body time range to block indices.
    const int firstBlock = static_cast<int>(std::floor(bodyStartSec / kBlockSeconds));
    const int lastBlock = static_cast<int>(std::ceil(bodyEndSec / kBlockSeconds));
    const int bodyBlocks = std::max(0, std::min(lastBlock, blockCount) - firstBlock);
    if (bodyBlocks < 8) { // < 160 ms
        return -1.0;
    }

    // Three regions: start, middle, end.
    const int regionBlocks = bodyBlocks / 3;
    if (regionBlocks < 2) {
        return -1.0;
    }

    auto hitsInRegion = [this, &grid](int fromBlock, int toBlock, const std::vector<float>& bandDb) {
        std::vector<double> phases; // where in the beat the hits fall (0..1)
        for (int b = fromBlock; b < toBlock; ++b) {
            if (b <= 0 || b >= blockCount - 1) {
                continue;
            }
            const double rise = bandDb[b] - bandDb[b - 1];
            if (rise < 1.0) { // need a clear rise
                continue;
            }
            const double timeSec = (b + 0.5) * kBlockSeconds;
            const double beat = grid.beatAt(timeSec);
            const double phase = beat - std::floor(beat);
            phases.push_back(phase);
        }
        return phases;
    };

    // For each band, measure drift across the three regions.
    auto driftForBand = [&](const std::vector<float>& bandDb) {
        std::vector<double> drifts;
        for (int r = 0; r < 3; ++r) {
            const int from = firstBlock + r * regionBlocks;
            const int to = std::min(firstBlock + (r + 1) * regionBlocks, lastBlock);
            const std::vector<double> phases = hitsInRegion(from, to, bandDb);
            if (phases.size() < kGridWindowBeats) {
                continue;
            }
            // Clarity: how concentrated are the phases?
            std::vector<int> hist(20, 0);
            for (double p : phases) {
                hist[static_cast<int>(p * 20)]++;
            }
            const int total = static_cast<int>(phases.size());
            int maxBin = 0;
            for (int h : hist) {
                maxBin = std::max(maxBin, h);
            }
            const double clarity = static_cast<double>(maxBin) / total;
            if (clarity < kGridMinClarity) {
                continue; // band doesn't show the beat clearly here
            }
            // Mean phase of the clear hits.
            double sum = 0.0;
            int count = 0;
            const int peakBin = std::distance(hist.begin(), std::max_element(hist.begin(), hist.end()));
            for (double p : phases) {
                if (static_cast<int>(p * 20) == peakBin) {
                    sum += p;
                    ++count;
                }
            }
            if (count > 0) {
                drifts.push_back(sum / count);
            }
        }
        // Need at least 3 regions (out of 4) with clear phase.
        if (drifts.size() < 3) {
            return -1.0;
        }
        // v5: leave out the one window that doesn't fit; the other three must agree.
        // We have 3 or 4 values. If 4, drop the outlier.
        if (drifts.size() == 4) {
            double mean = 0.0;
            for (double d : drifts) mean += d;
            mean /= 4.0;
            double maxDev = 0.0;
            int outlier = -1;
            for (int i = 0; i < 4; ++i) {
                const double dev = std::fabs(drifts[i] - mean);
                if (dev > maxDev) {
                    maxDev = dev;
                    outlier = i;
                }
            }
            drifts.erase(drifts.begin() + outlier);
        }
        // Max difference between the remaining (3) regions.
        double minP = drifts[0], maxP = drifts[0];
        for (double d : drifts) {
            minP = std::min(minP, d);
            maxP = std::max(maxP, d);
        }
        // Circular distance: 0.9 vs 0.1 is 0.2 beats apart, not 0.8.
        double diff = maxP - minP;
        if (diff > 0.5) {
            diff = 1.0 - diff;
        }
        return diff;
    };

    const double driftLow = driftForBand(m_blockLowDb);
    const double driftHigh = driftForBand(m_blockHighDb);

    // Use whichever band shows the beat most clearly (lower drift = better).
    if (driftLow < 0.0 && driftHigh < 0.0) {
        return -1.0;
    }
    if (driftLow < 0.0) {
        return driftHigh;
    }
    if (driftHigh < 0.0) {
        return driftLow;
    }
    return std::min(driftLow, driftHigh);
}

double EnergyCalculator::combine(double loudness01, double brightness01, double busyness01) {
    return 1.0 + 9.0 * (kWeightLoudness * loudness01
            + kWeightBrightness * brightness01
            + kWeightBusyness * busyness01);
}