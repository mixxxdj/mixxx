#include "util/db/fwdsqlquery.h"

#include "library/features/crates/cratehierarchy.h"
#include "library/features/crates/cratestorage.h"
#include "library/trackcollection.h"

uint CrateHierarchy::countCratesOnClosure() const {
    FwdSqlQuery query(m_database, QString(
             "SELECT COUNT(*) FROM crateClosure WHERE parentId = childId"));
    if (query.execPrepared() && query.next()) {
        uint result = query.fieldValue(0).toUInt();
        DEBUG_ASSERT(!query.next());
        return result;
    } else {
        return 0;
    }
}

bool CrateHierarchy::closureIsValid() const {
    return countCratesOnClosure() == m_pTrackCollection->crates().countCrates();
}

void CrateHierarchy::resetClosure() const {
    FwdSqlQuery query(m_database, QString(
         "DELETE FROM crateClosure"));
    if (!query.isPrepared()) {
        return;
    }
    if (!query.execPrepared()) {
        return;
    }
}


bool CrateHierarchy::initClosure() const {
    std::vector<CrateId> crateIds;

    CrateSelectResult crates(m_pTrackCollection->crates().selectCrates());
    Crate crate;

    while (crates.populateNext(&crate)) {
        crateIds.push_back(crate.getId());
    }

    FwdSqlQuery query(m_database, QString(
      "INSERT INTO crateClosure VALUES("
      ":parent, :child, 0)"));
    if (!query.isPrepared()) {
        return false;
    }

    while (crateIds.size() != 0) {
        query.bindValue(":parent", crateIds.back());
        query.bindValue(":child", crateIds.back());
        if (!query.execPrepared()) {
            return false;
        }
        crateIds.pop_back();
    }

    return true;
}

void CrateHierarchy::resetPath() const {
    FwdSqlQuery query(m_database, QString(
         "DELETE FROM cratePath"));
    if (!query.isPrepared()) {
        return;
    }
    if (!query.execPrepared()) {
        return;
    }
}

bool CrateHierarchy::writeCratePaths(CrateId id, QString namePath, QString idPath) const {
    FwdSqlQuery query(m_database, QString(
                                          "INSERT INTO cratePath(crateId, namePath, idPath)"
                                          "VALUES (:id, :namePath, :idPath)"));
    if (!query.isPrepared()) {
        return false;
    }
    query.bindValue(":id", id);
    query.bindValue(":namePath", namePath);
    query.bindValue(":idPath", idPath);
    if (!query.execPrepared()) {
        return false;
    }

    return true;
}

bool CrateHierarchy::generateAllPaths() const {
    CrateSelectResult crates(m_pTrackCollection->crates().selectCrates());
    Crate crate;
    while (crates.populateNext(&crate)) {
        QSqlQuery query(m_database);

        query.prepare(QString(
               "SELECT p.name, p.id FROM crateClosure "
               "JOIN crates p ON parentId = p.id "
               "JOIN crates c ON childId = c.id "
               "where c.name = :child and depth != 0 "
               "ORDER BY depth DESC"));

        query.bindValue(":child", crate.getName());
        query.setForwardOnly(true);
        QString namePath;
        QString idPath;

        if (query.exec()) {
            while (query.next()) {
                namePath = namePath + "/" + query.value(0).toString();
                idPath = idPath + "/" + query.value(1).toString();
            }
        } else {
            return false;
        }

        namePath = namePath + "/" + crate.getName();
        idPath = idPath + "/" + crate.getId().toString();

        if(!writeCratePaths(crate.getId(), namePath, idPath)) {
            return false;
        }
    }

    return true;
}

bool CrateHierarchy::generateCratePaths(Crate crate) const {
    QSqlQuery query(m_database);

    query.prepare(QString(
               "SELECT p.name, p.id FROM crateClosure "
               "JOIN crates p ON parentId = p.id "
               "JOIN crates c ON childId = c.id "
               "where c.name = :child and depth != 0 "
               "ORDER BY depth DESC"));

    query.bindValue(":child", crate.getName());
    query.setForwardOnly(true);
    QString namePath;
    QString idPath;

    if (query.exec()) {
        while (query.next()) {
            namePath = namePath + "/" + query.value(0).toString();
            idPath = idPath + "/" + query.value(1).toString();
        }
    } else {
        return false;
    }

    namePath = namePath + "/" + crate.getName();
    idPath = idPath + "/" + crate.getId().toString();

    if(!writeCratePaths(crate.getId(), namePath, idPath)) {
        return false;
    }

    return true;
}

bool CrateHierarchy::initClosureForCrate(CrateId id) const {
    FwdSqlQuery query(m_database, QString(
      "INSERT INTO crateClosure VALUES("
      ":parent, :child, 0)"));
    if (!query.isPrepared()) {
        return false;
    }

    query.bindValue(":parent", id);
    query.bindValue(":child", id);
    if (!query.execPrepared()) {
        return false;
    }

    return true;
}

bool CrateHierarchy::insertIntoClosure(CrateId parent, CrateId child) const {
    FwdSqlQuery query(m_database, QString(
        "INSERT INTO crateClosure(parentId, childId, depth) "
        "SELECT p.parentId, c.childId, p.depth + c.depth + 1 "
        "FROM crateClosure p, crateClosure c "
        "WHERE p.childId = :parent AND c.parentId = :child"));

    if (!query.isPrepared()) {
        return false;
    }

    query.bindValue(":parent", parent);
    query.bindValue(":child", child);

    if (!query.execPrepared()) {
        return false;
    }

    return true;
}

void CrateHierarchy::deleteCrate(CrateId id) const {
    FwdSqlQuery query(m_database, QString(
        "DELETE FROM cratePath WHERE crateId = :id"));
    query.bindValue(":id", id);
    if (!query.isPrepared()) {
        return;
    }
    if (!query.execPrepared()) {
        return;
    }

    FwdSqlQuery query2(m_database, QString(
        "DELETE FROM crateClosure WHERE childId = :id"));
    query2.bindValue(":id", id);
    if (!query2.isPrepared()) {
        return;
    }
    if (!query2.execPrepared()) {
        return;
    }
}

bool CrateHierarchy::hasChildern(CrateId id) const {
    FwdSqlQuery query(m_database, QString(
       "SELECT COUNT(*) FROM crateClosure "
       "WHERE parentId = :id AND depth != 0"));
    query.bindValue(":id", id);
    if (query.execPrepared() && query.next()) {
        return query.fieldValue(0).toUInt() != 0;
    }
    return false;
}

QStringList CrateHierarchy::collectIdPaths() const {
    QSqlQuery query(m_database);

    query.prepare(QString(
         "SELECT idPath FROM cratePath ORDER BY namePath"));

    query.setForwardOnly(true);

    QString idPath;
    QStringList idPaths = {};

    if (query.exec()) {
        while (query.next()) {
            idPath = query.value(0).toString();
            idPaths.append(idPath);
        }
    } else {
        return idPaths; //returns empty list if it fails
    }

    return idPaths;
}
