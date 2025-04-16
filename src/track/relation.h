#pragma once

#include <QDateTime>
#include <QMutex>
#include <QObject>
#include <QString>

#include "audio/frame.h"
#include "track/track.h"

using TrackPair = std::array<TrackId, 2>;
using PositionPair = std::array<std::optional<mixxx::audio::FramePos>, 2>;

class Relation : public QObject {
    Q_OBJECT

  public:
    Relation() = delete;

    /// Load entity from database
    Relation(
            DbId id,
            TrackPair tracks,
            const PositionPair& positions,
            const QString& comment,
            const QDateTime& dateAdded);

    /// Initialize new relation
    Relation(
            TrackPair tracks,
            const PositionPair& = PositionPair{std::nullopt, std::nullopt},
            const QString& comment = QString());

    ~Relation() override = default;

    DbId getId() const;
    TrackPair getTracks() const;
    void setTracks(TrackPair);
    PositionPair getPositions() const;
    void setPositions(const PositionPair&);
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
    PositionPair m_positions;
    QString m_comment;
    QDateTime m_dateAdded;

    friend class RelationDAO;
};
