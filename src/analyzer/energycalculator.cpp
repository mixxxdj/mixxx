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
        : m_sampleRate(sampleRate > 0.0 ? sampleRate : 44100.0),
          m_channelCount(std::max(1, channelCount)),
          m_blockFrames(std::max(1, static_cast<int>(m_sampleRate * kBlockSeconds))) {
    m_low1.a = m_low2.a = onePoleCoefficient(kLowCutoffHz, m_sampleRate);
    m_high1.a = m_high2.a = onePoleCoefficient(kHighCutoffHz, m_sampleRate);
}

void EnergyCalculator::process(const float* pInterleaved, std::int64_t sampleCount) {
    const std::int64_t frames = sampleCount / m_channelCount;
    for (std::int64_t f = 0; f < frames; ++f) {
        // Mix all channels to mono.
        double x = 0.0;
        const float* pFrame = pInterleaved + f * m_channelCount;
        for (int c = 0; c < m_channelCount; ++c) {
            x += pFrame[c];
        }
        x /= m_channelCount;

        const double low = m_low2.step(m_low1.step(x));
        const double high = x - m_high2.step(m_high1.step(x));

        m_blockSumFull += x * x;
        m_blockSumLow += low * low;
        m_blockSumHigh += high * high;
        if (++m_framesInBlock >= m_blockFrames) {
            endBlock();
        }
    }
}

void EnergyCalculator::endBlock() {
    const double meanFull = m_blockSumFull / m_framesInBlock;
    const double db = powerToDb(meanFull);
    m_blockDb.push_back(static_cast<float>(db));
    m_blockLowDb.push_back(static_cast<float>(powerToDb(m_blockSumLow / m_framesInBlock)));
    m_blockHighDb.push_back(static_cast<float>(powerToDb(m_blockSumHigh / m_framesInBlock)));
    if (db > kSilenceDb) {
        m_totalFull += m_blockSumFull;
        m_totalLow += m_blockSumLow;
        m_totalHigh += m_blockSumHigh;
    }
    m_framesInBlock = 0;
    m_blockSumFull = 0.0;
    m_blockSumLow = 0.0;
    m_blockSumHigh = 0.0;
}

double EnergyCalculator::gridDriftBeats(const phrasealign::Grid& grid,
        double bodyStartSec,
        double bodyEndSec) const {
    const int blockCount = static_cast<int>(m_blockLowDb.size());
    const double blockSec = m_blockFrames / m_sampleRate;
    // Everything is counted in beats of the grid, so a beat map that bends
    // with the music is judged beat by beat.
    constexpr double regionBeats = kGridWindowBeats * kGridWindowsPerRegion;
    if (!(grid.beatSec > 0.1) || blockCount < 2) {
        return -1.0;
    }
    const double bodyStartBeat = grid.beatAt(bodyStartSec);
    const double bodyEndBeat = grid.beatAt(bodyEndSec);
    if (!(bodyEndBeat - bodyStartBeat >= regionBeats)) {
        return -1.0;
    }
    // Circular distance between two phases, in beats (0..0.5).
    const auto distance = [](double a, double b) {
        const double d = std::fmod(std::fabs(a - b), 1.0);
        return std::min(d, 1.0 - d);
    };
    const double regionStarts[] = {
            // in beats
            bodyStartBeat,
            0.5 * (bodyStartBeat + bodyEndBeat - regionBeats),
            bodyEndBeat - regionBeats,
    };
    double best = -1.0;
    for (const std::vector<float>* pLevels : {&m_blockLowDb, &m_blockHighDb}) {
        const std::vector<float>& levels = *pLevels;
        // Sum of the hits in [fromSec, toSec) as a vector: its angle is where
        // in the beat they fall, its length (/ weight) how clearly.
        const auto window = [&](double fromSec, double toSec, double* pWeight) {
            double re = 0.0;
            double im = 0.0;
            double weight = 0.0;
            const int from = std::max(1, static_cast<int>(std::ceil(fromSec / blockSec)));
            const int to = std::min(blockCount, static_cast<int>(std::ceil(toSec / blockSec)));
            for (int b = from; b < to; ++b) {
                const double hit = std::max(0.0,
                        std::max<double>(levels[b], kGridFloorDb) -
                                std::max<double>(levels[b - 1], kGridFloorDb));
                if (hit <= 0.0) {
                    continue;
                }
                const double angle = kTwoPi * grid.beatAt(b * blockSec);
                re += hit * std::cos(angle);
                im += hit * std::sin(angle);
                weight += hit;
            }
            *pWeight = weight;
            return std::pair<double, double>(re, im);
        };
        const auto phaseOf = [](double re, double im) {
            return std::atan2(im, re) / kTwoPi;
        };
        double regionPhase[3];
        double worst = 0.0;
        bool clear = true;
        for (int r = 0; r < 3 && clear; ++r) {
            // Each window counts the same, however loud: add up the
            // windows' directions, each as long as that window is clear.
            double sumRe = 0.0;
            double sumIm = 0.0;
            double sumClarity = 0.0;
            double windowPhase[kGridWindowsPerRegion];
            double windowRe[kGridWindowsPerRegion] = {};
            double windowIm[kGridWindowsPerRegion] = {};
            for (int w = 0; w < kGridWindowsPerRegion; ++w) {
                const double fromBeat = regionStarts[r] + w * kGridWindowBeats;
                double weight = 0.0;
                const auto v = window(grid.beatTime(fromBeat),
                        grid.beatTime(fromBeat + kGridWindowBeats),
                        &weight);
                windowPhase[w] = phaseOf(v.first, v.second);
                if (weight > 0.0) {
                    windowRe[w] = v.first / weight;
                    windowIm[w] = v.second / weight;
                    sumRe += v.first / weight;
                    sumIm += v.second / weight;
                    sumClarity += std::hypot(v.first, v.second) / weight;
                }
            }
            if (sumClarity / kGridWindowsPerRegion < kGridMinClarity) {
                clear = false;
                break;
            }
            // One window that does not fit (a breakdown, or an intro before
            // the bass comes in) is the music changing, not the grid: a
            // grid that drifts moves ALL the windows. So the odd one out is
            // left out, and the other three must agree.
            int odd = 0;
            const double allPhase = phaseOf(sumRe, sumIm);
            for (int w = 1; w < kGridWindowsPerRegion; ++w) {
                if (distance(windowPhase[w], allPhase) > distance(windowPhase[odd], allPhase)) {
                    odd = w;
                }
            }
            regionPhase[r] = phaseOf(sumRe - windowRe[odd], sumIm - windowIm[odd]);
            // Drift inside the region (the 16 bars a mix can take).
            for (int w = 0; w < kGridWindowsPerRegion; ++w) {
                if (w != odd) {
                    worst = std::max(worst, distance(windowPhase[w], regionPhase[r]));
                }
            }
        }
        if (!clear) {
            continue; // this band does not show the beat clearly enough
        }
        // Drift between start, middle and end (a slightly wrong tempo).
        worst = std::max({worst,
                distance(regionPhase[0], regionPhase[1]),
                distance(regionPhase[1], regionPhase[2]),
                distance(regionPhase[0], regionPhase[2])});
        if (best < 0.0 || worst < best) {
            best = worst;
        }
    }
    return best;
}

