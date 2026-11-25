#include "analyzer/plugins/constantbeatgrid.h"

#include <QVector>
#include <algorithm>
#include <array>
#include <cmath>

#include "analyzer/plugins/onsetutils.h"
#include "track/beatutils.h"

namespace mixxx {

namespace {

using onsetutils::bandLevel;
using onsetutils::foldKernelSmoothed;
using onsetutils::kBinsPerFrame;
using onsetutils::kFoldKernel;
using onsetutils::kFoldKernelSum;
using onsetutils::median;
using onsetutils::wrap;
using onsetutils::wrapCentered;
using onsetutils::wrapIndex;


// Tempo search: +-1 % around the tracker tempo in 0.02 % steps, then +-0.02 %
// in 0.0005 % steps.
constexpr double kCoarseRange = 0.01;
constexpr int kCoarseSteps = 101;
constexpr double kFineRange = 0.0002;
constexpr int kFineSteps = 81;

// Dotted tempo: in drum and bass and other breakbeat music the tracker often
// follows 2/3 of the tempo (e.g. 116 instead of 174 BPM): its comb filter
// favours tempos near 120 BPM, and the kick and snare pattern has strong
// onsets 1.5 beats apart. The onsets folded at that period spread over three
// peaks a third of a period apart, so the fold at 2/3 of the period (3/2 of the
// tempo) is about as sharp or sharper, while it is clearly less sharp at the
// right tempo. The tempo is taken 3/2 higher if the fold there is at least
// kDottedMinSharpnessRatio times as sharp and the onsets are clearly periodic
// (kDottedMinSharpness): on a flat fold the two are close by chance.
constexpr double kDottedMinSharpnessRatio = 0.95;
constexpr double kDottedMinSharpness = 1.4;

// BPM rounding range: the periods whose beats drift less than this from the
// best ones over the rounding length, as in BeatUtils::makeConstBpm().
constexpr double kRoundingMaxDriftSecs = 0.025;

// Tracker beat intervals within this relative error of a period count as
// following that period, see octaveFollowedLongest().
constexpr double kOctaveMaxIntervalError = 0.06;
// Tracker beats further than this from the grid are ignored for the phase.
constexpr double kMaxSecsPhaseError = 0.025;

// Weight of each low band onset log ratio relative to the detection function's
// in the half beat decision. It is low because a bass line on the off-beats
// often has stronger low band onsets than the kicks on the beats, which show
// in the detection function. The tracker's phase wins a close decision.
constexpr double kLowBandWeight = 0.15;
constexpr double kTrackerPhaseBias = 0.1;
// Below this tempo a beat often spans a kick and a snare of a twice as fast
// pulse (half-time drum and bass at 85-88 BPM): the detection function peaks
// at the snare on the half beat, while the low bands follow the kicks.
constexpr double kHalfTimeMaxBpm = 90;
constexpr double kHalfTimeLowBandWeight = 0.5;

// Beat timing: finally the grid moves to the highest peak of the detection
// function folded at the beat period (kBinsPerFrame bins per frame, smoothed
// over +-kAlignSmoothBins bins) within +-kMaxAlignFrames frames, because the
// tracker beats are whole frames (11.6 ms).
constexpr int kAlignSmoothBins = 2;
constexpr double kMaxAlignFrames = 5;
// The aligned beats are on average 2 ms earlier than reference beat grids;
// the grid is moved by that much.
constexpr double kBeatDelaySecs = 0.002;

// Sharpness of the onsets folded at a period (in onset frames): peak of the
// smoothed circular histogram divided by its mean. Folds kFoldBatch periods
// in one pass over the onsets: a single fold is bound by the latency of its
// phase update, which the independent periods of a batch hide. Each period is
// folded with the same arithmetic as alone, so batching does not change the
// result.
constexpr int kFoldBatch = 4;
using FoldPeriods = std::array<double, kFoldBatch>;

class FoldSharpness {
  public:
    explicit FoldSharpness(const std::vector<double>& onsets)
            : m_onsets(onsets) {
    }

