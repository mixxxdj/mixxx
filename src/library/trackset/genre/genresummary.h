#pragma once

#include "library/trackset/genre/genre.h"
#include "util/duration.h"

// A Genre with aggregated track properties (total count + duration)
class GenreSummary : public Genre {
  public:
    explicit GenreSummary(GenreId id = GenreId())
            : Genre(id),
              m_trackCount(0),
              m_trackDuration(0.0) {
    }
    ~GenreSummary() override = default;

    // The number of all tracks in this Genre
    uint getTrackCount() const {
        return m_trackCount;
    }
    void setTrackCount(uint trackCount) {
        m_trackCount = trackCount;
    }

    // The total duration (in seconds) of all tracks in this Genre
    double getTrackDuration() const {
        return m_trackDuration;
    }
    void setTrackDuration(double trackDuration) {
        m_trackDuration = trackDuration;
    }
    // Returns the duration formatted as a string H:MM:SS
    QString getTrackDurationText() const {
        return mixxx::Duration::formatTime(getTrackDuration(), mixxx::Duration::Precision::SECONDS);
    }

  private:
    uint m_trackCount;
    double m_trackDuration;
};
