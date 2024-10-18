#pragma once

#include "library/trackset/smarties/smarties.h"
#include "util/duration.h"

// A smarties with aggregated track properties (total count + duration)
class SmartiesSummary : public Smarties {
  public:
    explicit SmartiesSummary(SmartiesId id = SmartiesId())
            : Smarties(id),
              m_trackCount(0),
              m_trackDuration(0.0) {
    }
    ~SmartiesSummary() override = default;

    // The number of all tracks in this smarties
    uint getTrackCount() const {
        return m_trackCount;
    }
    void setTrackCount(uint trackCount) {
        m_trackCount = trackCount;
    }

    // The total duration (in seconds) of all tracks in this smarties
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
