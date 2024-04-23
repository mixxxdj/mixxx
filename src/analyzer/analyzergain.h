/*
 * analyzergain.h
 *
 *  Created on: 13/ott/2010
 *      Author: Vittorio Colao
 *       */

#pragma once

#include <vector>

#include "analyzer/analyzer.h"
#include "preferences/replaygainsettings.h"

class ReplayGain;

class AnalyzerGain : public Analyzer {
  public:
    AnalyzerGain(UserSettingsPointer pConfig);
    ~AnalyzerGain() override;

    static bool isEnabled(const ReplayGainSettings& rgSettings) {
        return rgSettings.isAnalyzerEnabled(1);
    }

    bool initialize(const AnalyzerTrack& track,
            mixxx::audio::SampleRate sampleRate,
            SINT frameLength) override;
    bool processSamples(const CSAMPLE* pIn, SINT count) override;
    void storeResults(TrackPointer tio) override;
    void cleanup() override;

  private:
    ReplayGainSettings m_rgSettings;
    std::vector<CSAMPLE> m_pLeftTempBuffer;
    std::vector<CSAMPLE> m_pRightTempBuffer;
    ReplayGain* m_pReplayGain;
};
