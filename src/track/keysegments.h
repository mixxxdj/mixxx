#pragma once

#include <QMutex>
#include <QObject>
#include <memory>

#include "util/db/dbid.h"

class KeySegments;
using KeySegmentsPointer = std::shared_ptr<KeySegments>;

class KeySegments : public QObject {
    Q_OBJECT

  public:
    static constexpr double kNoPosition = -1.0;

    KeySegments() = delete;

    // Load from database
    KeySegments(
            DbId id,
            double startTime,
            double duration,
            int keyId,
            const QString& keyText,
            double rangeStart,
            double rangeEnd,
            const QString& type,
            double confidence);

    // Create new segment
    KeySegments(
            double startTime,
            double duration,
            int keyId,
            const QString& keyText,
            double rangeStart,
            double rangeEnd,
            const QString& type,
            double confidence);

    ~KeySegments() override = default;

    bool isDirty() const;
    DbId getId() const;

    double getStartTime() const;
    void setStartTime(double startTime);

    double getDuration() const;
    void setDuration(double duration);

    int getKeyId() const;
    void setKeyId(int keyId);

    QString getKeyText() const;
    void setKeyText(const QString& keyText);

    double getRangeStart() const;
    void setRangeStart(double rangeStart);

    double getRangeEnd() const;
    void setRangeEnd(double rangeEnd);

    QString getType() const;
    void setType(const QString& type);

    double getConfidence() const;
    void setConfidence(double confidence);

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
    int m_keyId;
    QString m_keyText;
    double m_rangeStart;
    double m_rangeEnd;
    QString m_type;
    double m_confidence;

    friend class Track;
    friend class SegmentsDAO;
};

class KeySegmentPointer : public std::shared_ptr<KeySegments> {
  public:
    KeySegmentPointer() = default;
    explicit KeySegmentPointer(KeySegments* pSegment);

  private:
    static void deleteLater(KeySegments* pSegment);
};
