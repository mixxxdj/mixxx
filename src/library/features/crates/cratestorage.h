#ifndef MIXXX_CRATESTORAGE_H
#define MIXXX_CRATESTORAGE_H

#include <QObject>
#include <QList>
#include <QSet>

#include "library/crate/cratesummary.h"
#include "library/features/crates/cratestoragehelpers.h"

#include "util/db/fwdsqlqueryselectresult.h"
#include "util/db/sqlsubselectmode.h"
#include "util/db/sqlstorage.h"

class CrateStorage: public virtual /*implements*/ SqlStorage {
  public:
    CrateStorage() = default;
    ~CrateStorage() override = default;

    void repairDatabase(
            QSqlDatabase database) override;

    void connectDatabase(
            QSqlDatabase database) override;
    void disconnectDatabase() override;

    /////////////////////////////////////////////////////////////////////////
    // Crate write operations (transactional, non-const)
    // Only invoked by TrackCollection!
    //
    // Naming conventions:
    //  on<present participle>...()
    //    - Invoked within active transaction
    //    - May fail
    //    - Performs only database modifications that are either committed
    //      or implicitly reverted on rollback
    //  after<present participle>...()
    //    - Invoked after preceding transaction has been committed (see above)
    //    - Must not fail
    //    - Typical use case: Update internal caches and compute change set
    //      for notifications
    /////////////////////////////////////////////////////////////////////////

    bool onInsertingCrate(
            const Crate& crate,
            CrateId* pCrateId = nullptr);

    bool onUpdatingCrate(
            const Crate& crate);

    bool onDeletingCrate(
            CrateId crateId);

    bool onAddingCrateTracks(
            CrateId crateId,
            const QList<TrackId>& trackIds);

    bool onRemovingCrateTracks(
            CrateId crateId,
            const QList<TrackId>& trackIds);

    bool onPurgingTracks(
            const QList<TrackId>& trackIds);


    /////////////////////////////////////////////////////////////////////////
    // Crate read operations (read-only, const)
    /////////////////////////////////////////////////////////////////////////

    uint countCrates() const;

    // Omit the pCrate parameter for checking if the corresponding crate exists.
    bool readCrateById(
            CrateId id,
            Crate* pCrate = nullptr) const;
    bool readCrateByName(
            const QString& name,
            Crate* pCrate = nullptr) const;

    // The following list results are ordered by crate name:
    //  - case-insensitive
    //  - locale-aware
    CrateSelectResult selectCrates() const; // all crates
    CrateSelectResult selectCratesByIds( // subset of crates
            const QString& subselectForCrateIds,
            SqlSubselectMode subselectMode) const;

    // TODO(XXX): Move this function into the AutoDJ component after
    // fixing various database design flaws in AutoDJ itself (see also:
    // crateschema.h). AutoDJ should use the function selectCratesByIds()
    // from this class for the actual implementation.
    // This refactoring should be deferred until consensus on the
    // redesign of the AutoDJ feature has been reached. The main
    // ideas of the new design should be documented for verification
    // before starting to code.
    CrateSelectResult selectAutoDjCrates(bool autoDjSource = true) const;

    // Crate content, i.e. the crate's tracks referenced by id
    uint countCrateTracks(CrateId crateId) const;

    // Format a subselect query for the tracks contained in crate.
    static QString formatSubselectQueryForCrateTrackIds(
            CrateId crateId); // no db access

    QString formatQueryForTrackIdsByCrateNameLike(
            const QString& crateNameLike) const; // no db access
    // Select the track ids of a crate or the crate ids of a track respectively.
    // The results are sorted (ascending) by the target id, i.e. the id that is
    // not provided for filtering. This enables the caller to perform efficient
    // binary searches on the result set after storing it in a list or vector.
    CrateTrackSelectResult selectCrateTracksSorted(
            CrateId crateId) const;
    CrateTrackSelectResult selectTrackCratesSorted(
            TrackId trackId) const;
    CrateTrackSelectResult selectTracksSortedByCrateNameLike(
            const QString& crateNameLike) const;

    // Returns the set of crate ids for crates that contain any of the
    // provided track ids.
    QSet<CrateId> collectCrateIdsOfTracks(
            const QList<TrackId>& trackIds) const;


    /////////////////////////////////////////////////////////////////////////
    // CrateSummary view operations (read-only, const)
    /////////////////////////////////////////////////////////////////////////

    // Track summaries of all crates:
    //  - Hidden tracks are excluded from the crate summary statistics
    //  - The result list is ordered by crate name:
    //     - case-insensitive
    //     - locale-aware
    CrateSummarySelectResult selectCrateSummaries() const; // all crates

    // Omit the pCrate parameter for checking if the corresponding crate exists.
    bool readCrateSummaryById(CrateId id, CrateSummary* pCrateSummary = nullptr) const;

    //////////////////////////////////////////////////////////////////////
    // Crate Hierarchy operations (const)
    //////////////////////////////////////////////////////////////////////

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

    // parent and child are assigned the corresponding crate
    // returns false if the crate does not have a parent (is level 1)
    bool findParentAndChildFromPath(Crate& parent,
                                    Crate& child,
                                    const QString& idPath) const;


    bool initClosureForCrate(CrateId id) const;
    bool insertIntoClosure(CrateId parent, CrateId child) const;

    void deleteCrate(CrateId id) const;
    bool hasChildern(CrateId id) const;

    QStringList collectIdPaths() const;
    QStringList tokenizeCratePath(CrateId id) const;
    QStringList collectRootCrates() const;

  private:
    void createViews();

    QSqlDatabase m_database;
};


#endif // MIXXX_CRATESTORAGE_H
