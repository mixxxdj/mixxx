#include "track/playcounter.h"


void PlayCounter::updatePlayed(bool bPlayed) {
    if (bPlayed) {
        // increment the play count when play during the current session
        ++m_iTimesPlayed;
        m_bPlayed = true;
    } else if (m_bPlayed) {
        // decrement the play count, but only if already played during
        // the current session
        if (0 < m_iTimesPlayed) {
            --m_iTimesPlayed;
        }
        m_bPlayed = false;
    }
}

bool operator==(const PlayCounter& lhs, const PlayCounter& rhs) {
    return (lhs.getTimesPlayed() == rhs.getTimesPlayed())
            && (lhs.isPlayed() == rhs.isPlayed());
}
