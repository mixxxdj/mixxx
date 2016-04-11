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

    populateCrateMembershipCache();
}

void CrateDAO::populateCrateMembershipCache() {
    // Minor optimization: reserve space in m_cratesTrackIsIn.
    QSqlQuery query(m_database);
    query.prepare("SELECT COUNT(*) from " CRATE_TRACKS_TABLE);
    if (query.exec() && query.next()) {
        m_cratesTrackIsIn.reserve(query.value(0).toInt());
    } else {
        LOG_FAILED_QUERY(query);
    }

    // now fetch all Tracks from all crates and insert them into the hashmap
    query.prepare("SELECT track_id, crate_id from " CRATE_TRACKS_TABLE);
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
    }

    const int trackIdColumn = query.record().indexOf("track_id");
    const int crateIdColumn = query.record().indexOf("crate_id");
    while (query.next()) {
        TrackId trackId(query.value(trackIdColumn));
        int crateId = query.value(crateIdColumn).toInt();
        m_cratesTrackIsIn.insert(std::move(trackId), crateId);
    }
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

bool CrateDAO::renameCrate(const int crateId, const QString& newName) {
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

bool CrateDAO::setCrateLocked(const int crateId, const bool locked) {
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

bool CrateDAO::isCrateLocked(const int crateId) {
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

QList<TrackId> CrateDAO::getTrackIds(const int crateId) {
    QList<TrackId> trackIds;

    QSqlQuery query(m_database);
    query.prepare("SELECT track_id from crate_tracks WHERE crate_id = :id");
    query.bindValue(":id", crateId);
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return trackIds;
    }
    const int trackIdColumn = query.record().indexOf("track_id");
    while (query.next()) {
        trackIds.append(TrackId(query.value(trackIdColumn)));
    }

    return trackIds;
}

#ifdef __AUTODJCRATES__

bool CrateDAO::setCrateInAutoDj(int crateId, bool bIn) {
    // SQLite3 doesn't support boolean value. Using integer instead.
    int iIn = bIn ? 1 : 0;

    // Mark this crate as being in/out-of the auto-DJ list.
    QSqlQuery query(m_database);
    // UPDATE crates SET autodj = :in WHERE id = :id AND autodj = :existing;
    query.prepare(QString("UPDATE " CRATE_TABLE
        " SET %1 = :in WHERE %2 = :id AND %1 = :existing")
        .arg(CRATETABLE_AUTODJ_SOURCE)     // %1
        .arg(CRATETABLE_ID));       // %2
    query.bindValue(":in", iIn);
    query.bindValue(":id", crateId);
    query.bindValue(":existing", 1 - iIn);

    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return false;
    }

    // Notify listeners if the auto-DJ status of a crate has changed.
    bool bChange = (query.numRowsAffected() > 0);
    if (bChange) {
        emit(autoDjChanged(crateId, bIn));
    }

    // Let our caller know if there was a change.
    return bChange;
}

bool CrateDAO::isCrateInAutoDj(int crateId) {
    // Query the database for this crate's auto-DJ status.
    QSqlQuery query(m_database);
    query.setForwardOnly(true);
    // SELECT autodj FROM crates WHERE id = :id;
    query.prepare(QString("SELECT %1 FROM " CRATE_TABLE " WHERE %2 = :id")
        .arg(CRATETABLE_AUTODJ_SOURCE) // %1
        .arg(CRATETABLE_ID)); // %2
    query.bindValue(":id", crateId);

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

QList<int> CrateDAO::getCrateTracks(int crateId) {
    // Get all track IDs that belong to this crate.
    QSqlQuery query(m_database);
    query.setForwardOnly(true);
    // SELECT track_id FROM crate_tracks WHERE crate_id = :id;
    query.prepare(QString("SELECT %1 FROM " CRATE_TRACKS_TABLE
        " WHERE %2 = :id")
        .arg(CRATETRACKSTABLE_TRACKID)      // %1
        .arg(CRATETRACKSTABLE_CRATEID));    // %2
    query.bindValue(":id", crateId);
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

void CrateDAO::getAutoDjCrates(bool trackSource, QMap<QString,int>* pCrateMap) {
    // Get the name and ID number of every crate in the auto-DJ queue.
    QSqlQuery query(m_database);
    query.setForwardOnly(true);
    // SELECT name, id FROM crates WHERE autodj = 1 ORDER BY name;
    query.prepare(QString("SELECT %1, %2 FROM " CRATE_TABLE
            " WHERE %3 = :in ORDER BY %1")
            .arg(CRATETABLE_NAME, // %1
                 CRATETABLE_ID, // %2
                 CRATETABLE_AUTODJ_SOURCE)); // %3
    query.bindValue(":in", (trackSource) ? 1 : 0);
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return;
    }

    // Create a map between the crate name and its ID number.
    while (query.next()) {
        pCrateMap->insert(query.value(0).toString(), query.value(1).toInt());
    }
}

#endif // __AUTODJCRATES__

bool CrateDAO::deleteCrate(const int crateId) {
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

    // Update in-memory map
    for (QMultiHash<TrackId, int>::iterator it = m_cratesTrackIsIn.begin();
         it != m_cratesTrackIsIn.end();) {
        if (it.value() == crateId) {
            it = m_cratesTrackIsIn.erase(it);
        } else {
            ++it;
        }
    }

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

int CrateDAO::getCrateId(const int position) {
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

QString CrateDAO::crateName(const int crateId) {
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

unsigned int CrateDAO::crateSize(const int crateId) {
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

void CrateDAO::copyCrateTracks(const int sourceCrateId, const int targetCrateId) {
    // Query Tracks from the source Playlist
    QSqlQuery query(m_database);
    query.prepare("SELECT track_id FROM crate_tracks "
                  "WHERE crate_id = :cid");
    query.bindValue(":cid", sourceCrateId);

    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return;
    }

    QList<TrackId> trackIds;
    while (query.next()) {
        trackIds.append(TrackId(query.value(0)));
    }
    addTracksToCrate(targetCrateId, &trackIds);
}

bool CrateDAO::addTrackToCrate(TrackId trackId, const int crateId) {
    QSqlQuery query(m_database);
    query.prepare("INSERT INTO " CRATE_TRACKS_TABLE
                  " (crate_id, track_id) VALUES (:crate_id, :track_id)");
    query.bindValue(":crate_id", crateId);
    query.bindValue(":track_id", trackId.toVariant());

    if (!query.exec()) {
        // It's normal for this query to fail with a constraint violation
        // (e.g. the track is already in the crate)
        LOG_FAILED_QUERY(query);
        return false;
    }

    m_cratesTrackIsIn.insert(trackId, crateId);
    emit(trackAdded(crateId, trackId));
    emit(changed(crateId));
    return true;
}


int CrateDAO::addTracksToCrate(const int crateId, QList<TrackId>* trackIdList) {
    ScopedTransaction transaction(m_database);
    QSqlQuery query(m_database);
    query.prepare("INSERT INTO " CRATE_TRACKS_TABLE " (crate_id, track_id) VALUES (:crate_id, :track_id)");

    for (int i = 0; i < trackIdList->size(); ++i) {
        query.bindValue(":crate_id", crateId);
        query.bindValue(":track_id", trackIdList->at(i).toVariant());
        if (!query.exec()) {
            LOG_FAILED_QUERY(query);
            // We must emit only those trackID that were added so we need to
            // remove the failed ones.
            trackIdList->removeAt(i);
            --i; // account for reduced size of list
        }
    }
    transaction.commit();

    // Emitting the trackAdded signals for each trackID outside the transaction
    for (const auto& trackId: *trackIdList) {
        m_cratesTrackIsIn.insert(trackId, crateId);
        emit(trackAdded(crateId, trackId));
    }

    emit(changed(crateId));

    // Return the number of tracks successfully added
    return trackIdList->size();
}

bool CrateDAO::removeTrackFromCrate(TrackId trackId, const int crateId) {
    QSqlQuery query(m_database);
    query.prepare("DELETE FROM " CRATE_TRACKS_TABLE " WHERE "
                  "crate_id = :crate_id AND track_id = :track_id");
    query.bindValue(":crate_id", crateId);
    query.bindValue(":track_id", trackId.toVariant());

    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return false;
    }

    m_cratesTrackIsIn.remove(trackId, crateId);
    emit(trackRemoved(crateId, trackId));
    emit(changed(crateId));
    return true;
}

bool CrateDAO::removeTracksFromCrate(const QList<TrackId>& trackIds, const int crateId) {
    QStringList idList;
    for (const auto& trackId: trackIds) {
        idList << trackId.toString();
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
    for (const auto& trackId: trackIds) {
        m_cratesTrackIsIn.remove(trackId, crateId);
        emit(trackRemoved(crateId, trackId));
    }
    emit(changed(crateId));
    return true;
}

void CrateDAO::removeTracksFromCrates(const QList<TrackId>& trackIds) {
    QStringList idList;
    for (const auto& trackId: trackIds) {
        idList << trackId.toString();
    }
    QSqlQuery query(m_database);
    query.prepare(QString("DELETE FROM crate_tracks "
                          "WHERE track_id in (%1)").arg(idList.join(",")));
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
    }

    // remove those tracks from memory-map
    for (const auto& trackId: trackIds) {
        m_cratesTrackIsIn.remove(trackId);
    }

    // TODO(XXX) should we emit this for all crates?
    // emit(trackRemoved(crateId, trackId));
    // emit(changed(crateId));
}

bool CrateDAO::isTrackInCrate(TrackId trackId, const int crateId) {
    return m_cratesTrackIsIn.contains(trackId, crateId);
}

void CrateDAO::getCratesTrackIsIn(TrackId trackId,
                                  QSet<int>* crateSet) const {
    crateSet->clear();
    for (QHash<TrackId, int>::const_iterator it = m_cratesTrackIsIn.find(trackId);
         it != m_cratesTrackIsIn.end() && it.key() == trackId; ++it) {
        crateSet->insert(it.value());
    }
}
