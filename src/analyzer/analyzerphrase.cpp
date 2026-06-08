#include "analyzer/analyzerphrase.h"

#include <QtDebug>
#include <cmath>

#include "track/beats.h"
#include "track/track.h"

namespace {

const mixxx::Logger kLogger("AnalyzerPhrase");

constexpr int kBeatsPerBar = 4;
constexpr int kMinBarsForAnalysis = 8;
constexpr double kNoveltyThreshold = 0.15;
constexpr int kSmoothingWindow = 4;

const ConfigKey kPhraseAnalysisEnabled(
        QStringLiteral("[Waveform]"),
        QStringLiteral("PhraseAnalysisEnabled"));

mixxx::PhraseType classifyPhrase(
        double energy,
        double avgEnergy,
        int phraseIndex,
        int totalPhrases) {
    if (phraseIndex == 0) {
        return mixxx::PhraseType::Intro;
    }
    if (phraseIndex == totalPhrases - 1) {
        return mixxx::PhraseType::Outro;
    }

    if (avgEnergy <= 0.0) {
        return mixxx::PhraseType::Verse;
    }

    double ratio = energy / avgEnergy;
    if (ratio > 1.3) {
        return mixxx::PhraseType::Drop;
    } else if (ratio > 1.1) {
        return mixxx::PhraseType::Chorus;
    } else if (ratio < 0.7) {
        return mixxx::PhraseType::Bridge;
    } else {
        return mixxx::PhraseType::Verse;
    }
}

QVector<double> smoothEnergy(const QVector<double>& barEnergy, int window) {
    QVector<double> smoothed(barEnergy.size());
    for (int i = 0; i < barEnergy.size(); ++i) {
        double sum = 0.0;
        int count = 0;
        for (int j = qMax(0, i - window / 2);
                j < qMin(barEnergy.size(), i + window / 2 + 1);
                ++j) {
            sum += barEnergy[j];
            ++count;
        }
        smoothed[i] = sum / count;
    }
    return smoothed;
}

} // anonymous namespace

AnalyzerPhrase::AnalyzerPhrase(UserSettingsPointer pConfig)
        : m_pConfig(pConfig),
          m_sampleRate(0),
          m_channelCount(0),
          m_totalFrames(0),
          m_currentWindowSum(0.0),
          m_framesInCurrentWindow(0) {
}

bool AnalyzerPhrase::isEnabled(const UserSettingsPointer& pConfig) {
    return pConfig->getValue(kPhraseAnalysisEnabled, false);
}

bool AnalyzerPhrase::initialize(const AnalyzerTrack& track,
        mixxx::audio::SampleRate sampleRate,
        mixxx::audio::ChannelCount channelCount,
        SINT frameLength) {
    if (!isEnabled(m_pConfig)) {
        return false;
    }

    mixxx::PhrasesPointer pPhrases = track.getTrack()->getPhrases();
    if (pPhrases && !pPhrases->isEmpty()) {
        kLogger.debug() << "Track already has phrases, skipping";
        return false;
    }

    m_sampleRate = sampleRate;
    m_channelCount = channelCount;
    m_totalFrames = frameLength;
    m_energyWindows.clear();
    m_currentWindowSum = 0.0;
    m_framesInCurrentWindow = 0;

    const SINT expectedWindows = (frameLength / kEnergyWindowFrames) + 1;
    m_energyWindows.reserve(expectedWindows);

    kLogger.debug() << "Initialized for" << frameLength << "frames";
    return true;
}

