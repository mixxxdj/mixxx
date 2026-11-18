#pragma once

#include <QString>
#include <cstddef>

#include "audio/types.h"
#include "control/controlobject.h"
#include "engine/filters/enginefilterlinkwitzriley4.h"
#include "util/samplebuffer.h"
#include "util/types.h"

namespace mixxx {

/// [AI-generated] Measures how much low, mid and high frequency content a
/// channel's signal contains and publishes the three levels as the read-only
/// controls band_meter_low, band_meter_mid and band_meter_high of the
/// channel's group. A level is the mean absolute sample value of the band,
/// in the range 0..1. It is fed by EngineMixer with the post-fader signal
/// of a deck, so the levels reflect the EQ, the volume fader, the crossfader
/// and the post-fader effects. All methods except the constructor are called
/// from the engine thread and do not allocate. [/AI-generated]
class EngineBandMeter {
  public:
    explicit EngineBandMeter(const QString& group);
    virtual ~EngineBandMeter() = default;

    /// [AI-generated] Measures bufferSize interleaved stereo samples of pIn.
    /// The published levels are updated once enough samples have been
    /// accumulated. [/AI-generated]
    void process(const CSAMPLE* pIn, std::size_t bufferSize, audio::SampleRate sampleRate);

    /// [AI-generated] Sets all levels to zero and forgets the filter history.
    /// This is cheap when the meter is already reset, so it can be called on
    /// every engine callback in which the channel is not metered.
    /// [/AI-generated]
    void reset();

  private:
    struct Band {
        explicit Band(const ConfigKey& key);

        ControlObject m_level;
        CSAMPLE m_sumAbs;
    };

    void accumulate(Band* pBand, std::size_t bufferSize);
    void publish(Band* pBand);

    Band m_low;
    Band m_mid;
    Band m_high;

    EngineFilterLinkwitzRiley4Low m_lowFilter;
    EngineFilterLinkwitzRiley4High m_midHighPassFilter;
    EngineFilterLinkwitzRiley4Low m_midLowPassFilter;
    EngineFilterLinkwitzRiley4High m_highFilter;
    SampleBuffer m_filtered;

    audio::SampleRate m_sampleRate;
    std::size_t m_samplesAccumulated;
    bool m_isReset;
};

} // namespace mixxx
