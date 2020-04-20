#pragma once
#include <keyfinder/keyfinder.h>

#include <QObject>
#include <vector>

#include "analyzer/plugins/analyzerplugin.h"
#include "util/memory.h"
#include "util/types.h"

namespace mixxx {

class AnalyzerKeyFinder : public AnalyzerKeyPlugin {
  public:
    static AnalyzerPluginInfo pluginInfo() {
        return AnalyzerPluginInfo(
                "keyfinder:2",
                "Ibrahim Sha'ath",
                "KeyFinder",
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
    mixxx::track::io::key::ChromaticKey chromaticKeyFromKeyFinderKeyT(KeyFinder::key_t);

    std::unique_ptr<KeyFinder::KeyFinder> m_pKeyFinder;
    std::unique_ptr<KeyFinder::Workspace> m_pWorkspace;
    std::unique_ptr<KeyFinder::AudioData> m_pAudioData;

    size_t m_currentFrame;
    mixxx::track::io::key::ChromaticKey m_previousKey;
    KeyChangeList m_resultKeys;
};

} // namespace mixxx
