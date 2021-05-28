#include "engine/sync/internalclock.h"

#include <QtDebug>

#include "control/controllinpotmeter.h"
#include "control/controlobject.h"
#include "control/controlpushbutton.h"
#include "engine/sync/enginesync.h"
#include "moc_internalclock.cpp"
#include "preferences/usersettings.h"
#include "util/logger.h"

namespace {
const mixxx::Logger kLogger("InternalClock");
} // namespace

InternalClock::InternalClock(const QString& group, SyncableListener* pEngineSync)
        : m_group(group),
          m_pEngineSync(pEngineSync),
          m_mode(SYNC_NONE),
          m_iOldSampleRate(44100),
          m_dOldBpm(124.0),
          m_dBaseBpm(124.0),
          m_dBeatLength(m_iOldSampleRate * 60.0 / m_dOldBpm),
          m_dClockPosition(0) {
    // Pick a wide range (1 to 200) and allow out of bounds sets. This lets you
    // map a soft-takeover MIDI knob to the leader BPM. This also creates bpm_up
    // and bpm_down controls.
    // bpm_up / bpm_down steps by 1
    // bpm_up_small / bpm_down_small steps by 0.1
    m_pClockBpm.reset(
            new ControlLinPotmeter(ConfigKey(m_group, "bpm"), 1, 200, 1, 0.1, true));
    connect(m_pClockBpm.data(),
            &ControlObject::valueChanged,
            this,
            &InternalClock::slotBaseBpmChanged,
            Qt::DirectConnection);

    // The relative position between two beats in the range 0.0 ... 1.0
    m_pClockBeatDistance.reset(new ControlObject(ConfigKey(m_group, "beat_distance")));
    connect(m_pClockBeatDistance.data(), &ControlObject::valueChanged,
            this, &InternalClock::slotBeatDistanceChanged,
            Qt::DirectConnection);

    m_pSyncLeaderEnabled.reset(
            new ControlPushButton(ConfigKey(m_group, "sync_leader")));
    m_pSyncLeaderEnabled->setButtonMode(ControlPushButton::TOGGLE);
    m_pSyncLeaderEnabled->setStates(3);
    m_pSyncLeaderEnabled->connectValueChangeRequest(
            this, &InternalClock::slotSyncLeaderEnabledChangeRequest, Qt::DirectConnection);
    ControlDoublePrivate::insertAlias(ConfigKey(m_group, "sync_master"),
            ConfigKey(m_group, "sync_leader"));
}

InternalClock::~InternalClock() {
}

void InternalClock::setSyncMode(SyncMode mode) {
    // Syncable has absolutely no say in the matter. This is what EngineSync
    // requires. Bypass confirmation by using setAndConfirm.
    m_mode = mode;
    m_pSyncLeaderEnabled->setAndConfirm(SyncModeToLeaderLight(mode));
}

void InternalClock::notifyUniquePlaying() {
    // No action necessary.
}

void InternalClock::requestSync() {
    // TODO(owilliams): This should probably be how we reset the internal beat distance.
}

void InternalClock::slotSyncLeaderEnabledChangeRequest(double state) {
    SyncMode mode = m_mode;
    //Note: internal clock is always sync enabled
    if (state > 0.0) {
        if (mode == SYNC_LEADER_EXPLICIT) {
            // Already leader.
            return;
        }
        if (mode == SYNC_LEADER_SOFT) {
            // user request: make leader explicit
            m_mode = SYNC_LEADER_EXPLICIT;
            return;
        }
        if (mode == SYNC_NONE) {
            m_dBaseBpm = m_dOldBpm;
        }
        m_pEngineSync->requestSyncMode(this, SYNC_LEADER_EXPLICIT);
    } else {
        // Turning off leader goes back to follower mode.
        if (mode == SYNC_FOLLOWER) {
            // Already not leader.
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

void InternalClock::updateLeaderBeatDistance(double beatDistance) {
    if (kLogger.traceEnabled()) {
        kLogger.trace() << "InternalClock::updateLeaderBeatDistance" << beatDistance;
    }
    m_dClockPosition = beatDistance * m_dBeatLength;
    m_pClockBeatDistance->set(beatDistance);
    // Make sure followers have an up-to-date beat distance.
    m_pEngineSync->notifyBeatDistanceChanged(this, beatDistance);
}

double InternalClock::getBaseBpm() const {
    return m_dBaseBpm;
}

double InternalClock::getBpm() const {
    return m_pClockBpm->get();
}

void InternalClock::updateLeaderBpm(double bpm) {
    if (kLogger.traceEnabled()) {
        kLogger.trace() << "InternalClock::setBpm" << bpm;
    }
    if (bpm == 0) {
        return;
    }
    m_pClockBpm->set(bpm);
    updateBeatLength(m_iOldSampleRate, bpm);
}

void InternalClock::updateInstantaneousBpm(double bpm) {
    if (kLogger.traceEnabled()) {
        kLogger.trace() << "InternalClock::setInstantaneousBpm" << bpm;
    }
    // Do nothing.
    Q_UNUSED(bpm);
}

void InternalClock::notifyLeaderParamSource() {
}

void InternalClock::reinitLeaderParams(double beatDistance, double baseBpm, double bpm) {
    if (kLogger.traceEnabled()) {
        kLogger.trace() << "InternalClock::reinitLeaderParams" << beatDistance << baseBpm << bpm;
    }
    if (bpm <= 0.0 || baseBpm <= 0.0) {
        return;
    }
    m_dBaseBpm = baseBpm;
    updateLeaderBpm(bpm);
    updateLeaderBeatDistance(beatDistance);
}

void InternalClock::slotBaseBpmChanged(double baseBpm) {
    m_dBaseBpm = baseBpm;
    updateBeatLength(m_iOldSampleRate, m_dBaseBpm);
    if (!isSynchronized()) {
        return;
    }
    m_pEngineSync->notifyBaseBpmChanged(this, m_dBaseBpm);
}

void InternalClock::slotBeatDistanceChanged(double beatDistance) {
    if (beatDistance < 0.0 || beatDistance > 1.0) {
        return;
    }
    updateLeaderBeatDistance(beatDistance);
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
        qDebug() << "WARNING: Leader bpm reported to be zero, internal clock guessing 124bpm";
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
    updateLeaderBeatDistance(oldBeatDistance);
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

    double beatDistance = getBeatDistance();
    m_pClockBeatDistance->set(beatDistance);
    m_pEngineSync->notifyBeatDistanceChanged(this, beatDistance);
}
