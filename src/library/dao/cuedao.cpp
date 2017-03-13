// cuedao.cpp
// Created 10/26/2009 by RJ Ryan (rryan@mit.edu)

#include <QtDebug>
#include <QtSql>
#include <QVariant>

#include "library/dao/cuedao.h"
#include "library/dao/cue.h"
#include "trackinfoobject.h"
#include "library/queryutil.h"
#include "util/assert.h"

CueDAO::CueDAO(QSqlDatabase& database)
        : m_database(database) {
}

CueDAO::~CueDAO() {
}

void CueDAO::initialize() {
    qDebug() << "CueDAO::initialize" << QThread::currentThread() << m_database.connectionName();
}

int CueDAO::cueCount() {
    qDebug() << "CueDAO::cueCount" << QThread::currentThread() << m_database.connectionName();
    QSqlQuery query(m_database);
    query.prepare("SELECT COUNT(*) FROM " CUE_TABLE);
    if (query.exec()) {
        if (query.next()) {
            return query.value(0).toInt();
        }
    } else {
        LOG_FAILED_QUERY(query);
    }
    //query.finish();
    return 0;
}

int CueDAO::numCuesForTrack(const int trackId) {
    qDebug() << "CueDAO::numCuesForTrack" << QThread::currentThread() << m_database.connectionName();
    QSqlQuery query(m_database);
    query.prepare("SELECT COUNT(*) FROM " CUE_TABLE " WHERE track_id = :id");
    query.bindValue(":id", trackId);
    if (query.exec()) {
        if (query.next()) {
            return query.value(0).toInt();
        }
    } else {
        LOG_FAILED_QUERY(query);
    }
    return 0;
}

Cue* CueDAO::cueFromRow(const QSqlQuery& query) const {
    QSqlRecord record = query.record();
    int id = record.value(record.indexOf("id")).toInt();
    int trackId = record.value(record.indexOf("track_id")).toInt();
    int type = record.value(record.indexOf("type")).toInt();
    int position = record.value(record.indexOf("position")).toInt();
    int length = record.value(record.indexOf("length")).toInt();
    int hotcue = record.value(record.indexOf("hotcue")).toInt();
    QString label = record.value(record.indexOf("label")).toString();
    Cue* cue = new Cue(id, trackId, (Cue::CueType)type,
                       position, length, hotcue, label);
    m_cues[id] = cue;
    return cue;
}

QList<Cue*> CueDAO::getCuesForTrack(const int trackId) const {
    //qDebug() << "CueDAO::getCuesForTrack" << QThread::currentThread() << m_database.connectionName();
    QList<Cue*> cues;
    // A hash from hotcue index to cue id and cue*, used to detect if more
    // than one cue has been assigned to a single hotcue id.
    QMap<int, QPair<int, Cue*> > dupe_hotcues;

    QSqlQuery query(m_database);
    query.prepare("SELECT * FROM " CUE_TABLE " WHERE track_id = :id");
    query.bindValue(":id", trackId);
    if (query.exec()) {
        const int idColumn = query.record().indexOf("id");
        const int hotcueIdColumn = query.record().indexOf("hotcue");
        while (query.next()) {
            Cue* cue = NULL;
            int cueId = query.value(idColumn).toInt();
            if (m_cues.contains(cueId)) {
                cue = m_cues[cueId];
            }
            if (cue == NULL) {
                cue = cueFromRow(query);
            }
            int hotcueId = query.value(hotcueIdColumn).toInt();
            if (hotcueId != -1) {
                if (dupe_hotcues.contains(hotcueId)) {
                    m_cues.remove(dupe_hotcues[hotcueId].first);
                    cues.removeOne(dupe_hotcues[hotcueId].second);
                }
                dupe_hotcues[hotcueId] = qMakePair(cueId, cue);
            }
            if (cue != NULL) {
                cues.push_back(cue);
            }
        }
    } else {
        LOG_FAILED_QUERY(query);
    }
    return cues;
}

bool CueDAO::deleteCuesForTrack(const int trackId) {
    qDebug() << "CueDAO::deleteCuesForTrack" << QThread::currentThread() << m_database.connectionName();
    QSqlQuery query(m_database);
    query.prepare("DELETE FROM " CUE_TABLE " WHERE track_id = :track_id");
    query.bindValue(":track_id", trackId);
    if (query.exec()) {
        return true;
    } else {
        LOG_FAILED_QUERY(query);
    }
    return false;
}

