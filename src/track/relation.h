#pragma once

#include <QDateTime>
#include <QMutex>
#include <QObject>
#include <QString>

#include "audio/frame.h"
#include "track/track.h"

class Relation : public QObject {
    Q_OBJECT

  public:
    Relation() = delete;

    /// Load entity from database
    Relation(
            DbId id,
            TrackId sourceTrackId,
            TrackId targetTrackId,
            mixxx::audio::FramePos sourcePosition,
            mixxx::audio::FramePos targetPosition,
            int bidirectional,
            const QString& comment,
            const QString& tag,
            const QDateTime& dateAdded);

    /// Initialize new relation
    Relation(
            TrackId sourceTrackId,
            TrackId targetTrackId,
            std::optional<mixxx::audio::FramePos> sourcePosition = std::nullopt,
            std::optional<mixxx::audio::FramePos> targetPosition = std::nullopt,
            int bidirectional = 0,
            const QString& comment = QString(),
            const QString& tag = QString());

    ~Relation() override = default;

    DbId getId() const;

    TrackId getSourceTrackId() const;
    void setSourceTrackId(TrackId trackId);

    TrackId getTargetTrackId() const;
    void setTargetTrackId(TrackId trackId);

    std::optional<mixxx::audio::FramePos> getSourcePosition() const;
    void setSourcePosition(mixxx::audio::FramePos);
    void clearSourcePosition();

    std::optional<mixxx::audio::FramePos> getTargetPosition() const;
    void setTargetPosition(mixxx::audio::FramePos);
    void clearTargetPosition();

    bool getBidirectional() const;
    void setBidirectional(bool);

    QString getcomment() const;
    void setComment(const QString&);

    QString getTag() const;
    void setTag(const QString&);

    QDateTime getDateAdded();
    void setDateAdded(const QDateTime&);

  signals:
    void updated();

  private:
    void setId(DbId dbId); // Remove?

    DbId m_dbId;
    TrackId m_sourceTrackId;
    TrackId m_targetTrackId;
    std::optional<mixxx::audio::FramePos> m_sourcePosition;
    std::optional<mixxx::audio::FramePos> m_targetPosition;
    bool m_bBidirectional;
    QString m_comment;
    QString m_tag;
    QDateTime m_dateAdded;

    friend class RelationDAO;
};
