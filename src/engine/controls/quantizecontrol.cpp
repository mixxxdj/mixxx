#include "engine/controls/quantizecontrol.h"

#include <QtDebug>

#include "control/controlframepos.h"
#include "control/controlpushbutton.h"
#include "engine/controls/enginecontrol.h"
#include "moc_quantizecontrol.cpp"
#include "preferences/usersettings.h"
#include "track/track.h"

QuantizeControl::QuantizeControl(const QString& group,
        UserSettingsPointer pConfig)
        : EngineControl(group, pConfig) {
    // Turn quantize OFF by default. See Bug #898213
    m_pCOQuantizeEnabled = new ControlPushButton(ConfigKey(group, "quantize"), true);
    m_pCOQuantizeEnabled->setButtonMode(ControlPushButton::TOGGLE);
    m_pCONextBeat = std::make_unique<ControlFramePos>(ConfigKey(group, "beat_next"));
    m_pCOPrevBeat = std::make_unique<ControlFramePos>(ConfigKey(group, "beat_prev"));
    m_pCOClosestBeat = std::make_unique<ControlFramePos>(ConfigKey(group, "beat_closest"));
}

QuantizeControl::~QuantizeControl() {
    delete m_pCOQuantizeEnabled;
}

void QuantizeControl::trackLoaded(TrackPointer pNewTrack) {
    if (pNewTrack) {
        m_pBeats = pNewTrack->getBeats();
        // Initialize prev and next beat as if current position was zero.
        // If there is a cue point, the value will be updated.
        lookupBeatPositions(mixxx::audio::kStartFramePos);
        updateClosestBeat(mixxx::audio::kStartFramePos);
    } else {
        m_pBeats.clear();
        m_pCOPrevBeat->set(mixxx::audio::kInvalidFramePos);
        m_pCONextBeat->set(mixxx::audio::kInvalidFramePos);
        m_pCOClosestBeat->set(mixxx::audio::kInvalidFramePos);
    }
}

void QuantizeControl::trackBeatsUpdated(mixxx::BeatsPointer pBeats) {
    m_pBeats = pBeats;
    const mixxx::audio::FramePos currentPosition = frameInfo().currentPosition;
    lookupBeatPositions(currentPosition);
    updateClosestBeat(currentPosition);
}

void QuantizeControl::setFrameInfo(mixxx::audio::FramePos currentPosition,
        mixxx::audio::FramePos trackEndPosition,
        mixxx::audio::SampleRate sampleRate) {
    EngineControl::setFrameInfo(currentPosition, trackEndPosition, sampleRate);
    playPosChanged(currentPosition);
}

void QuantizeControl::playPosChanged(mixxx::audio::FramePos position) {
    DEBUG_ASSERT(position.isValid());
    // We only need to update the prev or next if the current sample is
    // out of range of the existing beat positions or if we've been forced to
    // do so.
    // NOTE: This bypasses the epsilon calculation, but is there a way
    //       that could actually cause a problem?
    const auto prevBeatPosition = m_pCOPrevBeat->toFramePos();
    const auto nextBeatPosition = m_pCONextBeat->toFramePos();
    if (!prevBeatPosition.isValid() || position < prevBeatPosition ||
            !nextBeatPosition.isValid() || position > nextBeatPosition) {
        lookupBeatPositions(position);
    }
    updateClosestBeat(position);
}

void QuantizeControl::lookupBeatPositions(mixxx::audio::FramePos position) {
    DEBUG_ASSERT(position.isValid());
    mixxx::BeatsPointer pBeats = m_pBeats;
    if (pBeats) {
        mixxx::audio::FramePos prevBeatPosition;
        mixxx::audio::FramePos nextBeatPosition;
        pBeats->findPrevNextBeats(position, &prevBeatPosition, &nextBeatPosition, true);
        m_pCOPrevBeat->set(prevBeatPosition);
        m_pCONextBeat->set(nextBeatPosition);
    }
}

void QuantizeControl::updateClosestBeat(mixxx::audio::FramePos position) {
    DEBUG_ASSERT(position.isValid());
    if (!m_pBeats) {
        return;
    }
    const auto prevBeatPosition = m_pCOPrevBeat->toFramePos();
    const auto nextBeatPosition = m_pCONextBeat->toFramePos();

    // Calculate closest beat by hand since we want the beat locations themselves
    // and duplicating the work by calling the standard API would double
    // the number of mutex locks.
    if (!prevBeatPosition.isValid()) {
        if (nextBeatPosition.isValid()) {
            m_pCOClosestBeat->set(nextBeatPosition);
        } else {
            // Likely no beat information -- can't set closest beat value.
        }
    } else if (!nextBeatPosition.isValid()) {
        m_pCOClosestBeat->set(prevBeatPosition);
    } else {
        const mixxx::audio::FramePos currentClosestBeatPosition =
                (nextBeatPosition - position > position - prevBeatPosition)
                ? prevBeatPosition
                : nextBeatPosition;
        const auto closestBeatPosition = m_pCOClosestBeat->toFramePos();
        if (closestBeatPosition != currentClosestBeatPosition) {
            m_pCOClosestBeat->set(currentClosestBeatPosition);
        }
    }
}
