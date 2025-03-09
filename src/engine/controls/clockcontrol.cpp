#include "engine/controls/clockcontrol.h"

#include <cmath>

#include "control/controlobject.h"
#include "control/controlproxy.h"
#include "engine/controls/enginecontrol.h"
#include "moc_clockcontrol.cpp"
#include "preferences/usersettings.h"
#include "track/track.h"

namespace {
constexpr double kBlinkInterval = 0.20; // LED is on 20% of the beat period
constexpr double kStandStillTolerance =
        0.005; // (seconds) Minimum change, to he last evaluated position
constexpr double kSignificiantRateThreshold =
        0.1; // If rate is significiant, update indicator also inside standstill tolerance
} // namespace

ClockControl::ClockControl(const QString& group, UserSettingsPointer pConfig)
        : EngineControl(group, pConfig),
          m_pCOBeatActive(std::make_unique<ControlObject>(ConfigKey(group, "beat_active"))),
          m_pCOBeatActiveTempo_0_5(std::make_unique<ControlObject>(ConfigKey(group, "beat_active_0_5"))),
          m_pCOBeatActiveTempo_0_666(std::make_unique<ControlObject>(ConfigKey(group, "beat_active_0_666"))),
          m_pCOBeatActiveTempo_0_75(std::make_unique<ControlObject>(ConfigKey(group, "beat_active_0_75"))),
          m_pCOBeatActiveTempo_1_25(std::make_unique<ControlObject>(ConfigKey(group, "beat_active_1_25"))),
          m_pCOBeatActiveTempo_1_333(std::make_unique<ControlObject>(ConfigKey(group, "beat_active_1_333"))),
          m_pCOBeatActiveTempo_1_5(std::make_unique<ControlObject>(ConfigKey(group, "beat_active_1_5"))),
          m_pLoopEnabled(std::make_unique<ControlProxy>(group, "loop_enabled", this)),
          m_pLoopStartPosition(std::make_unique<ControlProxy>(group, "loop_start_position", this)),
          m_pLoopEndPosition(std::make_unique<ControlProxy>(group, "loop_end_position", this)),
          m_lastPlayDirectionWasForwards(true),
          m_lastEvaluatedPosition(mixxx::audio::kStartFramePos),
          m_prevBeatPosition(mixxx::audio::kStartFramePos),
          m_nextBeatPosition(mixxx::audio::kStartFramePos),
          m_blinkIntervalFrames(0.0),
          m_internalState(StateMachine::outsideIndicationArea) {
    m_pCOBeatActive->setReadOnly();
    m_pCOBeatActive->forceSet(0.0);
    
    // initialize fractional tempo beat active controls
    m_pCOBeatActiveTempo_0_5->setReadOnly();
    m_pCOBeatActiveTempo_0_5->forceSet(0.0);
    
    m_pCOBeatActiveTempo_0_666->setReadOnly();
    m_pCOBeatActiveTempo_0_666->forceSet(0.0);
    
    m_pCOBeatActiveTempo_0_75->setReadOnly();
    m_pCOBeatActiveTempo_0_75->forceSet(0.0);
    
    m_pCOBeatActiveTempo_1_25->setReadOnly();
    m_pCOBeatActiveTempo_1_25->forceSet(0.0);
    
    m_pCOBeatActiveTempo_1_333->setReadOnly();
    m_pCOBeatActiveTempo_1_333->forceSet(0.0);
    
    m_pCOBeatActiveTempo_1_5->setReadOnly();
    m_pCOBeatActiveTempo_1_5->forceSet(0.0);
}

ClockControl::~ClockControl() = default;

// called from an engine worker thread
void ClockControl::trackLoaded(TrackPointer pNewTrack) {
    mixxx::BeatsPointer pBeats;
    if (pNewTrack) {
        pBeats = pNewTrack->getBeats();
    }
    trackBeatsUpdated(pBeats);
}

