#ifndef MIXXX_CRATESTORAGE_H
#define MIXXX_CRATESTORAGE_H


#include <QObject>
#include <QList>
#include <QSet>

#include "library/crate/cratesummary.h"
#include "track/trackid.h"

#include "util/db/sqlstorage.h"
#include "util/db/fwdsqlqueryselectresult.h"
#include "util/db/sqlsubselectmode.h"


// forward declaration(s)
class SqlTransaction;

class CrateQueryFields {
  public:
    CrateQueryFields() {}
    explicit CrateQueryFields(const FwdSqlQuery& query);
    virtual ~CrateQueryFields() = default;

    CrateId getId(const FwdSqlQuery& query) const {
        return CrateId(query.fieldValue(m_iId));
    }
    QString getName(const FwdSqlQuery& query) const {
        return query.fieldValue(m_iName).toString();
    }
    bool isLocked(const FwdSqlQuery& query) const {
        return query.fieldValueBoolean(m_iLocked);
    }
    bool isAutoDjSource(const FwdSqlQuery& query) const {
        return query.fieldValueBoolean(m_iAutoDjSource);
    }

    void populateFromQuery(
            const FwdSqlQuery& query,
            Crate* pCrate) const;

  private:
    DbFieldIndex m_iId;
    DbFieldIndex m_iName;
    DbFieldIndex m_iLocked;
    DbFieldIndex m_iAutoDjSource;
};

class CrateSelectResult: public FwdSqlQuerySelectResult {
public:
    CrateSelectResult(CrateSelectResult&& other)
        : FwdSqlQuerySelectResult(std::move(other)),
          m_queryFields(std::move(other.m_queryFields)) {
    }
    ~CrateSelectResult() override = default;

    bool populateNext(Crate* pCrate) {
        if (next()) {
            m_queryFields.populateFromQuery(query(), pCrate);
            return true;
        } else {
            return false;
        }
    }

private:
    friend class CrateStorage;
    CrateSelectResult() = default;
    explicit CrateSelectResult(FwdSqlQuery&& query)
        : FwdSqlQuerySelectResult(std::move(query)),
          m_queryFields(FwdSqlQuerySelectResult::query()) {
    }

    CrateQueryFields m_queryFields;
};

class CrateSummaryQueryFields: public CrateQueryFields {
  public:
    CrateSummaryQueryFields() = default;
    explicit CrateSummaryQueryFields(const FwdSqlQuery& query);
    ~CrateSummaryQueryFields() override = default;

    uint getTrackCount(const FwdSqlQuery& query) const {
        QVariant varTrackCount = query.fieldValue(m_iTrackCount);
        if (varTrackCount.isNull()) {
            return 0; // crate is empty
        } else {
            return varTrackCount.toUInt();
        }
    }
    double getTrackDuration(const FwdSqlQuery& query) const {
        QVariant varTrackDuration = query.fieldValue(m_iTrackDuration);
        if (varTrackDuration.isNull()) {
            return 0.0; // crate is empty
        } else {
            return varTrackDuration.toDouble();
        }
    }

    void populateFromQuery(
            const FwdSqlQuery& query,
            CrateSummary* pCrateSummary) const;

  private:
    DbFieldIndex m_iTrackCount;
    DbFieldIndex m_iTrackDuration;
};

class CrateSummarySelectResult: public FwdSqlQuerySelectResult {
public:
    CrateSummarySelectResult(CrateSummarySelectResult&& other)
        : FwdSqlQuerySelectResult(std::move(other)),
          m_queryFields(std::move(other.m_queryFields)) {
    }
    ~CrateSummarySelectResult() override = default;

    bool populateNext(CrateSummary* pCrateSummary) {
        if (next()) {
            m_queryFields.populateFromQuery(query(), pCrateSummary);
            return true;
        } else {
            return false;
        }
    }

private:
    friend class CrateStorage;
    CrateSummarySelectResult() = default;
    explicit CrateSummarySelectResult(FwdSqlQuery&& query)
        : FwdSqlQuerySelectResult(std::move(query)),
          m_queryFields(FwdSqlQuerySelectResult::query()) {
    }

    CrateSummaryQueryFields m_queryFields;
};

class CrateTrackQueryFields {
  public:
    CrateTrackQueryFields() = default;
    explicit CrateTrackQueryFields(const FwdSqlQuery& query);
    virtual ~CrateTrackQueryFields() = default;

    CrateId crateId(const FwdSqlQuery& query) const {
        return CrateId(query.fieldValue(m_iCrateId));
    }
    TrackId trackId(const FwdSqlQuery& query) const {
        return TrackId(query.fieldValue(m_iTrackId));
    }

  private:
    DbFieldIndex m_iCrateId;
    DbFieldIndex m_iTrackId;
};

class CrateTrackSelectResult: public FwdSqlQuerySelectResult {
public:
    CrateTrackSelectResult(CrateTrackSelectResult&& other)
        : FwdSqlQuerySelectResult(std::move(other)),
          m_queryFields(std::move(other.m_queryFields)) {
    }
    ~CrateTrackSelectResult() override = default;

    CrateId crateId() const {
        return m_queryFields.crateId(query());
    }
    TrackId trackId() const {
        return m_queryFields.trackId(query());
    }

private:
    friend class CrateStorage;
    CrateTrackSelectResult() = default;
    explicit CrateTrackSelectResult(FwdSqlQuery&& query)
        : FwdSqlQuerySelectResult(std::move(query)),
          m_queryFields(FwdSqlQuerySelectResult::query()) {
    }

    CrateTrackQueryFields m_queryFields;
};

class CrateStorage: public SqlStorage {
  public:
    ~CrateStorage() final = default;


    /////////////////////////////////////////////////////////////////////////
    // Inherited operations (non-const)
    // Only invoked by TrackCollection!
    /////////////////////////////////////////////////////////////////////////

    void repairDatabase(QSqlDatabase database) final;

    void attachDatabase(QSqlDatabase database) final;
    void detachDatabase() final;


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
            SqlTransaction& transaction,
            const Crate& crate,
            CrateId* pCrateId = nullptr);

    bool onUpdatingCrate(
            SqlTransaction& transaction,
            const Crate& crate);

    bool onDeletingCrate(
            SqlTransaction& transaction,
            CrateId crateId);

    bool onAddingCrateTracks(
            SqlTransaction& transaction,
            CrateId crateId,
            const QList<TrackId>& trackIds);

    bool onRemovingCrateTracks(
            SqlTransaction& transaction,
            CrateId crateId,
            const QList<TrackId>& trackIds);

    bool onPurgingTracks(
            SqlTransaction& transaction,
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

    // Select the track ids of a crate or the crate ids of a track respectively.
    // The results are sorted (ascending) by the target id, i.e. the id that is
    // not provided for filtering. This enables the caller to perform efficient
    // binary searches on the result set after storing it in a list or vector.
    CrateTrackSelectResult selectCrateTracksSorted(
            CrateId crateId) const;
    CrateTrackSelectResult selectTrackCratesSorted(
            TrackId trackId) const;

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

  private:
    void createViews();

    QSqlDatabase m_database;
};


#endif // MIXXX_CRATESTORAGE_H
