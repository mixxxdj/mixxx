#pragma once
#include <keyfinder/keyfinder.h>

#include <QObject>

#include "analyzer/plugins/analyzerplugin.h"
#include "util/types.h"

namespace mixxx {

class AnalyzerKeyFinder : public AnalyzerKeyPlugin {
  public:
    static AnalyzerPluginInfo pluginInfo();

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
    KeyChangeList m_resultKeys;
};

} // namespace mixxx
