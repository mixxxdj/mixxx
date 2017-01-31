#include "engine/clockcontrol.h"

#include "control/controlobject.h"
#include "preferences/usersettings.h"
#include "engine/enginecontrol.h"
#include "control/controlproxy.h"

ClockControl::ClockControl(QString group, UserSettingsPointer pConfig)
        : EngineControl(group, pConfig) {
    m_pCOBeatActive = new ControlObject(ConfigKey(group, "beat_active"));
    m_pCOBeatActive->set(0.0);
    m_pCOSampleRate = new ControlProxy("[Master]","samplerate");
}

ClockControl::~ClockControl() {
    delete m_pCOBeatActive;
    delete m_pCOSampleRate;
}

void ClockControl::trackLoaded(TrackPointer pNewTrack, TrackPointer pOldTrack) {
    Q_UNUSED(pOldTrack);

    // Clear on-beat control
    m_pCOBeatActive->set(0.0);

    // Disconnect any previously loaded track/beats
    if (m_pTrack) {
        disconnect(m_pTrack.get(), SIGNAL(beatsUpdated()),
                   this, SLOT(slotBeatsUpdated()));
    }
    if (pNewTrack) {
        m_pTrack = pNewTrack;
        m_pBeats = m_pTrack->getBeats();
        connect(m_pTrack.get(), SIGNAL(beatsUpdated()),
                this, SLOT(slotBeatsUpdated()));
    } else {
        m_pBeats.clear();
        m_pTrack.reset();
    }

}

void ClockControl::slotBeatsUpdated() {
    if(m_pTrack) {
        m_pBeats = m_pTrack->getBeats();
    }
}

double ClockControl::process(const double dRate,
                             const double currentSample,
                             const double totalSamples,
                             const int iBuffersize) {
    Q_UNUSED(totalSamples);
    Q_UNUSED(iBuffersize);
    double samplerate = m_pCOSampleRate->get();

    // TODO(XXX) should this be customizable, or latency dependent?
    const double blinkSeconds = 0.100;

    // Multiply by two to get samples from frames. Interval is scaled linearly
    // by the rate.
    const double blinkIntervalSamples = 2.0 * samplerate * (1.0 * dRate) * blinkSeconds;

    if (m_pBeats) {
        double closestBeat = m_pBeats->findClosestBeat(currentSample);
        double distanceToClosestBeat = fabs(currentSample - closestBeat);
        m_pCOBeatActive->set(distanceToClosestBeat < blinkIntervalSamples / 2.0);
    }

    return kNoTrigger;
}
