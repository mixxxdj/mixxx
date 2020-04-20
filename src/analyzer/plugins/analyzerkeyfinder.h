#pragma once
#include <keyfinder/keyfinder.h>

#include <QObject>

#include "analyzer/plugins/analyzerplugin.h"
#include "util/types.h"

namespace mixxx {

class AnalyzerKeyFinder : public AnalyzerKeyPlugin {
  public:
    static AnalyzerPluginInfo pluginInfo() {
        return AnalyzerPluginInfo(
                // 2 is for the library version
                QStringLiteral("keyfinder:2"),
                QStringLiteral("Ibrahim Sha'ath"),
                QStringLiteral("KeyFinder"),
                false);
    }

    AnalyzerKeyFinder();
    ~AnalyzerKeyFinder() override = default;

    AnalyzerPluginInfo info() const override {
        return pluginInfo();
    }

    bool initialize(int samplerate) override;
    bool processSamples(const CSAMPLE* pIn, const int iLen) override;
    bool finalize() override;

    KeyChangeList getKeyChanges() const override {
        return m_resultKeys;
    }

  private:
    KeyFinder::KeyFinder m_keyFinder;
    KeyFinder::Workspace m_workspace;
    KeyFinder::AudioData m_audioData;

    SINT m_currentFrame;
    mixxx::track::io::key::ChromaticKey m_previousKey;
    KeyChangeList m_resultKeys;
};

} // namespace mixxx
