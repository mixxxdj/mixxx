#ifndef MIXXX_CRATEHIERARCHY_H
#define MIXXX_CRATEHIERARCHY_H

#include "util/db/dbconnection.h"

#include "library/trackcollection.h"

// This class handles the hiearachical structure in the database
// It uses a closure table as described here
// http://dirtsimple.org/2010/11/simplest-way-to-do-tree-based-queries.html

class CrateHierarchy {
  public:
    CrateHierarchy(TrackCollection* pTrackCollection)
        : m_database(pTrackCollection->database()),
          m_pTrackCollection(pTrackCollection){
    }

    uint countCratesOnClosure() const;

    // checks # of crates in closure table against # of crates in crates table
    bool closureIsValid() const;
    // empties the closure table
    void resetClosure() const;
    // fills the closure table with (self,self,0)
    bool initClosure() const;

    // empties the path table
    void resetPath() const;
    bool writeCratePaths(CrateId id, QString namePath, QString idPath) const;
    bool generateCratePaths(Crate crate) const;
    bool generateAllPaths() const;


    bool initClosureForCrate(CrateId id) const;
    bool insertIntoClosure(CrateId parent, CrateId child) const;

    void deleteCrate(CrateId id) const;
    bool hasChildern(CrateId id) const;

    QStringList collectIdPaths() const;

  private:
    QSqlDatabase m_database;
    TrackCollection* m_pTrackCollection;
};
#endif // MIXXX_CRATEHIERARCHY_H
