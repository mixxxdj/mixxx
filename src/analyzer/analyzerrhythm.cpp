#include "analyzer/analyzerrhythm.h"

#include <QHash>
#include <QString>
#include <QVector>
#include <QtDebug>
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
constexpr int kTempogramLog2WindowLength = 12;
constexpr int kTempogramLog2HopSize = 8;
constexpr int kTempogramLog2FftLength = 12;
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

    m_tempogramMinBPM = 11;
    m_tempogramMaxBPM = 68;
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
    m_downbeat.reset();
    delete m_window;
    delete[] m_fftImagOut;
    delete[] m_fftRealOut;
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

    /*
    for (size_t i = 0; i < beats.size(); ++i) {
        double result = (beats.at(i) * stepSize()) - (stepSize() / mixxx::kEngineChannelCount);
        m_resultBeats.push_back(result);
    }

    */

    int i = 1;
    for (const auto& x : m_noveltyCurve) {
        if (x) {
            double result = i * kNoveltyCurveHop;
            m_resultBeats.push_back(result);
        }
        i++;
    }

    return beats;
}

std::vector<double> AnalyzerRhythm::computeSnapGrid() {
    int size = m_detectionResults.size();

    int dfType = 3; // ComplexSD

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

    std::vector<double> allBeats;
    allBeats.reserve(size / 10);

    // This is a dynamic threshold that defines which SD is considered as a beat.
    double threshold = 0;

    // Find peak beats within a window of 9 SDs (100 ms)
    // This limits the detection result to 600 BPM
    for (int i = 0; i < size; ++i) {
        double beat = 0;
        double result = complexSdMinDiff[i];
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
    for (int i = 0; i < m_resultBeats.size(); i += 1) {
        double beatFramePos = m_resultBeats[i];
        double blockEnd =
                m_tempogramHopSize * (blockCounter + 1) * kNoveltyCurveHop;
        tempoSum += tempoList[i];
        tempoCounter += 1.0;
        if (blockEnd < beatFramePos) {
            // check if pulse is integer ratio of tempo
            double localTempo = tempoSum / tempoCounter;
            qDebug() << "at" << blockEnd << "local tempo is" << localTempo;
            QMap<int, double> pulsesLenghtsAndWeights;
            auto allPulses = m_metergram[blockCounter].keys();
            for (auto pulse : allPulses) {
                double ratio = localTempo / pulse;
                double beatLenghtOfPulse;
                double ratioDecimals = std::modf(ratio, &beatLenghtOfPulse);
                if (ratioDecimals < kCloseToIntTolerance) {
                    pulsesLenghtsAndWeights[static_cast<int>(
                            beatLenghtOfPulse)] +=
                            m_metergram[blockCounter][pulse];
                }
            }
            std::vector<int> metricalHierchy;
            double highestWeight = 0.0;
            //combine integer pulses into metrical hieranchies
            auto metricalPulses = pulsesLenghtsAndWeights.keys();
            for (int j = 0; j < metricalPulses.size(); j += 1) {
                std::vector<int> metricalHierchyCanditade;
                double metricalHierchyCanditadeWeight = 0.0;
                metricalHierchyCanditade.push_back(metricalPulses[j]);
                metricalHierchyCanditadeWeight += pulsesLenghtsAndWeights[metricalPulses[j]];
                for (int k = 0; k < metricalPulses.size(); k += 1) {
                    if (j == k) {
                        continue;
                    }
                    int size = metricalHierchyCanditade.size();
                    bool intergerDivisorOfAllUnits = false;
                    for (int ij = 0; ij < size; ij += 1) {
                        if (metricalPulses[k] % metricalHierchyCanditade[ij] != 0) {
                            intergerDivisorOfAllUnits = false;
                            break;
                        }
                        intergerDivisorOfAllUnits = true;
                    }
                    if (intergerDivisorOfAllUnits) {
                        metricalHierchyCanditade.push_back(metricalPulses[k]);
                        metricalHierchyCanditadeWeight += pulsesLenghtsAndWeights[metricalPulses[k]];
                    }
                }
                if (metricalHierchyCanditadeWeight > highestWeight) {
                    metricalHierchy = metricalHierchyCanditade;
                    highestWeight = metricalHierchyCanditadeWeight;
                }
            }
            qDebug() << metricalHierchy;
            // set for next iteration
            tempoSum = 0;
            tempoCounter = 0.0;
            blockCounter += 1;
        }
    }
    /*
    blockCounter = 0;
    for (auto block : m_metergram) {
        auto strongestTempo = block.end();
        int i = 1;
        int j = 1;
        auto lastStrongTempo = strongestTempo - j;
        std::vector<double> strongestPulses;
        strongestPulses.push_back(lastStrongTempo.value());
        while (i <= 20) {
            for (int k = 0; k < strongestPulses.size(); k += 1) {
                // if the tempo diff is less than 1.5 assume it's the same pulse..
                if (fabs((strongestTempo - j).value() - strongestPulses[k]) < 1.5) {
                    j += 1;
                    k = -1;
                }
            }
            strongestPulses.push_back((strongestTempo - j).value());
            i += 1;
        }
        double minute = ((m_tempogramHopSize * blockCounter * kNoveltyCurveHop)/static_cast<double>(m_iSampleRate))/60.0;
        double fractionalMinutes;
        double intMinutes;
        fractionalMinutes = std::modf(minute, &intMinutes);
        double seconds = (fractionalMinutes*60.0)/100;
        minute = intMinutes + seconds;
        qDebug() << "At time" << minute << "blockcounter" << blockCounter;
        int pulseCounter = 1;
        for (auto pulse : strongestPulses) {
            qDebug() << pulseCounter++ << "th strongest pulse is" << pulse;
        } 
        blockCounter += 1;
    }
    */
}

int AnalyzerRhythm::computeNoveltyCurve() {
    NoveltyCurveProcessor nc(static_cast<float>(m_iSampleRate), kNoveltyCurveWindow, kNoveltyCurveCompressionConstant);
    m_noveltyCurve = nc.spectrogramToNoveltyCurve(m_spectrogram);

    int i = 0;
    for (const auto& x : m_noveltyCurve) {
        qDebug() << i++ << x << "nnnnnnnnn";
    }
    return m_spectrogram.size();
}

void AnalyzerRhythm::computeTempogramByDFT() {
    auto hannWindow = std::vector<float>(m_tempogramWindowLength, 0.0);
    WindowFunction::hanning(&hannWindow[0], m_tempogramWindowLength);
    SpectrogramProcessor spectrogramProcessor(m_tempogramWindowLength,
            m_tempogramFftLength, m_tempogramHopSize);
    Spectrogram tempogramDFT = spectrogramProcessor.process(
            &m_noveltyCurve[0], m_noveltyCurve.size(), &hannWindow[0]);
    // convert y axis to bpm
    int tempogramMinBin = (std::max(static_cast<int>(floor(((m_tempogramMinBPM/60.0)
            /m_tempogramInputSampleRate)*m_tempogramFftLength)), 0));
    int tempogramMaxBin = (std::min(static_cast<int>(ceil(((m_tempogramMaxBPM/60.0)
            /m_tempogramInputSampleRate)*m_tempogramFftLength)), m_tempogramFftLength/2));
    int binCount = tempogramMaxBin - tempogramMinBin + 1;
    double highest;
    double bestBpm;
    int bin;
    for (int block = 0; block < tempogramDFT.size(); block++) {
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
    AutocorrelationProcessor autocorrelationProcessor(m_tempogramWindowLength, m_tempogramHopSize);
    Spectrogram tempogramACF = autocorrelationProcessor.process(&m_noveltyCurve[0], m_noveltyCurve.size());
    // Convert y axis to bpm
    int tempogramMinLag = std::max(static_cast<int>(ceil((60/ static_cast<double>((kNoveltyCurveHop) * m_tempogramMaxBPM))
                *m_iSampleRate)), 0);
    int tempogramMaxLag = std::min(static_cast<int>(floor((60/ static_cast<double>((kNoveltyCurveHop) * m_tempogramMinBPM))
                *m_iSampleRate)), m_tempogramWindowLength-1);
    qDebug() << tempogramMinLag << tempogramMaxLag;
    double highest;
    double bestBpm;
    int bin;
    for (int block = 0; block < tempogramACF.size(); block++) {
        //qDebug() << "block" << block;
        //qDebug() << "ACF tempogram";
        highest = .0;
        bestBpm = .0;
        bin = 0;
        QMap<double, double> acf;
        for (int lag = tempogramMaxLag; lag >= tempogramMinLag; lag--) {
            double bpm = 60/static_cast<double>(kNoveltyCurveHop * (lag/static_cast<double>(m_iSampleRate)));
            //qDebug() << "bin, bpm and value"<< bin++ << bpm << tempogramACF[block][lag];
            acf[bpm] = tempogramACF[block][lag];
            if (tempogramACF[block][lag] > highest) {
                highest = tempogramACF[block][lag];
                bestBpm = bpm;
            }
        }
        m_tempogramACF.push_back(acf);
        //qDebug() << "best bpm at block" << bestBpm << block;
    }
}

void AnalyzerRhythm::computeMetergram() {
    // the metergram is the inner product of the dft and act tempograms but
    // since they are mapped to different bpms first we need to interpolate
    // one of them...The idea is that the dft tempogram contains the meter pulses
    // with it's harmonics and others sporious peaks, while the acf has
    // the meter pulses, the subharmonics and also spororious peak, by multiplying
    // them we should enchance the meter pulses only..
    
    for (int i = 0; i < m_tempogramDFT.size(); i+=1) {
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

    auto notes = computeSnapGrid();
    setTempogramParameters();
    computeNoveltyCurve();    
    //for (auto nc : m_noveltyCurve) {qDebug() << nc;}
    computeTempogramByACF();
    computeTempogramByDFT();
    computeMetergram();
    computeBeats();
    computeMeter();
    

    // TODO(Cristiano&Harshit) THIS IS WHERE A BEAT VECTOR IS CREATED
    auto pBeatMap = new mixxx::BeatMap(*pTrack, m_iSampleRate, m_resultBeats);
    auto pBeats = mixxx::BeatsPointer(pBeatMap, &BeatFactory::deleteBeats);
    pTrack->setBeats(pBeats);
}
