#include "controllers/midi/midibeatclockreceiver.h"

namespace mixxx {

MidiBeatClockReceiver::MidiBeatClockReceiver()
        : m_bpm(Bpm::kValueUndefined),
          m_isPlaying(false) {
}

void MidiBeatClockReceiver::receive(unsigned char status, Duration timestamp) {
    Q_UNUSED(status);
    Q_UNUSED(timestamp);
};

} // namespace mixxx
