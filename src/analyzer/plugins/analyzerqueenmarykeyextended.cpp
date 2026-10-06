#include "analyzer/plugins/analyzerqueenmarykeyextended.h"

#include <dsp/keydetection/GetKeyMode.h>

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <algorithm>
#include <cmath>
#include <map>
#include <numeric>
#include <utility>
#include <vector>

#include "analyzer/constants.h"
#include "track/keyutils.h"
#include "util/assert.h"

using mixxx::track::io::key::ChromaticKey;

namespace mixxx {
namespace {
constexpr int kTuningFrequencyHertz = 440;
constexpr double kMinSegmentDurationSeconds = 2.0;
} // namespace

AnalyzerPluginInfo AnalyzerQueenMaryKeyExtended::pluginInfo() {
    return AnalyzerPluginInfo(
            "qm-keydetector-extended:0",
            QObject::tr("Queen Mary University London (Extended)"),
            QObject::tr("Queen Mary Key Detector with Key Change Detection & segmentation"),
            false);
}

AnalyzerQueenMaryKeyExtended::AnalyzerQueenMaryKeyExtended()
        : m_currentFrame(0),
          m_sampleRate(44100.0),
          m_trackDuration(0.0) {
}

AnalyzerQueenMaryKeyExtended::~AnalyzerQueenMaryKeyExtended() = default;

AnalyzerPluginInfo AnalyzerQueenMaryKeyExtended::info() const {
    return pluginInfo();
}

bool AnalyzerQueenMaryKeyExtended::initialize(mixxx::audio::SampleRate sampleRate) {
    m_sampleRate = sampleRate.toDouble();
    m_resultKeys.clear();
    m_keyResults.clear();
    m_keySegments.clear();
    m_currentFrame = 0;

    GetKeyMode::Config config(sampleRate, kTuningFrequencyHertz);
    m_pKeyMode = std::make_unique<GetKeyMode>(config);
    size_t windowSize = m_pKeyMode->getBlockSize();
    size_t stepSize = m_pKeyMode->getHopSize();

    qDebug() << "[QueenMaryKeyExtended] Initialized with sample rate:" << sampleRate
             << "Window size:" << windowSize << "Step size:" << stepSize;

    return m_helper.initialize(
            windowSize, stepSize, [this](double* pWindow, size_t) {
                int iKey = m_pKeyMode->process(pWindow);

                // Validate against actual QM-DSP output range (0-24)
                if (iKey < 0 || iKey > 24) {
                    qWarning() << "[QueenMaryKeyExtended] Invalid iKey from detector:" << iKey;
                    return true; // Skip this frame
                }

                // Skip frames where no key was detected
                if (iKey == 0) {
                    // No key detected - optionally log this less verbosely
                    // qDebug() << "[QueenMaryKeyExtended] No key detected at
                    // frame:" << m_currentFrame;
                    return true;
                }

                // Convert from QM-DSP key index (1-24) to ChromaticKey (0-23)
                // The formula is consistent for both major and minor:
                // ChromaticKey = (keyIndex - 1) % 12 for the pitch class,
                // and the major/minor distinction is preserved in ChromaticKey
                int chromaticKeyIndex = iKey - 1; // Now in range 0-23

                double* keyStrengths = m_pKeyMode->getKeyStrengths();

                // keyStrengths array is 0-23 (24 elements) - the detector converts
                // internally for convenience. Use the same index for consistency.
                ChromaticKey key = static_cast<ChromaticKey>(chromaticKeyIndex);
                double timeSeconds = static_cast<double>(m_currentFrame) / m_sampleRate;

                // Calculate confidence using the same index
                double confidence = calculateConfidence(keyStrengths, chromaticKeyIndex);

                // Store result
                KeyDetectionResult result;
                result.key = key;
                result.timeSeconds = timeSeconds;
                result.confidence = confidence;
                m_keyResults.push_back(result);
                m_resultKeys.push_back(qMakePair(key, static_cast<double>(m_currentFrame)));

                return true;
            });
}

bool AnalyzerQueenMaryKeyExtended::processSamples(const CSAMPLE* pIn, SINT iLen) {
    DEBUG_ASSERT(iLen % kAnalysisChannels == 0);
    if (!m_pKeyMode) {
        return false;
    }
    const size_t numInputFrames = iLen / kAnalysisChannels;
    m_currentFrame += numInputFrames;
    return m_helper.processStereoSamples(pIn, iLen);
}

bool AnalyzerQueenMaryKeyExtended::finalize() {
    m_helper.finalize();
    m_trackDuration = static_cast<double>(m_currentFrame) / m_sampleRate;

    // added to avoid crash when computing segments in small samples exported from hotcues
    if (m_trackDuration < kMinSegmentDurationSeconds) {
        qDebug() << "[QueenMaryKeyExtended] Track too short for key segments ("
                 << m_trackDuration << "s) - skipping";
        return true;
    }

    // smoothing
    smoothKeyResults();

    buildKeySegments();

    qDebug() << "[QueenMaryKeyExtended] Finalized. Duration:" << m_trackDuration << "s"
             << "Key results:" << m_keyResults.size()
             << "Segments:" << m_keySegments.size();

    // Calculate key distribution
    std::map<QString, double> keyDuration;
    for (const auto& seg : std::as_const(m_keySegments)) {
        keyDuration[seg.keyText] += seg.duration;
    }

    if (!keyDuration.empty()) {
        qDebug() << "[QueenMaryKeyExtended] Key distribution:";
        for (const auto& pair : std::as_const(keyDuration)) {
            if (pair.second > 0.1) {
                qDebug() << "  " << pair.first << ":" << pair.second << "s";
            }
        }
    }

    return true;
}

KeyChangeList AnalyzerQueenMaryKeyExtended::getKeyChanges() const {
    return m_resultKeys;
}

double AnalyzerQueenMaryKeyExtended::calculateConfidence(
        double* keyStrengths, int detectedKeyIndex) const {
    if (!keyStrengths) {
        return 50.0;
    }

    double best = keyStrengths[detectedKeyIndex];
    double sum = 0.0;
    double secondBest = 0.0;

    for (int i = 0; i < 24; ++i) {
        sum += keyStrengths[i];
        if (i != detectedKeyIndex && keyStrengths[i] > secondBest) {
            secondBest = keyStrengths[i];
        }
    }

    double avg = sum / 24.0;
    double peakToAvg = best / (avg + 0.0001);
    double peakToSecond = best / (secondBest + 0.0001);

    // Combined metric: higher confidence when both ratios are good
    double confidence;
    if (peakToSecond > 1.5 && peakToAvg > 2.0) {
        confidence = 80.0 + std::min(20.0, (peakToSecond - 1.5) * 40.0);
    } else if (peakToSecond > 1.2) {
        confidence = 60.0 + (peakToSecond - 1.2) * 66.7;
    } else if (peakToSecond > 1.05) {
        confidence = 50.0 + (peakToSecond - 1.05) * 200.0;
    } else {
        confidence = 50.0;
    }

    return qBound(0.0, confidence, 100.0);
}

void AnalyzerQueenMaryKeyExtended::smoothKeyResults() {
    if (m_keyResults.size() < 3) {
        return;
    }

    const int SMOOTHING_WINDOW_SIZE = 5;
    std::vector<KeyDetectionResult> smoothed;
    smoothed.reserve(m_keyResults.size());

    for (size_t i = 0; i < m_keyResults.size(); ++i) {
        // Collect keys in window
        std::map<ChromaticKey, int> frequency;
        int start = std::max(0, static_cast<int>(i) - SMOOTHING_WINDOW_SIZE / 2);
        int end = std::min(static_cast<int>(m_keyResults.size()) - 1,
                static_cast<int>(i) + SMOOTHING_WINDOW_SIZE / 2);

        for (int j = start; j <= end; ++j) {
            frequency[m_keyResults[j].key]++;
        }

        // Find most frequent key in window
        ChromaticKey mostFrequentKey = m_keyResults[i].key;
        int maxCount = 0;
        for (const auto& pair : std::as_const(frequency)) {
            if (pair.second > maxCount) {
                maxCount = pair.second;
                mostFrequentKey = pair.first;
            }
        }

        // smooth result
        KeyDetectionResult smoothedResult = m_keyResults[i];
        smoothedResult.key = mostFrequentKey;
        smoothed.push_back(smoothedResult);
    }

    m_keyResults = smoothed;
    qDebug() << "[QueenMaryKeyExtended] Applied smoothing with window size:"
             << SMOOTHING_WINDOW_SIZE;
}

void AnalyzerQueenMaryKeyExtended::buildKeySegments() {
    if (m_keyResults.empty() || m_trackDuration <= 0.0) {
        qDebug() << "[QueenMaryKeyExtended] No key results to build segments "
                    "or audiofile is too small";
        return;
    }

    m_keySegments.clear();

    // Segments shorter than this are noise, not a key change.
    constexpr double kMinSegmentSeconds = 0.5;

    auto isUsableKey = [](ChromaticKey key) {
        const int k = static_cast<int>(key);
        return k >= 0 && k <= 23;
    };

    double lastTime = 0.0;
    ChromaticKey lastKey = m_keyResults[0].key;
    double confidenceSum = m_keyResults[0].confidence;
    int confidenceCount = 1;

    for (size_t i = 1; i < m_keyResults.size(); ++i) {
        double currentTime = m_keyResults[i].timeSeconds;
        ChromaticKey currentKey = m_keyResults[i].key;
        double currentConfidence = m_keyResults[i].confidence;

        if (currentKey != lastKey) {
            // End current segment
            AnalysisKeySegment seg;
            seg.startTime = lastTime;
            seg.endTime = currentTime;
            seg.duration = currentTime - lastTime;
            seg.keyId = lastKey;
            seg.keyText = keyToString(static_cast<int>(lastKey));
            seg.type = "STABLE";
            seg.confidence = confidenceSum / confidenceCount;

            // Only add segments with minimum duration and a valid key
            if (seg.duration >= kMinSegmentSeconds &&
                    isUsableKey(seg.keyId) &&
                    !seg.keyText.isEmpty()) {
                m_keySegments.append(seg);

                qDebug() << "[QueenMaryKeyExtended] Segment:" << seg.startTime << "-" << seg.endTime
                         << "KeyId:" << static_cast<int>(seg.keyId)
                         << "KeyText:" << seg.keyText
                         << "Confidence:" << seg.confidence
                         << "Duration:" << seg.duration;
            }

            // Start new segment
            lastTime = currentTime;
            lastKey = currentKey;
            confidenceSum = currentConfidence;
            confidenceCount = 1;
        } else {
            confidenceSum += currentConfidence;
            confidenceCount++;
        }
    }

    // final segment
    if (lastTime < m_trackDuration) {
        AnalysisKeySegment seg;
        seg.startTime = lastTime;
        seg.endTime = m_trackDuration;
        seg.duration = m_trackDuration - lastTime;
        seg.keyId = lastKey;
        seg.keyText = keyToString(static_cast<int>(lastKey));
        seg.type = "STABLE";
        seg.confidence = confidenceSum / confidenceCount;

        if (seg.duration >= kMinSegmentSeconds &&
                isUsableKey(seg.keyId) &&
                !seg.keyText.isEmpty()) {
            m_keySegments.append(seg);
            qDebug() << "[QueenMaryKeyExtended] Final segment:" << seg.startTime
                     << "-" << seg.endTime
                     << "KeyId:" << static_cast<int>(seg.keyId)
                     << "KeyText:" << seg.keyText
                     << "Confidence:" << seg.confidence
                     << "Duration:" << seg.duration;
        }
    }
}

QString AnalyzerQueenMaryKeyExtended::keyToString(int key) const {
    if (key < 0 || key >= 24) {
        qWarning() << "[QueenMaryKeyExtended] Invalid key index:" << key;
        return QString("INVALID_%1").arg(key);
    }

    QString result = KeyUtils::keyToString(static_cast<ChromaticKey>(key));

    // Replace Unicode flat (U+266D) with ASCII 'b' (U+0062)
    result.replace(QChar(0x266D), 'b');
    // Replace Unicode sharp (U+266F) with ASCII '#' (U+0023)
    result.replace(QChar(0x266F), '#');
    // Replace Unicode natural (U+266E) with empty string
    result.replace(QChar(0x266E), "");

    return result;
}

QVector<AnalysisKeySegment> AnalyzerQueenMaryKeyExtended::getKeySegments() const {
    return m_keySegments;
}

QJsonArray AnalyzerQueenMaryKeyExtended::getKeySegmentsJson() const {
    QJsonArray segmentsArray;
    int segmentId = 1;
    for (const auto& seg : std::as_const(m_keySegments)) {
        if (seg.duration < 0.001) {
            continue;
        }
        QJsonObject segmentObj;
        segmentObj["id"] = segmentId++;
        segmentObj["type"] = seg.type;
        segmentObj["position"] = seg.startTime;
        segmentObj["duration"] = seg.duration;
        segmentObj["keyId"] = static_cast<int>(seg.keyId);
        segmentObj["keyText"] = seg.keyText;
        segmentObj["confidence"] = seg.confidence;
        segmentObj["range_start"] = seg.startTime;
        segmentObj["range_end"] = seg.endTime;
        segmentsArray.append(segmentObj);
    }
    return segmentsArray;
}

} // namespace mixxx
