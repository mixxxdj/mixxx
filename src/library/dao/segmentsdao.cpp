#include "library/dao/segmentsdao.h"

#include <QSqlQuery>
#include <QtDebug>

#include "library/queryutil.h"
#include "util/assert.h"
#include "util/db/fwdsqlquery.h"
#include "util/logger.h"

namespace {

const mixxx::Logger kLogger = mixxx::Logger("SegmentsDAO");

BpmSegmentsPointer BpmSegmentFromRow(const QSqlRecord& row) {
    const auto id = DbId(row.value(row.indexOf("id")));
    double startTime = row.value(row.indexOf("position")).toDouble();
    double duration = row.value(row.indexOf("duration")).toDouble();
    double bpmStart = row.value(row.indexOf("bpm_start")).toDouble();
    double bpmEnd = row.value(row.indexOf("bpm_end")).toDouble();
    double rangeStart = row.value(row.indexOf("range_start")).toDouble();
    double rangeEnd = row.value(row.indexOf("range_end")).toDouble();
    QString type = row.value(row.indexOf("type")).toString();

    BpmSegmentsPointer pSegment(new BpmSegments(
            id,
            startTime,
            duration,
            bpmStart,
            bpmEnd,
            rangeStart,
            rangeEnd,
            type));
    return pSegment;
}

KeySegmentsPointer KeySegmentFromRow(const QSqlRecord& row) {
    const auto id = DbId(row.value(row.indexOf("id")));
    double startTime = row.value(row.indexOf("position")).toDouble();
    double duration = row.value(row.indexOf("duration")).toDouble();
    int keyId = row.value(row.indexOf("key_id")).toInt(); // Read keyId
    QString keyText = row.value(row.indexOf("key_text")).toString();
    double rangeStart = row.value(row.indexOf("range_start")).toDouble();
    double rangeEnd = row.value(row.indexOf("range_end")).toDouble();
    QString type = row.value(row.indexOf("type")).toString();
    double confidence = row.value(row.indexOf("confidence")).toDouble();

    KeySegmentsPointer pSegment(new KeySegments(
            id,
            startTime,
            duration,
            keyId,
            keyText,
            rangeStart,
            rangeEnd,
            type,
            confidence));
    return pSegment;
}

} // namespace

QList<BpmSegmentsPointer> SegmentsDAO::getBpmSegmentsForTrack(TrackId trackId) const {
    QList<BpmSegmentsPointer> segments;

    if (!m_database.isOpen() || !trackId.isValid()) {
        return segments;
    }

    FwdSqlQuery query(m_database,
            QStringLiteral("SELECT * FROM " BPM_SEGMENTS_TABLE
                           " WHERE track_id=:id ORDER BY position ASC"));
    DEBUG_ASSERT(
            query.isPrepared() &&
            !query.hasError());
    query.bindValue(":id", trackId);
    if (!query.execPrepared()) {
        kLogger.warning()
                << "Failed to load BPM segments of track"
                << trackId;
        DEBUG_ASSERT(!"failed query");
        return segments;
    }

    while (query.next()) {
        BpmSegmentsPointer pSegment = BpmSegmentFromRow(query.record());
        if (!pSegment) {
            continue;
        }
        segments.push_back(pSegment);
    }
    return segments;
}

QList<KeySegmentsPointer> SegmentsDAO::getKeySegmentsForTrack(TrackId trackId) const {
    QList<KeySegmentsPointer> segments;

    if (!m_database.isOpen() || !trackId.isValid()) {
        return segments;
    }

    FwdSqlQuery query(m_database,
            QStringLiteral("SELECT * FROM " KEY_SEGMENTS_TABLE
                           " WHERE track_id=:id ORDER BY position ASC"));
    DEBUG_ASSERT(
            query.isPrepared() &&
            !query.hasError());
    query.bindValue(":id", trackId);
    if (!query.execPrepared()) {
        kLogger.warning()
                << "Failed to load key segments of track"
                << trackId;
        DEBUG_ASSERT(!"failed query");
        return segments;
    }

    while (query.next()) {
        KeySegmentsPointer pSegment = KeySegmentFromRow(query.record());
        if (!pSegment) {
            continue;
        }
        segments.push_back(pSegment);
    }
    return segments;
}

bool SegmentsDAO::deleteBpmSegmentsForTrack(TrackId trackId) const {
    if (!m_database.isOpen() || !trackId.isValid()) {
        return false;
    }

    QSqlQuery query(m_database);
    query.prepare(QStringLiteral("DELETE FROM " BPM_SEGMENTS_TABLE " WHERE track_id=:track_id"));
    query.bindValue(":track_id", trackId.toVariant());

    if (query.exec()) {
        qDebug() << "Deleted BPM segments for track" << trackId;
        return true;
    } else {
        LOG_FAILED_QUERY(query);
        return false;
    }
}

