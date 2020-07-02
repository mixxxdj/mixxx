#include "analyzer/analyzerrhythm.h"

#include <QHash>
#include <QString>
#include <QVector>
#include <QtDebug>
#include <unordered_set>

#include "analyzer/constants.h"
#include "engine/engine.h" // Included to get mixxx::kEngineChannelCount
#include "engine/engine.h"
#include "track/beatfactory.h"
#include "track/beats.h"
#include "track/track.h"

namespace {

// This determines the resolution of the resulting BeatMap.
// ~12 ms (86 Hz) is a fair compromise between accuracy and analysis speed,
// also matching the preferred window/step sizes from BeatTrack VAMP.
constexpr float kStepSecs = 0.0113378684807f;
// results in 43 Hz @ 44.1 kHz / 47 Hz @ 48 kHz / 47 Hz @ 96 kHz
constexpr int kMaximumBinSizeHz = 50; // Hz
// This is a quick hack to make a beatmap with only downbeats - will affect the bpm
constexpr bool useDownbeatOnly = true;
// The range of bpbs considered for detection, lower is included, higher excluded [)
constexpr int kLowerBeatsPerBar = 4;
constexpr int kHigherBeatsPerBar = 5;
// The number of types of detection functions
constexpr int kDfTypes = 5;

DFConfig makeDetectionFunctionConfig(int stepSize, int windowSize) {
    // These are the defaults for the VAMP beat tracker plugin
    DFConfig config;
    config.DFType = dfAll - dfBroadBand;
    config.stepSize = stepSize;
    config.frameLength = windowSize;
    config.dbRise = 3;
    config.adaptiveWhitening = 0;
    config.whiteningRelaxCoeff = -1;
    config.whiteningFloor = -1;
    return config;
}

} // namespace

AnalyzerRhythm::AnalyzerRhythm(UserSettingsPointer pConfig)
        : m_iTotalSamples(0),
          m_iMaxSamplesToProcess(0),
          m_iCurrentSample(0),
          m_iMinBpm(0),
          m_iMaxBpm(9999),
          m_beatsPerBar(4) {
}

inline int AnalyzerRhythm::stepSize() {
    return static_cast<int>(m_sampleRate * kStepSecs);
}

inline int AnalyzerRhythm::windowSize() {
    return MathUtilities::nextPowerOfTwo(m_sampleRate / kMaximumBinSizeHz);
}

bool AnalyzerRhythm::initialize(const AnalyzerTrack& track,
        mixxx::audio::SampleRate sampleRate,
        SINT totalSamples) {
    if (totalSamples == 0 or !shouldAnalyze(track.getTrack())) {
        return false;
    }

    m_sampleRate = sampleRate;
    m_iTotalSamples = totalSamples;
    m_iMaxSamplesToProcess = m_iTotalSamples;
    m_iCurrentSample = 0;

    m_pDetectionFunction = std::make_unique<DetectionFunction>(
            makeDetectionFunctionConfig(stepSize(), windowSize()));
    // decimation factor aims at resampling to c. 3KHz; must be power of 2
    int factor = MathUtilities::nextPowerOfTwo(m_sampleRate / 3000);
    m_downbeat = std::make_unique<DownBeat>(
            m_sampleRate, factor, stepSize());

    qDebug() << "input sample rate is " << m_sampleRate << ", step size is " << stepSize();

    m_processor.initialize(
            windowSize(), stepSize(), [this](double* pWindow, size_t) {
                m_detectionResults.push_back(
                        m_pDetectionFunction->processTimeDomain(pWindow));
                m_downbeat->pushAudioBlock(reinterpret_cast<float*>(pWindow));
                return true;
            });

    return true;
}

