#include "controllers/midi/midibeatclock.h"

namespace mixxx {

MidiBeatClock::MidiBeatClock(const QString& group)
        : m_group(group),
          m_syncMode(SYNC_INVALID) {
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