    // Sharpness at periods[0] ... periods[count - 1], 1 <= count <= kFoldBatch.
    FoldPeriods operator()(FoldPeriods periods, int count) {
        for (int j = count; j < kFoldBatch; ++j) {
            periods[j] = periods[count - 1];
        }
        std::array<int, kFoldBatch> numBins;
        FoldPeriods binsPerFrame;
        std::array<double*, kFoldBatch> bins;
        for (int j = 0; j < kFoldBatch; ++j) {
            numBins[j] = static_cast<int>(std::lround(periods[j] * kBinsPerFrame));
            binsPerFrame[j] = numBins[j] / periods[j];
            // One extra bin takes the interpolation weight past the last bin;
            // it is added to the first bin after the loop, so the loop needs
            // no modulo.
            m_histograms[j].assign(numBins[j] + 1, 0.0);
            bins[j] = m_histograms[j].data();
        }
        FoldPeriods phases{}; // frame index modulo period
        double sum = 0;
        for (const double onset : m_onsets) {
            for (int j = 0; j < kFoldBatch; ++j) {
                // The phase is never negative, so truncation is the floor.
                const double bin = phases[j] * binsPerFrame[j];
                int bin0 = static_cast<int>(bin);
                const double weight1 = bin - bin0;
                if (bin0 >= numBins[j]) {
                    bin0 -= numBins[j];
                }
                bins[j][bin0] += onset * (1 - weight1);
                bins[j][bin0 + 1] += onset * weight1;
                phases[j] += 1;
                if (phases[j] >= periods[j]) {
                    phases[j] -= periods[j];
                }
            }
            sum += onset;
        }
        FoldPeriods sharpness;
        for (int j = 0; j < kFoldBatch; ++j) {
            bins[j][0] += bins[j][numBins[j]];
            sharpness[j] = peakToMean(bins[j], numBins[j], sum);
        }
        return sharpness;
    }

  private:
    static double peakToMean(const double* pBins, int numBins, double sum) {
        double peak = 0;
        for (int i = 0; i < numBins; ++i) {
            peak = std::max(peak, foldKernelSmoothed(pBins, numBins, i));
        }
        const double mean = kFoldKernelSum * sum / numBins;
        return peak / (mean + 1e-12);
    }

