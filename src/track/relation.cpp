#include "relation.h"

#include <QDateTime>
#include <QObject>
#include <QString>

#include "audio/frame.h"
#include "moc_relation.cpp"

// Loading entity from database
Relation::Relation(
        DbId dbId,
        TrackId sourceTrackId,
        TrackId targetTrackId,
        std::optional<mixxx::audio::FramePos> sourcePosition,
        std::optional<mixxx::audio::FramePos> targetPosition,
        int bidirectional,
        const QString& comment,
        const QString& tag,
        const QDateTime& dateAdded)
        : m_dbId(dbId),
          m_sourceTrackId(sourceTrackId),
          m_targetTrackId(targetTrackId),
          m_sourcePosition(sourcePosition),
          m_targetPosition(targetPosition),
          m_bBidirectional(bidirectional),
          m_comment(comment),
          m_tag(tag),
          m_dateAdded(dateAdded) {
}

// Initialize new relation
Relation::Relation(
        TrackId sourceTrackId,
        TrackId targetTrackId,
        std::optional<mixxx::audio::FramePos> sourcePosition,
        std::optional<mixxx::audio::FramePos> targetPosition,
        int bidirectional,
        const QString& comment,
        const QString& tag)
        : m_sourceTrackId(sourceTrackId),
          m_targetTrackId(targetTrackId),
          m_sourcePosition(sourcePosition),
          m_targetPosition(targetPosition),
          m_bBidirectional(bidirectional),
          m_comment(comment),
          m_tag(tag) {
}

DbId Relation::getId() const {
    return m_dbId;
}

void Relation::setId(DbId dbId) {
    m_dbId = dbId;
    // Private. Do not emit updated() signal.
    // Only used when adding Relation object to the database.
}

TrackId Relation::getSourceTrackId() const {
    return m_sourceTrackId;
}

void Relation::setSourceTrackId(TrackId trackId) {
    if (!trackId.isValid() || m_sourceTrackId == trackId) {
        return;
    }
    m_sourceTrackId = trackId;
    emit updated();
}

TrackId Relation::getTargetTrackId() const {
    return m_targetTrackId;
}

void Relation::setTargetTrackId(TrackId trackId) {
    if (!trackId.isValid() || m_targetTrackId == trackId) {
        return;
    }
    m_targetTrackId = trackId;
    emit updated();
}

std::optional<mixxx::audio::FramePos> Relation::getSourcePosition() const {
    return m_sourcePosition;
}

void Relation::setSourcePosition(mixxx::audio::FramePos position) {
    if (!position.isValid() || m_sourcePosition == position) {
        return;
    }
    m_sourcePosition = position;
    emit updated();
}

void Relation::clearSourcePosition() {
    if (m_sourcePosition == std::nullopt) {
        return;
    }
    m_sourcePosition = std::nullopt;
    emit updated();
}

std::optional<mixxx::audio::FramePos> Relation::getTargetPosition() const {
    return m_targetPosition;
}

void Relation::setTargetPosition(mixxx::audio::FramePos position) {
    if (!position.isValid() || m_targetPosition == position) {
        return;
    }
    m_targetPosition = position;
    emit updated();
}

bool Relation::getBidirectional() const {
    return m_bBidirectional;
}

void Relation::setBidirectional(bool bidirectional) {
    if (m_bBidirectional == bidirectional) {
        return;
    }
    m_bBidirectional = bidirectional;
    emit updated();
}

QString Relation::getcomment() const {
    return m_comment;
}

void Relation::setComment(const QString& comment) {
    if (m_comment == comment) {
        return;
    }
    m_comment = comment;
    emit updated();
}

QString Relation::getTag() const {
    return m_tag;
}

void Relation::setTag(const QString& tag) {
    if (m_tag == tag) {
        return;
    }
    m_tag = tag;
    emit updated();
}

QDateTime Relation::getDateAdded() {
    return m_dateAdded;
}

void Relation::setDateAdded(const QDateTime& dateAdded) {
    if (m_dateAdded == dateAdded) {
        return;
    }
    m_dateAdded = dateAdded;
    emit updated();
}
