#include "util/db/sqllikewildcards.h"
#include "library/queryutil.h"
#include "library/features/crates/cratehierarchy.h"
#include "util/logger.h"

namespace {

const mixxx::Logger kLogger("CrateHierarchy");

} // anonymus namespace

void CrateHierarchy::initialize(const QSqlDatabase& database) {
    m_database = database;
}

void CrateHierarchy::reset(const CrateStorage* pCrateStorage) {
    resetClosure();
    initClosure(pCrateStorage->selectCrates());
    resetPath();
    generateAllPaths(pCrateStorage->selectCrates());
}

bool CrateHierarchy::onUpdatingCrate(const Crate& crate, const CrateStorage* pCrateStorage) {
    Q_UNUSED(crate);
    resetPath();
    return generateAllPaths(pCrateStorage->selectCrates());
}

void CrateHierarchy::addCrateToHierarchy(const Crate& crate, const Crate& parent) {
    if (parent.getId().isValid()) {
        initClosureForCrate(crate.getId());
        if (insertIntoClosure(parent.getId(), crate.getId())) {
            generateCratePaths(crate);
        }
    } else {
        initClosureForCrate(crate.getId());
        generateCratePaths(crate);
    }
}

uint CrateHierarchy::countCratesInClosure() const {
    FwdSqlQuery query(
      m_database, QString(
        "SELECT COUNT(*) FROM %1 "
        "WHERE %2 = %3").arg(
          CRATE_CLOSURE_TABLE,
          CLOSURE_PARENTID,
          CLOSURE_CHILDID));
    if (query.execPrepared() && query.next()) {
        uint result = query.fieldValue(0).toUInt();
        DEBUG_ASSERT(!query.next());
        return result;
    } else {
        kLogger.warning()
            << "Closure is empty";
        return 0;
    }
}

void CrateHierarchy::resetClosure() const {
    FwdSqlQuery query(
      m_database, QString(
        "DELETE FROM %1").arg(
          CRATE_CLOSURE_TABLE));
    if (!query.isPrepared()) {
        return;
    }
    if (!query.execPrepared()) {
        return;
    }
}

bool CrateHierarchy::initClosure(CrateSelectResult crates) const {
    std::vector<CrateId> crateIds;

    Crate crate;

    while (crates.populateNext(&crate)) {
        crateIds.push_back(crate.getId());
    }

    FwdSqlQuery query(
      m_database, QString(
        "INSERT INTO %1 VALUES("
        ":parent, :child, 0)").arg(
          CRATE_CLOSURE_TABLE));
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
    FwdSqlQuery query(
      m_database, QString(
        "DELETE FROM %1").arg(
          CRATE_PATH_TABLE));
    if (!query.isPrepared()) {
        return;
    }
    if (!query.execPrepared()) {
        return;
    }
}

