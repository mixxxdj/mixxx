#pragma once

#include <QAtomicInt>
#include <QString>
#include <cstddef>
#include <memory>
#include <vector>

#include "audio/types.h"
#include "control/controlobject.h"
#include "util/class.h"
#include "util/samplebuffer.h"
#include "util/types.h"

namespace mixxx {

/// Measures how much low, mid and high frequency content a channel's signal
/// contains and publishes the three levels as the read-only controls
/// band_meter_low, band_meter_mid and band_meter_high of the channel's
/// group. A level is the mean absolute sample value of the band, in the
/// range 0..1. It is fed by EngineMixer with the post-fader signal of a
/// deck, so the levels reflect the EQ, the volume fader, the crossfader and
/// the post-fader effects. All methods except the constructor are called
/// from the engine thread and do not allocate.
class EngineBandMeter {
  public:
    explicit EngineBandMeter(const QString& group);
    virtual ~EngineBandMeter();

    /// Measures bufferSize interleaved stereo samples of pIn. The published
    /// levels are updated once enough samples have been accumulated.
    void process(const CSAMPLE* pIn, std::size_t bufferSize, audio::SampleRate sampleRate);

    /// Sets all levels to zero and forgets the filter history. This is cheap
    /// when the meter is already reset, so it can be called on every engine
    /// callback in which the channel is not metered.
    void reset();

  private:
    struct Band {
        explicit Band(const ConfigKey& key);

        ControlObject m_level;
        CSAMPLE m_sumAbs;
    };

    /// The filters that split the signal into the bands at one sample rate.
    struct FilterSet;

    FilterSet* filterSetFor(audio::SampleRate sampleRate);
    void accumulate(Band* pBand, std::size_t bufferSize);
    void publish(Band* pBand);

    Band m_low;
    Band m_mid;
    Band m_high;

    /// Designing a filter allocates memory, which is not allowed in the
    /// engine thread. So the filters for all common sample rates are
    /// designed in advance.
    std::vector<std::unique_ptr<FilterSet>> m_filterSets;
    FilterSet* m_pFilterSet;
    SampleBuffer m_filtered;

    std::size_t m_samplesAccumulated;
    bool m_isReset;

    DISALLOW_COPY_AND_ASSIGN(EngineBandMeter);
};

/// Makes the engine run the band meters of the decks for as long as it
/// exists. The meters cost CPU time in the engine thread, so they are
/// switched off while nobody has requested them. Controller mappings and
/// skins request them with the control [App],band_meters_enabled instead.
class EngineBandMeterRequest {
  public:
    EngineBandMeterRequest();
    virtual ~EngineBandMeterRequest();

    /// Returns true while at least one request exists. Can be called from
    /// any thread.
    static bool anyActive();

  private:
    static QAtomicInt s_count;

    DISALLOW_COPY_AND_ASSIGN(EngineBandMeterRequest);
};

} // namespace mixxx
