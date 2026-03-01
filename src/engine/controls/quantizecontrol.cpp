#include "engine/controls/quantizecontrol.h"

#include "control/controlobject.h"
#include "control/controlpushbutton.h"
#include "engine/controls/enginecontrol.h"
#include "moc_quantizecontrol.cpp"
#include "preferences/usersettings.h"
#include "track/track.h"

QuantizeControl::QuantizeControl(const QString& group,
        UserSettingsPointer pConfig)
        : EngineControl(group, pConfig) {
    m_pCOQuantizeEnabled = new ControlPushButton(ConfigKey(group, "quantize"), true, 1.0);
    m_pCOQuantizeEnabled->setButtonMode(mixxx::control::ButtonMode::Toggle);
    m_pCONextBeat = new ControlObject(ConfigKey(group, "beat_next"));
    m_pCONextBeat->setKbdRepeatable(true);
    m_pCONextBeat->set(mixxx::audio::kInvalidFramePos.toEngineSamplePosMaybeInvalid());
    m_pCOPrevBeat = new ControlObject(ConfigKey(group, "beat_prev"));
    m_pCOPrevBeat->setKbdRepeatable(true);
    m_pCOPrevBeat->set(mixxx::audio::kInvalidFramePos.toEngineSamplePosMaybeInvalid());
    m_pCOClosestBeat = new ControlObject(ConfigKey(group, "beat_closest"));
    m_pCOClosestBeat->set(mixxx::audio::kInvalidFramePos.toEngineSamplePosMaybeInvalid());
}

QuantizeControl::~QuantizeControl() {
    delete m_pCOQuantizeEnabled;
    delete m_pCONextBeat;
    delete m_pCOPrevBeat;
    delete m_pCOClosestBeat;
}

void QuantizeControl::trackLoaded(TrackPointer pNewTrack) {
    if (pNewTrack) {
        m_pBeats = pNewTrack->getBeats();
        // Initialize prev and next beat as if current position was zero.
        // If there is a cue point, the value will be updated.
        lookupBeatPositions(mixxx::audio::kStartFramePos);
        updateClosestBeat(mixxx::audio::kStartFramePos);
    } else {
        m_pBeats.reset();
        m_pCOPrevBeat->set(mixxx::audio::kInvalidFramePos.toEngineSamplePosMaybeInvalid());
        m_pCONextBeat->set(mixxx::audio::kInvalidFramePos.toEngineSamplePosMaybeInvalid());
        m_pCOClosestBeat->set(mixxx::audio::kInvalidFramePos.toEngineSamplePosMaybeInvalid());
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
    FrameInfo frameInf = frameInfo();
    if (frameInf.currentPosition != currentPosition ||
            frameInf.trackEndPosition != trackEndPosition ||
            frameInf.sampleRate != sampleRate) {
        EngineControl::setFrameInfo(currentPosition, trackEndPosition, sampleRate);
        playPosChanged(currentPosition);
    }
}

void QuantizeControl::playPosChanged(mixxx::audio::FramePos position) {
    DEBUG_ASSERT(position.isValid());
    // We only need to update the prev or next if the current sample is
    // out of range of the existing beat positions or if we've been forced to
    // do so.
    const auto prevBeatPosition =
            mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                    m_pCOPrevBeat->get());
    const auto nextBeatPosition =
            mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                    m_pCONextBeat->get());
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
        pBeats->findPrevNextBeats(position, &prevBeatPosition, &nextBeatPosition, false);
        // FIXME: -1.0 is a valid frame position, should we set the COs to NaN?
        m_pCOPrevBeat->set(prevBeatPosition.toEngineSamplePosMaybeInvalid());
        m_pCONextBeat->set(nextBeatPosition.toEngineSamplePosMaybeInvalid());
    }
}

void QuantizeControl::updateClosestBeat(mixxx::audio::FramePos position) {
    DEBUG_ASSERT(position.isValid());
    if (!m_pBeats) {
        return;
    }
    const auto prevBeatPosition =
            mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                    m_pCOPrevBeat->get());
    const auto nextBeatPosition =
            mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                    m_pCONextBeat->get());

    // Calculate closest beat by hand since we want the beat locations themselves
    // and duplicating the work by calling the standard API would double
    // the number of mutex locks.
    if (!prevBeatPosition.isValid()) {
        if (nextBeatPosition.isValid()) {
            m_pCOClosestBeat->set(nextBeatPosition.toEngineSamplePos());
        } else {
            // Likely no beat information -- can't set closest beat value.
        }
    } else if (!nextBeatPosition.isValid()) {
        m_pCOClosestBeat->set(prevBeatPosition.toEngineSamplePos());
    } else {
        const mixxx::audio::FramePos currentClosestBeatPosition =
                (nextBeatPosition - position > position - prevBeatPosition)
                ? prevBeatPosition
                : nextBeatPosition;
        const auto closestBeatPosition =
                mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                        m_pCOClosestBeat->get());
        if (closestBeatPosition != currentClosestBeatPosition) {
            m_pCOClosestBeat->set(currentClosestBeatPosition.toEngineSamplePos());
        }
    }
}
