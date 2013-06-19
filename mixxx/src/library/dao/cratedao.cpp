// cratedao.cpp
// Created 10/22/2009 by RJ Ryan (rryan@mit.edu)

#include <QtDebug>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>

#include "library/dao/cratedao.h"
#include "library/queryutil.h"

CrateDAO::CrateDAO(QSqlDatabase& database)
        : m_database(database) {
}

CrateDAO::~CrateDAO() {
}

void CrateDAO::initialize() {
    qDebug() << "CrateDAO::initialize()";
    updateCratesTitleNum();
}

unsigned int CrateDAO::crateCount() {
    QSqlQuery query(m_database);
    query.prepare("SELECT count(*) FROM " CRATE_TABLE);

    if (!query.exec() || !query.next()) {
        LOG_FAILED_QUERY(query);
        return 0;
    }
    return query.value(0).toInt();
}

int CrateDAO::createCrate(const QString& name) {
    QSqlQuery query(m_database);
    query.prepare("INSERT INTO " CRATE_TABLE " (name) VALUES (:name)");
    query.bindValue(":name", name);

    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return -1;
    }

    int crateId = query.lastInsertId().toInt();
    updateCratesTitleNum();
    emit(added(crateId));
    return crateId;
}

bool CrateDAO::renameCrate(int crateId, const QString& newName) {
    QSqlQuery query(m_database);
    query.prepare("UPDATE " CRATE_TABLE " SET name = :name WHERE id = :id");
    query.bindValue(":name", newName);
    query.bindValue(":id", crateId);

    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return false;
    }
    updateCratesTitleNum();
    emit(renamed(crateId));
    return true;
}

bool CrateDAO::setCrateLocked(int crateId, bool locked) {
    // SQLite3 doesn't support boolean value. Using integer instead.
    int lock = locked ? 1 : 0;
    QSqlQuery query(m_database);
    query.prepare("UPDATE " CRATE_TABLE " SET locked = :lock WHERE id = :id");
    query.bindValue(":lock", lock);
    query.bindValue(":id", crateId);

    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return false;
    }
    emit(lockChanged(crateId));
    return true;
}

bool CrateDAO::isCrateLocked(int crateId) {
    QSqlQuery query(m_database);
    query.prepare("SELECT locked FROM " CRATE_TABLE " WHERE id = :id");
    query.bindValue(":id", crateId);

    if (query.exec()) {
        if (query.next()) {
            int lockValue = query.value(0).toInt();
            return lockValue == 1;
        }
    } else {
        LOG_FAILED_QUERY(query);
    }

    return false;
}

bool CrateDAO::deleteCrate(int crateId) {
    ScopedTransaction transaction(m_database);
    QSqlQuery query(m_database);
    query.prepare("DELETE FROM " CRATE_TRACKS_TABLE " WHERE crate_id = :id");
    query.bindValue(":id", crateId);

    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return false;
    }

    query.prepare("DELETE FROM " CRATE_TABLE " WHERE id = :id");
    query.bindValue(":id", crateId);

    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return false;
    }
    transaction.commit();

    emit(deleted(crateId));
    return true;
}

//int CrateDAO::getCrateIdByName(const QString& name) {
//    QSqlQuery query(m_database);
//    query.prepare("SELECT id FROM " CRATE_TABLE " WHERE name = (:name)");
//    query.bindValue(":name", name);
//    if (query.exec()) {
//        if (query.next()) {
//            int id = query.value(0).toInt();
//            return id;
//        }
//    } else {
//        LOG_FAILED_QUERY(query);
//    }
//    return -1;
//}

int CrateDAO::getCrateIdByName(const QString& name) {
    QString pattern("(.*)\x20(\\(([1-9]\\d*|0)\\))");
    QRegExp rxnum(pattern);

    QSqlQuery query(m_database);
    query.prepare("SELECT id,name FROM " CRATE_TABLE);

    if (query.exec()) {
        while (query.next()) {
            int queryID = query.value(0).toInt();
        	QString queryName = query.value(1).toString();

            if (queryName == name) {
                return queryID;
            } else if (rxnum.exactMatch(queryName)) {
                QString originalName = rxnum.cap(1);
                if (originalName == name) {
                    return queryID;
                }
            }
        }
        return -1;
    } else {
        LOG_FAILED_QUERY(query);
    }
    return -1;
}
int CrateDAO::getCrateId(int position) {
    QSqlQuery query(m_database);
    query.prepare("SELECT id FROM " CRATE_TABLE);
    if (query.exec()) {
        int currentRow = 0;
        while(query.next()) {
            if (currentRow++ == position) {
                int id = query.value(0).toInt();
                return id;
            }
        }
    } else {
        LOG_FAILED_QUERY(query);
    }
    return -1;
}

