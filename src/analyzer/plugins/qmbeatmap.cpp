#include "analyzer/plugins/qmbeatmap.h"

#include <algorithm>
#include <cmath>
#include <numeric>
#include <optional>
#include <utility>

#include "analyzer/plugins/constantbeatgrid.h"
#include "analyzer/plugins/onsetutils.h"
#include "analyzer/plugins/qmhalfbeat.h"
#include "track/beatutils.h"
#include "util/assert.h"

namespace mixxx {

namespace {

using onsetutils::foldKernelSmoothed;
using onsetutils::kBinsPerFrame;
using onsetutils::kFoldKernelSum;
using onsetutils::median;
using onsetutils::onsetFrame;
using onsetutils::onsetFramePosition;
using onsetutils::wrap;
using onsetutils::wrapCentered;
using onsetutils::wrapIndex;

// Regions (step 1): see mergeConstRegions(). The share of the tracker beats
// between two regions that must lie within kMaxSecsMergeGapError of the
// extended grid: the tracker often wanders through breakdowns, so it is less
// than half and the tolerance is wider than kMergeConstMaxSecsPhaseError.
constexpr int kMergeConstMinRegionBeats = 16;
constexpr double kMergeConstMaxSecsPhaseError = 0.025;
constexpr double kMinMergeGapOnGridShare = 0.4;
constexpr double kMaxSecsMergeGapError = 0.04;

// Region extension (step 2)
constexpr int kExtendMinRegionBeats = 16;
constexpr int kExtendWindowBeats = 16;
// A window quieter than this share of the median loudness (8 s chunks of the
// detection function) always agrees: breakdowns and fades.
constexpr double kExtendQuietShare = 0.25;
constexpr double kEnergyChunkSecs = 8;
constexpr double kExtendMaxSecsPeakError = 0.025;
// The kick is on one half of the beat when the log ratio of the low band
// onsets on the beats to those half way between exceeds this.
constexpr double kKickMinLogRatio = 0.2;
// The kick check is used for a region only if this share of its own loud
// windows have the kick on the same half of the beat.
constexpr double kKickMinConsistency = 0.8;
// Fewer beats than this are not checked for the kick or the onsets.
constexpr int kMinCheckedBeats = 4;

// Tempo refinement (step 3)
constexpr int kRefineMinBeats = 32;

// Neighbour merge and bridge (step 4)
constexpr double kMergeMaxTempoDifference = 0.003;
constexpr double kMergeMaxSecsResidual = 0.03;
constexpr int kBridgeMinRegionBeats = 16;
constexpr int kBridgeMaxGapBeats = 32;
constexpr double kBridgeMaxSecsPhaseError = 0.025;

// Boundary smoothing (step 6): an interval is bad when it is outside this
// range of the median of the 17 intervals around it.
constexpr double kMinIntervalRatio = 0.75;
constexpr double kMaxIntervalRatio = 1.25;
constexpr std::size_t kIntervalMedianRadius = 8;
constexpr std::size_t kSmoothBeats = 2;
constexpr int kSmoothPasses = 3;

// Constant tempo (step 7)
constexpr double kConstantMaxTempoDifference = 0.002;
constexpr double kConstantMinShare = 0.75;
// Ratios of beat lengths that count as the same constant tempo.
constexpr double kConstantTempoRatios[] = {1, 2, 0.5, 1.5, 2.0 / 3, 4.0 / 3, 0.75};
// The half beat score is taken on the beat map, so it only moves a constant
// grid that lies on the beat map or half way between its beats.
constexpr double kMaxSecsGridToBeatMap = 0.03;
// Once the regions at the constant tempo cover kConstantMinMainShare, a region
// off it still counts for it when its onsets fold nearly as sharply at the
// constant grid as at its own tempo: the tracker found no clearer beat there,
// as in a beatless intro.
constexpr double kConstantMinMainShare = 0.5;
constexpr double kConstantMinFoldShare = 0.9;

using qmbeatmap::Region;
using qmbeatmap::Regions;

// The number of beats of a region, up to the first beat of the next one.
int constRegionBeatCount(const QVector<BeatUtils::ConstRegion>& regions, int i) {
    return static_cast<int>(std::lround(
            (regions[i + 1].firstBeat - regions[i].firstBeat) / regions[i].beatLength));
}

int beatCount(const Regions& regions, std::size_t i) {
    return static_cast<int>(std::lround(
            (regions[i + 1].firstBeat - regions[i].firstBeat) / regions[i].beatLength));
}

// The beats of all regions, and the end marker moved onto the grid of the last
// region: it is the last tracker beat, which may be off that grid when the
// region was extended over it.
std::vector<double> beatPositions(const Regions& regions) {
    std::vector<double> positions;
    for (std::size_t i = 0; i + 1 < regions.size(); ++i) {
        const int count = beatCount(regions, i);
        for (int k = 0; k < count; ++k) {
            positions.push_back(regions[i].firstBeat + k * regions[i].beatLength);
        }
        if (i + 2 == regions.size()) {
            positions.push_back(regions[i].firstBeat + count * regions[i].beatLength);
        }
    }
    return positions;
}

// The end marker moves with the last region, so the last beat stays on its
// grid.
Regions movedByHalfBeat(Regions regions) {
    for (std::size_t i = 0; i + 1 < regions.size(); ++i) {
        regions[i].firstBeat += regions[i].beatLength / 2;
    }
    if (regions.size() >= 2) {
        regions.back().firstBeat += regions[regions.size() - 2].beatLength / 2;
    }
    return regions;
}

// Appends the first count beats of a region and their indices, from firstIndex
// on, for fitLine().
void appendBeats(const Region& region,
        int count,
        double firstIndex,
        std::vector<double>* pIndices,
        std::vector<double>* pPositions) {
    for (int j = 0; j < count; ++j) {
        pIndices->push_back(firstIndex + j);
        pPositions->push_back(region.firstBeat + j * region.beatLength);
    }
}

// Least-squares line position = firstBeat + beatLength * beatIndex.
Region fitLine(const std::vector<double>& beatIndices, const std::vector<double>& positions) {
    DEBUG_ASSERT(beatIndices.size() == positions.size() && beatIndices.size() >= 2);
    const auto count = static_cast<double>(beatIndices.size());
    const double meanIndex =
            std::accumulate(beatIndices.begin(), beatIndices.end(), 0.0) / count;
    const double meanPosition = std::accumulate(positions.begin(), positions.end(), 0.0) / count;
    double sumIndexIndex = 0;
    double sumIndexPosition = 0;
    for (std::size_t i = 0; i < beatIndices.size(); ++i) {
        const double index = beatIndices[i] - meanIndex;
        sumIndexIndex += index * index;
        sumIndexPosition += index * (positions[i] - meanPosition);
    }
    const double beatLength = sumIndexPosition / sumIndexIndex;
    return {meanPosition - beatLength * meanIndex, beatLength};
}

// The steps of makeQmBeatMap() that look at the onsets.
class BeatMapBuilder {
  public:
    BeatMapBuilder(const std::vector<double>& onsets,
            const std::vector<float>& bands,
            double sampleRate,
            int stepSizeFrames)
            : m_onsets(onsets),
              m_bands(bands),
              m_sampleRate(sampleRate),
              m_stepSizeFrames(stepSizeFrames),
              m_frameCount(static_cast<long>(onsets.size())) {
        const auto chunkFrames = std::max(1L,
                static_cast<long>(kEnergyChunkSecs * sampleRate / stepSizeFrames));
        std::vector<double> chunkMeans;
        for (long start = 0; start + chunkFrames <= m_frameCount; start += chunkFrames) {
            chunkMeans.push_back(meanOnset(start, start + chunkFrames));
        }
        m_medianLoudness = chunkMeans.empty() ? meanOnset(0, m_frameCount) : median(chunkMeans);
    }

