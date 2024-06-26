#pragma once

#include "audio/types.h"
#include "engine/bufferscalers/rubberbandtask.h"

/// RubberBandWrapper is a wrapper around RubberBand::RubberBandStretcher which
/// allows to distribute signal stretching over multiple instance, but interface
/// with it like if it was a single instance
class RubberBandWrapper {
  public:
    int getEngineVersion() const;
    void setTimeRatio(double ratio);
    std::size_t getSamplesRequired() const;
    int available() const;
    size_t retrieve(float* const* output, size_t samples, SINT channelBufferSize) const;
    size_t getInputIncrement() const;
    size_t getLatency() const;
    double getPitchScale() const;
    size_t getPreferredStartPad() const;
    size_t getStartDelay() const;
    void process(const float* const* input, size_t samples, bool final);
    void setPitchScale(double scale);
    void reset();

    // The following method are helper function and do not wrap any RubberBand calls
    void clear();
    void setup(mixxx::audio::SampleRate sampleRate,
            mixxx::audio::ChannelCount chCount,
            const RubberBand::RubberBandStretcher::Options& opt);
    bool isValid() const;

  private:
    // copy constructor of RubberBand::RubberBandStretcher is implicitly deleted.
    std::vector<std::unique_ptr<RubberBandTask>> m_pInstances;
    // Number of channel used for each instance. This may vary whether the track
    // is a stereo track or a stem track
    mixxx::audio::ChannelCount m_channelPerWorker;
};
