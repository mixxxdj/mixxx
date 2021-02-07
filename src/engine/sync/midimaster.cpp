#include "engine/sync/midimaster.h"

#include <QtDebug>

#include "control/control.h"
#include "control/controllinpotmeter.h"
#include "control/controlpushbutton.h"
#include "controllers/midi/midisourceclock.h"
#include "engine/sync/enginesync.h"
#include "preferences/configobject.h"
#include "util/time.h"

MidiMasterClock::MidiMasterClock(
        const QString& group, SyncableListener* pEngineSync)
        : m_group(group),
          m_pEngineSync(pEngineSync),
          m_pSyncMasterEnabled(std::make_unique<ControlPushButton>(
                  ConfigKey(group, "sync_master"))),
          m_pMidiSourceClockBpm(
                  std::make_unique<ControlObject>(ConfigKey(group, "bpm"))),
          m_pMidiSourceClockLastBeatTime(std::make_unique<ControlObject>(
                  ConfigKey(group, "last_beat_time"))),
          m_pMidiSourceClockBeatDistance(std::make_unique<ControlObject>(
                  ConfigKey(group, "beat_distance"))),
          m_pMidiSourceClockRunning(
                  std::make_unique<ControlPushButton>(ConfigKey(group, "run"))),
          m_pMidiSourceClockSyncAdjust(std::make_unique<ControlLinPotmeter>(
                  ConfigKey(group, "sync_adjust"),
                  -.5,
                  .5,
                  0.1,
                  0.01,
                  /*allow oob*/ true)),
          m_mode(SYNC_NONE) {
    m_pSyncMasterEnabled->setButtonMode(ControlPushButton::TOGGLE);
    m_pSyncMasterEnabled->connectValueChangeRequest(
            this, &MidiMasterClock::slotSyncMasterEnabledChangeRequest, Qt::DirectConnection);
}

MidiMasterClock::~MidiMasterClock(){};

void MidiMasterClock::notifySyncModeChanged(SyncMode mode) {
    // Syncable has absolutely no say in the matter. This is what EngineSync
    // requires. Bypass confirmation by using setAndConfirm.
    m_mode = mode;
    m_pSyncMasterEnabled->setAndConfirm(mode == SYNC_MASTER_SOFT);
}

void MidiMasterClock::notifyOnlyPlayingSyncable() {
    // No action necessary.
}

void MidiMasterClock::requestSync() {
    // TODO: Implement this?
}

void MidiMasterClock::requestSyncPhase() {
    // TODO: maybe tell MidiSourceClock to reset which tick is the beat tick??
    // but really, it's a read-only clock.
}

bool MidiMasterClock::isPlaying() const {
    // midi running / not running state
    return m_pMidiSourceClockRunning->get();
}

void MidiMasterClock::slotSyncMasterEnabledChangeRequest(double state) {
    bool currentlyMaster = getSyncMode() == SYNC_MASTER_SOFT;

    if (state > 0.0) {
        if (currentlyMaster) {
            // Already master.
            return;
        }
        m_pEngineSync->requestSyncMode(this, SYNC_MASTER_SOFT);
    } else {
        // TODO: midi follower (clock out?)
        m_pEngineSync->requestSyncMode(this, SYNC_NONE);
    }
}

double MidiMasterClock::getBeatDistance() const {
    const mixxx::Duration last_beat = mixxx::Duration::fromNanos(
            static_cast<qint64>(m_pMidiSourceClockLastBeatTime->get()));
    double raw_percent = MidiSourceClock::beatFraction(
            last_beat, mixxx::Time::elapsed(), m_pMidiSourceClockBpm->get());
    raw_percent += m_pMidiSourceClockSyncAdjust->get();
    // Fix beat loop-around.
    return raw_percent - floor(raw_percent);
}

void MidiMasterClock::setMasterBeatDistance(double beatDistance) {
    // Midi master is read-only.
    Q_UNUSED(beatDistance);
}

double MidiMasterClock::getBaseBpm() const {
    return m_pMidiSourceClockBpm->get();
}

void MidiMasterClock::setMasterBaseBpm(double bpm) {
    // Midi master is read-only.
    Q_UNUSED(bpm)
}

double MidiMasterClock::getBpm() const {
    return m_pMidiSourceClockBpm->get();
}

void MidiMasterClock::setMasterBpm(double bpm) {
    // Midi master is read-only.
    Q_UNUSED(bpm);
}

void MidiMasterClock::setInstantaneousBpm(double bpm) {
    // Midi master is read-only.
    Q_UNUSED(bpm);
}

void MidiMasterClock::setMasterParams(double beatDistance, double baseBpm, double bpm) {
    // Midi master is read-only.
    Q_UNUSED(beatDistance);
    Q_UNUSED(baseBpm);
    Q_UNUSED(bpm);
}

void MidiMasterClock::onCallbackStart(int sampleRate, int bufferSize) {
    Q_UNUSED(sampleRate)
    Q_UNUSED(bufferSize)
    double bpm = getBpm();
    m_pEngineSync->notifyInstantaneousBpmChanged(this, bpm);
    m_pEngineSync->notifyBpmChanged(this, bpm);
}

void MidiMasterClock::onCallbackEnd(int sampleRate, int bufferSize) {
    Q_UNUSED(sampleRate)
    Q_UNUSED(bufferSize)
    double beat_distance = getBeatDistance();
    m_pMidiSourceClockBeatDistance->set(beat_distance);
    m_pEngineSync->notifyBeatDistanceChanged(this, beat_distance);
}
