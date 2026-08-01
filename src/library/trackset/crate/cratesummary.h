#pragma once

#include "library/trackset/crate/crate.h"
#include "util/duration.h"

// A crate with aggregated track properties (total count + duration)
class CrateSummary : public Crate {
  public:
    explicit CrateSummary(CrateId id = CrateId())
            : Crate(id),
              m_trackCount(0),
              m_trackDuration(0.0) {
    }
    ~CrateSummary() override = default;

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

    // The full path of this crate, formatted as
    // "Root crate name / Parent crate name / Crate name".
    QString getFullPath() const {
        return m_fullPath;
    }
    void setFullPath(const QString& fullPath) {
        m_fullPath = fullPath;
    }

    // The full path of this crate's parent folder,
    // formatted as "Root crate name / Grandparent crate name / Parent crate name".
    QString getFolderPath() const {
        return m_folderPath;
    }
    void setFolderPath(const QString& folderPath) {
        m_folderPath = folderPath;
    }

  private:
    uint m_trackCount;
    double m_trackDuration;
    QString m_fullPath;
    QString m_folderPath;
};
