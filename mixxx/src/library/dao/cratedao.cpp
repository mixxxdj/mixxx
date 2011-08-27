// cratedao.cpp
// Created 10/22/2009 by RJ Ryan (rryan@mit.edu)

#include <QtDebug>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>

#include "library/dao/cratedao.h"

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
        qDebug() << "crateCount" << query.lastError();
        return 0;
    }
    return query.value(0).toInt();
}

int CrateDAO::createCrate(const QString& name) {
    QSqlQuery query(m_database);

    qDebug() << "createCrate()" << name;
    query.prepare("INSERT INTO " CRATE_TABLE " (name) VALUES (:name)");
    query.bindValue(":name", name);

    if (!query.exec()) {
        qDebug() << query.lastError();
        return -1;
    }

    int crateId = query.lastInsertId().toInt();
    emit(added(crateId));
    return crateId;
}

bool CrateDAO::renameCrate(int crateId, const QString& newName) {
    qDebug() << "renameCrate()";

    Q_ASSERT(m_database.transaction());
    QSqlQuery query;
    query.prepare("UPDATE " CRATE_TABLE " SET name = :name WHERE id = :id");
    query.bindValue(":name", newName);
    query.bindValue(":id", crateId);

    if (!query.exec()) {
        qDebug() << query.executedQuery() << query.lastError();
        Q_ASSERT(m_database.rollback());
        return false;
    }

    Q_ASSERT(m_database.commit());
    emit(renamed(crateId));
    return true;
}

bool CrateDAO::setCrateLocked(int crateId, bool locked) {
    // SQLite3 doesn't support boolean value. Using integer instead.
    int lock = locked ? 1 : 0;

    Q_ASSERT(m_database.transaction());
    QSqlQuery query;
    query.prepare("UPDATE " CRATE_TABLE " SET locked = :lock WHERE id = :id");
    query.bindValue(":lock", lock);
    query.bindValue(":id", crateId);

    if (!query.exec()) {
        qDebug() << query.executedQuery() << query.lastError();
        Q_ASSERT(m_database.rollback());
        return false;
    }

    Q_ASSERT(m_database.commit());
    emit(lockChanged(crateId));
    return true;
}

bool CrateDAO::isCrateLocked(int crateId) {
    QSqlQuery query;
    query.prepare("SELECT locked FROM " CRATE_TABLE " WHERE id = :id");
    query.bindValue(":id", crateId);

    if (query.exec()) {
        if (query.next()) {
            int lockValue = query.value(0).toInt();
            return lockValue == 1;
        }
    } else {
        qDebug() << query.lastError();
    }

    return false;
}

bool CrateDAO::deleteCrate(int crateId) {
    Q_ASSERT(m_database.transaction());

    qDebug() << "deleteCrate()" << crateId;

    QSqlQuery query(m_database);
    query.prepare("DELETE FROM " CRATE_TRACKS_TABLE " WHERE crate_id = :id");
    query.bindValue(":id", crateId);

    if (!query.exec()) {
        qDebug() << query.executedQuery() << query.lastError();
        Q_ASSERT(m_database.rollback());
        return false;
    }

    query.prepare("DELETE FROM " CRATE_TABLE " WHERE id = :id");
    query.bindValue(":id", crateId);

    if (!query.exec()) {
        qDebug() << query.executedQuery() << query.lastError();
        Q_ASSERT(m_database.rollback());
        return false;
    }

    Q_ASSERT(m_database.commit());

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
        qDebug() << query.lastError();
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
        qDebug() << query.lastError();
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
        qDebug() << query.lastError();
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
        qDebug() << query.lastError();
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
        qDebug() << query.lastError();
        return false;
    }

    emit(trackAdded(crateId, trackId));
    emit(changed(crateId));
    return true;
}

bool CrateDAO::removeTrackFromCrate(int trackId, int crateId) {
    QSqlQuery query(m_database);

    query.prepare("DELETE FROM " CRATE_TRACKS_TABLE " WHERE "
                  "crate_id = :crate_id AND track_id = :track_id");
    query.bindValue(":crate_id", crateId);
    query.bindValue(":track_id", trackId);

    if (!query.exec()) {
        qDebug() << query.lastError();
        return false;
    }

    emit(trackRemoved(crateId, trackId));
    emit(changed(crateId));
    return true;
}
