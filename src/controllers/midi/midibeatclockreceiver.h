#include "track/bpm.h"
#include "util/duration.h"

namespace mixxx {

class MidiBeatClockReceiver {
  public:
    MidiBeatClockReceiver();
    void receive(unsigned char status, Duration timestamp);

    bool isPlaying() const {
        return m_isPlaying;
    }

    Bpm bpm() const {
        return m_bpm;
    }

  private:
    Bpm m_bpm;
    bool m_isPlaying;
};

} // namespace mixxx
