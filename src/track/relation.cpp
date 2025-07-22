#include "relation.h"

#include <QDateTime>
#include <QObject>
#include <QString>

#include "moc_relation.cpp"

// Loading entity from database
Relation::Relation(
        DbId dbId,
        TrackPair tracks,
        const QString& comment,
        const QDateTime& dateAdded)
        : m_dbId(dbId),
          m_tracks(tracks),
          m_comment(comment),
          m_dateAdded(dateAdded) {
}

// Initialize new relation
Relation::Relation(
        TrackPair tracks,
        const QString& comment)
        : m_tracks(tracks),
          m_comment(comment) {
}

DbId Relation::getId() const {
    return m_dbId;
}

void Relation::setId(DbId dbId) {
    m_dbId = dbId;
    // Private. Do not emit updated() signal.
    // Only used when adding Relation object to the database.
}

TrackPair Relation::getTracks() const {
    return m_tracks;
}

void Relation::setTracks(TrackPair tracks) {
    if (m_tracks == tracks) {
        return;
    }
    m_tracks = tracks;
    emit updated();
}

QString Relation::getComment() const {
    return m_comment;
}

void Relation::setComment(const QString& comment) {
    if (m_comment == comment) {
        return;
    }
    m_comment = comment;
    emit updated();
}

QDateTime Relation::getDateAdded() const {
    return m_dateAdded;
}

void Relation::setDateAdded(const QDateTime& dateAdded) {
    if (m_dateAdded == dateAdded) {
        return;
    }
    m_dateAdded = dateAdded;
    emit updated();
}
