#include <QtDebug>

#include "engine/sync/internalclock.h"

#include "engine/sync/enginesync.h"
#include "controlobject.h"
#include "controllinpotmeter.h"
#include "controlpushbutton.h"
#include "configobject.h"

InternalClock::InternalClock(const char* pGroup, SyncableListener* pEngineSync)
        : m_group(pGroup),
          m_pEngineSync(pEngineSync),
          m_mode(SYNC_NONE),
          m_iOldSampleRate(44100),
          m_dOldBpm(124.0),
          m_dBeatLength(m_iOldSampleRate * 60.0 / m_dOldBpm),
          m_dClockPosition(0) {
    // Pick a wide range (1 to 200) and allow out of bounds sets. This lets you
    // map a soft-takeover MIDI knob to the master BPM. This also creates bpm_up
    // and bpm_down controls.
    // bpm_up / bpm_down steps by 1
    // bpm_up_small / bpm_down_small steps by 0.1
    m_pClockBpm.reset(new ControlLinPotmeter(ConfigKey(m_group, "bpm"),
                                          1, 200, 1, 0.1, true));
    connect(m_pClockBpm.data(), SIGNAL(valueChanged(double)),
            this, SLOT(slotBpmChanged(double)),
            Qt::DirectConnection);

    m_pClockBeatDistance.reset(new ControlObject(ConfigKey(m_group, "beat_distance")));
    connect(m_pClockBeatDistance.data(), SIGNAL(valueChanged(double)),
            this, SLOT(slotBeatDistanceChanged(double)),
            Qt::DirectConnection);

    m_pSyncMasterEnabled.reset(
        new ControlPushButton(ConfigKey(pGroup, "sync_master")));
    m_pSyncMasterEnabled->setButtonMode(ControlPushButton::TOGGLE);
    m_pSyncMasterEnabled->connectValueChangeRequest(
        this, SLOT(slotSyncMasterEnabledChangeRequest(double)),
        Qt::DirectConnection);
}

InternalClock::~InternalClock() {
}

void InternalClock::notifySyncModeChanged(SyncMode mode) {
    // Syncable has absolutely no say in the matter. This is what EngineSync
    // requires. Bypass confirmation by using setAndConfirm.
    m_mode = mode;
    m_pSyncMasterEnabled->setAndConfirm(mode == SYNC_MASTER);
}

void InternalClock::notifyOnlyPlayingSyncable() {
    // No action necessary.
}

void InternalClock::requestSyncPhase() {
    // TODO(owilliams): This should probably be how we reset the internal beat distance.
}

void InternalClock::slotSyncMasterEnabledChangeRequest(double state) {
    bool currentlyMaster = getSyncMode() == SYNC_MASTER;

    if (state > 0.0) {
        if (currentlyMaster) {
            // Already master.
            return;
        }
        m_pEngineSync->requestSyncMode(this, SYNC_MASTER);
    } else {
        // Turning off master goes back to follower mode.
        if (!currentlyMaster) {
            // Already not master.
            return;
        }
        m_pEngineSync->requestSyncMode(this, SYNC_FOLLOWER);
    }
}

double InternalClock::getBeatDistance() const {
    if (m_dBeatLength <= 0) {
        qDebug() << "ERROR: InternalClock beat length should never be less than zero";
        return 0.0;
    }
    return m_dClockPosition / m_dBeatLength;
}

void InternalClock::setMasterBeatDistance(double beatDistance) {
    //qDebug() << "InternalClock::setBeatDistance" << beatDistance;
    m_dClockPosition = beatDistance * m_dBeatLength;
    m_pClockBeatDistance->set(beatDistance);
    // Make sure followers have an up-to-date beat distance.
    m_pEngineSync->notifyBeatDistanceChanged(this, beatDistance);
}

double InternalClock::getBaseBpm() const {
    return m_dOldBpm;
}

void InternalClock::setMasterBaseBpm(double bpm) {
    Q_UNUSED(bpm)
}

double InternalClock::getBpm() const {
    return m_pClockBpm->get();
}

void InternalClock::setMasterBpm(double bpm) {
    //qDebug() << "InternalClock::setBpm" << bpm;
    if (bpm == 0) {
        return;
    }
    m_pClockBpm->set(bpm);
    updateBeatLength(m_iOldSampleRate, bpm);
}

void InternalClock::setInstantaneousBpm(double bpm) {
    //qDebug() << "InternalClock::setInstantaneousBpm" << bpm;
    // Do nothing.
    Q_UNUSED(bpm);
}

void InternalClock::setMasterParams(double beatDistance, double baseBpm, double bpm) {
    Q_UNUSED(baseBpm)
    if (bpm == 0) {
        return;
    }
    setMasterBpm(bpm);
    setMasterBeatDistance(beatDistance);
}

void InternalClock::slotBpmChanged(double bpm) {
    updateBeatLength(m_iOldSampleRate, bpm);
    m_pEngineSync->notifyBpmChanged(this, bpm);
}

void InternalClock::slotBeatDistanceChanged(double beat_distance) {
    if (beat_distance < 0.0 || beat_distance > 1.0) {
        return;
    }
    setMasterBeatDistance(beat_distance);
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
        qDebug() << "WARNING: Master bpm reported to be zero, internal clock guessing 124bpm";
        m_dBeatLength = (sampleRate * 60.0) / 124.0;
        m_dOldBpm = 124.0;
    } else {
        m_dOldBpm = bpm;
        m_dBeatLength = (sampleRate * 60.0) / bpm;
        if (m_dBeatLength <= 0) {
            qDebug() << "WARNING: Tried to set samples per beat <=0";
            m_dBeatLength = sampleRate;
        }
    }

    m_iOldSampleRate = sampleRate;

    // Restore the old beat distance.
    setMasterBeatDistance(oldBeatDistance);
}

void InternalClock::onCallbackStart(int sampleRate, int bufferSize) {
    Q_UNUSED(sampleRate)
    Q_UNUSED(bufferSize)
    m_pEngineSync->notifyInstantaneousBpmChanged(this, getBpm());
}

void InternalClock::onCallbackEnd(int sampleRate, int bufferSize) {
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

    double beat_distance = getBeatDistance();
    m_pClockBeatDistance->set(beat_distance);
    m_pEngineSync->notifyBeatDistanceChanged(this, beat_distance);
}
