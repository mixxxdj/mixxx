#pragma once

#include <QDateTime>
#include <QMutex>
#include <QObject>
#include <QString>

#include "audio/frame.h"
#include "track/track.h"

using TrackPair = std::array<TrackId, 2>;

class Relation : public QObject {
    Q_OBJECT

  public:
    Relation() = delete;

    /// Load entity from database
    Relation(
            DbId id,
            TrackPair tracks,
            const QString& comment,
            const QDateTime& dateAdded);

    /// Initialize new relation
    Relation(
            TrackPair tracks,
            const QString& comment = QString());

    ~Relation() override = default;

    DbId getId() const;
    TrackPair getTracks() const;
    void setTracks(TrackPair);
    QString getComment() const;
    void setComment(const QString&);
    QDateTime getDateAdded() const;
    void setDateAdded(const QDateTime&);

  signals:
    void updated();

  private:
    void setId(DbId dbId);

    DbId m_dbId;
    TrackPair m_tracks;
    QString m_comment;
    QDateTime m_dateAdded;

    friend class RelationDAO;
};
