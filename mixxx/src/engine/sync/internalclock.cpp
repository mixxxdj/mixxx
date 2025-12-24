#include "engine/sync/internalclock.h"

#include <QtDebug>

#include "control/controllinpotmeter.h"
#include "control/controlobject.h"
#include "control/controlpushbutton.h"
#include "moc_internalclock.cpp"
#include "util/logger.h"
#include "util/math.h"

namespace {
const mixxx::Logger kLogger("InternalClock");
constexpr mixxx::Bpm kDefaultBpm(124.0);
} // namespace

InternalClock::InternalClock(const QString& group, SyncableListener* pEngineSync)
        : m_group(group),
          m_pEngineSync(pEngineSync),
          m_mode(SyncMode::None),
          m_oldSampleRate(mixxx::audio::SampleRate{44100}),
          m_oldBpm(kDefaultBpm),
          m_baseBpm(kDefaultBpm),
          m_dBeatLength(m_oldSampleRate * 60.0 / m_oldBpm.value()),
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
            &InternalClock::slotBpmChanged,
            Qt::DirectConnection);

    // The relative position between two beats in the range 0.0 ... 1.0
    m_pClockBeatDistance.reset(new ControlObject(ConfigKey(m_group, "beat_distance")));
    connect(m_pClockBeatDistance.data(), &ControlObject::valueChanged,
            this, &InternalClock::slotBeatDistanceChanged,
            Qt::DirectConnection);

    m_pSyncLeaderEnabled.reset(
            new ControlPushButton(ConfigKey(m_group, "sync_leader")));
    m_pSyncLeaderEnabled->setBehavior(mixxx::control::ButtonMode::Toggle, 3);
    m_pSyncLeaderEnabled->connectValueChangeRequest(
            this, &InternalClock::slotSyncLeaderEnabledChangeRequest, Qt::DirectConnection);
    m_pSyncLeaderEnabled->addAlias(ConfigKey(m_group, QStringLiteral("sync_master")));
}

InternalClock::~InternalClock() {
}

void InternalClock::setSyncMode(SyncMode mode) {
    // Syncable has absolutely no say in the matter. This is what EngineSync
    // requires. Bypass confirmation by using setAndConfirm.
    m_mode = mode;
    m_pSyncLeaderEnabled->setAndConfirm(static_cast<double>(SyncModeToLeaderLight(mode)));
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
        if (mode == SyncMode::LeaderExplicit) {
            // Already leader.
            return;
        }
        if (mode == SyncMode::LeaderSoft) {
            // user request: make leader explicit
            m_mode = SyncMode::LeaderExplicit;
            return;
        }
        if (mode == SyncMode::None) {
            m_baseBpm = m_oldBpm;
        }
        m_pEngineSync->requestSyncMode(this, SyncMode::LeaderExplicit);
    } else {
        // Turning off leader goes back to follower mode.
        if (mode == SyncMode::Follower) {
            // Already not leader.
            return;
        }
        m_pEngineSync->requestSyncMode(this, SyncMode::Follower);
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
        kLogger.trace() << "updateLeaderBeatDistance" << beatDistance;
    }
    m_dClockPosition = beatDistance * m_dBeatLength;
    m_pClockBeatDistance->set(beatDistance);
    // Make sure followers have an up-to-date beat distance.
    m_pEngineSync->notifyBeatDistanceChanged(this, beatDistance);
}

mixxx::Bpm InternalClock::getBaseBpm() const {
    return m_baseBpm;
}

mixxx::Bpm InternalClock::getBpm() const {
    return mixxx::Bpm(m_pClockBpm->get());
}

void InternalClock::updateLeaderBpm(mixxx::Bpm bpm) {
    if (kLogger.traceEnabled()) {
        kLogger.trace() << "setBpm" << bpm;
    }
    if (!bpm.isValid()) {
        return;
    }
    m_pClockBpm->set(bpm.value());
    updateBeatLength(m_oldSampleRate, bpm);
}

void InternalClock::updateInstantaneousBpm(mixxx::Bpm bpm) {
    if (kLogger.traceEnabled()) {
        kLogger.trace() << "setInstantaneousBpm" << bpm;
    }
    // Do nothing.
    Q_UNUSED(bpm);
}

void InternalClock::notifyLeaderParamSource() {
}

void InternalClock::reinitLeaderParams(double beatDistance, mixxx::Bpm baseBpm, mixxx::Bpm bpm) {
    if (kLogger.traceEnabled()) {
        kLogger.trace() << "reinitLeaderParams" << beatDistance << baseBpm << bpm;
    }
    if (!bpm.isValid() || !baseBpm.isValid()) {
        return;
    }
    m_baseBpm = baseBpm;
    updateLeaderBpm(bpm);
    updateLeaderBeatDistance(beatDistance);
}

void InternalClock::slotBpmChanged(double bpm) {
    m_baseBpm = mixxx::Bpm(bpm);
    updateBeatLength(m_oldSampleRate, m_baseBpm);
    if (!isSynchronized()) {
        return;
    }
    // The internal clock doesn't have a rate slider, so treat
    // "base" bpm changes as rate changes -- this means the change will be
    // reflected in all synced decks.
    m_pEngineSync->notifyRateChanged(this, m_baseBpm);
}

void InternalClock::slotBeatDistanceChanged(double beatDistance) {
    if (beatDistance < 0.0 || beatDistance > 1.0) {
        return;
    }
    updateLeaderBeatDistance(beatDistance);
}

void InternalClock::updateBeatLength(mixxx::audio::SampleRate sampleRate, mixxx::Bpm bpm) {
    if (m_oldSampleRate == sampleRate && bpm == m_oldBpm) {
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

    if (!bpm.isValid()) {
        qDebug() << "WARNING: Leader bpm reported to be zero, internal clock guessing 124bpm";
        m_dBeatLength = (sampleRate * 60.0) / 124.0;
        m_oldBpm = kDefaultBpm;
    } else {
        m_oldBpm = bpm;
        m_dBeatLength = (sampleRate * 60.0) / bpm.value();
        if (m_dBeatLength <= 0) {
            qDebug() << "WARNING: Tried to set samples per beat <=0";
            m_dBeatLength = sampleRate;
        }
    }

    m_oldSampleRate = sampleRate;

    // Restore the old beat distance.
    updateLeaderBeatDistance(oldBeatDistance);
}

void InternalClock::onCallbackStart(mixxx::audio::SampleRate sampleRate, std::size_t bufferSize) {
    Q_UNUSED(sampleRate)
    Q_UNUSED(bufferSize)
    m_pEngineSync->notifyInstantaneousBpmChanged(this, getBpm());
}

void InternalClock::onCallbackEnd(mixxx::audio::SampleRate sampleRate, std::size_t bufferSize) {
    updateBeatLength(sampleRate, getBpm());

    // stereo samples, so divide by 2
    m_dClockPosition += bufferSize / 2;

    // Can't use mod because we're in double land.
    if (m_dBeatLength <= 0) {
        qDebug() << "ERROR: Calculated <= 0 samples per beat which is impossible.  Forcibly "
                 << "setting to about 124 bpm at 44.1Khz.";
        m_dBeatLength = 21338;
    }

    m_dClockPosition = fmod(m_dClockPosition, m_dBeatLength);
    double beatDistance = getBeatDistance();
    m_pClockBeatDistance->set(beatDistance);
    m_pEngineSync->notifyBeatDistanceChanged(this, beatDistance);
}