bool AnalyzerRhythm::shouldAnalyze(TrackPointer pTrack) const {
    bool bpmLock = pTrack->isBpmLocked();
    if (bpmLock) {
        qDebug() << "Track is BpmLocked: Beat calculation will not start";
        return false;
    }
    const mixxx::BeatsPointer pBeats = pTrack->getBeats();
    if (!pBeats) {
        return true;
    }
    if (!pBeats->getBpmInRange(mixxx::audio::kStartFramePos,
                       mixxx::audio::FramePos{
                               pTrack->getDuration() * pBeats->getSampleRate()})
                    .isValid()) {
        // Tracks with an invalid bpm <= 0 should be re-analyzed,
        // independent of the preference settings. We expect that
        // all tracks have a bpm > 0 when analyzed. Users that want
        // to keep their zero bpm tracks could lock them to prevent
        // this re-analysis (see the check above).
        qDebug() << "Re-analyzing track with invalid BPM despite preference settings.";
        return true;
    }

    qDebug() << "Track already has beats, and won't re-analyze";
    return false;
}

bool AnalyzerRhythm::processSamples(const CSAMPLE* pIn, SINT iLen) {
    m_iCurrentSample += iLen;
    if (m_iCurrentSample > m_iMaxSamplesToProcess) {
        return true; // silently ignore all remaining samples
    }
    return m_processor.processStereoSamples(pIn, iLen);
}

void AnalyzerRhythm::cleanup() {
    m_resultBeats.clear();
    m_detectionResults.clear();
    m_pDetectionFunction.reset();
}

std::vector<double> AnalyzerRhythm::computeBeats() {
    std::vector<std::vector<double>> allBeats(kDfTypes);
    for (int dfType = 0; dfType < kDfTypes; dfType += 1) {
        int nonZeroCount = m_detectionResults.size();
        while (nonZeroCount > 0 && m_detectionResults.at(nonZeroCount - 1).results[dfType] <= 0.0) {
            --nonZeroCount;
        }

        std::vector<double> noteOnsets;
        std::vector<double> beatPeriod;
        std::vector<double> tempi;
        const auto required_size = std::max(0, nonZeroCount - 2);
        noteOnsets.reserve(required_size);
        beatPeriod.reserve(required_size);

        // skip first 2 results as it might have detect noise as onset
        // that's how vamp does and seems works best this way
        for (int i = 2; i < nonZeroCount; ++i) {
            noteOnsets.push_back(m_detectionResults.at(i).results[dfType]);
            beatPeriod.push_back(0.0);
        }

        TempoTrackV2 tt(m_sampleRate, stepSize());
        tt.calculateBeatPeriod(noteOnsets, beatPeriod, tempi);

        tt.calculateBeats(noteOnsets, beatPeriod, allBeats[dfType]);
    }
    // Let's compare all beats positions and use the "best" one
    double maxAgreement = 0.0;
    int maxAgreementIndex = 0;
    for (int thisOne = 0; thisOne < kDfTypes; thisOne += 1) {
        double agreementPercentage;
        int agreement = 0;
        int maxPossibleAgreement = 1;
        std::unordered_set<double> thisOneAsSet(allBeats[thisOne].begin(), allBeats[thisOne].end());
        for (int theOther = 0; theOther < kDfTypes; theOther += 1) {
            if (thisOne == theOther) {
                continue;
            }
            for (size_t beat = 0; beat < allBeats[theOther].size(); beat += 1) {
                if (thisOneAsSet.find(allBeats[theOther][beat]) != thisOneAsSet.end()) {
                    agreement += 1;
                }
                maxPossibleAgreement += 1;
            }
        }
        agreementPercentage = agreement / static_cast<double>(maxPossibleAgreement);
        qDebug() << thisOne << "agreementPercentage is" << agreementPercentage;
        if (agreementPercentage > maxAgreement) {
            maxAgreement = agreementPercentage;
            maxAgreementIndex = thisOne;
        }
    }
    return allBeats[maxAgreementIndex];
}

std::vector<double> AnalyzerRhythm::computeBeatsSpectralDifference(std::vector<double>& beats) {
    size_t downLength = 0;
    const float* downsampled = m_downbeat->getBufferedAudio(downLength);
    std::vector<int> downbeats;
    m_downbeat->findDownBeats(downsampled, downLength, beats, downbeats);
    std::vector<double> beatsSpecDiff;
    m_downbeat->getBeatSD(beatsSpecDiff);
    return beatsSpecDiff;
}