bool CueDAO::deleteCuesForTracks(const QList<int>& ids) {
    qDebug() << "CueDAO::deleteCuesForTracks" << QThread::currentThread() << m_database.connectionName();

    QStringList idList;
    foreach (int id, ids) {
        idList << QString::number(id);
    }

    QSqlQuery query(m_database);
    query.prepare(QString("DELETE FROM " CUE_TABLE " WHERE track_id in (%1)")
                  .arg(idList.join(",")));
    if (query.exec()) {
        return true;
    } else {
        LOG_FAILED_QUERY(query);
    }
    return false;
}

bool CueDAO::saveCue(Cue* cue) {
    //qDebug() << "CueDAO::saveCue" << QThread::currentThread() << m_database.connectionName();
    DEBUG_ASSERT_AND_HANDLE(cue) {
        return false;
    }
    if (cue->getId() == -1) {
        // New cue
        QSqlQuery query(m_database);
        query.prepare("INSERT INTO " CUE_TABLE " (track_id, type, position, length, hotcue, label) VALUES (:track_id, :type, :position, :length, :hotcue, :label)");
        query.bindValue(":track_id", cue->getTrackId());
        query.bindValue(":type", cue->getType());
        query.bindValue(":position", cue->getPosition());
        query.bindValue(":length", cue->getLength());
        query.bindValue(":hotcue", cue->getHotCue());
        query.bindValue(":label", cue->getLabel());

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
        query.prepare("UPDATE " CUE_TABLE " SET "
                        "track_id = :track_id,"
                        "type = :type,"
                        "position = :position,"
                        "length = :length,"
                        "hotcue = :hotcue,"
                        "label = :label"
                        " WHERE id = :id");
        query.bindValue(":id", cue->getId());
        query.bindValue(":track_id", cue->getTrackId());
        query.bindValue(":type", cue->getType());
        query.bindValue(":position", cue->getPosition());
        query.bindValue(":length", cue->getLength());
        query.bindValue(":hotcue", cue->getHotCue());
        query.bindValue(":label", cue->getLabel());

        if (query.exec()) {
            cue->setDirty(false);
            return true;
        } else {
            LOG_FAILED_QUERY(query);
        }
    }
    return false;
}

bool CueDAO::deleteCue(Cue* cue) {
    //qDebug() << "CueDAO::deleteCue" << QThread::currentThread() << m_database.connectionName();
    if (cue->getId() != -1) {
        QSqlQuery query(m_database);
        query.prepare("DELETE FROM " CUE_TABLE " WHERE id = :id");
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

void CueDAO::saveTrackCues(const int trackId, TrackInfoObject* pTrack) {
    //qDebug() << "CueDAO::saveTrackCues" << QThread::currentThread() << m_database.connectionName();
    // TODO(XXX) transaction, but people who are already in a transaction call
    // this.
    QTime time;

    const QList<Cue*>& cueList = pTrack->getCuePoints();

    // qDebug() << "CueDAO::saveTrackCues old size:" << oldCueList.size()
    //          << "new size:" << cueList.size();

    QString list = "";

    time.start();
    // For each id still in the TIO, save or delete it.
    QListIterator<Cue*> cueIt(cueList);
    while (cueIt.hasNext()) {
        Cue* cue = cueIt.next();
        int cueId = cue->getId();
        bool newCue = cueId == -1;
        if (newCue) {
            // New cue
            cue->setTrackId(trackId);
        } else {
            //idList.append(QString("%1").arg(cueId));
            list.append(QString("%1,").arg(cueId));
        }
        // Update or save cue
        if (cue->isDirty()) {
            saveCue(cue);

            // Since this cue didn't have an id until now, add it to the list of
            // cues not to delete.
            if (newCue)
                list.append(QString("%1,").arg(cue->getId()));
        }
    }
    //qDebug() << "Saving cues took " << time.elapsed() << "ms";
    time.start();

    // Strip the last ,
    if (list.count() > 0)
        list.truncate(list.count()-1);

    // Delete cues that are no longer on the track.
    QSqlQuery query(m_database);
    query.prepare(QString("DELETE FROM cues where track_id=:track_id and not id in (%1)")
                  .arg(list));
    query.bindValue(":track_id", trackId);

    if (!query.exec()) {
        LOG_FAILED_QUERY(query) << "Delete cues failed.";
    }
    //qDebug() << "Deleting cues took " << time.elapsed() << "ms";
}