bool SegmentsDAO::deleteKeySegmentsForTrack(TrackId trackId) const {
    if (!m_database.isOpen() || !trackId.isValid()) {
        return false;
    }

    QSqlQuery query(m_database);
    query.prepare(QStringLiteral("DELETE FROM " KEY_SEGMENTS_TABLE " WHERE track_id=:track_id"));
    query.bindValue(":track_id", trackId.toVariant());
    if (query.exec()) {
        qDebug() << "Deleted key segments for track" << trackId;
        return true;
    } else {
        LOG_FAILED_QUERY(query);
    }
    return false;
}

bool SegmentsDAO::deleteBpmSegmentsForTracks(const QList<TrackId>& trackIds) const {
    qDebug() << "SegmentsDAO::deleteBpmSegmentsForTracks";

    QStringList idList;
    for (const auto& trackId : trackIds) {
        idList << trackId.toString();
    }

    QSqlQuery query(m_database);
    query.prepare(QStringLiteral("DELETE FROM " BPM_SEGMENTS_TABLE " WHERE track_id in (%1)")
                    .arg(idList.join(",")));
    if (query.exec()) {
        return true;
    } else {
        LOG_FAILED_QUERY(query);
    }
    return false;
}

bool SegmentsDAO::deleteKeySegmentsForTracks(const QList<TrackId>& trackIds) const {
    if (!m_database.isOpen() || trackIds.isEmpty()) {
        return false;
    }

    QStringList idList;
    for (const auto& trackId : trackIds) {
        idList << trackId.toString();
    }

    QSqlQuery query(m_database);
    query.prepare(QStringLiteral("DELETE FROM " KEY_SEGMENTS_TABLE " WHERE track_id in (%1)")
                    .arg(idList.join(",")));
    if (query.exec()) {
        return true;
    } else {
        LOG_FAILED_QUERY(query);
    }
    return false;
}

bool SegmentsDAO::saveBpmSegment(TrackId trackId, BpmSegments* pSegment) const {
    VERIFY_OR_DEBUG_ASSERT(pSegment) {
        return false;
    }

    QSqlQuery query(m_database);
    if (pSegment->getId().isValid()) {
        query.prepare(QStringLiteral("UPDATE " BPM_SEGMENTS_TABLE " SET "
                                     "track_id=:track_id,"
                                     "position=:position,"
                                     "duration=:duration,"
                                     "bpm_start=:bpm_start,"
                                     "bpm_end=:bpm_end,"
                                     "range_start=:range_start,"
                                     "range_end=:range_end,"
                                     "type=:type"
                                     " WHERE id=:id"));
        query.bindValue(":id", pSegment->getId().toVariant());
    } else {
        query.prepare(QStringLiteral(
                "INSERT INTO " BPM_SEGMENTS_TABLE
                " (track_id, position, duration, bpm_start, bpm_end, "
                "range_start, range_end, type) VALUES (:track_id, :position, "
                ":duration, :bpm_start, :bpm_end, :range_start, :range_end, "
                ":type)"));
    }

    query.bindValue(":track_id", trackId.toVariant());
    query.bindValue(":position", pSegment->getStartTime());
    query.bindValue(":duration", pSegment->getDuration());
    query.bindValue(":bpm_start", pSegment->getBpmStart());
    query.bindValue(":bpm_end", pSegment->getBpmEnd());
    query.bindValue(":range_start", pSegment->getRangeStart());
    query.bindValue(":range_end", pSegment->getRangeEnd());
    query.bindValue(":type", pSegment->getType());

    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return false;
    }

    if (!pSegment->getId().isValid()) {
        const auto newId = DbId(query.lastInsertId());
        DEBUG_ASSERT(newId.isValid());
        pSegment->setId(newId);
    }
    DEBUG_ASSERT(pSegment->getId().isValid());
    pSegment->setDirty(false);
    return true;
}