    const std::vector<double>& m_onsets;
    std::array<std::vector<double>, kFoldBatch> m_histograms;
};

double findSharpestPeriod(FoldSharpness& foldSharpness,
        double centerPeriod,
        double range,
        int steps,
        double* pSharpness) {
    double bestPeriod = centerPeriod;
    double bestSharpness = -1;
    for (int i = 0; i < steps; i += kFoldBatch) {
        const int count = std::min(kFoldBatch, steps - i);
        FoldPeriods periods{};
        for (int j = 0; j < count; ++j) {
            periods[j] = centerPeriod * (1 - range + 2 * range * (i + j) / (steps - 1));
        }
        const FoldPeriods sharpness = foldSharpness(periods, count);
        for (int j = 0; j < count; ++j) {
            if (sharpness[j] > bestSharpness) {
                bestSharpness = sharpness[j];
                bestPeriod = periods[j];
            }
        }
    }
    *pSharpness = bestSharpness;
    return bestPeriod;
}

// The period among half, once and twice period that the intervals of beats
// match for the longest total time. The tracker often follows half or double
// the tempo in parts of a track. BeatUtils::makeConstBpm() takes the tempo of
// the longest constant region, while the beats at the other tempo may be split
// into many shorter regions that together last longer.
double octaveFollowedLongest(const std::vector<double>& beats, double period) {
    const auto timeAt = [&beats](double target) {
        double time = 0;
        for (std::size_t i = 1; i < beats.size(); ++i) {
            const double interval = beats[i] - beats[i - 1];
            if (std::abs(interval / target - 1) < kOctaveMaxIntervalError) {
                time += interval;
            }
        }
        return time;
    };
    const double timeAtPeriod = timeAt(period);
    const double timeAtHalf = timeAt(period / 2);
    const double timeAtDouble = timeAt(period * 2);
    if (timeAtHalf > timeAtPeriod && timeAtHalf >= timeAtDouble) {
        return period / 2;
    }
    if (timeAtDouble > timeAtPeriod) {
        return period * 2;
    }
    return period;
}

// Mean of the per beat maximum of x within +-1 frame around phase + k * period.
double meanAtBeats(const std::vector<double>& x, double phase, double period) {
    const int size = static_cast<int>(x.size());
    double sum = 0;
    int count = 0;
    for (double t = phase; t < size; t += period) {
        const int i = static_cast<int>(std::lround(t));
        if (i < 1 || i >= size - 1) {
            continue;
        }
        sum += std::max({x[i - 1], x[i], x[i + 1]});
        ++count;
    }
    return count > 0 ? sum / count : 0;
}

double onOffBeatLogRatio(const std::vector<double>& x, double phase, double period) {
    const double onBeat = meanAtBeats(x, phase, period);
    const double offBeat = meanAtBeats(x, phase + period / 2, period);
    return std::log((onBeat + 1e-9) / (offBeat + 1e-9));
}

// Mean of x folded at a fractional period (in frames) into binCount bins,
// linearly interpolated.
std::vector<double> foldMean(const std::vector<double>& x, double period, int binCount) {
    std::vector<double> histogram(binCount, 0.0);
    const long count = static_cast<long>(static_cast<double>(x.size()) / period) - 1;
    if (count <= 0) {
        return histogram;
    }
    const double binFrames = period / binCount;
    for (long k = 0; k < count; ++k) {
        for (int bin = 0; bin < binCount; ++bin) {
            const double position = k * period + bin * binFrames;
            const auto i = static_cast<std::size_t>(position);
            const double fraction = position - static_cast<double>(i);
            histogram[bin] += x[i] * (1 - fraction) + x[i + 1] * fraction;
        }
    }
    for (double& value : histogram) {
        value /= static_cast<double>(count);
    }
    return histogram;
}

// 2/3 of period if the tracker followed the dotted tempo, see kDottedMinSharpness.
double correctDottedPeriod(const std::vector<double>& onsets, double period) {
    FoldSharpness foldSharpness(onsets);
    double sharpness;
    findSharpestPeriod(foldSharpness, period, kCoarseRange, kCoarseSteps, &sharpness);
    double dottedSharpness;
    const double dottedPeriod = findSharpestPeriod(foldSharpness,
            period * 2 / 3,
            kCoarseRange,
            kCoarseSteps,
            &dottedSharpness);
    if (sharpness >= kDottedMinSharpness &&
            dottedSharpness >= kDottedMinSharpnessRatio * sharpness) {
        return dottedPeriod;
    }
    return period;
}

// The phase of the tracker beats on a grid of period (all in onset frames):
// the median offset of the beats modulo half a beat, then the mean of the
// offsets within maxPhaseErrorFrames of it (as in BeatUtils::adjustPhase).
// Modulo half a beat, since the tracker often follows the off-beat in some
// parts of a track only, and the median of the two clusters would fall
// between them.
double trackerPhase(
        const std::vector<double>& beats, double period, double maxPhaseErrorFrames) {
    const double halfPeriod = period / 2;
    std::vector<double> offsets;
    offsets.reserve(beats.size());
    for (const double beat : beats) {
        offsets.push_back(wrapCentered(beat - beats.front(), halfPeriod));
    }
    double phase = wrap(beats.front() + median(offsets), period);
    double offsetSum = 0;
    int offsetCount = 0;
    for (const double beat : beats) {
        const double offset = wrapCentered(beat - phase, halfPeriod);
        if (std::abs(offset) < maxPhaseErrorFrames) {
            offsetSum += offset;
            ++offsetCount;
        }
    }
    if (offsetCount > 0) {
        phase = wrap(phase + offsetSum / offsetCount, period);
    }
    return phase;
}

// > 0 if the beats at phase are stronger than the off-beats: the log ratio of
// the detection function plus lowBandWeight times that of the flux of each
// low band.
double halfBeatScore(const std::vector<double>& onsets,
        const std::vector<float>& bands,
        double phase,
        double period,
        double lowBandWeight) {
    double score = onOffBeatLogRatio(onsets, phase, period);
    std::vector<double> flux(onsets.size());
    for (int band = 0; band < kConstantBeatGridLowBands; ++band) {
        double previous = 0;
        for (std::size_t i = 0; i < onsets.size(); ++i) {
            const double magnitude = bandLevel(bands[i * kQmOnsetBands + band]);
            flux[i] = i > 0 ? std::max(0.0, magnitude - previous) : 0.0;
            previous = magnitude;
        }
        score += lowBandWeight * onOffBeatLogRatio(flux, phase, period);
    }
    return score;
}

} // namespace

ConstantBeatGrid ConstantBeatGrid::movedByHalfBeat(mixxx::audio::SampleRate sampleRate) const {
    const double beatLength = beatLengthFrames(sampleRate);
    return {bpm, mixxx::audio::FramePos(wrap(firstBeat.value() + beatLength / 2, beatLength))};
}

double alignPhaseToOnsets(const std::vector<double>& onsets, double phase, double period) {
    const int binCount = std::max(1, static_cast<int>(std::lround(period)) * kBinsPerFrame);
    const std::vector<double> folded = foldMean(onsets, period, binCount);
    std::vector<double> histogram(binCount, 0.0);
    for (int bin = 0; bin < binCount; ++bin) {
        for (int k = -kAlignSmoothBins; k <= kAlignSmoothBins; ++k) {
            histogram[bin] += folded[wrapIndex(bin + k, binCount)];
        }
    }
    const double binsPerFrame = binCount / period;
    const double center = phase * binsPerFrame;
    int bestBin = -1;
    for (int bin = 0; bin < binCount; ++bin) {
        const double distance = wrap(bin - center + binCount / 2.0, binCount) - binCount / 2.0;
        if (std::abs(distance) <= kMaxAlignFrames * binsPerFrame &&
                (bestBin < 0 || histogram[bin] > histogram[bestBin])) {
            bestBin = bin;
        }
    }
    if (bestBin < 0 || histogram[bestBin] <= 0) {
        return phase;
    }
    // Parabolic interpolation of the peak.
    const double left = histogram[(bestBin + binCount - 1) % binCount];
    const double peak = histogram[bestBin];
    const double right = histogram[(bestBin + 1) % binCount];
    const double denominator = left - 2 * peak + right;
    const double offset = peak >= left && peak >= right && denominator < 0
            ? 0.5 * (left - right) / denominator
            : 0;
    return wrap((bestBin + offset) / binsPerFrame, period);
}

mixxx::Bpm findRoundedOnsetBpm(const std::vector<double>& onsets,
        double centerPeriod,
        double framesPerMinute,
        double roundingLength) {
    if (onsets.empty() || centerPeriod <= 0 || roundingLength <= 0) {
        return {};
    }
    FoldSharpness foldSharpness(onsets);
    double sharpness;
    double period = findSharpestPeriod(foldSharpness,
            centerPeriod,
            kCoarseRange,
            kCoarseSteps,
            &sharpness);
    period = findSharpestPeriod(foldSharpness, period, kFineRange, kFineSteps, &sharpness);
    const double range = kRoundingMaxDriftSecs * framesPerMinute / 60 / roundingLength;
    const double minPeriod = period * (1 - range);
    const double maxPeriod = period * (1 + range);
    return BeatUtils::roundBpmWithinRange(mixxx::Bpm(framesPerMinute / maxPeriod),
            mixxx::Bpm(framesPerMinute / period),
            mixxx::Bpm(framesPerMinute / minPeriod));
}

std::optional<ConstantBeatGrid> refineConstantBeatGrid(
        const std::vector<double>& onsets,
        const std::vector<float>& bands,
        const std::vector<double>& beats,
        mixxx::audio::SampleRate sampleRate,
        int stepSizeFrames) {
    if (!sampleRate.isValid() || stepSizeFrames <= 0 || beats.size() < 2 ||
            onsets.empty() ||
            bands.size() != onsets.size() * kQmOnsetBands) {
        return std::nullopt;
    }

    // Start from the same constant tempo that BeatFactory would make from the
    // tracker beats.
    QVector<mixxx::audio::FramePos> beatFrames;
    beatFrames.reserve(static_cast<int>(beats.size()));
    for (const double beat : beats) {
        beatFrames.push_back(mixxx::audio::FramePos(beat * stepSizeFrames + stepSizeFrames / 2));
    }
    mixxx::audio::FrameDiff_t roundingLength = 0;
    const mixxx::Bpm trackerBpm = BeatUtils::makeConstBpm(
            BeatUtils::retrieveConstRegions(beatFrames, sampleRate),
            sampleRate,
            nullptr,
            &roundingLength);
    if (!trackerBpm.isValid()) {
        return std::nullopt;
    }
    const double framesPerMinute = 60.0 * sampleRate / stepSizeFrames;
    const double trackerPeriod = correctDottedPeriod(onsets,
            octaveFollowedLongest(beats, framesPerMinute / trackerBpm.value()));

    // Tempo
    // The BPM is rounded as far as the beats drift by less than 25 ms over the
    // tracker's longest constant region, like BeatFactory's grid.
    const mixxx::Bpm bpm = findRoundedOnsetBpm(onsets,
            trackerPeriod,
            framesPerMinute,
            std::max(1.0, roundingLength / stepSizeFrames));
    if (!bpm.isValid()) {
        return std::nullopt;
    }
    const double period = framesPerMinute / bpm.value();

    // Phase, up to half a beat: the half beat check below decides between
    // beat and off-beat.
    double phase = trackerPhase(
            beats, period, kMaxSecsPhaseError * sampleRate / stepSizeFrames);

    // Half beat: compare on-beat and off-beat strength of the detection function
    // and of the low band onsets (half-wave rectified log magnitude flux).
    const bool halfTime = bpm.value() < kHalfTimeMaxBpm;
    const double lowBandWeight = halfTime ? kHalfTimeLowBandWeight : kLowBandWeight;
    const double score = halfBeatScore(onsets, bands, phase, period, lowBandWeight);
    if (score + (halfTime ? 0 : kTrackerPhaseBias) < 0) {
        phase = wrap(phase + period / 2, period);
    }

    phase = alignPhaseToOnsets(onsets, phase, period);
    phase = wrap(phase + kBeatDelaySecs * static_cast<double>(sampleRate) / stepSizeFrames,
            period);

    return ConstantBeatGrid{bpm,
            mixxx::audio::FramePos(phase * stepSizeFrames + stepSizeFrames / 2)};
}

} // namespace mixxx
