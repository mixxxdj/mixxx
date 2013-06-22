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
    emit(renamed(crateId, newName));
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

#ifdef __AUTODJCRATES__

bool CrateDAO::setCrateInAutoDj(int a_iCrateId, bool a_bIn) {
    // SQLite3 doesn't support boolean value. Using integer instead.
    int iIn = a_bIn ? 1 : 0;

    // Mark this crate as being in/out-of the auto-DJ list.
    QSqlQuery query(m_database);
    // UPDATE crates SET autodj = :in WHERE id = :id AND autodj = :existing;
    query.prepare(QString("UPDATE " CRATE_TABLE
        " SET %1 = :in WHERE %2 = :id AND %1 = :existing")
        .arg(CRATETABLE_AUTODJ)     // %1
        .arg(CRATETABLE_ID));       // %2
    query.bindValue(":in", iIn);
    query.bindValue(":id", a_iCrateId);
    query.bindValue(":existing", 1 - iIn);

    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return false;
    }

    // Notify listeners if the auto-DJ status of a crate has changed.
    bool bChange = (query.numRowsAffected() > 0);
    if (bChange) {
        emit(autoDjChanged(a_iCrateId,a_bIn));
    }

    // Let our caller know if there was a change.
    return bChange;
}

bool CrateDAO::isCrateInAutoDj(int a_iCrateId) {
    // Query the database for this crate's auto-DJ status.
    QSqlQuery query(m_database);
    query.setForwardOnly(true);
    // SELECT autodj FROM crates WHERE id = :id;
    query.prepare(QString("SELECT %1 FROM " CRATE_TABLE " WHERE %2 = :id")
        .arg(CRATETABLE_AUTODJ)     // %1
        .arg(CRATETABLE_ID));       // %2
    query.bindValue(":id", a_iCrateId);

    if (query.exec()) {
        if (query.next()) {
            //SQLite3 doesn't support boolean values, so convert the integer.
            int inValue = query.value(0).toInt();
            return inValue == 1;
        }
    } else {
        LOG_FAILED_QUERY(query);
    }

    return false;
}

QList<int> CrateDAO::getCrateTracks(int a_iCrateId) {
    // Get all track IDs that belong to this crate.
    QSqlQuery query(m_database);
    query.setForwardOnly(true);
    // SELECT track_id FROM crate_tracks WHERE crate_id = :id;
    query.prepare(QString("SELECT %1 FROM " CRATE_TRACKS_TABLE
        " WHERE %2 = :id")
        .arg(CRATETRACKSTABLE_TRACKID)      // %1
        .arg(CRATETRACKSTABLE_CRATEID));    // %2
    query.bindValue(":id", a_iCrateId);
    QList<int> ids;
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return ids;
    }

    // Put all those track IDs into a list.
    while (query.next()) {
        ids.append(query.value(0).toInt());
    }

    // Return the list to our caller.
    return ids;
}

void CrateDAO::getAutoDjCrates(QMap<QString,int> &ao_rCrateMap, bool a_bIn) {
    // Get the name and ID number of every crate in the auto-DJ queue.
    QSqlQuery query(m_database);
    query.setForwardOnly(true);
    // SELECT name, id FROM crates WHERE autodj = 1 ORDER BY name;
    query.prepare(QString("SELECT %1, %2 FROM " CRATE_TABLE
        " WHERE %3 = :in ORDER BY %1")
        .arg(CRATETABLE_NAME)        // %1
        .arg(CRATETABLE_ID)          // %2
        .arg(CRATETABLE_AUTODJ));    // %3
    query.bindValue(":in", (a_bIn) ? 1 : 0);
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return;
    }

    // Create a map between the crate name and its ID number.
    while (query.next()) {
        ao_rCrateMap.insert(query.value(0).toString(), query.value(1).toInt());
    }
}

#endif // __AUTODJCRATES__

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

int CrateDAO::getCrateIdByName(const QString& name) {
    QSqlQuery query(m_database);
    query.prepare("SELECT id FROM " CRATE_TABLE " WHERE name = (:name)");
    query.bindValue(":name", name);
    if (query.exec()) {
        if (query.next()) {
            int id = query.value(0).toInt();
            return id;
        }
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

    // TODO(XXX) should we emit this for all crates?
    // emit(trackRemoved(crateId, trackId));
    // emit(changed(crateId));
}
