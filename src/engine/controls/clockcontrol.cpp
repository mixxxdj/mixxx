#include "engine/controls/clockcontrol.h"

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
          m_pBeatCountNextCue(std::make_unique<ControlObject>(ConfigKey(group, "beat_count_next_cue"))),
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
    m_pBeatCountNextCue->forceSet(0.0);

}

ClockControl::~ClockControl() = default;

// called from an engine worker thread
void ClockControl::trackLoaded(TrackPointer pNewTrack) {
    mixxx::BeatsPointer pBeats;
    if (pNewTrack) {
        pBeats = pNewTrack->getBeats();
        m_pTrackCues = pNewTrack->getCuePoints();
    }
    trackBeatsUpdated(pBeats);
    QObject::connect(pNewTrack.get(), &Track::cuesUpdatedWithCueList, this, &ClockControl::trackCuesUpdated);
}

void ClockControl::trackBeatsUpdated(mixxx::BeatsPointer pBeats) {
    // Clear on-beat control
    m_pCOBeatActive->forceSet(0.0);
    m_pBeats = pBeats;
}

void ClockControl::trackCuesUpdated(QList<CuePointer> cuePointerList) {
    m_pTrackCues = cuePointerList;
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
        updateBeatCounter(pBeats, currentPosition);
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
}

void ClockControl::updateBeatCounter(mixxx::BeatsPointer pBeats, 
    mixxx::audio::FramePos currentFramePos) {

    QList<mixxx::audio::FramePos> cuesFromCurrentPosition =
            QList<mixxx::audio::FramePos>();

    //Iterate through current Track cues and create a list with the FramePos of the ones
    // that are after the current play position. We add them ordered in the new list
    for (int i = 0; i < m_pTrackCues.count(); ++i) {
        CuePointer cue = m_pTrackCues[i];
        mixxx::audio::FramePos cueFramePos = cue->getPosition();
        if (cueFramePos.isValid()) {
            if (cueFramePos >= currentFramePos) {
                if (cuesFromCurrentPosition.isEmpty()) {
                    cuesFromCurrentPosition.append(cueFramePos);
                } else if (cuesFromCurrentPosition.first() < cueFramePos) {
                    cuesFromCurrentPosition.append(cueFramePos);
                } else {
                    cuesFromCurrentPosition.insert(0, cueFramePos);
                }
            }
        }
    }

    //Since the cuesFromCurrentPosition is ordered, we only need to calculate the difference from the current position
    //with the first element of the list, which would be the closest CUE point to the current play position
    //ToDo (Maldini) - Get beat counters for every cue point to help with multi drop mixes
    std::double_t beatsToNextCue = 0.0;
    if (!cuesFromCurrentPosition.isEmpty()) {
        mixxx::audio::FramePos closestCueFramePos =
                cuesFromCurrentPosition.first();
        pBeats->numBeatsInRange(
                currentFramePos, closestCueFramePos);
        beatsToNextCue = pBeats->numBeatsInRange(
                        currentFramePos, closestCueFramePos);
    } 

    //ToDo (Maldini) - Count until outro after the last cue point
    m_pBeatCountNextCue->forceSet(beatsToNextCue);
}