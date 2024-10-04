#include "library/autodj/track/trackattributes.h"

#include "moc_trackattributes.cpp"
#include "track/track.h"

TrackAttributes::TrackAttributes(TrackPointer pTrack)
        : m_pTrack(pTrack) {
}

mixxx::audio::FramePos TrackAttributes::introStartPosition() const {
    auto pIntro = m_pTrack->findCueByType(mixxx::CueType::Intro);
    if (pIntro) {
        return pIntro->getPosition();
    }
    return mixxx::audio::FramePos();
}

mixxx::audio::FramePos TrackAttributes::introEndPosition() const {
    auto pIntro = m_pTrack->findCueByType(mixxx::CueType::Intro);
    if (pIntro) {
        return pIntro->getEndPosition();
    }
    return mixxx::audio::FramePos();
}

mixxx::audio::FramePos TrackAttributes::outroStartPosition() const {
    auto pOutro = m_pTrack->findCueByType(mixxx::CueType::Outro);
    if (pOutro) {
        return pOutro->getPosition();
    }
    return mixxx::audio::FramePos();
}

mixxx::audio::FramePos TrackAttributes::outroEndPosition() const {
    auto pOutro = m_pTrack->findCueByType(mixxx::CueType::Outro);
    if (pOutro) {
        return pOutro->getEndPosition();
    }
    return mixxx::audio::FramePos();
}

mixxx::audio::SampleRate TrackAttributes::sampleRate() const {
    return m_pTrack->getSampleRate();
}

double TrackAttributes::playPosition() const {
    return 0.0;
}

double TrackAttributes::rateRatio() const {
    return 1.0;
}

mixxx::audio::FramePos TrackAttributes::trackEndPosition() const {
    // Instead of actually loading the file, we simply infer
    // the number of frames from duration and sample rate stored
    // in the database. This isn't entirely accurate due to
    // rounding errors, but more than accurate enough for
    // estimating the remaining play time of the AutoDJ queue.
    double approxNumSamples =
            m_pTrack->getSampleRate() * m_pTrack->getDuration();

    return mixxx::audio::FramePos(approxNumSamples);
}
