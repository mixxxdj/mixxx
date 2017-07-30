#include "track/playcounter.h"


void PlayCounter::setPlayedAndUpdateTimesPlayed(bool bPlayed) {
    // This should never happen if the play counter is used
    // as intended! But since this class provides independent
    // setters for both members we need to check and re-establish
    // the class invariant just in case.
    VERIFY_OR_DEBUG_ASSERT(!m_bPlayed || (0 < m_iTimesPlayed)) {
        // Make sure that the number of times played does
        // not become negative!
        m_iTimesPlayed = 1;
    }
    if (bPlayed) {
        // Increment always
        ++m_iTimesPlayed;
    } else if (m_bPlayed) {
        // Decrement only if already marked as played
        DEBUG_ASSERT(0 < m_iTimesPlayed);
        --m_iTimesPlayed;
    }
    m_bPlayed = bPlayed;
}

bool operator==(const PlayCounter& lhs, const PlayCounter& rhs) {
    return (lhs.getTimesPlayed() == rhs.getTimesPlayed())
            && (lhs.isPlayed() == rhs.isPlayed());
}
