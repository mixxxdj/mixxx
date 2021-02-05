#pragma once

#include <QObject>

#include "library/trackset/crate/crate.h"
#include "util/duration.h"

// A crate with aggregated track properties (total count + duration)
class CrateSummary : public Crate {
  public:
    CrateSummary(CrateId id = CrateId())
            : Crate(id),
              m_trackCount(0),
              m_trackDuration(0.0) {
    }

    ~CrateSummary(){};

    bool isValid() {
        return getId().isValid();
    }

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

class CrateSummaryWrapper : public QObject {
    Q_OBJECT
  public:
    Q_PROPERTY(uint trackCount READ getTrackCount WRITE setTrackCount)
    Q_PROPERTY(double trackDuration READ getTrackDuration WRITE setTrackDuration)
    Q_PROPERTY(QString name READ getName WRITE setName)

    CrateSummaryWrapper(CrateSummary& summary);
    ~CrateSummaryWrapper();

    QString getName() const {
        return m_summary.getName();
    };
    void setName(const QString& name) {
        m_summary.setName(name);
    };

    // The number of all tracks in this crate
    uint getTrackCount() const {
        return m_summary.getTrackCount();
    }
    void setTrackCount(uint trackCount) {
        m_summary.setTrackCount(trackCount);
    }

    // The total duration (in seconds) of all tracks in this crate
    double getTrackDuration() const {
        return m_summary.getTrackDuration();
    }
    void setTrackDuration(double trackDuration) {
        m_summary.setTrackDuration(trackDuration);
    }

  private:
    CrateSummary m_summary;
};
