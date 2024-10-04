#pragma once

#include "library/autodj/track/fadeabletrackordeckattributes.h"

/// Exposes the attributes of a track from the Auto DJ queue.
class TrackAttributes : public FadeableTrackOrDeckAttributes {
    Q_OBJECT
  public:
    TrackAttributes(TrackPointer pTrack);

    virtual mixxx::audio::FramePos introStartPosition() const override;
    virtual mixxx::audio::FramePos introEndPosition() const override;
    virtual mixxx::audio::FramePos outroStartPosition() const override;
    virtual mixxx::audio::FramePos outroEndPosition() const override;
    virtual mixxx::audio::SampleRate sampleRate() const override;
    virtual mixxx::audio::FramePos trackEndPosition() const override;
    virtual double playPosition() const override;
    virtual double rateRatio() const override;

    TrackPointer getLoadedTrack() const override {
        return m_pTrack;
    }

  private:
    TrackPointer m_pTrack;
};