bool SegmentsDAO::saveKeySegment(TrackId trackId, KeySegments* pSegment) const {
    if (!m_database.isOpen() || !trackId.isValid()) {
        return false;
    }

    VERIFY_OR_DEBUG_ASSERT(pSegment) {
        return false;
    }

    QSqlQuery query(m_database);
    if (pSegment->getId().isValid()) {
        query.prepare(QStringLiteral(
                "UPDATE " KEY_SEGMENTS_TABLE
                " SET "
                "track_id=:track_id,"
                "position=:position,"
                "duration=:duration,"
                "key_id=:key_id,"
                "key_text=:key_text,"
                "range_start=:range_start,"
                "range_end=:range_end,"
                "type=:type,"
                "confidence=:confidence"
                " WHERE id=:id"));
        query.bindValue(":id", pSegment->getId().toVariant());
    } else {
        query.prepare(
                QStringLiteral(
                        "INSERT INTO " KEY_SEGMENTS_TABLE " "
                        " (track_id, position, duration, key_id, key_text, range_start, "
                        "range_end, type, confidence)"
                        " VALUES (:track_id, :position, :duration, :key_id, :key_text, "
                        ":range_start, :range_end, :type, :confidence)"));
    }

    query.bindValue(":track_id", trackId.toVariant());
    query.bindValue(":position", pSegment->getStartTime());
    query.bindValue(":duration", pSegment->getDuration());
    query.bindValue(":key_id", pSegment->getKeyId());
    query.bindValue(":key_text", pSegment->getKeyText());
    query.bindValue(":range_start", pSegment->getRangeStart());
    query.bindValue(":range_end", pSegment->getRangeEnd());
    query.bindValue(":type", pSegment->getType());
    query.bindValue(":confidence", pSegment->getConfidence());

    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return false;
    }

    if (!pSegment->getId().isValid()) {
        const auto newId = DbId(query.lastInsertId());
        DEBUG_ASSERT(newId.isValid());
        pSegment->setId(newId);
    }
    DEBUG_ASSERT(pSegment->getId().isValid());
    pSegment->setDirty(false);
    return true;
}

void SegmentsDAO::saveTrackBpmSegments(
        TrackId trackId,
        const QList<BpmSegmentsPointer>& segmentList) const {
    if (!m_database.isOpen() || !trackId.isValid()) {
        return;
    }

    DEBUG_ASSERT(trackId.isValid());
    QStringList segmentIds;
    segmentIds.reserve(segmentList.size());
    for (const auto& pSegment : segmentList) {
        DEBUG_ASSERT(pSegment->getId().isValid() || pSegment->isDirty());
        if (pSegment->isDirty()) {
            saveBpmSegment(trackId, pSegment.get());
        }
        VERIFY_OR_DEBUG_ASSERT(pSegment->getId().isValid()) {
            continue;
        }
        segmentIds.append(pSegment->getId().toString());
    }

    // Delete orphaned segments
    FwdSqlQuery query(m_database,
            QStringLiteral("DELETE FROM " BPM_SEGMENTS_TABLE
                           " WHERE track_id=:track_id AND id NOT IN (%1)")
                    .arg(segmentIds.join(QChar(','))));
    DEBUG_ASSERT(
            query.isPrepared() &&
            !query.hasError());
    query.bindValue(":track_id", trackId);
    if (!query.execPrepared()) {
        kLogger.warning()
                << "Failed to delete orphaned BPM segments of track"
                << trackId;
        DEBUG_ASSERT(!"failed query");
        return;
    }
    if (query.numRowsAffected() > 0) {
        kLogger.debug()
                << "Deleted"
                << query.numRowsAffected()
                << "orphaned BPM segment(s) of track"
                << trackId;
    }
}

void SegmentsDAO::saveTrackKeySegments(
        TrackId trackId,
        const QList<KeySegmentsPointer>& segmentList) const {
    if (!m_database.isOpen() || !trackId.isValid()) {
        return;
    }

    DEBUG_ASSERT(trackId.isValid());
    QStringList segmentIds;
    segmentIds.reserve(segmentList.size());
    for (const auto& pSegment : segmentList) {
        DEBUG_ASSERT(pSegment->getId().isValid() || pSegment->isDirty());
        if (pSegment->isDirty()) {
            saveKeySegment(trackId, pSegment.get());
        }
        VERIFY_OR_DEBUG_ASSERT(pSegment->getId().isValid()) {
            continue;
        }
        segmentIds.append(pSegment->getId().toString());
    }

    // Delete orphaned segments
    FwdSqlQuery query(m_database,
            QStringLiteral("DELETE FROM " KEY_SEGMENTS_TABLE
                           " WHERE track_id=:track_id AND id NOT IN (%1)")
                    .arg(segmentIds.join(QChar(','))));
    DEBUG_ASSERT(
            query.isPrepared() &&
            !query.hasError());
    query.bindValue(":track_id", trackId);
    if (!query.execPrepared()) {
        kLogger.warning()
                << "Failed to delete orphaned key segments of track"
                << trackId;
        DEBUG_ASSERT(!"failed query");
        return;
    }
    if (query.numRowsAffected() > 0) {
        kLogger.debug()
                << "Deleted"
                << query.numRowsAffected()
                << "orphaned key segment(s) of track"
                << trackId;
    }
}