QString CrateDAO::crateName(int crateId) {
    QSqlQuery query(m_database);
    query.prepare("SELECT name FROM " CRATE_TABLE " WHERE id = (:id)");
    query.bindValue(":id", crateId);
    if (query.exec()) {
        if (query.next()) {
            return query.value(0).toString();
        }
    } else {
        LOG_FAILED_QUERY(query);
    }
    return QString();
}

unsigned int CrateDAO::crateSize(int crateId) {
    QSqlQuery query(m_database);
    query.prepare("SELECT COUNT(*) FROM " CRATE_TRACKS_TABLE " WHERE crate_id = (:id)");
    query.bindValue(":id", crateId);
    if (query.exec()) {
        if (query.next()) {
            return query.value(0).toInt();
        }
    } else {
        LOG_FAILED_QUERY(query);
    }
    return 0;
}

void CrateDAO::copyCrateTracks(int sourceCrateId, int targetCrateId) {
    // Query Tracks from the source Playlist
    QSqlQuery query(m_database);
    query.prepare("SELECT track_id FROM crate_tracks "
                  "WHERE crate_id = :cid");
    query.bindValue(":cid", sourceCrateId);

    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return;
    }

    QList<int> trackIds;
    while (query.next()) {
        trackIds.append(query.value(0).toInt());
    }
    addTracksToCrate(trackIds, targetCrateId);
}

bool CrateDAO::addTrackToCrate(int trackId, int crateId) {
    QSqlQuery query(m_database);
    query.prepare("INSERT INTO " CRATE_TRACKS_TABLE
                  " (crate_id, track_id) VALUES (:crate_id, :track_id)");
    query.bindValue(":crate_id", crateId);
    query.bindValue(":track_id", trackId);

    if (!query.exec()) {
        // It's normal for this query to fail with a constraint violation
        // (e.g. the track is already in the crate)
        LOG_FAILED_QUERY(query);
        return false;
    }
    updateCratesTitleNum();
    emit(trackAdded(crateId, trackId));
    emit(changed(crateId));
    return true;
}


int CrateDAO::addTracksToCrate(QList<int> trackIdList, int crateId) {
    ScopedTransaction transaction(m_database);
    QSqlQuery query(m_database);
    query.prepare("INSERT INTO " CRATE_TRACKS_TABLE " (crate_id, track_id) VALUES (:crate_id, :track_id)");

    for (int i = 0; i < trackIdList.size(); ++i) {
        query.bindValue(":crate_id", crateId);
        query.bindValue(":track_id", trackIdList.at(i));
        if (!query.exec()) {
            LOG_FAILED_QUERY(query);
            // We must emit only those trackID that were added so we need to
            // remove the failed ones.
            trackIdList.removeAt(i);
            --i; // account for reduced size of list
        }
    }
    transaction.commit();
    updateCratesTitleNum();
    // Emitting the trackAdded signals for each trackID outside the transaction
    foreach(int trackId, trackIdList) {
        emit(trackAdded(crateId, trackId));
    }

    emit(changed(crateId));

    // Return the number of tracks successfully added
    return trackIdList.size();
}

bool CrateDAO::removeTrackFromCrate(int trackId, int crateId) {
    QSqlQuery query(m_database);
    query.prepare("DELETE FROM " CRATE_TRACKS_TABLE " WHERE "
                  "crate_id = :crate_id AND track_id = :track_id");
    query.bindValue(":crate_id", crateId);
    query.bindValue(":track_id", trackId);

    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return false;
    }
    updateCratesTitleNum();
    emit(trackRemoved(crateId, trackId));
    emit(changed(crateId));
    return true;
}

