#pragma once

#include "analyzer/analyzer.h"
#include "audio/frame.h"
#include "preferences/usersettings.h"

class CuePointer;

class AnalyzerSilence : public Analyzer {
  public:
    explicit AnalyzerSilence(UserSettingsPointer pConfig);
    ~AnalyzerSilence() override = default;

    bool initialize(TrackPointer pTrack,
            mixxx::audio::SampleRate sampleRate,
            int totalSamples) override;
    bool processSamples(const CSAMPLE* pIn, SINT iLen) override;
    void storeResults(TrackPointer pTrack) override;
    void cleanup() override;

    /// returns the index of the first sample in the buffer that is above -60 dB
    /// or iLen if no sample is found
    static SINT findFirstSoundInChunk(const CSAMPLE* pIn, SINT iLen);

    /// returns the index of the last sample in the buffer that is above -60 dB
    /// or -1 if no sample is found. signalStart can be set to a known sample above
    /// -60 dB or -1 to start with the following index.
    static SINT findLastSoundInChunk(const CSAMPLE* pIn, SINT iLen);

    /// Returns true if the first sound if found at the given frame and logs a warning message if not.
    /// This can be uses to detect changes since the last analysis run and is an indicator for
    /// file edits or decoder changes/issues
    static bool verifyFirstSound(const CSAMPLE* pIn,
            SINT iLen,
            mixxx::audio::FramePos firstSoundFrame);

  private:
    UserSettingsPointer m_pConfig;
    int m_iFramesProcessed;
    int m_iSignalStart;
    int m_iSignalEnd;
};
