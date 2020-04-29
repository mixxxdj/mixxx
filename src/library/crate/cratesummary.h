#ifndef MIXXX_CRATESUMMARY_H
#define MIXXX_CRATESUMMARY_H


#include "library/crate/crate.h"

#include "util/duration.h"


// A crate with aggregated track properties (total count + duration)
class CrateSummary: public Crate {
public:
    explicit CrateSummary(CrateId id = CrateId())
        : Crate(id),
          m_trackCount(0),
          m_trackDuration(0.0) {
    }
    ~CrateSummary() override {}

    // The number of all tracks in this crate
    uint getTrackCount() const {
        return m_trackCount;
    }
    void setTrackCount(uint trackCount) {
        m_trackCount = trackCount;
    }

    // The total duration (in seconds) of all tracks in this crate
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


#endif // MIXXX_CRATESUMMARY_H
