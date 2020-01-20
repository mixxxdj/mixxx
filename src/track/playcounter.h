#pragma once

#include "util/assert.h"


// Counts the total number of times a track has been played
// and if the track has been played during the current session.
class PlayCounter {
public:
    explicit PlayCounter(int timesPlayed = 0):
        m_iTimesPlayed(timesPlayed),
        m_bPlayed(false) {
    }

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
