// cuedao.cpp
// Created 10/26/2009 by RJ Ryan (rryan@mit.edu)

#include "library/dao/cuedao.h"

#include <QVariant>
#include <QtDebug>
#include <QtSql>

#include "library/queryutil.h"
#include "track/track.h"
#include "util/assert.h"
#include "util/color/rgbcolor.h"
#include "util/db/fwdsqlquery.h"
#include "util/logger.h"
#include "util/performancetimer.h"

namespace {

const mixxx::Logger kLogger = mixxx::Logger("CueDAO");

// The label column is not nullable!
const QVariant kEmptyLabel = QVariant(QStringLiteral(""));

inline const QVariant labelToQVariant(const QString& label) {
    if (label.isNull()) {
        return kEmptyLabel; // null -> empty
    } else {
        return label;
    }
}

// Empty labels are read as null strings
inline QString labelFromQVariant(const QVariant& value) {
    const auto label = value.toString();
    if (label.isEmpty()) {
        return QString(); // empty -> null
    } else {
        return label;
    }
}

CuePointer cueFromRow(const QSqlRecord& row) {
    int id = row.value(row.indexOf("id")).toInt();
    TrackId trackId(row.value(row.indexOf("track_id")));
    int type = row.value(row.indexOf("type")).toInt();
    int position = row.value(row.indexOf("position")).toInt();
    int length = row.value(row.indexOf("length")).toInt();
    int hotcue = row.value(row.indexOf("hotcue")).toInt();
    QString label = labelFromQVariant(row.value(row.indexOf("label")));
    mixxx::RgbColor::optional_t color = mixxx::RgbColor::fromQVariant(row.value(row.indexOf("color")));
    VERIFY_OR_DEBUG_ASSERT(color) {
        return CuePointer();
    }
    CuePointer pCue(new Cue(id,
            static_cast<mixxx::CueType>(type),
            position,
            length,
            hotcue,
            label,
            *color));
    return pCue;
}

} // namespace

QList<CuePointer> CueDAO::getCuesForTrack(TrackId trackId) const {
    //qDebug() << "CueDAO::getCuesForTrack" << QThread::currentThread() << m_database.connectionName();
    QList<CuePointer> cues;

    FwdSqlQuery query(
            m_database,
            QStringLiteral("SELECT * FROM " CUE_TABLE " WHERE track_id=:id"));
    DEBUG_ASSERT(
            query.isPrepared() &&
            !query.hasError());
    query.bindValue(":id", trackId.toVariant());
    VERIFY_OR_DEBUG_ASSERT(query.execPrepared()) {
        kLogger.warning()
                << "Failed to load cues of track"
                << trackId;
        return cues;
    }
    QMap<int, CuePointer> hotCuesByNumber;
    while (query.next()) {
        CuePointer pCue = cueFromRow(query.record());
        VERIFY_OR_DEBUG_ASSERT(pCue) {
            continue;
        }
        int hotCueNumber = pCue->getHotCue();
        if (hotCueNumber != Cue::kNoHotCue) {
            const auto pDuplicateCue = hotCuesByNumber.take(hotCueNumber);
            if (pDuplicateCue) {
                kLogger.warning()
                        << "Dropping hot cue"
                        << pDuplicateCue->getId()
                        << "with duplicate number"
                        << hotCueNumber;
                cues.removeOne(pDuplicateCue);
            }
            hotCuesByNumber.insert(hotCueNumber, pCue);
        }
        cues.push_back(pCue);
    }
    return cues;
}

bool CueDAO::deleteCuesForTrack(TrackId trackId) const {
    qDebug() << "CueDAO::deleteCuesForTrack" << QThread::currentThread() << m_database.connectionName();
    QSqlQuery query(m_database);
    query.prepare(QStringLiteral("DELETE FROM " CUE_TABLE " WHERE track_id=:track_id"));
    query.bindValue(":track_id", trackId.toVariant());
    if (query.exec()) {
        return true;
    } else {
        LOG_FAILED_QUERY(query);
    }
    return false;
}

bool CueDAO::deleteCuesForTracks(const QList<TrackId>& trackIds) const {
    qDebug() << "CueDAO::deleteCuesForTracks" << QThread::currentThread() << m_database.connectionName();

    QStringList idList;
    for (const auto& trackId: trackIds) {
        idList << trackId.toString();
    }

    QSqlQuery query(m_database);
    query.prepare(QStringLiteral("DELETE FROM " CUE_TABLE " WHERE track_id in (%1)")
                  .arg(idList.join(",")));
    if (query.exec()) {
        return true;
    } else {
        LOG_FAILED_QUERY(query);
    }
    return false;
}