bool CrateHierarchy::writeCratePaths(CrateId id, QString namePath, QString idPath) const {
    FwdSqlQuery query(
      m_database, QString(
        "INSERT INTO %1 "
        "VALUES (:id, :idPath, :namePath)").arg(
          CRATE_PATH_TABLE));
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

bool CrateHierarchy::generateCratePaths(Crate crate) const {
    QSqlQuery query(m_database);

    query.prepare(
      QString(
        "SELECT p.%1, p.%2 FROM %3 "
        "JOIN %4 p ON %5 = p.%2 "
        "JOIN %4 c ON %6 = c.%2 "
        "where c.%2 = :childId and %7 != 0 "
        "ORDER BY %7 DESC").arg(
          CRATETABLE_NAME,
          CRATETABLE_ID,
          CRATE_CLOSURE_TABLE,
          CRATE_TABLE,
          CLOSURE_PARENTID,
          CLOSURE_CHILDID,
          CLOSURE_DEPTH));

    query.bindValue(":childId", crate.getId().toString());
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

    return writeCratePaths(crate.getId(), namePath, idPath);
}

bool CrateHierarchy::generateAllPaths(CrateSelectResult crates) const {
    Crate crate;
    while (crates.populateNext(&crate)) {
        generateCratePaths(crate);
    }

    return true;
}


bool CrateHierarchy::findParentAndChildIdFromPath(CrateId& parentId,
                                                CrateId& childId,
                                                const QString& idPath) const {
    QStringList ids = idPath.split("/", QString::SkipEmptyParts);

    // get the last item (childId)
    childId = CrateId(ids.back().toInt());
    if (ids.size() > 1) {
        // get the second to last item (parentId)
        parentId = CrateId(ids.at(ids.size() - 2));
    } else {
        // if there isn't one return false
        return false;
    }
    return true;
}


bool CrateHierarchy::initClosureForCrate(CrateId id) const {
    FwdSqlQuery query(
      m_database, QString(
        "INSERT INTO %1 "
        "VALUES(:parent, :child, 0)").arg(
          CRATE_CLOSURE_TABLE));

    if (!query.isPrepared()) {
        return false;
    }

    // closure dependacy
    query.bindValue(":parent", id);
    query.bindValue(":child", id);
    if (!query.execPrepared()) {
        return false;
    }

    return true;
}

bool CrateHierarchy::insertIntoClosure(CrateId parent, CrateId child) const {
    FwdSqlQuery query(
      m_database, QString(
        "INSERT INTO %1(%2, %3, %4) "
        "SELECT p.%2, c.%3, p.%4 + c.%4 + 1 "
        "FROM %1 p, %1 c "
        "WHERE p.%3 = :parent AND c.%2 = :child").arg(
          CRATE_CLOSURE_TABLE,
          CLOSURE_PARENTID,
          CLOSURE_CHILDID,
          CLOSURE_DEPTH));

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
    {
        FwdSqlQuery query(
          m_database, QString(
            "DELETE FROM cratePath WHERE crateId = :id"));
        query.bindValue(":id", id);
        if (!query.isPrepared()) {
            return;
        }
        if (!query.execPrepared()) {
            return;
        }
    }

    {
        FwdSqlQuery query(
          m_database, QString(
            "DELETE FROM crateClosure WHERE childId = :id"));
        query.bindValue(":id", id);
        if (!query.isPrepared()) {
            return;
        }
        if (!query.execPrepared()) {
            return;
        }
    }
}

QString CrateHierarchy::formatQueryForTrackIdsByCratePathLike(const QString& cratePathLike) const {
    FieldEscaper escaper(m_database);
    QString escapedArgument = escaper.escapeString(kSqlLikeMatchAll + cratePathLike + kSqlLikeMatchAll);

    return QString(
        "SELECT DISTINCT %1 FROM %2 "
        "JOIN %3 ON %4=%5 "
        "WHERE %6 LIKE %7 "
        "ORDER BY %1").arg(
            CRATETRACKSTABLE_TRACKID,
            CRATE_TRACKS_TABLE,
            CRATE_PATH_TABLE,
            PATHTABLE_CRATEID,
            CRATETRACKSTABLE_CRATEID,
            PATHTABLE_NAME_PATH,
            escapedArgument);
}

bool CrateHierarchy::canBeRenamed(const QString& newName,
                                  const Crate& crate,
                                  const CrateId parentId) const {
    if (parentId.isValid()) {
        if (collectParentCrateNames(crate).contains(newName) ||
            collectChildCrateNames(crate).contains(newName)) {
            return false;
        }
    } else {
        if (collectRootCrateNames().contains(newName) ||
            collectChildCrateNames(crate).contains(newName)) {
            return false;
        }
    }
    return true;
}

bool CrateHierarchy::nameIsValidForHierarchy(const QString& newName,
                                             const Crate parent) const {
    if (parent.getId().isValid()) {
        if (tokenizeCratePath(parent.getId()).contains(newName) ||
            collectChildCrateNames(parent).contains(newName)) {
            return false;
        }
    } else {
        return !collectRootCrateNames().contains(newName);
    }
    return true;
}

QString CrateHierarchy::getNamePathFromId(CrateId id) const {
    FwdSqlQuery query(
      m_database, QString(
        "SELECT %1 FROM %2 "
        "WHERE %3 = :id").arg(
          PATHTABLE_NAME_PATH,
          CRATE_PATH_TABLE,
          PATHTABLE_CRATEID));

    query.bindValue(":id", id);
    if (query.execPrepared() && query.next()) {
        return query.fieldValue(0).toString();
    }
    return QString();
}

bool CrateHierarchy::hasChildren(CrateId id) const {
    FwdSqlQuery query(
      m_database, QString(
        "SELECT COUNT(*) FROM %1 "
        "WHERE %2 = :id AND %3 != 0").arg(
          CRATE_CLOSURE_TABLE,
          CLOSURE_PARENTID,
          CLOSURE_DEPTH));

    query.bindValue(":id", id);
    if (query.execPrepared() && query.next()) {
        return query.fieldValue(0).toUInt() != 0;
    }
    return false;
}

CrateId CrateHierarchy::getParentId(const CrateId id) const {
    FwdSqlQuery query(
      m_database, QString(
        "SELECT %1 FROM %2 "
        "WHERE %3 = :id "
        "AND %4 = 1").arg(
          CLOSURE_PARENTID,
          CRATE_CLOSURE_TABLE,
          CLOSURE_CHILDID,
          CLOSURE_DEPTH));

    query.bindValue(":id", id);
    if (query.execPrepared() && query.next()) {
        return CrateId(query.fieldValue(0).toInt());
    }
    // no parent found
    return CrateId();
}

QStringList CrateHierarchy::collectIdPaths() const {
    QSqlQuery query(m_database);

    query.prepare(
      QString(
        "SELECT %1 FROM %2 "
        "ORDER BY %3").arg(
          PATHTABLE_ID_PATH,
          CRATE_PATH_TABLE,
          PATHTABLE_NAME_PATH));

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

QStringList CrateHierarchy::tokenizeCratePath(CrateId id) const {
    FwdSqlQuery query(
      m_database, QString(
        "SELECT %1 FROM %2 "
        "WHERE %3 = :id").arg(
          PATHTABLE_NAME_PATH,
          CRATE_PATH_TABLE,
          PATHTABLE_CRATEID));

    query.bindValue(":id", id);
    if (query.execPrepared() && query.next()) {
            return query.fieldValue(0).toString().
                split("/", QString::SkipEmptyParts);
    }

    return QStringList();
}

QStringList CrateHierarchy::collectRootCrateNames() const {
    QStringList names;

    FwdSqlQuery query(
      m_database, QString(
        "SELECT %1 FROM %2 "
        "JOIN %3 ON %4 = %5 "
        "GROUP BY %5 "
        "HAVING COUNT(*) = 1").arg(
          CRATETABLE_NAME,
          CRATE_CLOSURE_TABLE,
          CRATE_TABLE,
          CRATETABLE_ID,
          CLOSURE_CHILDID));

    if (query.execPrepared())
        while (query.next()) {
            names << query.fieldValue(0).toString();
    }

    return names;
}

QStringList CrateHierarchy::collectParentCrateNames(const Crate& crate) const {
    FwdSqlQuery query(
      m_database, QString(
        "SELECT p.%1 FROM %2 "
        "JOIN %3 c ON c.%4 = %5 "
        "JOIN %3 p ON p.%4 = %6 "
        "WHERE c.%4 = :id "
        "AND %7 != 0").arg(
          CRATETABLE_NAME,
          CRATE_CLOSURE_TABLE,
          CRATE_TABLE,
          CRATETABLE_ID,
          CLOSURE_CHILDID,
          CLOSURE_PARENTID,
          CLOSURE_DEPTH));

    query.bindValue("id", crate.getId());

    QStringList names;

    if (query.execPrepared())
        while (query.next()) {
            names << query.fieldValue(0).toString();
    }

    return names;
}

QStringList CrateHierarchy::collectChildCrateNames(const Crate& crate) const {
    FwdSqlQuery query(
      m_database, QString(
        "SELECT c.%1 FROM %2 "
        "JOIN %3 c ON c.%4 = %5 "
        "JOIN %3 p ON p.%4 = %6 "
        "WHERE p.%4 = :id "
        "AND %7 != 0").arg(
          CRATETABLE_NAME,
          CRATE_CLOSURE_TABLE,
          CRATE_TABLE,
          CRATETABLE_ID,
          CLOSURE_CHILDID,
          CLOSURE_PARENTID,
          CLOSURE_DEPTH));

    query.bindValue("id", crate.getId());

    QStringList names;

    if (query.execPrepared())
        while (query.next()) {
            names << query.fieldValue(0).toString();
    }

    return names;
}

QStringList CrateHierarchy::collectChildCrateIds(const Crate& crate) const {
    FwdSqlQuery query(
      m_database, QString(
        "SELECT c.%1 FROM %2 "
        "JOIN %3 c ON c.%4 = %5 "
        "JOIN %3 p ON p.%4 = %6 "
        "WHERE p.%4 = :id "
        "AND %7 != 0 "
        // this is the deletion order
        // bottom to top
        "ORDER BY %7 DESC").arg(
          CRATETABLE_ID,
          CRATE_CLOSURE_TABLE,
          CRATE_TABLE,
          CRATETABLE_ID,
          CLOSURE_CHILDID,
          CLOSURE_PARENTID,
          CLOSURE_DEPTH));

    query.bindValue("id", crate.getId());

    QStringList ids;

    if (query.execPrepared())
        while (query.next()) {
            ids << query.fieldValue(0).toString();
    }

    return ids;
}

QString CrateHierarchy::formatQueryForChildCrateIds(const Crate& crate) const {
    QString query = QString(
      "SELECT c.%1 FROM %2 "
      "JOIN %3 c ON c.%4 = %5 "
      "JOIN %3 p ON p.%4 = %6 "
      "WHERE p.%4 = %7 "
      "AND %8 != 0").arg(
        CRATETABLE_ID,
        CRATE_CLOSURE_TABLE,
        CRATE_TABLE,
        CRATETABLE_ID,
        CLOSURE_CHILDID,
        CLOSURE_PARENTID,
        crate.getId().toString(),
        CLOSURE_DEPTH);

    return query;
}
