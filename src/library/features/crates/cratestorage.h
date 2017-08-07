#ifndef MIXXX_CRATESTORAGE_H
#define MIXXX_CRATESTORAGE_H

#include <QObject>
#include <QList>
#include <QSet>

#include "library/crate/cratesummary.h"
#include "library/features/crates/cratestoragehelpers.h"
#include "library/features/crates/cratehierarchy.h"
#include "library/dao/dao.h"

#include "util/db/fwdsqlqueryselectresult.h"
#include "util/db/sqlsubselectmode.h"

class CrateStorage: public virtual DAO {
  public:
    CrateStorage() {}
    ~CrateStorage() override {}

    void initialize(const QSqlDatabase& database) override;

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

    // crate hierarchy needs access to selectCrates();
    friend class CrateHierarchy;
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

    QStringList collectCrateIdsByCrateNameLike(const QString& crateNameLike) const;

  private:
    QSqlDatabase m_database;
};


#endif // MIXXX_CRATESTORAGE_H
