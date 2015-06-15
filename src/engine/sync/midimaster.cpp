#include <QtDebug>

#include "engine/sync/midimaster.h"

#include "engine/sync/enginesync.h"
#include "controllers/midi/midiclock.h"
#include "controlobject.h"
#include "controlpushbutton.h"
#include "configobject.h"
#include "util/time.h"

MidiMasterClock::MidiMasterClock(const char* pGroup, SyncableListener* pEngineSync)
        : m_group(pGroup),
          m_pEngineSync(pEngineSync),
          m_mode(SYNC_NONE) {
    m_pMidiClockBpm.reset(new ControlObject(ConfigKey(pGroup, "bpm")));
    m_pMidiClockLastBeatTime.reset(
            new ControlObject(ConfigKey(pGroup, "last_beat_time")));

    m_pMidiClockRunning.reset(
            new ControlPushButton(ConfigKey(pGroup, "play")));

    m_pSyncMasterEnabled.reset(
            new ControlPushButton(ConfigKey(pGroup, "sync_master")));
    m_pSyncMasterEnabled->setButtonMode(ControlPushButton::TOGGLE);
    m_pSyncMasterEnabled->connectValueChangeRequest(
            this, SLOT(slotSyncMasterEnabledChangeRequest(double)),
            Qt::DirectConnection);
}

MidiMasterClock::~MidiMasterClock() {
};

void MidiMasterClock::notifySyncModeChanged(SyncMode mode) {
    // Syncable has absolutely no say in the matter. This is what EngineSync
    // requires. Bypass confirmation by using setAndConfirm.
    m_mode = mode;
    m_pSyncMasterEnabled->setAndConfirm(mode == SYNC_MASTER);
}

void MidiMasterClock::notifyOnlyPlayingSyncable() {
    // No action necessary.
}

void MidiMasterClock::requestSyncPhase() {
    // TODO: maybe tell midiclock to reset which tick is the beat tick??
    // but really, it's a read-only clock.
}

void MidiMasterClock::slotSyncMasterEnabledChangeRequest(double state) {
    bool currentlyMaster = getSyncMode() == SYNC_MASTER;

    if (state > 0.0) {
        if (currentlyMaster) {
            // Already master.
            return;
        }
        m_pEngineSync->requestSyncMode(this, SYNC_MASTER);
    } else {
        // TODO: midi follower (clock out?)
        m_pEngineSync->requestSyncMode(this, SYNC_NONE);
    }
}

double MidiMasterClock::getBeatDistance() const {
    qint64 last_beat = static_cast<qint64>(m_pMidiClockLastBeatTime->get());
    return MidiClock::beatPercentage(last_beat, Time::elapsed(),
                                     m_pMidiClockBpm->get());
}

void MidiMasterClock::setMasterBeatDistance(double beatDistance) {
    // Midi master is read-only.
    Q_UNUSED(beatDistance);
}

double MidiMasterClock::getBaseBpm() const {
    return m_pMidiClockBpm->get();
}

void MidiMasterClock::setMasterBaseBpm(double bpm) {
    // Midi master is read-only.
    Q_UNUSED(bpm)
}

double MidiMasterClock::getBpm() const {
    return m_pMidiClockBpm->get();
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
    m_pEngineSync->notifyInstantaneousBpmChanged(this, getBpm());
}

void MidiMasterClock::onCallbackEnd(int sampleRate, int bufferSize) {
    Q_UNUSED(sampleRate)
    Q_UNUSED(bufferSize)
    m_pEngineSync->notifyBeatDistanceChanged(this, getBeatDistance());
}