void ClockControl::trackBeatsUpdated(mixxx::BeatsPointer pBeats) {
    // Clear on-beat control
    m_pCOBeatActive->forceSet(0.0);
    
    // Clear fractional tempo beat active controls
    m_pCOBeatActiveTempo_0_5->forceSet(0.0);
    m_pCOBeatActiveTempo_0_666->forceSet(0.0);
    m_pCOBeatActiveTempo_0_75->forceSet(0.0);
    m_pCOBeatActiveTempo_1_25->forceSet(0.0);
    m_pCOBeatActiveTempo_1_333->forceSet(0.0);
    m_pCOBeatActiveTempo_1_5->forceSet(0.0);
    
    m_pBeats = pBeats;
}

void ClockControl::updateFractionalTempoIndicator(double dRate,
        mixxx::audio::FramePos currentPosition,
        mixxx::audio::FramePos trackStartPosition,
        mixxx::audio::FrameDiff_t beatLength,
        double tempoRatio,
        std::unique_ptr<ControlObject>& pCOBeatActive) {
    
    // calculate the scaled beat length
    mixxx::audio::FrameDiff_t scaledBeatLength = beatLength / tempoRatio;
    
    // calculate the position within the scaled beat pattern
    // this ensures the phase starts from the beginning of the track
    mixxx::audio::FrameDiff_t positionOffset = currentPosition - trackStartPosition;
    mixxx::audio::FrameDiff_t positionInPattern = std::fmod(positionOffset, scaledBeatLength);
    
    // calculate the blink interval for this tempo
    mixxx::audio::FrameDiff_t blinkIntervalFrames = scaledBeatLength * kBlinkInterval;
    
    // determine if we're in the active region of the beat
    if (dRate > 0.0) {
        if (m_lastPlayDirectionWasForwards) {
            // forward playing, check if we're in the active region at the start of the beat
            if (positionInPattern < blinkIntervalFrames) {
                pCOBeatActive->forceSet(1.0);
            } else {
                pCOBeatActive->forceSet(0.0);
            }
        }
    } else if (dRate < 0.0) {
        if (!m_lastPlayDirectionWasForwards) {
            // reverse playing, check if we're in the active region at the end of the beat
            if ((scaledBeatLength - positionInPattern) < blinkIntervalFrames) {
                pCOBeatActive->forceSet(2.0);
            } else {
                pCOBeatActive->forceSet(0.0);
            }
        }
    }
}

