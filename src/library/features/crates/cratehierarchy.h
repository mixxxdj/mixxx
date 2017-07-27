#ifndef MIXXX_CRATEHIERARCHY_H
#define MIXXX_CRATEHIERARCHY_H

#include <QObject>
#include <QList>
#include <QStringList>
#include <QSet>

#include "library/dao/dao.h"
#include "library/crate/cratesummary.h"
#include "library/features/crates/cratestoragehelpers.h"

#include "util/db/fwdsqlqueryselectresult.h"
#include "util/db/sqlsubselectmode.h"

class CrateHierarchy : public virtual DAO {
  public:
    CrateHierarchy() {}
    ~CrateHierarchy() override {}

    void initialize(const QSqlDatabase& database) override;

    ////////////////////////////////////
    uint countCratesInClosure() const;

    // empties the closure table
    void resetClosure() const;
    // fills the closure table with (self,self,0)
    bool initClosure(CrateSelectResult crates) const;

    // empties the path table
    void resetPath() const;
    bool writeCratePaths(CrateId id, QString namePath, QString idPath) const;
    bool generateCratePaths(Crate crate) const;
    bool generateAllPaths(CrateSelectResult crates) const;

    // parent and child are assigned the corresponding crate
    // returns false if the crate does not have a parent (is level 1)
    bool findParentAndChildIdFromPath(CrateId& parentId,
                                      CrateId& childId,
                                      const QString& idPath) const;

    QString formatQueryForTrackIdsByCratePathLike(const QString& cratePathLike) const;
    QString getNamePathFromId(CrateId id) const;

    bool initClosureForCrate(CrateId id) const;
    bool insertIntoClosure(CrateId parent, CrateId child) const;

    void deleteCrate(CrateId id) const;
    bool hasChildern(CrateId id) const;

    QStringList collectIdPaths() const;
    QStringList tokenizeCratePath(CrateId id) const;
    QStringList collectRootCrateNames() const;

  private:
    QSqlDatabase m_database;
};

#endif // MIXXX_CRATEHIERARCHY_H
