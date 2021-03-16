#include "controllers/midi/midibeatclock.h"

namespace mixxx {

MidiBeatClock::MidiBeatClock(const QString& group)
        : ControllerSyncable(group) {
    // Pick a wide range (1 to 200) and allow out of bounds sets. This lets you
    // map a soft-takeover MIDI knob to the master BPM. This also creates bpm_up
    // and bpm_down controls.
    // bpm_up / bpm_down steps by 1
    // bpm_up_small / bpm_down_small steps by 0.1
    m_pClockBpm.reset(new ControlObject(ConfigKey(m_group, "bpm")));
    m_pClockBpm->setReadOnly();
    // The relative position between two beats in the range 0.0 ... 1.0
    m_pClockBeatDistance.reset(new ControlObject(ConfigKey(m_group, "beat_distance")));
    m_pClockBeatDistance->setReadOnly();
}

void MidiBeatClock::receive(unsigned char status, Duration timestamp) {
    MidiBeatClockReceiver::receive(status, timestamp);
    m_pClockBpm->setAndConfirm(bpm().getValue());
    m_pClockBeatDistance->setAndConfirm(beatDistance());
}

void MidiBeatClock::setMasterBeatDistance(double beatDistance) {
    Q_UNUSED(beatDistance);
};

void MidiBeatClock::setMasterBpm(double bpm) {
    Q_UNUSED(bpm);
};

void MidiBeatClock::setMasterParams(double beatDistance, double baseBpm, double bpm) {
    Q_UNUSED(beatDistance);
    Q_UNUSED(baseBpm);
    Q_UNUSED(bpm);
};

void MidiBeatClock::setInstantaneousBpm(double bpm) {
    Q_UNUSED(bpm);
};

} // namespace mixxx
