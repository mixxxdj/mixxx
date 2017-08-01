#ifndef MIXXX_CRATEHIERARCHY_H
#define MIXXX_CRATEHIERARCHY_H

#include <QObject>
#include <QList>
#include <QStringList>
#include <QSet>

#include "library/dao/dao.h"
#include "library/crate/cratesummary.h"
#include "library/features/crates/cratestorage.h"
#include "library/features/crates/cratestoragehelpers.h"

#include "util/db/fwdsqlqueryselectresult.h"
#include "util/db/sqlsubselectmode.h"

// forward declarations
class CrateStorage;

class CrateHierarchy : public virtual DAO {
  public:
    CrateHierarchy() {}
    ~CrateHierarchy() override {}

    void initialize(const QSqlDatabase& database) override;

    ////////////////////////////////////
    void reset(const CrateStorage* pCrateStorage);

    // crate hierarchy only cares whether a crates name has changed or not
    bool onUpdatingCrate(const Crate& crate, const CrateStorage* pCrateStorage);

    void addCrateToHierarchy(const Crate& crate,
                             const Crate& parent = Crate());

    uint countCratesInClosure() const;

    // parent and child are assigned the corresponding crate
    // returns false if the crate does not have a parent (is level 1)
    bool findParentAndChildIdFromPath(CrateId& parentId,
                                      CrateId& childId,
                                      const QString& idPath) const;

    QString formatQueryForTrackIdsByCratePathLike(const QString& cratePathLike) const;
    // namePath is the string that represents the placement of the crate in the tree
    // just like a path in a file system
    QString getNamePathFromId(CrateId id) const;

    bool canBeRenamed(const QString& newName,
                      const Crate& crate,
                      const CrateId parentId = CrateId()) const;

    // checks whether a name is valid for the hierarchy
    // parent id only applies to subcrates.
    bool nameIsValidForHierarchy(const QString& newName,
                                 const Crate parent = Crate()) const;

    void deleteCrate(CrateId id) const;
    bool hasChildern(CrateId id) const;

    // returns -1 when there is no parent
    int getParentId(const CrateId id) const;

    QStringList collectIdPaths() const;

  private:
    // empties the closure table
    void resetClosure() const;
    // fills the closure table with (self,self,0)
    bool initClosure(CrateSelectResult crates) const;
    // empties the path table
    void resetPath() const;
    bool generateAllPaths(CrateSelectResult crates) const;

    bool initClosureForCrate(CrateId id) const;
    bool insertIntoClosure(CrateId parent, CrateId child) const;
    bool generateCratePaths(Crate crate) const;

    bool writeCratePaths(CrateId id, QString namePath, QString idPath) const;
    QStringList tokenizeCratePath(CrateId id) const;
    QStringList collectRootCrateNames() const;

    // returns a list with names that exist in the path of
    // the crate so that it can't be named like them
    QStringList collectParentCrateNames(const Crate& crate) const;
    QStringList collectChildCrateNames(const Crate& crate) const;

    QSqlDatabase m_database;
};

#endif // MIXXX_CRATEHIERARCHY_H
