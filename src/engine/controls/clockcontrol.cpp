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
        : EngineControl(group, pConfig) {
    m_pCOBeatActive = new ControlObject(ConfigKey(group, "beat_active"));
    m_pCOBeatActive->setReadOnly();
    m_pCOBeatActive->forceSet(0.0);
    m_lastEvaluatedSample = 0;
    m_PrevBeatSamples = 0;
    m_InternalState = StateMachine::outsideIndicationArea;
    m_NextBeatSamples = 0;
    m_blinkIntervalSamples = 0;
    m_pLoopEnabled = new ControlProxy(group, "loop_enabled", this);
    m_pLoopStartPosition = new ControlProxy(group, "loop_start_position", this);
    m_pLoopEndPosition = new ControlProxy(group, "loop_end_position", this);
}

ClockControl::~ClockControl() {
    delete m_pCOBeatActive;
}

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
    m_pBeats = pBeats;
}

void ClockControl::process(const double dRate,
        const double currentSample,
        const int iBuffersize) {
    Q_UNUSED(dRate);
    Q_UNUSED(currentSample);
    Q_UNUSED(iBuffersize);
}

void ClockControl::updateIndicators(const double dRate,
        const double currentSample,
        const double sampleRate) {
    /* This method sets the control beat_active is set to the following values:
    *  0.0 --> No beat indication (outside 20% area or play direction changed while indication was on)
    *  1.0 --> Forward playing, set at the beat and set back to 0.0 at 20% of beat distance
    *  2.0 --> Reverse playing, set at the beat and set back to 0.0 at -20% of beat distance
    */

    // No position change since last indicator update (e.g. deck stopped) -> No indicator update needed
    // The kSignificiantRateThreshold condition ensures an immediate indicator update, when the play/cue button is pressed
    if ((currentSample <= (m_lastEvaluatedSample + kStandStillTolerance * sampleRate)) &&
            (currentSample >= (m_lastEvaluatedSample - kStandStillTolerance * sampleRate)) &&
            (fabs(dRate) <= kSignificiantRateThreshold)) {
        return;
    }

    // Position change more significiantly, but rate is zero. Occurs when pressing a cue point
    // The m_InternalState needs to be taken into account here to prevent uneccessary events (state 0 -> state 0)
    if ((dRate == 0.0) && (m_InternalState != StateMachine::outsideIndicationArea)) {
        m_InternalState = StateMachine::outsideIndicationArea;
        m_pCOBeatActive->forceSet(0.0);
    }

    double prevIndicatorSamples;
    double nextIndicatorSamples;

    const mixxx::BeatsPointer pBeats = m_pBeats;
    if (pBeats) {
        if ((currentSample >= m_NextBeatSamples) ||
                (currentSample <= m_PrevBeatSamples)) {
            mixxx::audio::FramePos prevBeatPosition;
            mixxx::audio::FramePos nextBeatPosition;
            pBeats->findPrevNextBeats(mixxx::audio::FramePos::fromEngineSamplePos(currentSample),
                    &prevBeatPosition,
                    &nextBeatPosition,
                    false); // Precise compare without tolerance needed
            m_PrevBeatSamples = prevBeatPosition.isValid()
                    ? prevBeatPosition.toEngineSamplePos()
                    : -1;
            m_NextBeatSamples = nextBeatPosition.isValid()
                    ? nextBeatPosition.toEngineSamplePos()
                    : -1;
        }
    } else {
        m_PrevBeatSamples = -1;
        m_NextBeatSamples = -1;
    }

    // Loops need special handling
    if (m_pLoopEnabled->toBool()) {
        const double loop_start_position = m_pLoopStartPosition->get();
        const double loop_end_position = m_pLoopEndPosition->get();

        if ((m_PrevBeatSamples < loop_start_position) && (m_NextBeatSamples >= loop_end_position)) {
            // No beat inside loop -> show beat indication at loop_start_position
            prevIndicatorSamples = loop_start_position;
            nextIndicatorSamples = loop_end_position;
        } else {
            prevIndicatorSamples = m_PrevBeatSamples;
            nextIndicatorSamples = m_NextBeatSamples;
        }

        if ((m_PrevBeatSamples != -1) && (m_NextBeatSamples != -1)) {
            // Don't overwrite interval at begin/end of track
            if ((loop_end_position - loop_start_position) <
                    (m_NextBeatSamples - m_PrevBeatSamples)) {
                // Loops smaller than beat distance -> Set m_blinkIntervalSamples based on loop period
                m_blinkIntervalSamples =
                        (loop_end_position - loop_start_position) *
                        kBlinkInterval;
            } else {
                m_blinkIntervalSamples =
                        (nextIndicatorSamples - prevIndicatorSamples) *
                        kBlinkInterval;
            }
        }
    } else {
        prevIndicatorSamples = m_PrevBeatSamples;
        nextIndicatorSamples = m_NextBeatSamples;

        if ((prevIndicatorSamples != -1) && (nextIndicatorSamples != -1)) {
            // Don't overwrite interval at begin/end of track
            m_blinkIntervalSamples =
                    (nextIndicatorSamples - prevIndicatorSamples) *
                    kBlinkInterval;
        }
    }

    // The m_InternalState needs to be taken into account, to show a reliable beat indication for loops
    if (dRate > 0.0) {
        if (m_lastPlayDirection == true) {
            if ((currentSample > prevIndicatorSamples) &&
                    (currentSample <
                            prevIndicatorSamples + m_blinkIntervalSamples) &&
                    (m_InternalState != StateMachine::afterBeatActive) &&
                    (m_InternalState != StateMachine::afterBeatDirectionChanged)) {
                m_InternalState = StateMachine::afterBeatActive;
                m_pCOBeatActive->forceSet(1.0);
            } else if ((currentSample > prevIndicatorSamples +
                                       m_blinkIntervalSamples) &&
                    ((m_InternalState == StateMachine::afterBeatActive) ||
                            (m_InternalState == StateMachine::afterBeatDirectionChanged))) {
                m_InternalState = StateMachine::outsideIndicationArea;
                m_pCOBeatActive->forceSet(0.0);
            }
        } else {
            // Play direction changed while beat indicator was on and forward playing
            if ((currentSample < nextIndicatorSamples) &&
                    (currentSample >=
                            nextIndicatorSamples - m_blinkIntervalSamples) &&
                    (m_InternalState != StateMachine::beforeBeatDirectionChanged)) {
                m_InternalState = StateMachine::beforeBeatDirectionChanged;
                m_pCOBeatActive->forceSet(0.0);
            }
        }
        m_lastPlayDirection = true; // Forward
    } else if (dRate < 0.0) {
        if (m_lastPlayDirection == false) {
            if ((currentSample < nextIndicatorSamples) &&
                    (currentSample >
                            nextIndicatorSamples - m_blinkIntervalSamples) &&
                    (m_InternalState != StateMachine::beforeBeatActive) &&
                    (m_InternalState != StateMachine::beforeBeatDirectionChanged)) {
                m_InternalState = StateMachine::beforeBeatActive;
                m_pCOBeatActive->forceSet(2.0);
            } else if ((currentSample < nextIndicatorSamples -
                                       m_blinkIntervalSamples) &&
                    ((m_InternalState == StateMachine::beforeBeatActive) ||
                            (m_InternalState == StateMachine::beforeBeatDirectionChanged))) {
                m_InternalState = StateMachine::outsideIndicationArea;
                m_pCOBeatActive->forceSet(0.0);
            }
        } else {
            // Play direction changed while beat indicator was on and reverse playing
            if ((currentSample > prevIndicatorSamples) &&
                    (currentSample <=
                            prevIndicatorSamples + m_blinkIntervalSamples) &&
                    (m_InternalState != StateMachine::afterBeatDirectionChanged)) {
                m_InternalState = StateMachine::afterBeatDirectionChanged;
                m_pCOBeatActive->forceSet(0.0);
            }
        }
        m_lastPlayDirection = false; // Reverse
    }
    m_lastEvaluatedSample = currentSample;
}