bool CueDAO::saveCue(TrackId trackId, Cue* cue) const {
    //qDebug() << "CueDAO::saveCue" << QThread::currentThread() << m_database.connectionName();
    VERIFY_OR_DEBUG_ASSERT(cue) {
        return false;
    }
    if (cue->getId() == -1) {
        // New cue
        QSqlQuery query(m_database);
        query.prepare(QStringLiteral("INSERT INTO " CUE_TABLE " (track_id, type, position, length, hotcue, label, color) VALUES (:track_id, :type, :position, :length, :hotcue, :label, :color)"));
        query.bindValue(":track_id", trackId.toVariant());
        query.bindValue(":type", static_cast<int>(cue->getType()));
        query.bindValue(":position", cue->getPosition());
        query.bindValue(":length", cue->getLength());
        query.bindValue(":hotcue", cue->getHotCue());
        query.bindValue(":label", labelToQVariant(cue->getLabel()));
        query.bindValue(":color", mixxx::RgbColor::toQVariant(cue->getColor()));

        if (query.exec()) {
            int id = query.lastInsertId().toInt();
            cue->setId(id);
            cue->setDirty(false);
            return true;
        }
        qDebug() << query.executedQuery() << query.lastError();
    } else {
        // Update cue
        QSqlQuery query(m_database);
        query.prepare(QStringLiteral("UPDATE " CUE_TABLE " SET "
                        "track_id=:track_id,"
                        "type=:type,"
                        "position=:position,"
                        "length=:length,"
                        "hotcue=:hotcue,"
                        "label=:label,"
                        "color=:color"
                        " WHERE id=:id"));
        query.bindValue(":id", cue->getId());
        query.bindValue(":track_id", trackId.toVariant());
        query.bindValue(":type", static_cast<int>(cue->getType()));
        query.bindValue(":position", cue->getPosition());
        query.bindValue(":length", cue->getLength());
        query.bindValue(":hotcue", cue->getHotCue());
        query.bindValue(":label", labelToQVariant(cue->getLabel()));
        query.bindValue(":color", mixxx::RgbColor::toQVariant(cue->getColor()));

        if (query.exec()) {
            cue->setDirty(false);
            return true;
        } else {
            LOG_FAILED_QUERY(query);
        }
    }
    return false;
}

bool CueDAO::deleteCue(Cue* cue) const {
    //qDebug() << "CueDAO::deleteCue" << QThread::currentThread() << m_database.connectionName();
    if (cue->getId() != -1) {
        QSqlQuery query(m_database);
        query.prepare(QStringLiteral("DELETE FROM " CUE_TABLE " WHERE id=:id"));
        query.bindValue(":id", cue->getId());
        if (query.exec()) {
            return true;
        } else {
            LOG_FAILED_QUERY(query);
        }
    } else {
        return true;
    }
    return false;
}

void CueDAO::saveTrackCues(
        TrackId trackId,
        const QList<CuePointer>& cueList) const {
    DEBUG_ASSERT(trackId.isValid());
    QStringList cueIds;
    cueIds.reserve(cueList.size());
    for (const auto& pCue : cueList) {
        // New cues (without an id) must always be marked as dirty
        DEBUG_ASSERT(pCue->getId() >= 0 || pCue->isDirty());
        // Update or save cue
        if (pCue->isDirty()) {
            saveCue(trackId, pCue.get());
        }
        // After saving each cue must have a valid id
        VERIFY_OR_DEBUG_ASSERT(pCue->getId() >= 0) {
            continue;
        }
        cueIds.append(QString::number(pCue->getId()));
    }

    // Delete orphaned cues
    FwdSqlQuery query(
            m_database,
            QStringLiteral("DELETE FROM " CUE_TABLE " WHERE track_id=:track_id AND id NOT IN (%1)")
                    .arg(cueIds.join(QChar(','))));
    DEBUG_ASSERT(
            query.isPrepared() &&
            !query.hasError());
    query.bindValue(":track_id", trackId.toVariant());
    VERIFY_OR_DEBUG_ASSERT(query.execPrepared()) {
        kLogger.warning()
                << "Failed to delete orphaned cues of track"
                << trackId;
        return;
    }
    if (query.numRowsAffected() > 0) {
        kLogger.debug()
                << "Deleted"
                << query.numRowsAffected()
                << "orphaned cue(s) of track"
                << trackId;
    }
}
