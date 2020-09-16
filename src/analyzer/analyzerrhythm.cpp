#include "analyzer/analyzerrhythm.h"

#include <QHash>
#include <QString>
#include <QVector>
#include <QtDebug>
#include <set>
#include <unordered_set>
#include <vector>

#include "analyzer/analyzerrhythmstats.h"
#include "analyzer/constants.h"
#include "engine/engine.h" // Included to get mixxx::kEngineChannelCount
#include "track/beatfactory.h"
#include "track/beatmap.h"
#include "track/track.h"

namespace {

// This determines the resolution of the resulting BeatMap.
// ~12 ms (86 Hz) is a fair compromise between accuracy and analysis speed,
// also matching the preferred window/step sizes from BeatTrack VAMP.
constexpr float kStepSecs = 0.0113378684807;
// results in 43 Hz @ 44.1 kHz / 47 Hz @ 48 kHz / 47 Hz @ 96 kHz
constexpr int kMaximumBinSizeHz = 50; // Hz
// This is a quick hack to make a beatmap with only downbeats - will affect the bpm
constexpr bool useDownbeatOnly = false;
// The number of types of detection functions
constexpr int kDfTypes = 5;
// tempogram resolution constants
constexpr float kNoveltyCurveMinDB = -54.0;
constexpr float kNoveltyCurveCompressionConstant = 400.0;
constexpr int kTempogramLog2WindowLength = 10; // ~ 11.6 s the maximal measure length we can detect
constexpr int kTempogramLog2HopSize = 7;       // ~ 1.45 s Resolution for detecting regions
constexpr int kTempogramLog2FftLength = 10;
constexpr int kNoveltyCurveHop = 512;
constexpr int kNoveltyCurveWindow = 1024;

constexpr double kThressholDecay = 0.04;
constexpr double kThressholRecover = 0.5;
constexpr double kFloorFactor = 0.5;

constexpr double kCloseToIntTolerance = 0.05;

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

double AnalyzerRhythm::frameToMinutes(int frame) {
    double minute = (frame/static_cast<double>(m_iSampleRate))/60.0;
    double intMinutes;
    double fractionalMinutes = std::modf(minute, &intMinutes);
    double seconds = (fractionalMinutes*60.0)/100;
    return intMinutes + seconds;
}

AnalyzerRhythm::AnalyzerRhythm(UserSettingsPointer pConfig)
        : m_iSampleRate(0),
          m_iTotalSamples(0),
          m_iMaxSamplesToProcess(0),
          m_iCurrentSample(0),
          m_iMinBpm(0),
          m_iMaxBpm(9999),
          m_noveltyCurveMinV(pow(10, kNoveltyCurveMinDB / 20.0)) {
}

inline int AnalyzerRhythm::stepSize() {
    return m_iSampleRate * kStepSecs;
}

inline int AnalyzerRhythm::windowSize() {
    return MathUtilities::nextPowerOfTwo(m_iSampleRate / kMaximumBinSizeHz);
}

bool AnalyzerRhythm::initialize(TrackPointer pTrack, int sampleRate, int totalSamples) {
    if (totalSamples == 0 or !shouldAnalyze(pTrack)) {
        return false;
    }

    m_iSampleRate = sampleRate;
    m_iTotalSamples = totalSamples;
    m_iMaxSamplesToProcess = m_iTotalSamples;
    m_iCurrentSample = 0;

    // decimation factor aims at resampling to c. 3KHz; must be power of 2
    int factor = MathUtilities::nextPowerOfTwo(m_iSampleRate / 3000);
    m_downbeat = std::make_unique<DownBeat>(
            m_iSampleRate, factor, stepSize());

    m_fft = std::make_unique<FFTReal>(windowSize());
    m_fftRealOut = new double[windowSize()];
    m_fftImagOut = new double[windowSize()];

    m_window = new Window<double>(HammingWindow, windowSize());
    m_pDetectionFunction = std::make_unique<DetectionFunction>(
            makeDetectionFunctionConfig(stepSize(), windowSize()));

    qDebug() << "input sample rate is " << m_iSampleRate << ", step size is " << stepSize();

    m_onsetsProcessor.initialize(
            windowSize(), stepSize(), [this](double* pWindow, size_t) {
                DFresults onsets;
                m_window->cut(pWindow);
                m_fft->forward(pWindow, m_fftRealOut, m_fftImagOut);
                onsets = m_pDetectionFunction->processFrequencyDomain(m_fftRealOut, m_fftImagOut);
                m_detectionResults.push_back(onsets);
                return true;
            });

    m_downbeatsProcessor.initialize(
            windowSize(), stepSize(), [this](double* pWindow, size_t) {
                m_downbeat->pushAudioBlock(reinterpret_cast<float*>(pWindow));
                return true;
            });

    m_noveltyfft = std::make_unique<FFTReal>(kNoveltyCurveWindow);
    m_noveltyfftRealOut = new double[kNoveltyCurveWindow];
    m_noveltyfftImagOut = new double[kNoveltyCurveWindow];
    m_noveltyWindow = new Window<double>(HammingWindow, kNoveltyCurveWindow);

    m_noveltyCurveProcessor.initialize(kNoveltyCurveWindow,
            kNoveltyCurveHop,
            [this](double* pWindow, size_t) {
                m_noveltyWindow->cut(pWindow);
                m_noveltyfft->forward(pWindow, m_noveltyfftRealOut, m_noveltyfftImagOut);
                //calculate magnitude of FrequencyDomain input
                std::vector<float> fftCoefficients;
                for (int i = 0; i < kNoveltyCurveWindow / 2 + 1; i++) {
                    float magnitude = sqrt(
                            m_noveltyfftRealOut[i] * m_noveltyfftRealOut[i] +
                            m_noveltyfftImagOut[i] * m_noveltyfftImagOut[i]);
                    magnitude = magnitude > m_noveltyCurveMinV
                            ? magnitude
                            : m_noveltyCurveMinV;
                    fftCoefficients.push_back(magnitude);
                }
                m_spectrogram.push_back(fftCoefficients);
                return true;
            });

    return true;
}

void AnalyzerRhythm::setTempogramParameters() {
    m_tempogramWindowLength = pow(2, kTempogramLog2WindowLength);
    m_tempogramHopSize = pow(2, kTempogramLog2HopSize);
    m_tempogramFftLength = pow(2, kTempogramLog2FftLength);

    m_tempogramMinBPM = 10;
    m_tempogramMaxBPM = 70;
    m_tempogramInputSampleRate = m_iSampleRate / kNoveltyCurveHop;
}

bool AnalyzerRhythm::shouldAnalyze(TrackPointer pTrack) const {
    bool bpmLock = pTrack->isBpmLocked();
    if (bpmLock) {
        qDebug() << "Track is BpmLocked: Beat calculation will not start";
        return true;
    }
    mixxx::BeatsPointer pBeats = pTrack->getBeats();
    if (!pBeats) {
        return true;
    } else if (!mixxx::Bpm::isValidValue(pBeats->getBpm())) {
        qDebug() << "Re-analyzing track with invalid BPM despite preference "
                    "settings.";
        return true;
    } else {
        qDebug() << "Track already has beats, and won't re-analyze";
        return false;
    }
    return true;
}

bool AnalyzerRhythm::processSamples(const CSAMPLE* pIn, const int iLen) {
    m_iCurrentSample += iLen;
    if (m_iCurrentSample > m_iMaxSamplesToProcess) {
        return true; // silently ignore all remaining samples
    }
    bool onsetReturn = m_onsetsProcessor.processStereoSamples(pIn, iLen);
    bool downbeatsReturn = m_downbeatsProcessor.processStereoSamples(pIn, iLen);
    bool noveltyCurvReturn =
            m_noveltyCurveProcessor.processStereoSamples(pIn, iLen);
    return onsetReturn & downbeatsReturn & noveltyCurvReturn;
}

void AnalyzerRhythm::cleanup() {
    m_resultBeats.clear();
    m_detectionResults.clear();
    m_noveltyCurve.clear();
    m_tempogramDFT.clear();
    m_tempogramACF.clear();
    m_metergram.clear();
    m_spectrogram.clear();
    m_pDetectionFunction.reset();
    m_fft.reset();
    m_noveltyfft.reset();
    m_downbeat.reset();
    delete m_noveltyWindow;
    delete m_window;
    delete[] m_fftImagOut;
    delete[] m_fftRealOut;
    delete[] m_noveltyfftRealOut;
    delete[] m_noveltyfftImagOut;
    delete[] m_noveltyfftMagnitude;
}

std::vector<double> AnalyzerRhythm::computeBeats() {
    std::vector<double> beats;
    int nonZeroCount = m_detectionResults.size();
    while (nonZeroCount > 0 &&
            m_detectionResults[nonZeroCount - 1].t.complexSpecDiff <= 0.0) {
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
        noteOnsets.push_back(m_detectionResults[i].t.complexSpecDiff);
        beatPeriod.push_back(0.0);
    }

    TempoTrackV2 tt(m_iSampleRate, stepSize());
    tt.calculateBeatPeriod(noteOnsets, beatPeriod, tempi);
    //qDebug() << beatPeriod.size() << tempi.size();
    //qDebug() << tempi;

    tt.calculateBeats(noteOnsets, beatPeriod, beats);
    //qDebug() << allBeats[dfType].size();
    // Let's compare all beats positions and use the "best" one
    /*
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
    */
    for (size_t i = 0; i < beats.size(); ++i) {
        double result = (beats.at(i) * stepSize()) - (stepSize() / mixxx::kEngineChannelCount);
        m_resultBeats.push_back(result);
    }
    return beats;
}

std::vector<int> AnalyzerRhythm::computeSnapGrid() {
    int size = m_detectionResults.size();

    std::vector<double> complexSdMinDiff;
    std::vector<double> minWindow;
    std::vector<double> maxWindow;
    complexSdMinDiff.reserve(size);

    // Calculate the change from the minimum of three previous SDs
    // This ensures we find the beat start and not the noisiest place within the beat.
    complexSdMinDiff.push_back(m_detectionResults[0].results[3]);
    for (int i = 1; i < size; ++i) {
        double result = m_detectionResults[i].results[3];
        double min = result;
        for (int j = i - 3; j < i + 2; ++j) {
            if (j >= 0 && j < size) {
                double value = m_detectionResults[j].results[3];
                if (value < min) {
                    min = value;
                }
            }
        }
        complexSdMinDiff.push_back(result - min * kFloorFactor);
    }

    std::vector<int> allBeats;
    allBeats.reserve(size / 10);

    // This is a dynamic threshold that defines which SD is considered as a beat.
    double threshold = 0.000001;

    // Find peak beats within a window of 9 SDs (100 ms)
    // This limits the detection result to 600 BPM
    for (int i = 0; i < size; ++i) {
        double beat = 0;
        double result = complexSdMinDiff[i];
        m_detectionResults[i].results[1] = complexSdMinDiff[i];
        m_detectionResults[i].results[2] = complexSdMinDiff[i] / threshold;
        if (result > threshold) {
            double max = result;
            for (int j = i - 4; j < i + 4; ++j) {
                if (j >= 0 && j < size) {
                    double value = complexSdMinDiff[j];
                    if (value > max) {
                        max = value;
                    }
                }
            }
            if (max == result) {
                // Beat found
                beat = threshold;
                threshold = threshold * (1 - kThressholRecover) +
                        result * kThressholRecover;
                allBeats.push_back(i);
            }
        }
        threshold =
                threshold * (1 - kThressholDecay) + result * kThressholDecay;
        /*
        qDebug() << i
                 << m_detectionResults[i].results[3]
                 << complexSdMinDiff[i]
                 << threshold << beat;
        */
    }

    return allBeats;
}

std::vector<double> AnalyzerRhythm::computeBeatsSpectralDifference(
        std::vector<double>& beats) {
    size_t downLength = 0;
    const float* downsampled = m_downbeat->getBufferedAudio(downLength);

    std::vector<int> downbeats;
    m_downbeat->findDownBeats(downsampled, downLength, beats, m_downbeats);
    std::vector<double> beatsSpecDiff;
    m_downbeat->getBeatSD(beatsSpecDiff);
    return beatsSpecDiff;
}

void AnalyzerRhythm::computeMeter() {
    auto [tempoList, tempoFrequency] =
            computeRawTemposAndFrequency(m_resultBeats);
    int blockCounter = 0;
    double tempoSum = 0;
    double tempoCounter = 0.0;
    QMap<int, double> accumulatedPulsesLenghtsAndWeights;
    std::vector<QMap<int, double>> blockWisePulsesLenghtsAndWeights;
    for (int i = 0; i < m_resultBeats.size(); i += 1) {
        double beatFramePos = m_resultBeats[i];
        double blockEnd =
                m_tempogramHopSize * (blockCounter + 1) * kNoveltyCurveHop;
        tempoSum += tempoList[i];
        tempoCounter += 1.0;
        if (blockEnd < beatFramePos) {
            // check if pulse is integer ratio of tempo
            double localTempo = tempoSum / tempoCounter;
            qDebug() << "at" << frameToMinutes(m_tempogramHopSize * blockCounter * kNoveltyCurveHop) << "local tempo is" << localTempo;
            QMap<int, double> pulsesLenghtsAndWeights;
            if (blockCounter < static_cast<int>(m_metergram.size())) {
                auto allPulses = m_metergram[blockCounter].keys();
                for (auto pulse : allPulses) {
                    double ratio = localTempo / pulse;
                    double beatLenghtOfPulse;
                    double ratioDecimals = std::modf(ratio, &beatLenghtOfPulse);
                    if (ratioDecimals < kCloseToIntTolerance) {
                        accumulatedPulsesLenghtsAndWeights[static_cast<int>(
                                beatLenghtOfPulse)] +=
                                m_metergram[blockCounter][pulse];
                        pulsesLenghtsAndWeights[static_cast<int>(
                                beatLenghtOfPulse)] +=
                                m_metergram[blockCounter][pulse];
                        blockWisePulsesLenghtsAndWeights.push_back(pulsesLenghtsAndWeights);
                    }
                }
            }
            // set for next iteration
            tempoSum = 0;
            tempoCounter = 0.0;
            blockCounter += 1;
        }
    }
    std::vector<std::vector<int>> metricalHierarchy;
    //combine integer pulses into metrical hieranchies
    auto metricalPulses = accumulatedPulsesLenghtsAndWeights.keys();
    auto primeMetricalPulses = metricalPulses;
    for (auto pulse : metricalPulses) {
        if (pulse == 1) {continue;}
        for (int j = pulse * pulse; j <= metricalPulses.last(); j+=pulse) {
            primeMetricalPulses.removeAt(primeMetricalPulses.indexOf(j));
        }
    }
    for (auto pulse : primeMetricalPulses) {
        std::vector<int> pulseMultiples;
        for (auto nextPulse : metricalPulses) {
            if (nextPulse <= pulse) {continue;}
            if (nextPulse % pulse == 0) {
                pulseMultiples.push_back(nextPulse);
            }
        }
        auto pulseHierarchies = computeMeterHierarchies(pulse, pulseMultiples);
        metricalHierarchy.insert(metricalHierarchy.end(), pulseHierarchies.begin(), pulseHierarchies.end());
    }
    for (size_t i = 0; i < metricalHierarchy.size(); i += 1) {
        if (metricalHierarchy[i].size() == 3) {
            auto firstMetricPair = metricalHierarchy[i];
            auto secondMetricPair = metricalHierarchy[i];
            firstMetricPair.pop_back();
            secondMetricPair.erase(secondMetricPair.begin());
            metricalHierarchy.push_back(firstMetricPair);
            metricalHierarchy.push_back(secondMetricPair);
        }
        else if (metricalHierarchy[i].size() == 4) {
            auto firstMetricTuple = metricalHierarchy[i];
            auto lastMetricTuple = metricalHierarchy[i];
            firstMetricTuple.pop_back();
            lastMetricTuple.erase(lastMetricTuple.begin());
            metricalHierarchy.push_back(firstMetricTuple);
            metricalHierarchy.push_back(lastMetricTuple);
        }
    }
    double highestWeight = 0.0;
    std::vector<int> bestHierarchy;
    for (auto meter : metricalHierarchy) {
        double pulseWeight = 0.0;
        for (auto pulse : meter) {
            pulseWeight += accumulatedPulsesLenghtsAndWeights[pulse];
        }
        double meterWeight = pulseWeight/meter.size();
        qDebug() << meter << meterWeight;
        if (meterWeight > highestWeight) {
            highestWeight = meterWeight;
            bestHierarchy = meter;
        }
    }
    qDebug() << bestHierarchy;
}

std::vector<std::vector<int>> AnalyzerRhythm::computeMeterHierarchies
        (int pulse, std::vector<int> const &pulseMultiples) {
    // This function combines all the possible combinations of common
    // pulse interger divisors in pulseMultiples into seperate vectors
    // ex: 2 and <4,6,8,10,12> | <2,4,8>, <2,4,12> <2,6,12> <2,10>
    std::vector<std::vector<int>> pulseHierarchies;
    std::vector<int> lowestPulse;
    lowestPulse.push_back(pulse);
    pulseHierarchies.push_back(lowestPulse);
    for (size_t i = 0; i < pulseMultiples.size(); i += 1) {
        bool pulseBelongs = false;
        for (auto &testHierarchy : pulseHierarchies) {
            // handle standart case in which multiple of divisible by
            // the last element already in the hierarchy
            if (pulseMultiples[i] % testHierarchy.back() == 0) {
                testHierarchy.push_back(pulseMultiples[i]);
                pulseBelongs = true;
            } else {
                // handle edge cases in which multiple is not divisible
                // by the last element but might be from some element before
                auto last = testHierarchy.end() -1;
                while (last != testHierarchy.begin()){
                    if (pulseMultiples[i] % (*last) == 0) {
                        pulseBelongs = true;
                        std::vector<int> hierarchyCopy;
                        std::copy(testHierarchy.begin(), last+1, std::back_inserter(hierarchyCopy));
                        hierarchyCopy.push_back(pulseMultiples[i]);
                        // this checks that two hierarchies have not reduced to the same
                        // ex pulse = 20, with <2,4,12> and <2,4,8> both will reduce to <2,4>
                        if( hierarchyCopy == pulseHierarchies.back()) {
                            break;
                        }
                        pulseHierarchies.push_back(hierarchyCopy);
                        break;
                        
                    }
                    last -= 1;
                }
            }
        }
        // handle cases in which multiple does not belong in any hierarchy
        if (!pulseBelongs) {
            pulseHierarchies.push_back(lowestPulse);
            pulseHierarchies.back().push_back(pulseMultiples[i]);
        }
    }
    return pulseHierarchies;
}

int AnalyzerRhythm::computeNoveltyCurve() {
    NoveltyCurveProcessor nc(static_cast<float>(m_iSampleRate), kNoveltyCurveWindow, kNoveltyCurveCompressionConstant);
    m_noveltyCurve = nc.spectrogramToNoveltyCurve(m_spectrogram);
    return m_spectrogram.size();
}

void AnalyzerRhythm::computeTempogramByDFT() {
    auto hannWindow = std::vector<float>(m_tempogramWindowLength, 1.0);
    //WindowFunction::hanning(&hannWindow[0], m_tempogramWindowLength);
    SpectrogramProcessor spectrogramProcessor(m_tempogramWindowLength,
            m_tempogramFftLength, m_tempogramHopSize);

    //Spectrogram tempogramDFT = spectrogramProcessor.process(
    //        &m_noveltyCurve[0], m_noveltyCurve.size(), &hannWindow[0]);

    std::vector<float> complexSdCurve;
    for (size_t i = 0; i < m_detectionResults.size(); i++) {
        complexSdCurve.push_back(m_detectionResults[i].results[3]);
    }

    Spectrogram tempogramDFT = spectrogramProcessor.process(
            &complexSdCurve[0], complexSdCurve.size(), &hannWindow[0]);

    /*
    int firstFullAC = m_tempogramWindowLength / 2 / m_tempogramHopSize;
    for(int i = 0; i < tempogramDFT[0].size(); i++) {
        qDebug() << i
              << tempogramDFT[firstFullAC * 4 + 1][i]
              << tempogramDFT[firstFullAC * 4 + 2][i]
              << tempogramDFT[firstFullAC * 4 + 3][i]
              << tempogramDFT[firstFullAC * 4 + 4][i]
              << tempogramDFT[firstFullAC * 4 + 5][i]
              << tempogramDFT[firstFullAC * 4 + 6][i]
              << tempogramDFT[firstFullAC * 4 + 7][i]
              << tempogramDFT[firstFullAC * 4 + 8][i]
              << tempogramDFT[firstFullAC * 4 + 9][i]
              << tempogramDFT[firstFullAC * 4 + 10][i]
              << tempogramDFT[firstFullAC * 4 + 11][i]
              << tempogramDFT[firstFullAC * 4 + 12][i];
    }
    */

    // convert y axis to bpm
    int tempogramMinBin = (std::max(static_cast<int>(floor(((m_tempogramMinBPM/60.0)
            /m_tempogramInputSampleRate)*m_tempogramFftLength)), 0));
    int tempogramMaxBin = (std::min(static_cast<int>(ceil(((m_tempogramMaxBPM/60.0)
            /m_tempogramInputSampleRate)*m_tempogramFftLength)), m_tempogramFftLength/2));
    int binCount = tempogramMaxBin - tempogramMinBin + 1;
    double highest;
    double bestBpm;
    int bin;
    for (size_t block = 0; block < tempogramDFT.size(); block++) {
        // dft
        //qDebug() << "block" << block;
        //qDebug() << "DFT tempogram";
        highest = .0;
        bestBpm = .0;
        bin = 0;
        QMap<double, double> dft;
        for (int k = tempogramMinBin; k <= tempogramMaxBin; k++){
            double w = (k/static_cast<double>(m_tempogramFftLength))*(m_tempogramInputSampleRate);
            double bpm = w*60;
            dft[bpm] = tempogramDFT[block][k];
            //qDebug() << "bin, bpm and value"<< bin++ << bpm << tempogramDFT[block][k];
            if (tempogramDFT[block][k] > highest) {
                highest = tempogramDFT[block][k];
                bestBpm = bpm;
            }
        }
        m_tempogramDFT.push_back(dft);
        //qDebug() << "best bpm at block" << bestBpm << block;
    }
}

void AnalyzerRhythm::computeTempogramByACF() {
    // Compute acf tempogram
    // m_tempogramWindowLength = 4096 ~ 446 s
    // m_tempogramHopSize = 512 ~ 2,9 s
    AutocorrelationProcessor autocorrelationProcessor(m_tempogramWindowLength, m_tempogramHopSize);

    std::vector<float> complexSdCurve;
    complexSdCurve.reserve(m_detectionResults.size());
    for (size_t i = 0; i < m_detectionResults.size(); i++) {
        complexSdCurve.push_back(m_detectionResults[i].results[3]);
    }
    Spectrogram tempogramACF = autocorrelationProcessor.process(
            &complexSdCurve[0], complexSdCurve.size());

    int firstFullAC = m_tempogramWindowLength / 2 / m_tempogramHopSize;
    for (size_t i = 0; i < tempogramACF[0].size(); i++) {
        qDebug() << i
                 << tempogramACF[27][i]
                 << tempogramACF[28][i]
                 << tempogramACF[29][i]
                 << tempogramACF[30][i]
                 << tempogramACF[31][i]
                 << tempogramACF[32][i]
                 << tempogramACF[33][i];

        //                 << tempogramACF[firstFullAC * 4 + 8][i]
        //                 << tempogramACF[firstFullAC * 4 + 9][i]
        //                 << tempogramACF[firstFullAC * 4 + 10][i]
        //                 << tempogramACF[firstFullAC * 4 + 11][i]
        //                 << tempogramACF[firstFullAC * 4 + 12][i];
    }

    int tempogramMinLag = std::max(static_cast<int>(ceil((60/ static_cast<double>((kNoveltyCurveHop) * m_tempogramMaxBPM))
                *m_iSampleRate)), 0);
    int tempogramMaxLag = std::min(static_cast<int>(floor((60/ static_cast<double>((kNoveltyCurveHop) * m_tempogramMinBPM))
                *m_iSampleRate)), m_tempogramWindowLength-1);
    qDebug() << tempogramMinLag << tempogramMaxLag;

    for (size_t block = 0; block < tempogramACF.size(); block++) {
        QMap<double, double> acf;
        double max = 0;
        int max_lag = 0;
        int lag = 0;
        constexpr int kLookAhead = 8;
        for (; lag < static_cast<int>(tempogramACF[block].size()) - kLookAhead; ++lag) {
            for (int i = lag; i <= lag + kLookAhead; ++i) {
                double compensated = tempogramACF[block][i] * (1 - (1 / ((i / 2.0) + 1)));
                if (max < compensated) {
                    max = compensated;
                    max_lag = i;
                }
            }
            if (max_lag == lag) {
                acf[lag] = max;
            }
        }
        m_tempogramACF.push_back(acf);
        //        qDebug() << "measurelength" << block << acf.keys();
    }

    for (size_t block = 0; block < m_tempogramACF.size(); block++) {
        qDebug() << "measurelength" << block << m_tempogramACF[block].keys();
    }

    std::vector<float> complexSdMinDiff;
    complexSdMinDiff.reserve(m_detectionResults.size());

    // Calculate the change from the minimum of three previous SDs
    // This ensures we find the beat start and not the noisiest place within the beat.
    complexSdMinDiff.push_back(complexSdCurve[0]);
    for (int i = 1; i < static_cast<int>(complexSdCurve.size()); ++i) {
        double result = complexSdCurve[i];
        double min = result;
        for (int j = i - 3; j < i + 2; ++j) {
            if (j >= 0 && j < static_cast<int>(complexSdCurve.size())) {
                double value = complexSdCurve[j];
                if (value < min) {
                    min = value;
                }
            }
        }
        complexSdMinDiff.push_back(result - min * kFloorFactor);
    }

    // Insert find onsets
    int lastNote = -m_tempogramWindowLength / 2;
    for (int block = 0; block < m_tempogramACF.size(); block++) {
        std::vector<int> periods;
        QList<double> keys = m_tempogramACF[block].keys();
        int minPeriod = 10; // ~530 BPM max 1/16 @ 132 BPM
        for (double period : keys) {
            if (period > minPeriod) {
                minPeriod = period;
                break;
            }
        }
        for (double period : keys) {
            periods.push_back(period);
        }

        while (lastNote < (128 * block - m_tempogramWindowLength / 2)) {
            int offset = autocorrelationProcessor.findBeat(
                    &complexSdMinDiff[0], complexSdMinDiff.size(), periods, lastNote + 10);

            int noteToAdd = lastNote + 10 + offset;

            qDebug() << "note found" << noteToAdd;
            m_notes.push_back(noteToAdd);
            lastNote = noteToAdd;
        }
    }

    /*
    // Insert missing onsets
    std::set<int> metronome_sorted;
    int lastNote = 0;
    int lastBlock = -1;
    int minPeriod = 10;
    std::vector<int> periods;
    for (int note : m_notes) {
        int block = note / m_tempogramHopSize;
        if (block != lastBlock) {
            QList<double> keys = m_tempogramACF[block].keys();
            minPeriod = 10;
            for (double period : keys) {
                if (period > minPeriod) {
                    minPeriod = period;
                    break;
                }
            }
            periods.clear();
            for (double period : keys) {
                periods.push_back(period);
            }
            lastBlock = block;
        }
        int delta = note - lastNote;
        // qDebug() << "delta" << delta << minPeriod;
        if (delta > minPeriod * 5 / 2 || delta > 45) {
            while (lastNote < note) {
                int offset = autocorrelationProcessor.findBeat(
                        &snapGrid[0], snapGrid.size(), periods, lastNote + 10);

                int noteToAdd = lastNote + 10 + offset;

                qDebug() << "delta to big" << note << lastNote << minPeriod << offset << noteToAdd;

                if (noteToAdd + 5 > note) {
                    break;
                }

                metronome_sorted.insert(noteToAdd);
                lastNote = noteToAdd;
            }
        }

        metronome_sorted.insert(note);
        lastNote = note;
    }
*/

    /*

    // Find offset
    std::vector<int> offsets;
    for (size_t block = 0; block < m_tempogramACF.size(); block++) {
        QList<double> keys = m_tempogramACF[block].keys();
        if (keys.size() < 1) {
            continue;
        }
        std::vector<int> periods;
        for (double period : keys) {
            periods.push_back(period);
        }
        int offset = autocorrelationProcessor.processOffset(
                &snapGrid[0], snapGrid.size(), block, periods);
        qDebug() << "measurelength" << block << offset << m_tempogramACF[block].keys();

        offsets.push_back(offset);

        //double result = (m_tempogramHopSize * block + offset - m_windowLength / 2) * stepSize();
        //m_resultBeats.push_back(result);
    }

    std::set<int> possible_downbeats_sorted;
    std::set<int> possible_downbeats_auto;
    int lastPos = 0;
    for (size_t block = 0; block < m_tempogramACF.size(); block++) {

        int pos = block * 128 - m_tempogramWindowLength / 2;
        // possible_downbeats_auto.insert(pos);
        ///possible_downbeats_auto.insert(pos);
        possible_downbeats_auto.insert(pos + offsets[block]);

        QList<double> keys = m_tempogramACF[block].keys();
        if (keys.size() < 2) {
            continue;
        }
        int measureK = keys[1];
        for (int k = 2; k < keys.size(); k++) {
            if (keys[k] > measureK) {
                measureK = keys[k];
            }
            if (measureK > 160) {
                // TODO: Use a more sophisticated
                // Algorithm to find sensible measures
                break;
            }
        }
        //while (measureK <= 160) {
        //    measureK *= 2;
        //}
        int beatK = keys[0];
        for (int k = 0; k < keys.size(); k++) {
            if (beatK < (measureK / 15)) {
                // We don't want 1/16 notes
                beatK = keys[k];
            } else {
                break;
            }
        }

        Spectrogram tempogramPhase = autocorrelationProcessor.processPhase(
                &complexSdCurve[0], complexSdCurve.size(), block, minK, maxK, offsets[block]);
        std::vector<int> possible_downbeats;
        for (size_t i = 0; i < tempogramPhase.size(); i++) {
            // auto deb = qDebug();
            float max = tempogramPhase[i][0];
            float j_max = 0;
            size_t j = 0;
            for (; j < tempogramPhase[i].size() && j < maxK / minK; j++) {
                //deb << tempogramPhase[i][j];
                if (max < tempogramPhase[i][j]) {
                    //deb << i << j << tempogramPhase[i][j];
                    max = tempogramPhase[i][j];
                }
            }
            for (; j < tempogramPhase[i].size(); j++) {
                //deb << tempogramPhase[i][j];
                if (max < tempogramPhase[i][j]) {
                    //deb << i << j << tempogramPhase[i][j];
                    max = tempogramPhase[i][j];
                    j_max = (i + 1) * maxK + j * minK + offsets[block] +
                            block * m_tempogramHopSize -
                            m_tempogramWindowLength / 2;
                }
            }
            if (j_max > 0) {
                possible_downbeats.push_back(j_max);
                // Store boundaries for viualsation
                possible_downbeats_sorted.insert(j_max);
            }

            */
    /*
            // auto deb = qDebug();
            float min = tempogramPhase[i][0];
            float j_min = 0;
            size_t j = 0;
            for (; j < tempogramPhase[i].size() && j < maxK / minK; j++) {
                //deb << tempogramPhase[i][j];
                if (min > tempogramPhase[i][j]) {
                    //deb << i << j << tempogramPhase[i][j];
                    min = tempogramPhase[i][j];
                }
            }
            for (; j < tempogramPhase[i].size(); j++) {
                //deb << tempogramPhase[i][j];
                if (min > tempogramPhase[i][j]) {
                    //deb << i << j << tempogramPhase[i][j];
                    min = tempogramPhase[i][j];
                    j_min = 1 * maxK + j * minK + offsets[block] +
                            block * m_tempogramHopSize -
                            m_tempogramWindowLength / 2;
                }
            }
            if (j_min > 0) {
                possible_downbeats.push_back(j_min);
                // Store boundaries for viualsation
                possible_downbeats_sorted.insert(j_min);
            }
            */

    /*
        std::vector<std::pair<int, float>> tempogramPhase = autocorrelationProcessor.processPhase(
                &complexSdCurve[0], complexSdCurve.size(), block, beatK, measureK, offsets[block]);

        float max = 0;
        float min = 0;
        int k_max = 0;
        int k_min = 0;
        int i_max = 0;
        int i_min = 0;
        for (int i = 0; i < (static_cast<int>(tempogramPhase.size()) - (measureK / beatK)); i++) {
            float delta = tempogramPhase[i + (measureK / beatK)].second - tempogramPhase[i].second;
            qDebug() << block << i << delta;
            if (min > delta) {
                min = delta;
                k_min = i * beatK - m_tempogramWindowLength / 2 + block * 128 + offsets[block];
                i_min = i;
            }
            if (max < delta) {
                max = delta;
                k_max = i * beatK + measureK - m_tempogramWindowLength / 2 + block * 128 + offsets[block];
                i_max = i;
            }
        }

        if (min < -0.1 ) { //&& i_min > 0 && i_min < (static_cast<int>(tempogramPhase.size()) - (measureK / beatK) - 1)) {
            qDebug() << block << beatK << measureK << "min" << k_min << i_min;
            possible_downbeats_sorted.insert(k_min);
        }
        if (max > 0.1 ) { // && i_min > 0 && i_max < (static_cast<int>(tempogramPhase.size()) - (measureK / beatK) - 1)) {
            qDebug() << block << beatK << measureK << "max" << k_max << i_max;
            possible_downbeats_sorted.insert(k_max);
        }
        */

    /*
        if (i_max > 0 && i_max < tempogramPhase.back().first) {
            // Store boundaries for visualization
            possible_downbeats_sorted.insert(i_max);
        }
        */

    //qDebug() << "measure:" << block << beatK << measureK
    //         << ((measureK + beatK / 2) * 2 / beatK) * 0.5 << i_max;

    //int delta = pos - lastPos;
    //if (delta > (measureK + 1) / 2) {
    //    possible_downbeats_auto.insert(pos);
    //    lastPos = pos;
    //}
    //}

    std::set<int> measures_sorted;
    for (size_t block = 0; block < m_tempogramACF.size(); block++) {
        QList<double> keys = m_tempogramACF[block].keys();
        for (double measures : keys) {
            measures_sorted.insert(measures);
        }
    }

    {
        QDebug deb = qDebug() << "measure Length";
        for (int measure : measures_sorted) {
            deb << measure;
        }
    }

    std::vector<std::vector<float>> tempogramPhase3D;
    for (int note : m_notes) {
        std::vector<float> tempogramPhase = autocorrelationProcessor.processPhase2(
                &complexSdCurve[0], complexSdCurve.size(), measures_sorted, note);
        QDebug deb2 = qDebug() << note;
        for (float phase : tempogramPhase) {
            deb2 << phase;
        }
        tempogramPhase3D.push_back(tempogramPhase);
    }

    std::vector<int> measures_vector;
    for (auto const& measure : measures_sorted) {
        measures_vector.push_back(measure);
    }

    std::vector<int> possible_downbeats;
    std::vector<float> downbeat_score;
    for (int note = 0; note < static_cast<int>(tempogramPhase3D.size()); ++note) {
        float max = 0.0f;
        int max_tempo = 0;
        for (int tempo = tempogramPhase3D[note].size() - 1; tempo >= 0; --tempo) {
            if (measures_vector[tempo] < 40) {
                // too short
                break;
            }
            float previous = 0.0;
            if ((note - 1) >= 0) {
                previous = tempogramPhase3D[note - 1][tempo];
            }
            float next = 0.0;
            if ((note + 1) < static_cast<int>(tempogramPhase3D.size())) {
                next = tempogramPhase3D[note + 1][tempo];
            }

            if (tempogramPhase3D[note][tempo] > max &&
                    tempogramPhase3D[note][tempo] * 1.01 > previous &&
                    tempogramPhase3D[note][tempo] * 1.01 > next) {
                // Peak or Flat top found
                max = tempogramPhase3D[note][tempo];
                max_tempo = measures_vector[tempo];
            }
        }
        qDebug() << "down" << m_notes[note] << max_tempo << max;
        downbeat_score.push_back(max);
        if (max > 0.8f) {
            possible_downbeats.push_back(m_notes[note] + max_tempo * 2);
        }
    }

    //for (const auto& beat : possible_downbeats) {

    for (const auto& beat : m_notes) {
        // Output possible downbeat positions
        double result = beat * stepSize();
        m_resultBeats.push_back(result);
    }
}

void AnalyzerRhythm::computeMetergram() {
    // the metergram is the inner product of the dft and act tempograms but
    // since they are mapped to different bpms first we need to interpolate
    // one of them...The idea is that the dft tempogram contains the meter pulses
    // with it's harmonics and others sporious peaks, while the acf has
    // the meter pulses, the subharmonics and also spororious peak, by multiplying
    // them we should enchance the meter pulses only..

    for (size_t i = 0; i < m_tempogramACF.size() && i < m_tempogramDFT.size(); i += 1) {
        QMap<double, double> metergramBlock;
        for (auto act = m_tempogramACF[i].begin(); act != m_tempogramACF[i].end(); act +=1) {
            auto nextDFT = m_tempogramDFT[i].lowerBound(act.key());
            auto previousDFT = nextDFT - 1;
            double x0 = previousDFT.key();
            double y0 = previousDFT.value();
            double x1 = nextDFT.key();
            double y1 = nextDFT.value();
            double xp = act.key();
            double yp = y0 + ((y1-y0)/(x1-x0)) * (xp - x0);
            // we also reverse the key here so we can get the highest bpms easily..
            metergramBlock[act.key()] = act.value() * yp;
        }
        m_metergram.push_back(metergramBlock);
    }
}

void AnalyzerRhythm::storeResults(TrackPointer pTrack) {
    m_onsetsProcessor.finalize();
    m_downbeatsProcessor.finalize();
    m_noveltyCurveProcessor.finalize();

    // m_notes = computeSnapGrid();

    /*
    // Visualizes Snap Grid as Beats
    for (const auto& beat : m_notes) {
        double result = beat * stepSize();
        m_resultBeats.push_back(result);
    }
    */

    setTempogramParameters();
    //computeNoveltyCurve();

    for (size_t i = 0; i < m_detectionResults.size(); i++) {
        qDebug() << i
                 << m_detectionResults[i].results[3]
                 << m_detectionResults[i].results[1]
                 << m_detectionResults[i].results[2] * 400;
        // << m_noveltyCurve[i] * 400;
    }

    computeTempogramByACF();
    //computeTempogramByDFT();
    //computeMetergram();
    //computeBeats();
    //computeMeter();

    // TODO(Cristiano&Harshit) THIS IS WHERE A BEAT VECTOR IS CREATED
    auto pBeatMap = new mixxx::BeatMap(*pTrack, m_iSampleRate, m_resultBeats);
    auto pBeats = mixxx::BeatsPointer(pBeatMap, &BeatFactory::deleteBeats);
    pTrack->setBeats(pBeats);
}
