#pragma once

#include "analyzer/analyzer.h"
#include "preferences/usersettings.h"
#include "util/span.h"

class AnalyzerTrack;
class Track;

namespace mixxx {
namespace audio {

class FramePos;
class SampleRate;

} // namespace audio
} // namespace mixxx

class AnalyzerSilence : public Analyzer {
  public:
    explicit AnalyzerSilence(UserSettingsPointer pConfig);
    ~AnalyzerSilence() override = default;

    bool initialize(const AnalyzerTrack& track,
            mixxx::audio::SampleRate sampleRate,
            SINT frameLength) override;
    bool processSamples(const CSAMPLE* pIn, SINT count) override;
    void storeResults(TrackPointer pTrack) override;
    void cleanup() override;

    static void setupMainAndIntroCue(Track* pTrack,
            mixxx::audio::FramePos firstSoundPosition,
            UserSettings* pConfig);
    static void setupOutroCue(Track* pTrack, mixxx::audio::FramePos lastSoundPosition);

    /// returns the index of the first sample in the buffer that is above -60 dB
    /// or samples.size() if no sample is found
    static SINT findFirstSoundInChunk(std::span<const CSAMPLE> samples);

    /// returns the index of the last sample in the buffer that is above -60 dB
    /// or samples.size() if no sample is found
    static SINT findLastSoundInChunk(std::span<const CSAMPLE> samples);

    /// Returns true if the first sound if found at the given frame and logs a
    /// warning message if not. This can be uses to detect changes since the
    /// last analysis run and is an indicator for file edits or decoder
    /// changes/issues
    static bool verifyFirstSound(std::span<const CSAMPLE> samples,
            mixxx::audio::FramePos firstSoundFrame);

  private:
    UserSettingsPointer m_pConfig;
    SINT m_framesProcessed;
    SINT m_signalStart;
    SINT m_signalEnd;
};
