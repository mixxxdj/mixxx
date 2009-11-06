// cuedao.cpp
// Created 10/26/2009 by RJ Ryan (rryan@mit.edu)

#include <QtDebug>
#include <QtSql>
#include <QVariant>

#include "library/dao/cuedao.h"
#include "library/dao/cue.h"
#include "trackinfoobject.h"

CueDAO::CueDAO(QSqlDatabase& database)
        : m_database(database) {
}

CueDAO::~CueDAO() {

}

void CueDAO::initialize() {
    qDebug() << "CueDAO::initialize()";

    QSqlQuery query;
    query.prepare("CREATE TABLE IF NOT EXISTS " CUE_TABLE " ("
                  "id integer PRIMARY KEY AUTOINCREMENT,"
                  "track_id integer NOT NULL REFERENCES library(id),"
                  "type integer DEFAULT 0 NOT NULL,"
                  "position integer DEFAULT -1 NOT NULL,"
                  "length integer DEFAULT 0 NOT NULL,"
                  "hotcue integer DEFAULT -1 NOT NULL,"
                  "label text DEFAULT '' NOT NULL"
                  ")");

    if (!query.exec()) {
        qDebug() << "Creating cue table failed:" << query.lastError();
    }
}

int CueDAO::cueCount() {
    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM " CUE_TABLE);
    if (query.exec()) {
        if (query.next()) {
            return query.value(0).toInt();
        }
    } else {
        qDebug() << query.lastError();
    }
    return 0;
}

int CueDAO::numCuesForTrack(int trackId) {
    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM " CUE_TABLE " WHERE track_id = :id");
    query.bindValue(":id", trackId);
    if (query.exec()) {
        if (query.next()) {
            return query.value(0).toInt();
        }
    } else {
        qDebug() << query.lastError();
    }
    return 0;
}

Cue* CueDAO::cueFromRow(QSqlQuery& query) const {
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

Cue* CueDAO::getCue(int cueId) {
    if (m_cues.contains(cueId)) {
        return m_cues[cueId];
    }

    QSqlQuery query("SELECT * FROM " CUE_TABLE " WHERE id = :id");
    query.bindValue(":id", cueId);
    if (query.exec()) {
        if (query.next()) {
            return cueFromRow(query);
        }
    } else {
        qDebug() << query.lastError();
    }
    return NULL;
}

QList<Cue*> CueDAO::getCuesForTrack(int trackId) const {
    QList<Cue*> cues;
    QSqlQuery query("SELECT * FROM " CUE_TABLE " WHERE track_id = :id");
    query.bindValue(":id", trackId);
    if (query.exec()) {
        while (query.next()) {
            Cue* cue = NULL;
            int cueId = query.value(query.record().indexOf("id")).toInt();
            if (m_cues.contains(cueId)) {
                cue = m_cues[cueId];
            }
            if (cue == NULL) {
                cue = cueFromRow(query);
            }
            if (cue != NULL) {
                cues.push_back(cue);
            }
        }
    } else {
        qDebug() << query.lastError();
    }
    return cues;
}

bool CueDAO::deleteCuesForTrack(int trackId) {
    QSqlQuery query("DELETE FROM " CUE_TABLE " WHERE track_id = :track_id");
    query.bindValue(":track_id", trackId);
    if (query.exec()) {
        return true;
    } else {
        qDebug() << query.lastError();
    }
    return false;
}

bool CueDAO::saveCue(Cue* cue) {
    Q_ASSERT(cue);
    if (cue->getId() == -1) {
        //Start the transaction
        QSqlDatabase::database().transaction();

        // New cue
        QSqlQuery query("INSERT INTO " CUE_TABLE " (track_id, type, position, length, hotcue, label) VALUES (:track_id, :type, :position, :length, :hotcue, :label)");
        query.bindValue(":track_id", cue->getTrackId());
        query.bindValue(":type", cue->getType());
        query.bindValue(":position", cue->getPosition());
        query.bindValue(":length", cue->getLength());
        query.bindValue(":hotcue", cue->getHotCue());
        query.bindValue(":label", cue->getLabel());

        if (query.exec()) {
            query.prepare("SELECT last_insert_rowid()");
            if (query.exec()) {
                Q_ASSERT(query.next());
                int id = query.value(0).toInt();
                cue->setId(id);
                cue->setDirty(false);
                QSqlDatabase::database().commit();
                return true;
            } else {
                qDebug() << query.executedQuery() << query.lastError();
            }
        } else {
            qDebug() << query.executedQuery() << query.lastError();
        }
        QSqlDatabase::database().rollback();

    } else {
        // Update cue
        QSqlQuery query("UPDATE " CUE_TABLE " SET "
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
            qDebug() << query.executedQuery() << query.lastError();
        }
    }
    return false;
}

bool CueDAO::deleteCue(Cue* cue) {
    if (cue->getId() != -1) {
        QSqlQuery query("DELETE FROM " CUE_TABLE " WHERE id = :id");
        query.bindValue(":id", cue->getId());
        if (query.exec()) {
            return true;
        } else {
            qDebug() << query.lastError();
        }
    } else {
        return true;
    }
    return false;
}

void CueDAO::saveTrackCues(int trackId, TrackInfoObject* pTrack) {

    // TODO(XXX) transaction, but people who are already in a transaction call
    // this.
    const QList<Cue*>& cueList = pTrack->getCuePoints();
    const QList<Cue*>& oldCueList = getCuesForTrack(trackId);

    QListIterator<Cue*> oldCues(oldCueList);
    QSet<int> oldIds;

    // Build set of old cue ids
    while(oldCues.hasNext()) {
        Cue* cue = oldCues.next();
        oldIds.insert(cue->getId());
    }

    // For each id still in the TIO, save or delete it.
    QListIterator<Cue*> cueIt(cueList);
    while (cueIt.hasNext()) {
        Cue* cue = cueIt.next();
        if (cue->getId() == -1) {
            // New cue
            saveCue(cue);
        } else if (oldIds.contains(cue->getId())) {
            // Update cue
            saveCue(cue);
        } else {
            // Delete cue
            deleteCue(cue);
        }
    }
}