// This naive approach for bpb did't work
// but that's how QM Lib compute the downbeat
// leaving the outer for for now as it might be useful later
std::tuple<int, int> AnalyzerRhythm::computeMeter(std::vector<double>& beatsSD) {
    int candidateDownbeatPosition = 0;
    int candidateBeatsPerBar = 0;
    std::vector<std::vector<double>> specDiffSeries(kHigherBeatsPerBar - kLowerBeatsPerBar);
    // let's considers all bpb candidates
    for (candidateBeatsPerBar = kLowerBeatsPerBar;
            candidateBeatsPerBar < kHigherBeatsPerBar;
            candidateBeatsPerBar += 1) {
        specDiffSeries[candidateBeatsPerBar - kLowerBeatsPerBar] =
                std::vector<double>(candidateBeatsPerBar, 0);
        // and all downbeats position candidates
        for (candidateDownbeatPosition = 0;
                candidateDownbeatPosition < candidateBeatsPerBar;
                candidateDownbeatPosition += 1) {
            int count = 0;
            // to compute the mean spec diff of all possible measures
            for (int barBegin = candidateDownbeatPosition - 1;
                    barBegin < static_cast<int>(beatsSD.size());
                    barBegin += candidateBeatsPerBar) {
                if (barBegin >= 0) {
                    specDiffSeries[candidateBeatsPerBar - kLowerBeatsPerBar]
                                  [candidateDownbeatPosition] += beatsSD[barBegin];
                    count += 1;
                }
            }
            specDiffSeries[candidateBeatsPerBar - kLowerBeatsPerBar]
                          [candidateDownbeatPosition] /= count;
        }
    }
    // here we find the series with the largest mean spec diff
    int bestBpb = 0, bestDownbeatPos = 0;
    double value = 0;
    for (int i = 0; i < static_cast<int>(specDiffSeries.size()); i += 1) {
        for (int j = 0; j < static_cast<int>(specDiffSeries[i].size()); j += 1) {
            if (specDiffSeries[i][j] > value) {
                value = specDiffSeries[i][j];
                bestBpb = i;
                bestDownbeatPos = j;
            }
        }
    }
    return std::make_tuple(bestBpb + kLowerBeatsPerBar, bestDownbeatPos);
}

void AnalyzerRhythm::storeResults(TrackPointer pTrack) {
    m_processor.finalize();
    std::vector<double> beats = computeBeats();
    auto beatsSpecDiff = computeBeatsSpectralDifference(beats);
    auto [bpb, firstDownbeat] = computeMeter(beatsSpecDiff);

    m_beatsPerBar = bpb;
    size_t nextDownbeat = firstDownbeat;
    // convert beats positions from df increments to frams
    for (size_t i = 0; i < beats.size(); ++i) {
        auto result = mixxx::audio::FramePos((beats.at(i) * stepSize()) -
                stepSize() / mixxx::kEngineChannelCount);
        if (i == nextDownbeat) {
            m_downbeats.push_back(i);
            nextDownbeat += bpb;
        }
        m_resultBeats.push_back(result);
    }
    QMap<int, double> beatsWithNewTempo;
    std::tie(m_resultBeats, beatsWithNewTempo) = FixBeatsPositions();
    // quick hack for manual testing of downbeats
    if (useDownbeatOnly) {
        QVector<mixxx::audio::FramePos> downbeats;
        auto dbIt = m_downbeats.begin();
        for (int i = 0; i < m_resultBeats.size(); i++) {
            if (i == *dbIt) {
                downbeats.push_back(m_resultBeats[i]);
                dbIt += 1;
            }
        }
        m_resultBeats = downbeats;
    }
    //  TODO(Cristiano&Harshit) THIS IS WHERE A BEAT VECTOR IS CREATED
    mixxx::BeatsPointer pBeats = BeatFactory::makePreferredBeats(
            m_resultBeats,
            QHash<QString, QString>{},
            false,
            m_sampleRate);
    pTrack->trySetBeats(pBeats);
}