bool CrateDAO::removeTracksFromCrate(QList<int> ids, int crateId) {
    QStringList idList;
    foreach (int id, ids) {
        idList << QString::number(id);
    }
    QSqlQuery query(m_database);
    query.prepare(QString("DELETE FROM " CRATE_TRACKS_TABLE " WHERE "
                          "crate_id = :crate_id AND track_id in (%1)")
                  .arg(idList.join(",")));
    query.bindValue(":crate_id", crateId);

    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return false;
    }
    foreach (int trackId, ids) {
        emit(trackRemoved(crateId, trackId));
    }
    updateCratesTitleNum();
    emit(changed(crateId));
    return true;
}

void CrateDAO::removeTracksFromCrates(QList<int> ids) {
    QStringList idList;
    foreach (int id, ids) {
        idList << QString::number(id);
    }
    QSqlQuery query(m_database);
    query.prepare(QString("DELETE FROM crate_tracks "
                          "WHERE track_id in (%1)").arg(idList.join(",")));
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
    }
    updateCratesTitleNum();

    // TODO(XXX) should we emit this for all crates?
    // emit(trackRemoved(crateId, trackId));
    // emit(changed(crateId));
}
void CrateDAO::updateCratesTitleNum() {
    QString pattern("(.*)\x20(\\(([1-9]\\d*|0)\\))");
    QRegExp rxnum(pattern);

    m_database.transaction();
    QSqlQuery selectQuery(m_database);
    //selectQuery.prepare(" SELECT Crates.name,count(*),Crates.id FROM Crates, Crate_tracks "
    //                    " WHERE Crates.id = Crate_tracks.crate_id "
    //                    " GROUP BY Crates.id ");
    selectQuery.prepare(" SELECT Crates.name,Count_table.count,Crates.id FROM Crates INNER JOIN "
                        " (SELECT crate_id, count(*) AS count FROM Crate_tracks GROUP BY crate_id) "
                        " AS Count_table ON Count_table.crate_id = Crates.id ");
    if (!selectQuery.exec()) {
        LOG_FAILED_QUERY(selectQuery);
    } else {
        while (selectQuery.next()) {
            QString newNameWithNum;
            QString oldName = selectQuery.value(0).toString();
            QString tracksNum = selectQuery.value(1).toString();
            int cratesID = selectQuery.value(2).toInt();

            if (!rxnum.exactMatch(oldName)) {
                //qDebug() << "no:"<<oldName;
                newNameWithNum = oldName+" (" + tracksNum + ")";
            } else {
                //qDebug() << "yes:"<<oldName;
                if (rxnum.cap(2) == tracksNum){
                    continue;
                } else {
                    newNameWithNum = oldName.replace(rxnum.cap(2), "(" + tracksNum + ")");
                }
            }

            QSqlQuery updateQuery(m_database);
            updateQuery.prepare("UPDATE Crates SET name = :name WHERE id = :id");
            updateQuery.bindValue(":name", newNameWithNum);
            updateQuery.bindValue(":id", cratesID);

            if (!updateQuery.exec()) {
                LOG_FAILED_QUERY(updateQuery);
                m_database.rollback();
            }
            emit(cratesTitleUpdate(cratesID));
            //qDebug() << "PlaylistName:" <<selectQuery.value(0).toString()
            //		 << "Number of tracks:" << selectQuery.value(1).toInt();
        }
    }

    selectQuery.prepare(" SELECT name,id FROM Crates "
                        " WHERE id NOT IN "
                        " (SELECT DISTINCT crate_id from Crate_tracks) ");
    if (!selectQuery.exec()) {
        LOG_FAILED_QUERY(selectQuery);
    } else {
        while (selectQuery.next()) {
            QString oldName = selectQuery.value(0).toString();
            int cratesID = selectQuery.value(1).toInt();
            if (rxnum.exactMatch(oldName)) {
                QString newNameWithNum = rxnum.cap(1);
                QSqlQuery updateQuery(m_database);
                updateQuery.prepare("UPDATE Crates SET name = :name WHERE id = :id");
                updateQuery.bindValue(":name", newNameWithNum);
                updateQuery.bindValue(":id", cratesID);

                if (!updateQuery.exec()) {
                    LOG_FAILED_QUERY(updateQuery);
                    m_database.rollback();
                }
                emit(cratesTitleUpdate(cratesID));
            }
    	}
    }
    m_database.commit();

}
