#ifndef MIXXX_PLAYCOUNTER_H
#define MIXXX_PLAYCOUNTER_H

#include "control/controlproxy.h"
#include "util/assert.h"

#include "util/memory.h"

// Counts the total number of times a track has been played
// and if the track has been played during the current session.
class PlayCounter {
public:
    explicit PlayCounter(int timesPlayed = 0);

    // Sets total number of times a track has been played
    void setTimesPlayed(int iTimesPlayed) {
        DEBUG_ASSERT(0 <= iTimesPlayed);
        m_iTimesPlayed = iTimesPlayed;
    }
    // Returns the total number of times a track has been played
    int getTimesPlayed() const {
        return m_iTimesPlayed;
    }

    // Sets the played status of a track for the current session
    // without affecting the play count.
    void setPlayed(bool bPlayed = true) {
        m_bPlayed = bPlayed;
    }
    // Returns true if track has been played during the current session
    bool isPlayed() const {
        return m_bPlayed;
    }

    // Sets the played status of a track for the current session and
    // increments or decrements the total play count accordingly.
    void setPlayedAndUpdateTimesPlayed(bool bPlayed = true);

private:
    int m_iTimesPlayed;
    bool m_bPlayed;
    std::shared_ptr<ControlProxy> m_pPracticemodeEnabled;
};

bool operator==(const PlayCounter& lhs, const PlayCounter& rhs);

inline
bool operator!=(const PlayCounter& lhs, const PlayCounter& rhs) {
    return !(lhs == rhs);
}

inline
QDebug operator<<(QDebug dbg, const PlayCounter& arg) {
    return dbg << "played =" << arg.isPlayed() << "/" << "count =" << arg.getTimesPlayed();
}
#endif // MIXXX_PLAYCOUNTER_H
