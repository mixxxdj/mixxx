#pragma once

#include <QDateTime>
#include <QMetaType>
#include <QtDebug>

#include "util/assert.h"

/// Counts the total number of times a track has been played
/// and if the track has been played during the current session.
class PlayCounter final {
  public:
    explicit PlayCounter(int timesPlayed = 0)
            : m_timesPlayed(timesPlayed),
              m_playedLatch(false) {
    }

    /// Set the total number of times a track has been played
    /// while keeping the played flag and time stamp as is.
    void setTimesPlayed(int timesPlayed) {
        DEBUG_ASSERT(timesPlayed >= 0);
        m_timesPlayed = timesPlayed;
    }

    /// Returns the total number of times a track has been played
    int getTimesPlayed() const {
        return m_timesPlayed;
    }

    /// Sets the (last) played at time stamp of a track without
    /// affecting the play count.
    void setLastPlayedAt(const QDateTime& lastPlayedAt) {
        m_lastPlayedAt = lastPlayedAt;
    }

    /// Returns the actual time at which the track has been played
    /// during this session.
    ///
    /// Note: Not stored persistently, so might be null or invalid
    /// even if the corresponding boolean flags is true!
    const QDateTime& getLastPlayedAt() const {
        return m_lastPlayedAt;
    }

    /// Returns true if track is considered as played during the current session.
    ///
    /// This latch overrides the last played time stamp, which could never
    /// be reset.
    bool isPlayed() const {
        return m_playedLatch;
    }

    /// Sets or resets the played latch.
    ///
    /// The latch is also set implicitly to true when updating the
    /// played at time stamp. It overrides the played at time stamp,
    /// which could never be reset.
    void setPlayedLatch(bool setPlayed) {
        m_playedLatch = setPlayed;
    }

    /// Resets the played latch.
    void resetPlayedLatch() {
        setPlayedLatch(false);
    }

    /// Sets the played status of a track for the current session but
    /// keeps the total play count.
    void triggerLastPlayedNow() {
        setLastPlayedNow();
        setPlayedLatch(true);
    }

    /// Sets the played status of a track for the current session and
    /// increments or decrements the total play count accordingly.
    void updateLastPlayedNowAndTimesPlayed(bool setPlayed);

  private:
    void incTimesPlayed() {
        DEBUG_ASSERT(m_timesPlayed >= 0);
        ++m_timesPlayed;
    }
    void decTimesPlayed() {
        VERIFY_OR_DEBUG_ASSERT(m_timesPlayed > 0) {
            // Prevent invalid state
            m_timesPlayed = 1;
        }
        --m_timesPlayed;
    }
    void setLastPlayedNow() {
        setLastPlayedAt(QDateTime::currentDateTime());
    }

    int m_timesPlayed;
    QDateTime m_lastPlayedAt;
    bool m_playedLatch;
};

Q_DECLARE_METATYPE(PlayCounter)

bool operator==(const PlayCounter& lhs, const PlayCounter& rhs);

inline bool operator!=(const PlayCounter& lhs, const PlayCounter& rhs) {
    return !(lhs == rhs);
}

QDebug operator<<(QDebug dbg, const PlayCounter& arg);
