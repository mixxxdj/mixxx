#include "track/bpm.h"
#include "util/duration.h"

namespace mixxx {

class MidiBeatClockReceiver {
  public:
    MidiBeatClockReceiver();
    static bool canReceiveMidiStatus(unsigned char status);
    void receive(unsigned char status, Duration timestamp);

    bool isPlaying() const {
        return m_isPlaying;
    }

    Bpm bpm() const {
        return m_bpm;
    }

    double beatDistance() const;

  private:
    Bpm m_bpm;
    bool m_isPlaying;
    Duration m_lastTimestamp;
    int m_clockTickIndex;
};

} // namespace mixxx
