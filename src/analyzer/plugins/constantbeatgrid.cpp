#include "analyzer/plugins/constantbeatgrid.h"

#include <QVector>
#include <algorithm>
#include <array>
#include <cmath>

#include "track/beatutils.h"

namespace mixxx {

namespace {

// Resolution of the folded onset histogram, in bins per onset frame.
constexpr int kBinsPerFrame = 4;
// Circular smoothing of the folded histogram, +-0.75 onset frames.
constexpr std::array<double, 7> kFoldKernel = {1, 2, 3, 4, 3, 2, 1};
constexpr double kFoldKernelSum = 16;

// Tempo search: +-1 % around the tracker tempo in 0.02 % steps, then +-0.02 %
// in 0.0005 % steps.
constexpr double kCoarseRange = 0.01;
constexpr int kCoarseSteps = 101;
constexpr double kFineRange = 0.0002;
constexpr int kFineSteps = 81;

// BPM rounding range: the periods around the best one (up to +-0.4 %, in
// 0.005 % steps) whose fold sharpness is within 2 % of the best one.
constexpr double kRoundingStep = 0.00005;
constexpr int kMaxRoundingSteps = 80;
constexpr double kRoundingTolerance = 0.02;

// Tracker beats further than this from the grid are ignored for the phase.
constexpr double kMaxSecsPhaseError = 0.025;

// Weight of each low band onset log ratio relative to the detection function's
// in the half beat decision.
constexpr double kLowBandWeight = 0.5;

// Beat timing: finally the grid moves to the highest peak of the detection
// function folded at the beat period (kBinsPerFrame bins per frame, smoothed
// over +-kAlignSmoothBins bins) within +-kMaxAlignFrames frames, because the
// tracker beats are whole frames (11.6 ms).
constexpr int kAlignSmoothBins = 2;
constexpr double kMaxAlignFrames = 5;
// The aligned beats are on average 2 ms earlier than the rekordbox beats (and
// than the Pixabay reference beats); the grid is moved by that much.
constexpr double kBeatDelaySecs = 0.002;

double wrap(double x, double period) {
    return x - period * std::floor(x / period);
}

// x wrapped into (-period / 2, period / 2].
double wrapCentered(double x, double period) {
    x = wrap(x, period);
    return x > period / 2 ? x - period : x;
}

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
        constexpr int kHalfKernel = static_cast<int>(kFoldKernel.size()) / 2;
        double peak = 0;
        for (int i = 0; i < numBins; ++i) {
            double smoothed = 0;
            for (int k = -kHalfKernel; k <= kHalfKernel; ++k) {
                int index = i + k;
                if (index < 0) {
                    index += numBins;
                } else if (index >= numBins) {
                    index -= numBins;
                }
                smoothed += kFoldKernel[k + kHalfKernel] * pBins[index];
            }
            peak = std::max(peak, smoothed);
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

double median(std::vector<double> values) {
    const std::size_t n = values.size();
    std::sort(values.begin(), values.end());
    return n % 2 ? values[n / 2] : (values[n / 2 - 1] + values[n / 2]) / 2;
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

// Returns the phase (in frames) of the highest peak of the onsets folded at the
// period within kMaxAlignFrames of phase, to a fraction of a frame.
double alignToOnsetPeak(const std::vector<double>& onsets, double phase, double period) {
    const int binCount = std::max(1, static_cast<int>(std::lround(period)) * kBinsPerFrame);
    const std::vector<double> folded = foldMean(onsets, period, binCount);
    std::vector<double> histogram(binCount, 0.0);
    for (int bin = 0; bin < binCount; ++bin) {
        for (int k = -kAlignSmoothBins; k <= kAlignSmoothBins; ++k) {
            histogram[bin] += folded[((bin + k) % binCount + binCount) % binCount];
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

} // namespace

std::optional<ConstantBeatGrid> refineConstantBeatGrid(
        const std::vector<double>& onsets,
        const std::vector<float>& lowBands,
        const std::vector<double>& beats,
        mixxx::audio::SampleRate sampleRate,
        int stepSizeFrames) {
    if (!sampleRate.isValid() || stepSizeFrames <= 0 || beats.size() < 2 ||
            onsets.empty() ||
            lowBands.size() != onsets.size() * kConstantBeatGridLowBands) {
        return std::nullopt;
    }

    // Start from the same constant tempo that BeatFactory would make from the
    // tracker beats.
    QVector<mixxx::audio::FramePos> beatFrames;
    beatFrames.reserve(static_cast<int>(beats.size()));
    for (const double beat : beats) {
        beatFrames.push_back(mixxx::audio::FramePos(beat * stepSizeFrames + stepSizeFrames / 2));
    }
    const mixxx::Bpm trackerBpm = BeatUtils::makeConstBpm(
            BeatUtils::retrieveConstRegions(beatFrames, sampleRate), sampleRate, nullptr);
    if (!trackerBpm.isValid()) {
        return std::nullopt;
    }

    // Tempo
    const double framesPerMinute = 60.0 * sampleRate / stepSizeFrames;
    FoldSharpness foldSharpness(onsets);
    double sharpness;
    double period = findSharpestPeriod(foldSharpness,
            framesPerMinute / trackerBpm.value(),
            kCoarseRange,
            kCoarseSteps,
            &sharpness);
    period = findSharpestPeriod(foldSharpness, period, kFineRange, kFineSteps, &sharpness);
    // Widen the range step by step in both directions while the fold stays
    // nearly as sharp.
    double minPeriod = period;
    double maxPeriod = period;
    for (int direction = -1; direction <= 1; direction += 2) {
        bool sharp = true;
        for (int i = 1; sharp && i <= kMaxRoundingSteps; i += kFoldBatch) {
            const int count = std::min(kFoldBatch, kMaxRoundingSteps - i + 1);
            FoldPeriods candidates{};
            for (int j = 0; j < count; ++j) {
                candidates[j] = period * (1 + direction * (i + j) * kRoundingStep);
            }
            const FoldPeriods candidateSharpness = foldSharpness(candidates, count);
            for (int j = 0; j < count; ++j) {
                if (candidateSharpness[j] < sharpness * (1 - kRoundingTolerance)) {
                    sharp = false;
                    break;
                }
                minPeriod = std::min(minPeriod, candidates[j]);
                maxPeriod = std::max(maxPeriod, candidates[j]);
            }
        }
    }
    const mixxx::Bpm bpm = BeatUtils::roundBpmWithinRange(
            mixxx::Bpm(framesPerMinute / maxPeriod),
            mixxx::Bpm(framesPerMinute / period),
            mixxx::Bpm(framesPerMinute / minPeriod));
    if (!bpm.isValid()) {
        return std::nullopt;
    }
    period = framesPerMinute / bpm.value();

    // Phase: median offset of the tracker beats to the grid, then the mean of
    // the offsets close to it (as in BeatUtils::adjustPhase).
    std::vector<double> offsets;
    offsets.reserve(beats.size());
    for (const double beat : beats) {
        offsets.push_back(wrapCentered(beat - beats.front(), period));
    }
    double phase = wrap(beats.front() + median(offsets), period);
    const double maxPhaseErrorFrames = kMaxSecsPhaseError * sampleRate / stepSizeFrames;
    double offsetSum = 0;
    int offsetCount = 0;
    for (const double beat : beats) {
        const double offset = wrapCentered(beat - phase, period);
        if (std::abs(offset) < maxPhaseErrorFrames) {
            offsetSum += offset;
            ++offsetCount;
        }
    }
    if (offsetCount > 0) {
        phase = wrap(phase + offsetSum / offsetCount, period);
    }

    // Half beat: compare on-beat and off-beat strength of the detection function
    // and of the low band onsets (half-wave rectified log magnitude flux).
    double score = onOffBeatLogRatio(onsets, phase, period);
    std::vector<double> flux(onsets.size());
    for (int band = 0; band < kConstantBeatGridLowBands; ++band) {
        double previous = 0;
        for (std::size_t i = 0; i < onsets.size(); ++i) {
            const double magnitude = std::log1p(
                    10.0 * lowBands[i * kConstantBeatGridLowBands + band]);
            flux[i] = i > 0 ? std::max(0.0, magnitude - previous) : 0.0;
            previous = magnitude;
        }
        score += kLowBandWeight * onOffBeatLogRatio(flux, phase, period);
    }
    if (score < 0) {
        phase = wrap(phase + period / 2, period);
    }

    phase = alignToOnsetPeak(onsets, phase, period);
    phase = wrap(phase + kBeatDelaySecs * static_cast<double>(sampleRate) / stepSizeFrames,
            period);

    return ConstantBeatGrid{bpm,
            mixxx::audio::FramePos(phase * stepSizeFrames + stepSizeFrames / 2)};
}

} // namespace mixxx
