#ifndef MIXXX_PLAYCOUNTER_H
#define MIXXX_PLAYCOUNTER_H


class PlayCounter {
public:
    explicit PlayCounter(int timesPlayed = 0):
        m_iTimesPlayed(timesPlayed),
        m_bPlayed(false) {
    }

    // Set number of times the track has been played
    void setTimesPlayed(int timesPlayed) {
        m_iTimesPlayed = timesPlayed;
    }
    // Resets the play count back to 0
    void resetTimesPlayed() {
        setTimesPlayed(0);
    }
    // Return number of times the track has been played
    int getTimesPlayed() const {
        return m_iTimesPlayed;
    }

    // Set played status for the current session without affecting
    // the play count
    void setPlayed(bool bPlayed = true) {
        m_bPlayed = bPlayed;
    }
    // Reset played status for the current session without affecting
    // the play count
    void resetPlayed() {
        setPlayed(false);
    }
    // Returns true if track has been played during this session
    bool isPlayed() const {
        return m_bPlayed;
    }

    // Set played status and increment or decrement the play count accordingly.
    void updatePlayed(bool bPlayed = true);

private:
    int m_iTimesPlayed;
    bool m_bPlayed;
};

bool operator==(const PlayCounter& lhs, const PlayCounter& rhs);

inline bool operator!=(const PlayCounter& lhs, const PlayCounter& rhs) {
    return !(lhs == rhs);
}

#endif // MIXXX_PLAYCOUNTER_H