void ClockControl::updateIndicators(const double dRate,
        mixxx::audio::FramePos currentPosition,
        mixxx::audio::SampleRate sampleRate) {
    /* This method sets the control beat_active is set to the following values:
    *  0.0 --> No beat indication (outside 20% area or play direction changed while indication was on)
    *  1.0 --> Forward playing, set at the beat and set back to 0.0 at 20% of beat distance
    *  2.0 --> Reverse playing, set at the beat and set back to 0.0 at -20% of beat distance
    */

    // No position change since last indicator update (e.g. deck stopped) -> No indicator update needed
    // The kSignificiantRateThreshold condition ensures an immediate indicator update, when the play/cue button is pressed
    if ((currentPosition <= (m_lastEvaluatedPosition + kStandStillTolerance * sampleRate)) &&
            (currentPosition >= (m_lastEvaluatedPosition - kStandStillTolerance * sampleRate)) &&
            (fabs(dRate) <= kSignificiantRateThreshold)) {
        return;
    }

    // Position change more significiantly, but rate is zero. Occurs when pressing a cue point
    // The m_internalState needs to be taken into account here to prevent unnecessary events (state 0 -> state 0)
    if ((dRate == 0.0) && (m_internalState != StateMachine::outsideIndicationArea)) {
        m_internalState = StateMachine::outsideIndicationArea;
        m_pCOBeatActive->forceSet(0.0);
        
        // reset fractional tempo beat active controls
        m_pCOBeatActiveTempo_0_5->forceSet(0.0);
        m_pCOBeatActiveTempo_0_666->forceSet(0.0);
        m_pCOBeatActiveTempo_0_75->forceSet(0.0);
        m_pCOBeatActiveTempo_1_25->forceSet(0.0);
        m_pCOBeatActiveTempo_1_333->forceSet(0.0);
        m_pCOBeatActiveTempo_1_5->forceSet(0.0);
    }

    mixxx::audio::FramePos prevIndicatorPosition;
    mixxx::audio::FramePos nextIndicatorPosition;

    const mixxx::BeatsPointer pBeats = m_pBeats;
    if (pBeats) {
        if (!m_prevBeatPosition.isValid() || !m_nextBeatPosition.isValid() ||
                currentPosition >= m_nextBeatPosition ||
                currentPosition <= m_prevBeatPosition) {
            pBeats->findPrevNextBeats(currentPosition,
                    &m_prevBeatPosition,
                    &m_nextBeatPosition,
                    false); // Precise compare without tolerance needed
        }
    } else {
        m_prevBeatPosition = mixxx::audio::kInvalidFramePos;
        m_nextBeatPosition = mixxx::audio::kInvalidFramePos;
    }

    // Loops need special handling
    if (m_pLoopEnabled->toBool()) {
        const auto loopStartPosition =
                mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                        m_pLoopStartPosition->get());
        const auto loopEndPosition =
                mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                        m_pLoopEndPosition->get());

        if (m_prevBeatPosition.isValid() && m_nextBeatPosition.isValid() &&
                loopStartPosition.isValid() && loopEndPosition.isValid() &&
                m_prevBeatPosition < loopStartPosition &&
                m_nextBeatPosition >= loopEndPosition) {
            // No beat inside loop -> show beat indication at loop_start_position
            prevIndicatorPosition = loopStartPosition;
            nextIndicatorPosition = loopEndPosition;
        } else {
            prevIndicatorPosition = m_prevBeatPosition;
            nextIndicatorPosition = m_nextBeatPosition;
        }

        if (m_prevBeatPosition.isValid() && m_nextBeatPosition.isValid()) {
            // Don't overwrite interval at begin/end of track
            if ((loopEndPosition - loopStartPosition) < (m_nextBeatPosition - m_prevBeatPosition)) {
                // Loops smaller than beat distance -> Set m_blinkIntervalFrames based on loop period
                m_blinkIntervalFrames = (loopEndPosition - loopStartPosition) * kBlinkInterval;
            } else {
                m_blinkIntervalFrames =
                        (nextIndicatorPosition - prevIndicatorPosition) *
                        kBlinkInterval;
            }
        }
    } else {
        prevIndicatorPosition = m_prevBeatPosition;
        nextIndicatorPosition = m_nextBeatPosition;

        if (prevIndicatorPosition.isValid() && nextIndicatorPosition.isValid()) {
            // Don't overwrite interval at begin/end of track
            m_blinkIntervalFrames =
                    (nextIndicatorPosition - prevIndicatorPosition) *
                    kBlinkInterval;
        }
    }

    // The m_internalState needs to be taken into account, to show a reliable beat indication for loops
    if (dRate > 0.0) {
        if (m_lastPlayDirectionWasForwards) {
            if (prevIndicatorPosition.isValid() &&
                    currentPosition > prevIndicatorPosition &&
                    currentPosition <
                            (prevIndicatorPosition + m_blinkIntervalFrames) &&
                    (m_internalState != StateMachine::afterBeatActive) &&
                    (m_internalState !=
                            StateMachine::afterBeatDirectionChanged)) {
                m_internalState = StateMachine::afterBeatActive;
                m_pCOBeatActive->forceSet(1.0);
            } else if (prevIndicatorPosition.isValid() &&
                    currentPosition >
                            (prevIndicatorPosition + m_blinkIntervalFrames) &&
                    (m_internalState == StateMachine::afterBeatActive ||
                            m_internalState ==
                                    StateMachine::afterBeatDirectionChanged)) {
                m_internalState = StateMachine::outsideIndicationArea;
                m_pCOBeatActive->forceSet(0.0);
            }
        } else {
            // Play direction changed while beat indicator was on and forward playing
            if (nextIndicatorPosition.isValid() &&
                    currentPosition < nextIndicatorPosition &&
                    currentPosition >=
                            nextIndicatorPosition - m_blinkIntervalFrames &&
                    m_internalState !=
                            StateMachine::beforeBeatDirectionChanged) {
                m_internalState = StateMachine::beforeBeatDirectionChanged;
                m_pCOBeatActive->forceSet(0.0);
            }
        }
        m_lastPlayDirectionWasForwards = true;
    } else if (dRate < 0.0) {
        if (!m_lastPlayDirectionWasForwards) {
            if (nextIndicatorPosition.isValid() &&
                    currentPosition < nextIndicatorPosition &&
                    currentPosition >
                            (nextIndicatorPosition - m_blinkIntervalFrames) &&
                    m_internalState != StateMachine::beforeBeatActive &&
                    m_internalState !=
                            StateMachine::beforeBeatDirectionChanged) {
                m_internalState = StateMachine::beforeBeatActive;
                m_pCOBeatActive->forceSet(2.0);
            } else if (nextIndicatorPosition.isValid() &&
                    currentPosition <
                            (nextIndicatorPosition - m_blinkIntervalFrames) &&
                    (m_internalState == StateMachine::beforeBeatActive ||
                            m_internalState ==
                                    StateMachine::beforeBeatDirectionChanged)) {
                m_internalState = StateMachine::outsideIndicationArea;
                m_pCOBeatActive->forceSet(0.0);
            }
        } else {
            // Play direction changed while beat indicator was on and reverse playing
            if (prevIndicatorPosition.isValid() &&
                    currentPosition > prevIndicatorPosition &&
                    currentPosition <=
                            (prevIndicatorPosition + m_blinkIntervalFrames) &&
                    m_internalState !=
                            StateMachine::afterBeatDirectionChanged) {
                m_internalState = StateMachine::afterBeatDirectionChanged;
                m_pCOBeatActive->forceSet(0.0);
            }
        }
        m_lastPlayDirectionWasForwards = false;
    }

    m_lastEvaluatedPosition = currentPosition;
    
    // Update fractional tempo beat indicators if we have valid beat information
    if (pBeats && prevIndicatorPosition.isValid() && nextIndicatorPosition.isValid()) {
        // calculate the beat length
        mixxx::audio::FrameDiff_t beatLength = nextIndicatorPosition - prevIndicatorPosition;
        
        // get track start position
        mixxx::audio::FramePos trackStartPosition = mixxx::audio::kStartFramePos;
        
        // update each fractional tempo indicator
        updateFractionalTempoIndicator(dRate, currentPosition, trackStartPosition, beatLength, 0.5, m_pCOBeatActiveTempo_0_5);
        updateFractionalTempoIndicator(dRate, currentPosition, trackStartPosition, beatLength, 0.666, m_pCOBeatActiveTempo_0_666);
        updateFractionalTempoIndicator(dRate, currentPosition, trackStartPosition, beatLength, 0.75, m_pCOBeatActiveTempo_0_75);
        updateFractionalTempoIndicator(dRate, currentPosition, trackStartPosition, beatLength, 1.25, m_pCOBeatActiveTempo_1_25);
        updateFractionalTempoIndicator(dRate, currentPosition, trackStartPosition, beatLength, 1.333, m_pCOBeatActiveTempo_1_333);
        updateFractionalTempoIndicator(dRate, currentPosition, trackStartPosition, beatLength, 1.5, m_pCOBeatActiveTempo_1_5);
    }
}