    void extend(Regions* pRegions) const;
    void refine(Regions* pRegions) const;
    void mergeNeighbours(Regions* pRegions) const;
    void bridge(Regions* pRegions) const;
    // The share of the track in regions at the constant tempo, see
    // makeQmBeatMap() step 7.
    double constantTempoShare(const Regions& regions, double gridBeatLength) const;

  private:
    double meanOnset(long begin, long end) const {
        if (end <= begin) {
            return 0;
        }
        return std::accumulate(m_onsets.begin() + begin, m_onsets.begin() + end, 0.0) /
                static_cast<double>(end - begin);
    }

    // The onset frames of positions [begin, end), clamped to the track.
    std::pair<long, long> frameRange(double begin, double end) const {
        return {std::max(0L, onsetFrame(begin, m_stepSizeFrames)),
                std::min(m_frameCount, onsetFrame(end, m_stepSizeFrames))};
    }
    // Log ratio of the low band flux on the beats of a grid within [begin,
    // end] to the flux half way between them; 0 if there are fewer than
    // kMinCheckedBeats.
    double kickLogRatio(double firstBeat, double beatLength, double begin, double end) const;
    // kickLogRatio() of a region, or 0 if the kick does not stay on the same
    // half of the beat within the region itself.
    double consistentKickLogRatio(
            double firstBeat, double beatLength, double begin, double end) const;
    // Whether the onsets in [begin, end) agree with a grid, see extend().
    bool agrees(double firstBeat,
            double beatLength,
            double begin,
            double end,
            double kickRatio) const;
    // The detection function in [beginFrame, endFrame) folded at a grid into
    // kBinsPerFrame bins per onset frame.
    std::vector<double> foldHistogram(long beginFrame,
            long endFrame,
            double periodFrames,
            double originFrames) const;
    // The position of the peak of foldHistogram(), as a fraction of a beat
    // modulo 1/2.
    double foldPeakOffset(long beginFrame,
            long endFrame,
            double periodFrames,
            double originFrames) const;
    // Peak to mean ratio of foldHistogram().
    double foldSharpness(long beginFrame, long endFrame, double periodFrames) const;
    // The region from firstBeat to end with its tempo refined.
    Region refined(Region region, double end) const;

