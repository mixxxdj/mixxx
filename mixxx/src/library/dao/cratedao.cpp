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

    int crateAddFails = 0;
    for (int i = 0; i < trackIdList.size(); ++i) {
        query.bindValue(":crate_id", crateId);
        query.bindValue(":track_id", trackIdList.at(i));
        if (!query.exec()) {
            LOG_FAILED_QUERY(query);
            crateAddFails++;
            // We must emit only those trackID that were added so we need to
            // remove the failed ones.
            trackIdList.removeAt(i);
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
