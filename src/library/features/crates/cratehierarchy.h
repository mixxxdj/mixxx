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

// INFO:
// The hierarchy is made possible with a closure table.
// You can read all about it here:
// http://dirtsimple.org/2010/11/simplest-way-to-do-tree-based-queries.html

// The CrateHierarchy class is initialized inside CrateManager witch is in turn
// initialized in the TrackCollection(name might change)
// To get access to the functions we pass a pointer to CrateManager and use the
// CrateManager::hierarchy() function that returns a const refference to the object.
// Thus outside CrateManager you can use only functions marked as const.


class CrateHierarchy : public virtual DAO {
  public:
    CrateHierarchy() {}
    ~CrateHierarchy() override {}

    ///////////////////////////////////////////////////////////
    // Non const functions, usable only withint crateManager //
    ///////////////////////////////////////////////////////////

    void initialize(const QSqlDatabase& database) override;

    void reset(const CrateStorage* pCrateStorage);

    void addCrateToHierarchy(const Crate& crate,
                             const Crate& parent = Crate());

    // crate hierarchy only cares whether a crates name has changed or not
    bool onUpdatingCrate(const Crate& crate, const CrateStorage* pCrateStorage);

    //////////////////////////////////////////////////////////////
    // Const functions, usable with CrateManager::hierarchy()   //
    // function wherever the crateManager pointer is avaliable. //
    //////////////////////////////////////////////////////////////

    uint countCratesInClosure() const;

    // parent and child are assigned the corresponding crate
    // returns false if the crate does not have a parent (is level 1)
    bool findParentAndChildIdFromPath(CrateId& parentId,
                                      CrateId& childId,
                                      const QString& idPath) const;

    // used in crate filter
    // set flag to true if you want to get both crate_id and track_id to be
    // used it CrateTrackSelectResults
    QString formatQueryForTrackIdsByCrateNameLikeRecursively(const QString& crateNameLike,
                                                             bool flag = false) const;
    CrateTrackSelectResult selectTracksSortedByCrateNameLikeRecursively(const QString& crateNameLike) const;

    // namePath is the string that represents the placement of the crate in the tree
    // just like a path in a file system
    QString getNamePathFromId(const CrateId& id) const;

    // checks whether a name is valid for the hierarchy
    // parent id only applies to subcrates.
    // the selectedCrate variable is only valid when we rename a crate and it already exists
    // if we are creating it we pass a Crate() instead
    bool isNameValidForHierarchy(const QString& newName,
                                 const Crate& selectedCrate,
                                 const Crate& parent = Crate()) const;

    void deleteCrate(const CrateId& id) const;
    bool hasChildren(const CrateId& id) const;

    CrateId getParentId(const CrateId& id) const;

    QStringList collectIdPaths() const;
    QStringList collectChildCrateIds(const CrateId& crateId) const;

    // returns a list of the children 1 level down
    QStringList collectImmediateChildren(const Crate& parent) const;
    QString formatQueryForChildCrateIds(const Crate& crate) const;

    // selects all the crates that the selected crate can move to
    // (All crates other than itself and it's decendants)
    CrateSelectResult selectCratesToMove(const Crate& crate) const;

  private:
    // empties the closure table
    void resetClosure() const;
    // fills the closure table with (self,self,0)
    bool initClosure(CrateSelectResult crates) const;
    // empties the path table
    void resetPath() const;
    bool generateAllPaths(CrateSelectResult crates) const;

    bool initClosureForCrate(const CrateId& id) const;
    bool insertIntoClosure(const CrateId& parent, const CrateId& child) const;
    bool generateCratePaths(const Crate& crate) const;

    bool writeCratePaths(const CrateId& id,
                         const QString& namePath,
                         const QString& idPath) const;
    QStringList tokenizeCratePath(const CrateId& id) const;
    QStringList collectRootCrateNames() const;

    // returns a list with names that exist in the path of
    // the crate so that it can't be named like them
    QStringList collectParentCrateNames(const Crate& crate) const;
    QStringList collectChildCrateNames(const Crate& crate) const;

    CrateSelectResult selectCrateIdsByCrateNameLike(const QString& crateNameLike) const;

    QSqlDatabase m_database;
};

#endif // MIXXX_CRATEHIERARCHY_H
