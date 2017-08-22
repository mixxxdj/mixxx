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

bool CrateHierarchy::writeCratePaths(const CrateId& id,
                                     const QString& namePath,
                                     const QString& idPath) const {
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

bool CrateHierarchy::generateCratePaths(const Crate& crate) const {
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


bool CrateHierarchy::initClosureForCrate(const CrateId& id) const {
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

bool CrateHierarchy::insertIntoClosure(const CrateId& parent,
                                       const CrateId& child) const {
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

void CrateHierarchy::deleteCrate(const CrateId& id) const {
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

void CrateHierarchy::moveCrate(const Crate &crate,
                               CrateId destinationCrateId) const {
    {
        QString subquery(
          QString(
            "SELECT %1 FROM %2 "
            "WHERE %3 = %4").arg(
              CLOSURE_CHILDID,
              CRATE_CLOSURE_TABLE,
              CLOSURE_PARENTID,
              crate.getId().toString()));
        FwdSqlQuery query(
          m_database, QString(
            "DELETE FROM %1 "
            "WHERE %2 IN (%3)"
            "AND %4 NOT IN (%3)").arg(
              CRATE_CLOSURE_TABLE,
              CLOSURE_CHILDID,
              subquery,
              CLOSURE_PARENTID));

        if (!query.isPrepared()) {
            return;
        }
        if (!query.execPrepared()) {
            return;
        }
    }
    if (destinationCrateId.isValid()) {
        FwdSqlQuery query(
          m_database, QString(
            "INSERT INTO %1 (%2, %3, %4) "
            "SELECT a.%2, b.%3, a.%4 + b.%4 + 1 "
            "FROM %1 AS a "
            "JOIN %1 AS b "
            "WHERE a.%3 = :destinatnion "
            "AND b.%2 = :source").arg(
              CRATE_CLOSURE_TABLE,
              CLOSURE_PARENTID,
              CLOSURE_CHILDID,
              CLOSURE_DEPTH));

        query.bindValue(":destination", destinationCrateId.toString());
        query.bindValue(":source", crate.getId().toString());

        if (!query.isPrepared()) {
            return;
        }
        if (!query.execPrepared()) {
            return;
        }
    }
}

CrateSelectResult CrateHierarchy::selectCrateIdsByCrateNameLike(const QString& crateNameLike) const {
    FieldEscaper escaper(m_database);
    QString escapedArgument = escaper.escapeString(kSqlLikeMatchAll + crateNameLike + kSqlLikeMatchAll);

    FwdSqlQuery query(
      m_database, QString(
        "SELECT * FROM %1 "
        "WHERE %2 LIKE %3").arg(
          CRATE_TABLE,
          CRATETABLE_NAME,
          escapedArgument));

    if (query.execPrepared()) {
        return CrateSelectResult(std::move(query));
    } else {
        return CrateSelectResult();
    }
}

CrateTrackSelectResult CrateHierarchy::selectTracksSortedByCrateNameLikeRecursively(const QString& crateNameLike) const {
    FwdSqlQuery query(m_database, formatQueryForTrackIdsByCrateNameLikeRecursively(crateNameLike, true));

    if (query.execPrepared()) {
        return CrateTrackSelectResult(std::move(query));
    } else {
        return CrateTrackSelectResult();
    }
}


QString CrateHierarchy::formatQueryForTrackIdsByCrateNameLikeRecursively(const QString& crateNameLike, bool flag) const {
    QStringList ids, childIds;
    CrateSelectResult crates(selectCrateIdsByCrateNameLike(crateNameLike));

    Crate crate;
    while (crates.populateNext(&crate)) {
        ids << crate.getId().toString();
        childIds += collectChildCrateIds(crate.getId());
    }

    QString selection = flag?CRATETRACKSTABLE_CRATEID  + "," + CRATETRACKSTABLE_TRACKID:CRATETRACKSTABLE_TRACKID;

    return QString(
        "SELECT DISTINCT %1 FROM %2 "
        "WHERE %3 IN (%4) "
        "OR %3 IN (%5) "
        "ORDER BY %6").arg(
          selection,
          CRATE_TRACKS_TABLE,
          CRATETRACKSTABLE_CRATEID,
          ids.join(","),
          childIds.join(","),
          CRATETRACKSTABLE_TRACKID);
}

QStringList CrateHierarchy::collectImmediateChildren(const Crate& parent) const {
    QStringList names;
    FwdSqlQuery query(
      m_database, QString(
        "SELECT c.%1 FROM %2 "
        "JOIN %3 c ON c.%4 = %5 "
        "JOIN %3 p ON p.%4 = %6 "
        "WHERE p.%4 = :id "
        "AND %7 = 1 ").arg(
          CRATETABLE_NAME,
          CRATE_CLOSURE_TABLE,
          CRATE_TABLE,
          CRATETABLE_ID,
          CLOSURE_CHILDID,
          CLOSURE_PARENTID,
          CLOSURE_DEPTH));

    query.bindValue(":id", parent.getId().toString());

    if (query.execPrepared())
        while (query.next()) {
            names << query.fieldValue(0).toString();
        }

    return names;
}

bool CrateHierarchy::isNameValidForHierarchy(const QString& newName,
                                             const Crate& selectedCrate,
                                             const Crate& parent) const {
    // if crate is valid it means we are trying to rename an existing crate
    // else it's a new crate that is not yet in the database
    // so we use the corresponding functions
    if (selectedCrate.getId().isValid()) {
        if (parent.getId().isValid()) {
            if (collectImmediateChildren(parent).contains(newName) || // siblings
                collectImmediateChildren(selectedCrate).contains(newName) || // children
                parent.getName() == newName) { // parent
                return false;
            }
        } else {
            if (collectRootCrateNames().contains(newName) || // siblings
                collectImmediateChildren(selectedCrate).contains(newName)) { // children
                return false;
            }
        }
        return true;
    } else {
        if (parent.getId().isValid()) {
            if (collectImmediateChildren(parent).contains(newName) || // siblings
                parent.getName() == newName) { // parent
                return false;
            }
        } else {
            return !collectRootCrateNames().contains(newName); // siblings
        }
        return true;
    }
}

QString CrateHierarchy::getNamePathFromId(const CrateId& id) const {
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

bool CrateHierarchy::hasChildren(const CrateId& id) const {
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

CrateId CrateHierarchy::getParentId(const CrateId& id) const {
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

QStringList CrateHierarchy::tokenizeCratePath(const CrateId& id) const {
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


QStringList CrateHierarchy::collectChildCrateIds(const CrateId& crateId) const {
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

    query.bindValue("id", crateId);

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
