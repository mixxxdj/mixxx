#pragma once

#include <QMutex>
#include <QObject>
#include <memory>

#include "util/db/dbid.h"

class BpmSegments;
using BpmSegmentsPointer = std::shared_ptr<BpmSegments>;

class BpmSegments : public QObject {
    Q_OBJECT

  public:
    static constexpr double kNoPosition = -1.0;

    BpmSegments() = delete;

    // Load from database
    BpmSegments(
            DbId id,
            double startTime,
            double duration,
            double bpmStart,
            double bpmEnd,
            double rangeStart,
            double rangeEnd,
            const QString& type);

    // Create new segment
    BpmSegments(
            double startTime,
            double duration,
            double bpmStart,
            double bpmEnd,
            double rangeStart,
            double rangeEnd,
            const QString& type);

    ~BpmSegments() override = default;

    bool isDirty() const;
    DbId getId() const;

    double getStartTime() const;
    void setStartTime(double startTime);

    double getDuration() const;
    void setDuration(double duration);

    double getBpmStart() const;
    void setBpmStart(double bpmStart);

    double getBpmEnd() const;
    void setBpmEnd(double bpmEnd);

    double getRangeStart() const;
    void setRangeStart(double rangeStart);

    double getRangeEnd() const;
    void setRangeEnd(double rangeEnd);

    QString getType() const;
    void setType(const QString& type);

  signals:
    void updated();

  private:
    void setDirty(bool dirty);
    void setId(DbId dbId);

    mutable QMutex m_mutex;

    bool m_bDirty;
    DbId m_dbId;
    double m_startTime;
    double m_duration;
    double m_bpmStart;
    double m_bpmEnd;
    double m_rangeStart;
    double m_rangeEnd;
    QString m_type;

    friend class Track;
    friend class SegmentsDAO;
};

class BpmSegmentPointer : public std::shared_ptr<BpmSegments> {
  public:
    BpmSegmentPointer() = default;
    explicit BpmSegmentPointer(BpmSegments* pSegment);

  private:
    static void deleteLater(BpmSegments* pSegment);
};