// static
double EnergyCalculator::combine(double loudness01, double brightness01, double busyness01) {
    const double mix = kWeightLoudness * clamp01(loudness01) +
            kWeightBrightness * clamp01(brightness01) +
            kWeightBusyness * clamp01(busyness01);
    return 1.0 + 9.0 * clamp01(mix);
}

bool EnergyCalculator::finish(Result* pResult) const {
    const int blockCount = static_cast<int>(m_blockDb.size());
    int soundBlocks = 0;
    for (float db : m_blockDb) {
        if (db > kSilenceDb) {
            ++soundBlocks;
        }
    }
    const double soundSeconds = soundBlocks * kBlockSeconds;
    if (soundSeconds < kMinSoundSeconds || m_totalFull <= 0.0) {
        return false;
    }

    // Loudness: 90th percentile of 1 s window levels (ignoring silence).
    std::vector<double> timeline;    // every 1 s window in time order
    std::vector<double> timelineLow; // same, bass band only
    std::vector<double> windowDb;    // only the non-silent ones
    std::vector<double> windowLowDb; // bass of the non-silent ones
    for (int start = 0; start + kBlocksPerWindow <= blockCount; start += kBlocksPerWindow) {
        double sumPower = 0.0;
        double sumLowPower = 0.0;
        for (int b = start; b < start + kBlocksPerWindow; ++b) {
            sumPower += std::pow(10.0, m_blockDb[b] / 10.0);
            sumLowPower += std::pow(10.0, m_blockLowDb[b] / 10.0);
        }
        const double db = powerToDb(sumPower / kBlocksPerWindow);
        const double lowDb = powerToDb(sumLowPower / kBlocksPerWindow);
        timeline.push_back(db);
        timelineLow.push_back(lowDb);
        if (db > kSilenceDb) {
            windowDb.push_back(db);
            windowLowDb.push_back(lowDb);
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
    const int windows = static_cast<int>(timeline.size());
    const auto spanMeanDb = [&timeline](int first, int count) {
        double sum = 0.0;
        for (int w = first; w < first + count; ++w) {
            sum += timeline[w];
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
            if (timelineLow[w] >= typicalLowDb - kBodyStartFirstWindowMarginDb &&
                    spanMedian(timelineLow, w, span) >= typicalLowDb - kBodyStartMarginDb &&
                    spanMedian(timeline, w, span) >= typicalDb - kBodyStartMarginDb) {
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
