#include "track/playcounter.h"

void PlayCounter::updateLastPlayedNowAndTimesPlayed(bool setPlayed) {
    if (setPlayed) {
        // Increment unconditionally
        incTimesPlayed();
        setLastPlayedNow();
    } else if (isPlayed()) {
        // Decrement only if already marked as played during
        // the current session. Keep the last played at time
        // stamp!
        decTimesPlayed();
    }
    setPlayedFlag(setPlayed);
}

bool operator==(const PlayCounter& lhs, const PlayCounter& rhs) {
    return lhs.getTimesPlayed() == rhs.getTimesPlayed() &&
            lhs.getLastPlayedAt() == rhs.getLastPlayedAt() &&
            lhs.isPlayed() == rhs.isPlayed();
}

QDebug operator<<(QDebug dbg, const PlayCounter& arg) {
    return dbg << "timesPlayed =" << arg.getTimesPlayed()
               << '/'
               << "lastPlayedAt =" << arg.getLastPlayedAt()
               << '/'
               << "isPlayed =" << arg.isPlayed();
}