    const std::vector<double>& m_onsets;
    const std::vector<float>& m_bands;
    const double m_sampleRate;
    const int m_stepSizeFrames;
    const long m_frameCount;
    double m_medianLoudness;
};

double BeatMapBuilder::kickLogRatio(
        double firstBeat, double beatLength, double begin, double end) const {
    const double firstIndex = std::ceil((begin - firstBeat) / beatLength);
    const double lastIndex = std::floor((end - firstBeat) / beatLength);
    if (lastIndex - firstIndex + 1 < kMinCheckedBeats) {
        return 0;
    }
    std::vector<double> beats;
    std::vector<double> midpoints;
    for (double index = firstIndex; index <= lastIndex; ++index) {
        beats.push_back(firstBeat + index * beatLength);
        if (index < lastIndex) {
            midpoints.push_back(beats.back() + beatLength / 2);
        }
    }
    return qmLowBandFluxLogRatio(beats, midpoints, m_onsets, m_bands, m_stepSizeFrames);
}

std::vector<double> BeatMapBuilder::foldHistogram(
        long beginFrame, long endFrame, double periodFrames, double originFrames) const {
    const int binCount = std::max(8, static_cast<int>(std::lround(kBinsPerFrame * periodFrames)));
    std::vector<double> histogram(binCount, 0.0);
    for (long frame = beginFrame; frame < endFrame; ++frame) {
        const double beats = (frame - originFrames) / periodFrames;
        // With fast math the fraction may round to slightly below 0 or to 1.
        const int bin = static_cast<int>(std::floor((beats - std::floor(beats)) * binCount));
        histogram[wrapIndex(bin, binCount)] += m_onsets[frame];
    }
    return histogram;
}

double BeatMapBuilder::foldPeakOffset(
        long beginFrame, long endFrame, double periodFrames, double originFrames) const {
    const std::vector<double> histogram =
            foldHistogram(beginFrame, endFrame, periodFrames, originFrames);
    const int binCount = static_cast<int>(histogram.size());
    int peakBin = 0;
    double peak = -1;
    for (int bin = 0; bin < binCount; ++bin) {
        const double smoothed = foldKernelSmoothed(histogram.data(), binCount, bin);
        if (smoothed > peak) {
            peak = smoothed;
            peakBin = bin;
        }
    }
    return std::fmod(static_cast<double>(peakBin) / binCount, 0.5);
}

double BeatMapBuilder::foldSharpness(long beginFrame, long endFrame, double periodFrames) const {
    const std::vector<double> histogram = foldHistogram(beginFrame, endFrame, periodFrames, 0);
    const int binCount = static_cast<int>(histogram.size());
    double peak = 0;
    for (int bin = 0; bin < binCount; ++bin) {
        peak = std::max(peak, foldKernelSmoothed(histogram.data(), binCount, bin));
    }
    const double mean = kFoldKernelSum *
            std::accumulate(histogram.begin(), histogram.end(), 0.0) / binCount;
    return mean > 0 ? peak / mean : 0;
}

double BeatMapBuilder::consistentKickLogRatio(
        double firstBeat, double beatLength, double begin, double end) const {
    const double kickRatio = kickLogRatio(firstBeat, beatLength, begin, end);
    if (std::abs(kickRatio) <= kKickMinLogRatio) {
        return kickRatio;
    }
    const double windowLength = kExtendWindowBeats * beatLength;
    int windows = 0;
    int consistent = 0;
    for (double windowBegin = begin; windowBegin < end - kMinCheckedBeats * beatLength;
            windowBegin += windowLength) {
        const double windowKickRatio = kickLogRatio(
                firstBeat, beatLength, windowBegin, std::min(windowBegin + windowLength, end));
        if (std::abs(windowKickRatio) > kKickMinLogRatio) {
            ++windows;
            if (windowKickRatio * kickRatio > 0) {
                ++consistent;
            }
        }
    }
    if (windows > 0 && consistent < kKickMinConsistency * windows) {
        return 0;
    }
    return kickRatio;
}

bool BeatMapBuilder::agrees(double firstBeat,
        double beatLength,
        double begin,
        double end,
        double kickRatio) const {
    const double periodFrames = beatLength / m_stepSizeFrames;
    const double originFrames = onsetFramePosition(firstBeat, m_stepSizeFrames);
    double windowBegin = begin;
    while (windowBegin < end - 0.5 * beatLength) {
        const double windowEnd = std::min(end, windowBegin + kExtendWindowBeats * beatLength);
        const auto [beginFrame, endFrame] = frameRange(windowBegin, windowEnd);
        windowBegin = windowEnd;
        // Windows of fewer than kMinCheckedBeats beats are not checked, so a
        // shorter segment always agrees and is absorbed: so few beats cannot
        // tell a real tempo or phase change from tracker jitter.
        if (endFrame - beginFrame < kMinCheckedBeats * periodFrames ||
                meanOnset(beginFrame, endFrame) < kExtendQuietShare * m_medianLoudness) {
            continue;
        }
        if (std::abs(kickRatio) > kKickMinLogRatio) {
            // The last kExtendWindowBeats beats up to windowEnd, reaching back
            // before windowBegin when the last window is short.
            const double windowKickRatio = kickLogRatio(firstBeat,
                    beatLength,
                    windowEnd - kExtendWindowBeats * beatLength,
                    windowEnd);
            if (std::abs(windowKickRatio) > kKickMinLogRatio && windowKickRatio * kickRatio < 0) {
                // The kick moved to the other half of the beat.
                return false;
            }
        }
        const double offset = foldPeakOffset(beginFrame, endFrame, periodFrames, originFrames);
        if (std::min(offset, 0.5 - offset) * beatLength > kExtendMaxSecsPeakError * m_sampleRate) {
            return false;
        }
    }
    return true;
}

void BeatMapBuilder::extend(Regions* pRegions) const {
    Regions& regions = *pRegions;
    if (regions.size() < 3) {
        return;
    }
    struct ExtendedRegion {
        double begin;
        double beatLength;
        double firstBeat; // of the grid, the region begins on it
        bool done;
    };
    std::vector<ExtendedRegion> extended;
    for (std::size_t i = 0; i + 1 < regions.size(); ++i) {
        extended.push_back({regions[i].firstBeat,
                regions[i].beatLength,
                regions[i].firstBeat,
                false});
    }
    extended.push_back({regions.back().firstBeat, 0, 0, true});
    while (true) {
        // The longest region not extended yet.
        int longest = -1;
        double longestBeats = 0;
        for (std::size_t i = 0; i + 1 < extended.size(); ++i) {
            const double beats = (extended[i + 1].begin - extended[i].begin) /
                    extended[i].beatLength;
            if (!extended[i].done && std::lround(beats) >= kExtendMinRegionBeats &&
                    (longest < 0 || beats > longestBeats)) {
                longest = static_cast<int>(i);
                longestBeats = beats;
            }
        }
        if (longest < 0) {
            break;
        }
        auto i = static_cast<std::size_t>(longest);
        const double firstBeat = extended[i].firstBeat;
        const double beatLength = extended[i].beatLength;
        const double kickRatio = consistentKickLogRatio(
                firstBeat, beatLength, extended[i].begin, extended[i + 1].begin);
        while (i + 2 < extended.size() &&
                agrees(firstBeat,
                        beatLength,
                        extended[i + 1].begin,
                        extended[i + 2].begin,
                        kickRatio)) {
            extended.erase(extended.begin() + i + 1);
        }
        while (i > 0 &&
                agrees(firstBeat,
                        beatLength,
                        extended[i - 1].begin,
                        extended[i].begin,
                        kickRatio)) {
            // Begin on the first beat of the grid in the region before. The
            // first region of the track begins on the nearest beat instead,
            // which keeps a first beat up to half a beat early.
            const double beats = (firstBeat - extended[i - 1].begin) / beatLength;
            double begin = firstBeat - std::floor(beats + 1e-9) * beatLength;
            if (i == 1 && begin - beatLength >= 0 &&
                    extended[0].begin - (begin - beatLength) <= beatLength / 2) {
                begin -= beatLength;
            }
            extended[i].begin = begin;
            extended.erase(extended.begin() + i - 1);
            --i;
        }
        extended[i].done = true;
    }
    regions.clear();
    for (std::size_t i = 0; i + 1 < extended.size(); ++i) {
        regions.push_back({extended[i].begin, extended[i].beatLength});
    }
    regions.push_back({extended.back().begin, regions.back().beatLength});
}

Region BeatMapBuilder::refined(Region region, double end) const {
    const int beats = static_cast<int>(std::lround((end - region.firstBeat) / region.beatLength));
    if (beats < kRefineMinBeats) {
        return region;
    }
    const auto [beginFrame, endFrame] = frameRange(region.firstBeat, end);
    if (endFrame <= beginFrame) {
        return region;
    }
    const std::vector<double> onsets(m_onsets.begin() + beginFrame, m_onsets.begin() + endFrame);
    const mixxx::Bpm bpm = findRoundedOnsetBpm(onsets,
            region.beatLength / m_stepSizeFrames,
            60.0 * m_sampleRate / m_stepSizeFrames,
            static_cast<double>(onsets.size()));
    if (!bpm.isValid()) {
        return region;
    }
    // Keep the centre beat, where the tracker beats fit best, align it with the
    // onsets, then begin within half a beat of the old first beat.
    const double beatLength = 60.0 * m_sampleRate / bpm.value();
    const double centre = (beats - 1) / 2.0;
    double firstBeat = region.firstBeat + centre * (region.beatLength - beatLength);
    const double periodFrames = beatLength / m_stepSizeFrames;
    const double phase = wrap(
            onsetFramePosition(firstBeat, m_stepSizeFrames) - beginFrame, periodFrames);
    firstBeat += wrapCentered(alignPhaseToOnsets(onsets, phase, periodFrames) - phase,
                         periodFrames) *
            m_stepSizeFrames;
    if (firstBeat < region.firstBeat - beatLength / 2) {
        firstBeat += std::ceil((region.firstBeat - beatLength / 2 - firstBeat) / beatLength) *
                beatLength;
    }
    if (firstBeat > region.firstBeat + beatLength / 2) {
        firstBeat -= std::ceil((firstBeat - region.firstBeat - beatLength / 2) / beatLength) *
                beatLength;
    }
    return {firstBeat, beatLength};
}

void BeatMapBuilder::refine(Regions* pRegions) const {
    Regions& regions = *pRegions;
    for (std::size_t i = 0; i + 1 < regions.size(); ++i) {
        regions[i] = refined(regions[i], regions[i + 1].firstBeat);
    }
}

void BeatMapBuilder::mergeNeighbours(Regions* pRegions) const {
    Regions& regions = *pRegions;
    while (true) {
        // The pair with the smallest tempo difference first.
        int best = -1;
        double bestDifference = 0;
        Region bestRegion{};
        for (std::size_t i = 0; i + 2 < regions.size(); ++i) {
            const Region& a = regions[i];
            const Region& b = regions[i + 1];
            const double difference = std::abs(a.beatLength / b.beatLength - 1);
            if (difference > kMergeMaxTempoDifference ||
                    (best >= 0 && difference >= bestDifference)) {
                continue;
            }
            const int beatsA = beatCount(regions, i);
            const int beatsB = beatCount(regions, i + 1);
            if (beatsA < 1 || beatsB < 1) {
                continue;
            }
            std::vector<double> indices;
            std::vector<double> positions;
            appendBeats(a, beatsA, 0, &indices, &positions);
            appendBeats(b, beatsB, beatsA, &indices, &positions);
            const Region line = fitLine(indices, positions);
            double maxResidual = 0;
            for (std::size_t k = 0; k < indices.size(); ++k) {
                maxResidual = std::max(maxResidual,
                        std::abs(line.firstBeat + indices[k] * line.beatLength - positions[k]));
            }
            if (maxResidual > kMergeMaxSecsResidual * m_sampleRate) {
                // A phase jump between them.
                continue;
            }
            best = static_cast<int>(i);
            bestDifference = difference;
            bestRegion = line;
        }
        if (best < 0) {
            return;
        }
        const auto i = static_cast<std::size_t>(best);
        regions.erase(regions.begin() + i + 1);
        regions[i] = refined(bestRegion, regions[i + 1].firstBeat);
    }
}

void BeatMapBuilder::bridge(Regions* pRegions) const {
    Regions& regions = *pRegions;
    bool changed = true;
    while (changed) {
        changed = false;
        for (std::size_t i = 0; i + 1 < regions.size() && !changed; ++i) {
            if (beatCount(regions, i) < kBridgeMinRegionBeats) {
                continue;
            }
            int gapBeats = 0;
            for (std::size_t k = i + 2; k + 1 < regions.size(); ++k) {
                gapBeats += beatCount(regions, k - 1);
                if (gapBeats > kBridgeMaxGapBeats) {
                    break;
                }
                const Region& a = regions[i];
                const Region& c = regions[k];
                if (beatCount(regions, k) < kBridgeMinRegionBeats ||
                        std::abs(a.beatLength / c.beatLength - 1) > kMergeMaxTempoDifference) {
                    continue;
                }
                const double beatsToC = (c.firstBeat - a.firstBeat) / a.beatLength;
                if (std::abs(beatsToC - std::round(beatsToC)) * a.beatLength >
                        kBridgeMaxSecsPhaseError * m_sampleRate) {
                    continue;
                }
                // One line through the beats of A and C.
                std::vector<double> indices;
                std::vector<double> positions;
                appendBeats(a, beatCount(regions, i), 0, &indices, &positions);
                appendBeats(c, beatCount(regions, k), std::round(beatsToC), &indices, &positions);
                regions[i] = fitLine(indices, positions);
                regions.erase(regions.begin() + i + 1, regions.begin() + k + 1);
                regions[i] = refined(regions[i], regions[i + 1].firstBeat);
                changed = true;
                break;
            }
        }
    }
}

// The share of the track in regions at the median tempo or at a simple ratio
// of it, weighted by duration. At a ratio the tracker follows another metric
// level of the same grid, such as triplets. A region at another tempo still
// counts when the constant grid explains its onsets about as well as its own
// tempo does.
double BeatMapBuilder::constantTempoShare(const Regions& regions, double gridBeatLength) const {
    struct Tempo {
        double beatLength;
        double duration;
        std::size_t region;
    };
    std::vector<Tempo> tempos;
    double total = 0;
    for (std::size_t i = 0; i + 1 < regions.size(); ++i) {
        const double duration = regions[i + 1].firstBeat - regions[i].firstBeat;
        tempos.push_back({regions[i].beatLength, duration, i});
        total += duration;
    }
    if (tempos.empty() || total <= 0) {
        return 0;
    }
    // Sorted by BPM, ascending.
    std::sort(tempos.begin(), tempos.end(), [](const Tempo& a, const Tempo& b) {
        return a.beatLength > b.beatLength;
    });
    double medianBeatLength = tempos.back().beatLength;
    double sum = 0;
    for (const Tempo& tempo : tempos) {
        sum += tempo.duration;
        if (sum >= total / 2) {
            medianBeatLength = tempo.beatLength;
            break;
        }
    }
    const auto atConstantTempo = [&](const Tempo& tempo) {
        for (const double ratio : kConstantTempoRatios) {
            if (std::abs(medianBeatLength * ratio / tempo.beatLength - 1) <=
                    kConstantMaxTempoDifference) {
                return true;
            }
        }
        return false;
    };
    const auto explainedByGrid = [&](const Tempo& tempo) {
        const auto [beginFrame, endFrame] = frameRange(
                regions[tempo.region].firstBeat, regions[tempo.region + 1].firstBeat);
        return foldSharpness(beginFrame, endFrame, gridBeatLength / m_stepSizeFrames) >=
                kConstantMinFoldShare *
                foldSharpness(beginFrame, endFrame, tempo.beatLength / m_stepSizeFrames);
    };
    double within = 0;
    std::vector<const Tempo*> others;
    for (const Tempo& tempo : tempos) {
        if (atConstantTempo(tempo)) {
            within += tempo.duration;
        } else {
            others.push_back(&tempo);
        }
    }
    if (within >= kConstantMinMainShare * total) {
        for (const Tempo* pTempo : others) {
            if (explainedByGrid(*pTempo)) {
                within += pTempo->duration;
            }
        }
    }
    return within / total;
}

// Indices of the intervals between positions that are much shorter or longer
// than the intervals around them.
std::vector<std::size_t> badIntervals(const std::vector<double>& positions) {
    std::vector<std::size_t> bad;
    if (positions.size() < 2 * kIntervalMedianRadius + 1) {
        return bad;
    }
    std::vector<double> intervals;
    for (std::size_t i = 1; i < positions.size(); ++i) {
        intervals.push_back(positions[i] - positions[i - 1]);
    }
    for (std::size_t i = 0; i < intervals.size(); ++i) {
        const std::size_t begin = i >= kIntervalMedianRadius ? i - kIntervalMedianRadius : 0;
        const std::size_t end = std::min(intervals.size(), i + kIntervalMedianRadius + 1);
        const double ratio = intervals[i] /
                median(std::vector<double>(intervals.begin() + begin, intervals.begin() + end));
        if (ratio < kMinIntervalRatio || ratio > kMaxIntervalRatio) {
            bad.push_back(i);
        }
    }
    return bad;
}

// Median distance of positions to the nearest beat of a grid.
double medianGridDistance(
        const std::vector<double>& positions, double firstBeat, double beatLength) {
    std::vector<double> distances;
    distances.reserve(positions.size());
    for (const double position : positions) {
        distances.push_back(std::abs(wrapCentered(position - firstBeat, beatLength)));
    }
    return median(std::move(distances));
}

// Step 7: the constant grid moved by half a beat if the half beat model is
// confident (score, see qmOffBeatScore()) that the beat map is on the off-beats
// and the grid lies on the beat map, or the other way round.
ConstantBeatGrid phasedOnBeatMap(const ConstantBeatGrid& grid,
        const std::vector<double>& beatMap,
        double score,
        audio::SampleRate sampleRate) {
    const double beatLength = grid.beatLengthFrames(sampleRate);
    const double firstBeat = grid.firstBeat.value();
    const double onDistance = medianGridDistance(beatMap, firstBeat, beatLength);
    const double offDistance = medianGridDistance(beatMap, firstBeat + beatLength / 2, beatLength);
    const bool gridOnBeatMap = onDistance <= offDistance;
    const bool gridFitsBeatMap =
            std::min(onDistance, offDistance) <= kMaxSecsGridToBeatMap * sampleRate;
    const bool beatMapOffBeat = score > 0;
    if (std::abs(score) > kQmMinOffBeatScoreToMoveGrid && gridFitsBeatMap &&
            gridOnBeatMap == beatMapOffBeat) {
        return grid.movedByHalfBeat(sampleRate);
    }
    return grid;
}

QVector<audio::FramePos> toFramePositions(const std::vector<double>& positions) {
    QVector<audio::FramePos> framePositions;
    framePositions.reserve(static_cast<int>(positions.size()));
    for (const double position : positions) {
        framePositions.append(audio::FramePos(position));
    }
    return framePositions;
}

} // namespace

namespace qmbeatmap {

QVector<BeatUtils::ConstRegion> mergeConstRegions(
        const QVector<BeatUtils::ConstRegion>& constantRegions,
        const QVector<mixxx::audio::FramePos>& coarseBeats,
        mixxx::audio::SampleRate sampleRate) {
    const mixxx::audio::FrameDiff_t maxPhaseError = kMergeConstMaxSecsPhaseError * sampleRate;
    const mixxx::audio::FrameDiff_t maxGapError = kMaxSecsMergeGapError * sampleRate;
    QVector<BeatUtils::ConstRegion> regions = constantRegions;
    // The coarse beats within [lower, upper].
    const auto beatsBetween = [&coarseBeats](mixxx::audio::FramePos lower,
                                      mixxx::audio::FramePos upper) {
        return std::make_pair(
                std::lower_bound(coarseBeats.cbegin(), coarseBeats.cend(), lower),
                std::upper_bound(coarseBeats.cbegin(), coarseBeats.cend(), upper));
    };
    bool merged = true;
    while (merged) {
        merged = false;
        // The last region has zero length and only marks the end.
        for (int i = 0; i < regions.size() - 1 && !merged; ++i) {
            if (constRegionBeatCount(regions, i) < kMergeConstMinRegionBeats) {
                continue;
            }
            const mixxx::audio::FramePos firstBeat = regions[i].firstBeat;
            const mixxx::audio::FrameDiff_t beatLength = regions[i].beatLength;
            // The distance of a position to the grid of region i, extended
            // over the whole track.
            const auto gridError = [firstBeat, beatLength](mixxx::audio::FramePos position) {
                const double beats = (position - firstBeat) / beatLength;
                return std::fabs(beats - std::round(beats)) * beatLength;
            };
            // Try the farthest region first, so one merge covers as much as possible.
            for (int k = regions.size() - 2; k > i + 1; --k) {
                const int beatCount = constRegionBeatCount(regions, k);
                if (beatCount < kMergeConstMinRegionBeats) {
                    continue;
                }
                const mixxx::audio::FramePos endBeat =
                        regions[k].firstBeat + beatCount * regions[k].beatLength;
                if (gridError(regions[k].firstBeat) > maxPhaseError ||
                        gridError(endBeat) > maxPhaseError) {
                    // A different tempo or phase.
                    continue;
                }
                // Enough of the coarse beats in the regions between are on the grid.
                const auto [gapBegin, gapEnd] = beatsBetween(
                        regions[i + 1].firstBeat + maxPhaseError,
                        regions[k].firstBeat - maxPhaseError);
                int gapBeats = 0;
                int gapBeatsOnGrid = 0;
                for (auto it = gapBegin; it < gapEnd; ++it) {
                    ++gapBeats;
                    if (gridError(*it) <= maxGapError) {
                        ++gapBeatsOnGrid;
                    }
                }
                if (gapBeatsOnGrid < kMinMergeGapOnGridShare * gapBeats) {
                    continue;
                }
                // Least-squares line through the coarse beats on the grid,
                // over beat numbers, relative to firstBeat for precision.
                const auto [spanBegin, spanEnd] = beatsBetween(
                        firstBeat - maxPhaseError, endBeat + maxPhaseError);
                double n = 0;
                double sumN = 0;
                double sumX = 0;
                double sumNN = 0;
                double sumNX = 0;
                for (auto it = spanBegin; it < spanEnd; ++it) {
                    if (gridError(*it) > maxPhaseError) {
                        continue;
                    }
                    const double x = *it - firstBeat;
                    const double beatIndex = std::round(x / beatLength);
                    n += 1;
                    sumN += beatIndex;
                    sumX += x;
                    sumNN += beatIndex * beatIndex;
                    sumNX += beatIndex * x;
                }
                const double det = n * sumNN - sumN * sumN;
                if (n < 2 || det <= 0) {
                    continue;
                }
                const double fittedBeatLength = (n * sumNX - sumN * sumX) / det;
                const double fittedOffset = (sumX - fittedBeatLength * sumN) / n;
                regions[i] = {firstBeat + fittedOffset, fittedBeatLength};
                regions.erase(regions.begin() + i + 1, regions.begin() + k + 1);
                merged = true;
                break;
            }
        }
    }
    return regions;
}

Regions mergeNeighbours(Regions regions,
        const std::vector<double>& onsets,
        const std::vector<float>& bands,
        audio::SampleRate sampleRate,
        int stepSizeFrames) {
    BeatMapBuilder(onsets, bands, sampleRate, stepSizeFrames).mergeNeighbours(&regions);
    return regions;
}

Regions bridge(Regions regions,
        const std::vector<double>& onsets,
        const std::vector<float>& bands,
        audio::SampleRate sampleRate,
        int stepSizeFrames) {
    BeatMapBuilder(onsets, bands, sampleRate, stepSizeFrames).bridge(&regions);
    return regions;
}

// Spaces the kSmoothBeats beats on each side of a bad interval evenly
// between the beats around them.
std::vector<double> smoothBoundaries(std::vector<double> positions) {
    for (int pass = 0; pass < kSmoothPasses; ++pass) {
        const std::vector<std::size_t> bad = badIntervals(positions);
        if (bad.empty()) {
            break;
        }
        std::vector<double> smoothed;
        std::size_t done = 0;
        for (const std::size_t interval : bad) {
            const std::size_t first = std::max(done,
                    interval >= kSmoothBeats ? interval - kSmoothBeats : 0);
            const std::size_t last = std::min(positions.size() - 1, interval + 1 + kSmoothBeats);
            if (first >= last) {
                continue;
            }
            const std::size_t contextBegin =
                    first >= kIntervalMedianRadius ? first - kIntervalMedianRadius : 0;
            const std::size_t contextEnd =
                    std::min(positions.size(), last + kIntervalMedianRadius + 1);
            std::vector<double> intervals;
            for (std::size_t i = contextBegin + 1; i < contextEnd; ++i) {
                intervals.push_back(positions[i] - positions[i - 1]);
            }
            const double span = positions[last] - positions[first];
            const long count = std::max(1L, std::lround(span / median(intervals)));
            smoothed.insert(smoothed.end(), positions.begin() + done, positions.begin() + first);
            for (long k = 0; k < count; ++k) {
                smoothed.push_back(positions[first] + span * k / count);
            }
            done = last;
        }
        smoothed.insert(smoothed.end(), positions.begin() + done, positions.end());
        positions = std::move(smoothed);
    }
    return positions;
}

} // namespace qmbeatmap

QmBeatMap makeQmBeatMap(
        const QVector<audio::FramePos>& beats,
        const std::vector<double>& onsets,
        const std::vector<float>& bands,
        audio::SampleRate sampleRate,
        int stepSizeFrames,
        const std::optional<ConstantBeatGrid>& constantGrid) {
    VERIFY_OR_DEBUG_ASSERT(bands.size() == onsets.size() * kQmOnsetBands &&
            sampleRate.isValid() && stepSizeFrames > 0) {
        return {};
    }
    if (beats.size() < 2) {
        return {};
    }
    // 1. Regions
    const auto constRegions = qmbeatmap::mergeConstRegions(
            BeatUtils::retrieveConstRegions(beats, sampleRate), beats, sampleRate);
    Regions regions;
    for (const auto& region : constRegions) {
        regions.push_back({region.firstBeat.value(), region.beatLength});
    }
    if (regions.size() < 2) {
        return {beats, std::nullopt};
    }
    if (onsets.size() < 3) {
        return {toFramePositions(beatPositions(regions)), std::nullopt};
    }
    const BeatMapBuilder builder(onsets, bands, sampleRate, stepSizeFrames);
    // 2. and 3.
    builder.extend(&regions);
    builder.refine(&regions);
    // The half beat model looks at the beats of step 3, before the least
    // squares lines of step 4 can slightly blur the beats of the regions they
    // join.
    const double score = qmOffBeatScore(
            qmbeatmap::smoothBoundaries(beatPositions(regions)), onsets, bands, stepSizeFrames);
    // 4.
    builder.mergeNeighbours(&regions);
    builder.bridge(&regions);
    // 7.
    if (constantGrid && constantGrid->bpm.isValid() &&
            builder.constantTempoShare(regions, constantGrid->beatLengthFrames(sampleRate)) >=
                    kConstantMinShare) {
        return {{},
                phasedOnBeatMap(*constantGrid,
                        qmbeatmap::smoothBoundaries(beatPositions(regions)),
                        score,
                        sampleRate)};
    }
    // 5. and 6. Unlike the constant grid, the beat map has no other phase
    // estimate to fall back on: a confidence threshold here lost more tracks
    // with tempo changes than it saved.
    if (score > 0) {
        regions = movedByHalfBeat(std::move(regions));
    }
    return {toFramePositions(qmbeatmap::smoothBoundaries(beatPositions(regions))), std::nullopt};
}

} // namespace mixxx
