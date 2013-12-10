#include <QtDebug>

#include "engine/internalclock.h"
#include "controlobject.h"
#include "configobject.h"

InternalClock::InternalClock(const char* pGroup)
        : m_group(pGroup),
          m_iOldSampleRate(44100),
          m_dOldBpm(0.0),
          m_dBeatLength(0),
          m_dClockPosition(0) {
    m_pClockBpm = new ControlObject(ConfigKey(m_group, "bpm"));
    connect(m_pClockBpm, SIGNAL(valueChanged(double)),
            this, SLOT(slotBpmChanged(double)),
            Qt::DirectConnection);
}

InternalClock::~InternalClock() {
    delete m_pClockBpm;
}

double InternalClock::getBeatDistance() const {
    if (m_dBeatLength <= 0) {
        qDebug() << "ERROR: InternalClock beat length should never be less than zero";
        return 0.0;
    }
    return m_dClockPosition / m_dBeatLength;
}

void InternalClock::setBeatDistance(double beatDistance) {
    m_dClockPosition = beatDistance * m_dBeatLength;
}

double InternalClock::getBpm() const {
    return m_pClockBpm->get();
}

void InternalClock::setBpm(double bpm) {
    m_pClockBpm->set(bpm);
    updateBeatLength(m_iOldSampleRate, bpm);
}

void InternalClock::slotBpmChanged(double bpm) {
    updateBeatLength(m_iOldSampleRate, bpm);
}

void InternalClock::updateBeatLength(int sampleRate, double bpm) {
    if (m_iOldSampleRate == sampleRate && bpm == m_dOldBpm) {
        return;
    }

    // Changing the beat length changes the beat distance. Record the current
    // beat distance so we can restore it when we are done.
    double oldBeatDistance = getBeatDistance();

    //to get samples per beat, do:
    //
    // samples   samples     60 seconds     minutes
    // ------- = -------  *  ----------  *  -------
    //   beat    second       1 minute       beats

    // that last term is 1 over bpm.

    if (qFuzzyCompare(bpm, 0)) {
        qDebug() << "WARNING: Master bpm reported to be zero, internal clock guessing 60bpm";
        m_dBeatLength = sampleRate;
    } else {
        m_dBeatLength = (sampleRate * 60.0) / bpm;
        if (m_dBeatLength <= 0) {
            qDebug() << "WARNING: Tried to set samples per beat <=0";
            m_dBeatLength = sampleRate;
        }
    }

    m_dOldBpm = bpm;
    m_iOldSampleRate = sampleRate;

    // Restore the old beat distance.
    setBeatDistance(oldBeatDistance);
}

void InternalClock::onCallbackStart(int sampleRate, int bufferSize) {
    updateBeatLength(sampleRate, m_pClockBpm->get());

    // stereo samples, so divide by 2
    m_dClockPosition += bufferSize / 2;

    // Can't use mod because we're in double land.
    if (m_dBeatLength <= 0) {
        qDebug() << "ERROR: Calculated <= 0 samples per beat which is impossible.  Forcibly "
                 << "setting to about 124 bpm at 44.1Khz.";
        m_dBeatLength = 21338;
    }

    while (m_dClockPosition >= m_dBeatLength) {
        m_dClockPosition -= m_dBeatLength;
    }
}