bool AnalyzerPhrase::processSamples(const CSAMPLE* pIn, SINT count) {
    const SINT frames = count / m_channelCount;
    SINT processed = 0;

    while (processed < frames) {
        const SINT remaining = frames - processed;
        const SINT windowRemaining = kEnergyWindowFrames - m_framesInCurrentWindow;
        const SINT toProcess = qMin(remaining, windowRemaining);

        for (SINT i = 0; i < toProcess; ++i) {
            const SINT sampleIdx = (processed + i) * m_channelCount;
            float monoSample = 0.0f;
            for (int ch = 0; ch < m_channelCount; ++ch) {
                monoSample += pIn[sampleIdx + ch];
            }
            monoSample /= m_channelCount;
            m_currentWindowSum += static_cast<double>(monoSample) * monoSample;
        }

        m_framesInCurrentWindow += toProcess;
        processed += toProcess;

        if (m_framesInCurrentWindow >= kEnergyWindowFrames) {
            const float rms = static_cast<float>(
                    std::sqrt(m_currentWindowSum / kEnergyWindowFrames));
            m_energyWindows.append(rms);
            m_currentWindowSum = 0.0;
            m_framesInCurrentWindow = 0;
        }
    }

    return true;
}

void AnalyzerPhrase::storeResults(TrackPointer pTrack) {
    if (m_framesInCurrentWindow > 0) {
        const float rms = static_cast<float>(
                std::sqrt(m_currentWindowSum / m_framesInCurrentWindow));
        m_energyWindows.append(rms);
    }

    if (m_energyWindows.isEmpty()) {
        kLogger.debug() << "No energy data collected";
        return;
    }

    mixxx::BeatsPointer pBeats = pTrack->getBeats();
    if (!pBeats) {
        kLogger.debug() << "No beats available, skipping phrase analysis";
        return;
    }

    const int beatsPerBar = kBeatsPerBar;
    const int downbeatOffset = pBeats->downbeatOffset();
    const auto trackEnd = mixxx::audio::FramePos(m_totalFrames);

    // Collect beat positions within the track
    QVector<mixxx::audio::FramePos> beatPositions;
    auto firstMarker = pBeats->cfirstmarker();
    for (auto it = firstMarker; it != pBeats->cend(); ++it) {
        auto pos = *it;
        if (!pos.isValid() || pos > trackEnd) {
            break;
        }
        beatPositions.append(pos);
    }

    if (beatPositions.size() < beatsPerBar * kMinBarsForAnalysis) {
        kLogger.debug() << "Too few beats for phrase analysis:"
                        << beatPositions.size();
        return;
    }

    // Compute per-bar energy
    int firstDownbeatIdx = downbeatOffset;
    if (firstDownbeatIdx >= beatPositions.size()) {
        firstDownbeatIdx = 0;
    }

    QVector<double> barEnergy;
    QVector<mixxx::audio::FramePos> barStartPositions;
    QVector<mixxx::audio::FramePos> barEndPositions;

    for (int i = firstDownbeatIdx;
            i + beatsPerBar < beatPositions.size();
            i += beatsPerBar) {
        double barStartFrame = beatPositions[i].value();
        double barEndFrame = beatPositions[i + beatsPerBar].value();

        barStartPositions.append(beatPositions[i]);
        barEndPositions.append(beatPositions[i + beatsPerBar]);

        int startWindow = static_cast<int>(barStartFrame / kEnergyWindowFrames);
        int endWindow = static_cast<int>(barEndFrame / kEnergyWindowFrames);
        startWindow = qMax(0, startWindow);
        endWindow = qMin(m_energyWindows.size() - 1, endWindow);

        double totalEnergy = 0.0;
        int windowCount = 0;
        for (int w = startWindow; w <= endWindow; ++w) {
            totalEnergy += m_energyWindows[w];
            ++windowCount;
        }

        barEnergy.append(windowCount > 0 ? totalEnergy / windowCount : 0.0);
    }

    if (barEnergy.size() < kMinBarsForAnalysis) {
        kLogger.debug() << "Too few bars for analysis:" << barEnergy.size();
        return;
    }

    // Smooth energy profile
    QVector<double> smoothed = smoothEnergy(barEnergy, kSmoothingWindow);

    // Compute track average energy
    double totalAvg = 0.0;
    for (double e : smoothed) {
        totalAvg += e;
    }
    totalAvg /= smoothed.size();

    // Find max energy for normalization
    double maxEnergy = 0.0;
    for (double e : smoothed) {
        maxEnergy = qMax(maxEnergy, e);
    }

    if (maxEnergy <= 0.0) {
        kLogger.debug() << "Track has no energy, skipping";
        return;
    }

    // Detect novelty (normalized energy change between consecutive bars)
    QVector<double> novelty(smoothed.size(), 0.0);
    for (int i = 1; i < smoothed.size(); ++i) {
        novelty[i] = std::abs(smoothed[i] - smoothed[i - 1]) / maxEnergy;
    }

    // Find phrase boundaries at novelty peaks
    QVector<int> boundaries;
    boundaries.append(0);

    for (int i = 1; i < novelty.size() - 1; ++i) {
        if (novelty[i] >= kNoveltyThreshold &&
                novelty[i] >= novelty[i - 1] &&
                novelty[i] >= novelty[i + 1]) {
            boundaries.append(i);
        }
    }

    // If no boundaries found, try with a lower threshold
    if (boundaries.size() <= 1) {
        double halfThreshold = kNoveltyThreshold / 2.0;
        for (int i = 1; i < novelty.size() - 1; ++i) {
            if (novelty[i] >= halfThreshold &&
                    novelty[i] >= novelty[i - 1] &&
                    novelty[i] >= novelty[i + 1]) {
                boundaries.append(i);
            }
        }
    }

    // Sort and add end boundary
    std::sort(boundaries.begin(), boundaries.end());
    if (boundaries.last() != barEnergy.size()) {
        boundaries.append(barEnergy.size());
    }

    // Quantize boundaries to musical phrase lengths and merge close ones
    QVector<int> quantized;
    quantized.append(0);
    for (int i = 1; i < boundaries.size() - 1; ++i) {
        int barIdx = boundaries[i];
        // Snap to nearest multiple of 4
        int snapped = ((barIdx + 2) / 4) * 4;
        snapped = qBound(0, snapped, barEnergy.size());

        // Skip if too close to previous boundary
        if (!quantized.isEmpty() && snapped - quantized.last() < 4) {
            continue;
        }
        quantized.append(snapped);
    }
    quantized.append(barEnergy.size());

    // Remove duplicates
    quantized.erase(std::unique(quantized.begin(), quantized.end()),
            quantized.end());

    if (quantized.size() < 2) {
        kLogger.debug() << "No phrase boundaries detected";
        return;
    }

    // Create phrases
    int totalPhrases = quantized.size() - 1;
    mixxx::PhrasesPointer pPhrases = std::make_shared<const mixxx::Phrases>();

    for (int i = 0; i < totalPhrases; ++i) {
        int startBar = quantized[i];
        int endBar = quantized[i + 1];

        if (startBar >= barStartPositions.size() ||
                endBar > barEndPositions.size() ||
                startBar >= endBar) {
            continue;
        }

        // Compute average energy for this phrase
        double phraseEnergy = 0.0;
        int barCount = 0;
        for (int b = startBar; b < endBar && b < barEnergy.size(); ++b) {
            phraseEnergy += barEnergy[b];
            ++barCount;
        }
        phraseEnergy = barCount > 0 ? phraseEnergy / barCount : 0.0;

        auto startPos = barStartPositions[startBar];
        auto endPos = (endBar < barEndPositions.size())
                ? barEndPositions[endBar - 1]
                : barEndPositions.last();

        mixxx::PhraseType type = classifyPhrase(
                phraseEnergy, totalAvg, i, totalPhrases);

        mixxx::Phrase phrase(startPos, endPos, type);
        auto result = pPhrases->tryAddPhrase(phrase);
        if (result.has_value()) {
            pPhrases = *result;
        }
    }

    if (pPhrases->isEmpty()) {
        kLogger.debug() << "No phrases generated";
        return;
    }

    pTrack->trySetPhrases(pPhrases);
    kLogger.debug() << "Stored" << pPhrases->size() << "phrases";
}

void AnalyzerPhrase::cleanup() {
    m_energyWindows.clear();
    m_currentWindowSum = 0.0;
    m_framesInCurrentWindow = 0;
}
