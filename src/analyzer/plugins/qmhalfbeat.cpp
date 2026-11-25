#include "analyzer/plugins/qmhalfbeat.h"

#include <algorithm>
#include <array>
#include <cmath>

#include "analyzer/plugins/onsetutils.h"
#include "util/assert.h"

namespace mixxx {

namespace {

using onsetutils::bandLevel;

constexpr int kFeatureCount = 2 * kQmOnsetBands + 2;

// Logistic regression over the features of offBeatFeatures(), fitted on
// beat maps of constant tempo tracks with reference beat grids as
// reference: the label is whether the beat map moved by half a beat hits
// more reference beats within 15 ms, the sample weight is by how much.
constexpr double kOffBeatIntercept = -0.43971014366722233;
constexpr std::array<double, kFeatureCount> kOffBeatWeights = {
        0.4626864901678554,
        0.24817529212431652,
        0.3652276280644409,
        0.7179865711641762,
        0.4193713549660598,
        1.0273604217167258,
        0.22045977127662586,
        0.6064756805088649,
        0.08587178631818695,
        0.4062360164578328,
        0.2608754172147088,
        0.5922435697599832,
        -0.057376761979748225,
        0.027800053023750455,
        -0.23956405873530218,
        -0.6618308980633695,
        0.16192266790215243,
        0.2948588811119269};

// Means over the onset frames of a set of positions; each position takes the
// maximum of its frame and the two neighbours. Positions outside the track
// are left out.
class PositionSampler {
  public:
    PositionSampler(const std::vector<double>& onsets,
            const std::vector<float>& bands,
            int stepSizeFrames,
            const std::vector<double>& positions)
            : m_onsets(onsets),
              m_bands(bands) {
        const auto frameCount = static_cast<long>(m_onsets.size());
        for (double position : positions) {
            const long frame = onsetutils::onsetFrame(position, stepSizeFrames);
            if (frame >= 1 && frame < frameCount - 1) {
                m_frames.push_back(static_cast<std::size_t>(frame));
            }
        }
    }


    // Mean of the detection function.
    double meanOnset() const {
        return mean([this](std::size_t frame) { return m_onsets[frame]; });
    }

    // Mean of the half-wave rectified log magnitude flux of bands
    // [firstBand, lastBand).
    double meanFlux(int firstBand, int lastBand) const {
        return mean([this, firstBand, lastBand](std::size_t frame) {
            double flux = 0;
            if (frame == 0) {
                return flux;
            }
            for (int band = firstBand; band < lastBand; ++band) {
                flux += std::max(0.0, level(frame, band) - level(frame - 1, band));
            }
            return flux;
        });
    }

    // Mean log magnitude of a band.
    double meanLevel(int band) const {
        return mean([this, band](std::size_t frame) { return level(frame, band); });
    }

  private:
    double level(std::size_t frame, int band) const {
        return bandLevel(m_bands[frame * kQmOnsetBands + band]);
    }

    template<typename Value>
    double mean(Value value) const {
        if (m_frames.empty()) {
            return 0;
        }
        double sum = 0;
        for (std::size_t frame : m_frames) {
            sum += std::max({value(frame - 1), value(frame), value(frame + 1)});
        }
        return sum / static_cast<double>(m_frames.size());
    }

    const std::vector<double>& m_onsets;
    const std::vector<float>& m_bands;
    std::vector<std::size_t> m_frames;
};

double logRatio(double numerator, double denominator) {
    constexpr double kEpsilon = 1e-6;
    return std::log((numerator + kEpsilon) / (denominator + kEpsilon));
}

// Per band: flux and level half way between the beats relative to the beats,
// then the same for the detection function and the low band flux.
std::array<double, kFeatureCount> offBeatFeatures(
        const std::vector<double>& beats,
        const std::vector<double>& onsets,
        const std::vector<float>& bands,
        int stepSizeFrames) {
    std::vector<double> midpoints;
    for (std::size_t i = 1; i < beats.size(); ++i) {
        midpoints.push_back((beats[i - 1] + beats[i]) / 2);
    }
    const PositionSampler onBeat(onsets, bands, stepSizeFrames, beats);
    const PositionSampler offBeat(onsets, bands, stepSizeFrames, midpoints);
    std::array<double, kFeatureCount> features{};
    for (int band = 0; band < kQmOnsetBands; ++band) {
        features[2 * band] = logRatio(
                offBeat.meanFlux(band, band + 1), onBeat.meanFlux(band, band + 1));
        features[2 * band + 1] = offBeat.meanLevel(band) - onBeat.meanLevel(band);
    }
    features[2 * kQmOnsetBands] = logRatio(offBeat.meanOnset(), onBeat.meanOnset());
    features[2 * kQmOnsetBands + 1] = logRatio(
            offBeat.meanFlux(0, kConstantBeatGridLowBands),
            onBeat.meanFlux(0, kConstantBeatGridLowBands));
    return features;
}

} // namespace

double qmOffBeatScore(const std::vector<double>& beats,
        const std::vector<double>& onsets,
        const std::vector<float>& bands,
        int stepSizeFrames) {
    VERIFY_OR_DEBUG_ASSERT(bands.size() == onsets.size() * kQmOnsetBands) {
        return 0;
    }
    if (beats.size() < 2 || onsets.size() < 3) {
        return 0;
    }
    const auto features = offBeatFeatures(beats, onsets, bands, stepSizeFrames);
    double score = kOffBeatIntercept;
    for (int i = 0; i < kFeatureCount; ++i) {
        score += kOffBeatWeights[i] * features[i];
    }
    return score;
}

std::optional<ConstantBeatGrid> checkQmConstantGridHalfBeat(
        const std::optional<ConstantBeatGrid>& constantGrid,
        const std::vector<double>& onsets,
        const std::vector<float>& bands,
        audio::SampleRate sampleRate,
        int stepSizeFrames) {
    if (!constantGrid || !constantGrid->bpm.isValid() || !sampleRate.isValid() ||
            stepSizeFrames <= 0 || bands.size() != onsets.size() * kQmOnsetBands) {
        return constantGrid;
    }
    const double beatLength = constantGrid->beatLengthFrames(sampleRate);
    const double endFrame = static_cast<double>(onsets.size()) * stepSizeFrames;
    std::vector<double> beats;
    for (double beat = constantGrid->firstBeat.value(); beat < endFrame; beat += beatLength) {
        beats.push_back(beat);
    }
    if (qmOffBeatScore(beats, onsets, bands, stepSizeFrames) > kQmMinOffBeatScoreToMoveGrid) {
        return constantGrid->movedByHalfBeat(sampleRate);
    }
    return constantGrid;
}

} // namespace mixxx
