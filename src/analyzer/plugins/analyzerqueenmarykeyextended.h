#pragma once

#include <QJsonArray>
#include <QJsonObject>
#include <memory>
#include <vector>

#include "analyzer/plugins/analyzerplugin.h"
#include "analyzer/plugins/buffering_utils.h"
#include "track/keys.h"
#include "track/keyutils.h"

using mixxx::track::io::key::ChromaticKey;

class GetKeyMode;

namespace mixxx {

struct KeyDetectionResult {
    ChromaticKey key;
    double timeSeconds;
    double confidence;
};

struct AnalysisKeySegment {
    double startTime;
    double endTime;
    double duration;
    ChromaticKey keyId;
    QString keyText;
    QString type;
    double confidence;

    AnalysisKeySegment()
            : startTime(0),
              endTime(0),
              duration(0),
              keyId(ChromaticKey::INVALID),
              confidence(0) {
    }
};

class AnalyzerQueenMaryKeyExtended : public AnalyzerKeyPlugin {
  public:
    static AnalyzerPluginInfo pluginInfo();

    AnalyzerQueenMaryKeyExtended();
    ~AnalyzerQueenMaryKeyExtended() override;

    AnalyzerPluginInfo info() const override;
    bool initialize(mixxx::audio::SampleRate sampleRate) override;
    bool processSamples(const CSAMPLE* pIn, SINT iLen) override;
    bool finalize() override;
    KeyChangeList getKeyChanges() const override;

    QVector<AnalysisKeySegment> getKeySegments() const;
    QJsonArray getKeySegmentsJson() const;

  private:
    void buildKeySegments();
    void smoothKeyResults();
    QString keyToString(int key) const;
    double calculateConfidence(const double* keyStrengths, int detectedKeyIndex) const;

    std::unique_ptr<GetKeyMode> m_pKeyMode;
    DownmixAndOverlapHelper m_helper;
    size_t m_currentFrame;
    double m_sampleRate;
    KeyChangeList m_resultKeys;
    std::vector<KeyDetectionResult> m_keyResults;
    double m_trackDuration;
    QVector<AnalysisKeySegment> m_keySegments;
};

} // namespace mixxx
