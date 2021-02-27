#include "engine/controls/clockcontrol.h"

#include "control/controlobject.h"
#include "control/controlproxy.h"
#include "engine/controls/enginecontrol.h"
#include "moc_clockcontrol.cpp"
#include "preferences/usersettings.h"
#include "track/track.h"

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
        const double currentSample) {
    /* This method sets the control beat_active is set to the following values:
    * +1.0 --> Forward playing, set at the beat and set back to 0.0 at 20% of beat distance
    *  0.0 --> No beat indication (ouside 20% area or play direction changed while indication was on)
    * -1.0 --> Reverse playing, set at the beat and set back to 0.0 at -20% of beat distance
    */

    // TODO(XXX) should this be customizable, or latency dependent?
    const double kBlinkInterval = 0.20; // LED is on 20% of the beat period

    if ((currentSample == m_lastEvaluatedSample) ||
            (dRate == 0.0)) {
        return; // No position change (e.g. deck stopped) -> No indicator update needed
    }

    mixxx::BeatsPointer pBeats = m_pBeats;
    if (pBeats) {
        if ((currentSample >= m_NextBeatSamples) ||
                (currentSample <= m_PrevBeatSamples)) {
            /*qDebug() << "### findPrevNextBeats ### " <<
                " currentSample: " << currentSample <<
                " m_lastEvaluatedSample: " << m_lastEvaluatedSample
                << " m_PrevBeatSamples: " << m_PrevBeatSamples
                << " m_NextBeatSamples: " << m_NextBeatSamples;*/

            pBeats->findPrevNextBeats(currentSample,
                    &m_PrevBeatSamples,
                    &m_NextBeatSamples,
                    true); // Precise compare without tolerance needed

            m_blinkIntervalSamples = (m_NextBeatSamples - m_PrevBeatSamples) * kBlinkInterval;
        }
        /*qDebug() << "dRate:" << dRate <<
            " m_lastPlayDirection:" << m_lastPlayDirection <<
            " m_pCOBeatActive->get(): " << m_pCOBeatActive->get() <<
            " currentSample: " << currentSample <<
            " m_lastEvaluatedSample: " << m_lastEvaluatedSample <<
            " m_PrevBeatSamples: " << m_PrevBeatSamples <<
            " m_NextBeatSamples: " << m_NextBeatSamples <<
            " m_blinkIntervalSamples: " << m_blinkIntervalSamples;*/

        // The m_InternalState needs to be taken into account, to show a reliable beat indication for loops
        if (dRate >= 0.0) {
            if (m_lastPlayDirection == true) {
                if ((currentSample > m_PrevBeatSamples) &&
                        (currentSample <
                                m_PrevBeatSamples + m_blinkIntervalSamples) &&
                        (m_InternalState != StateMachine::afterBeatActive) &&
                        (m_InternalState != StateMachine::afterBeatDirectionChanged)) {
                    m_InternalState = StateMachine::afterBeatActive;
                    m_pCOBeatActive->forceSet(1.0);
                } else if ((currentSample > m_PrevBeatSamples +
                                           m_blinkIntervalSamples) &&
                        ((m_InternalState == StateMachine::afterBeatActive) ||
                                (m_InternalState == StateMachine::afterBeatDirectionChanged))) {
                    m_InternalState = StateMachine::outsideIndicationArea;
                    m_pCOBeatActive->forceSet(0.0);
                }
            } else {
                // Play direction changed while beat indicator was on and forward playing
                if ((currentSample < m_NextBeatSamples) &&
                        (currentSample >=
                                m_NextBeatSamples - m_blinkIntervalSamples) &&
                        (m_InternalState != StateMachine::beforeBeatDirectionChanged)) {
                    m_InternalState = StateMachine::beforeBeatDirectionChanged;
                    m_pCOBeatActive->forceSet(0.0);
                }
            }
            m_lastPlayDirection = true; // Forward
        } else {
            if (m_lastPlayDirection == false) {
                if ((currentSample < m_NextBeatSamples) &&
                        (currentSample >
                                m_NextBeatSamples - m_blinkIntervalSamples) &&
                        (m_InternalState != StateMachine::beforeBeatActive) &&
                        (m_InternalState != StateMachine::beforeBeatDirectionChanged)) {
                    m_InternalState = StateMachine::beforeBeatActive;
                    m_pCOBeatActive->forceSet(-1.0);
                } else if ((currentSample < m_NextBeatSamples -
                                           m_blinkIntervalSamples) &&
                        ((m_InternalState == StateMachine::beforeBeatActive) ||
                                (m_InternalState == StateMachine::beforeBeatDirectionChanged))) {
                    m_InternalState = StateMachine::outsideIndicationArea;
                    m_pCOBeatActive->forceSet(0.0);
                }
            } else {
                // Play direction changed while beat indicator was on and reverse playing
                if ((currentSample > m_PrevBeatSamples) &&
                        (currentSample <=
                                m_PrevBeatSamples + m_blinkIntervalSamples) &&
                        (m_InternalState != StateMachine::afterBeatDirectionChanged)) {
                    m_InternalState = StateMachine::afterBeatDirectionChanged;
                    m_pCOBeatActive->forceSet(0.0);
                }
            }
            m_lastPlayDirection = false; // Reverse
        }
        m_lastEvaluatedSample = currentSample;
    }
}
