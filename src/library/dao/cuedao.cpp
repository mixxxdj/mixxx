// cuedao.cpp
// Created 10/26/2009 by RJ Ryan (rryan@mit.edu)

#include "library/dao/cuedao.h"

#include <QVariant>
#include <QtDebug>
#include <QtSql>

#include "library/queryutil.h"
#include "track/cue.h"
#include "track/track.h"
#include "util/assert.h"
#include "util/color/rgbcolor.h"
#include "util/db/fwdsqlquery.h"
#include "util/performancetimer.h"

namespace {

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
            trackId,
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
    VERIFY_OR_DEBUG_ASSERT(
            query.isPrepared() &&
            !query.hasError()) {
        return cues;
    }
    query.bindValue(":id", trackId.toVariant());
    VERIFY_OR_DEBUG_ASSERT(query.execPrepared()) {
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

bool CueDAO::saveCue(Cue* cue) const {
    //qDebug() << "CueDAO::saveCue" << QThread::currentThread() << m_database.connectionName();
    VERIFY_OR_DEBUG_ASSERT(cue) {
        return false;
    }
    if (cue->getId() == -1) {
        // New cue
        QSqlQuery query(m_database);
        query.prepare(QStringLiteral("INSERT INTO " CUE_TABLE " (track_id, type, position, length, hotcue, label, color) VALUES (:track_id, :type, :position, :length, :hotcue, :label, :color)"));
        query.bindValue(":track_id", cue->getTrackId().toVariant());
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
        query.bindValue(":track_id", cue->getTrackId().toVariant());
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
    //qDebug() << "CueDAO::saveTrackCues" << QThread::currentThread() << m_database.connectionName();
    // TODO(XXX) transaction, but people who are already in a transaction call
    // this.
    PerformanceTimer time;

    // qDebug() << "CueDAO::saveTrackCues old size:" << oldCueList.size()
    //          << "new size:" << cueList.size();

    QString list = "";

    time.start();
    // For each id still in the TIO, save or delete it.
    QListIterator<CuePointer> cueIt(cueList);
    while (cueIt.hasNext()) {
        CuePointer pCue(cueIt.next());
        int cueId = pCue->getId();
        bool newCue = cueId == -1;
        if (newCue) {
            // New cue
            pCue->setTrackId(trackId);
        } else {
            //idList.append(QString("%1").arg(cueId));
            list.append(QStringLiteral("%1,").arg(cueId));
        }
        // Update or save cue
        if (pCue->isDirty()) {
            saveCue(pCue.get());

            // Since this cue didn't have an id until now, add it to the list of
            // cues not to delete.
            if (newCue)
                list.append(QStringLiteral("%1,").arg(pCue->getId()));
        }
    }
    //qDebug() << "Saving cues took " << time.formatMillisWithUnit();
    time.start();

    // Strip the last ,
    if (list.count() > 0)
        list.truncate(list.count()-1);

    // Delete cues that are no longer on the track.
    QSqlQuery query(m_database);
    query.prepare(QStringLiteral("DELETE FROM " CUE_TABLE " WHERE track_id=:track_id AND NOT id IN (%1)")
                  .arg(list));
    query.bindValue(":track_id", trackId.toVariant());

    if (!query.exec()) {
        LOG_FAILED_QUERY(query) << "Delete cues failed.";
    }
    //qDebug() << "Deleting cues took " << time.formatMillisWithUnit();
}
