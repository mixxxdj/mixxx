#include <atomic>

#include "track/bpm.h"
#include "util/duration.h"

namespace mixxx {

class MidiBeatClockReceiver {
  public:
    static constexpr int kPulsesPerQuarterNote = 24;

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
    std::atomic<Bpm> m_bpm;
    std::atomic<bool> m_isPlaying;
    std::atomic<int> m_clockTickIndex;

    Duration m_lastTimestamp;
    Duration m_intervalRingBuffer[kPulsesPerQuarterNote];
};

} // namespace mixxx
