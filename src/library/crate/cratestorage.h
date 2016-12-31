#ifndef MIXXX_CRATESTORAGE_H
#define MIXXX_CRATESTORAGE_H


#include <QObject>
#include <QVector>
#include <QList>
#include <QSet>
#include <QMultiHash>

#include "library/crate/cratesummary.h"
#include "track/trackid.h"

#include "util/db/sqlstorage.h"
#include "util/db/sqlselectiterator.h"
#include "util/db/sqlsubselectmode.h"


// forward declaration(s)
class SqlTransaction;

class CrateQueryFields {
  public:
    CrateQueryFields() {}
    explicit CrateQueryFields(const FwdSqlQuery& query);
    virtual ~CrateQueryFields() {}

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

    void readValues(const FwdSqlQuery& query, Crate* pCrate) const;

  private:
    DbFieldIndex m_iId;
    DbFieldIndex m_iName;
    DbFieldIndex m_iLocked;
    DbFieldIndex m_iAutoDjSource;
};

class CrateSelectIterator: public SqlSelectIterator {
public:
    CrateSelectIterator(CrateSelectIterator&& other)
        : SqlSelectIterator(std::move(other)),
          m_queryFields(std::move(other.m_queryFields)) {
    }
    ~CrateSelectIterator() override {}

    bool readNext(Crate* pCrate) {
        if (next()) {
            m_queryFields.readValues(query(), pCrate);
            return true;
        } else {
            return false;
        }
    }

private:
    friend class CrateStorage;
    CrateSelectIterator() {}
    explicit CrateSelectIterator(FwdSqlQuery query)
        : SqlSelectIterator(query),
          m_queryFields(SqlSelectIterator::query()) {
    }

    CrateQueryFields m_queryFields;
};

class CrateSummaryQueryFields: public CrateQueryFields {
  public:
    CrateSummaryQueryFields() {}
    explicit CrateSummaryQueryFields(const FwdSqlQuery& query);
    ~CrateSummaryQueryFields() override {}

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

    void readValues(
            const FwdSqlQuery& query,
            CrateSummary* pCrateSummary) const;

  private:
    DbFieldIndex m_iTrackCount;
    DbFieldIndex m_iTrackDuration;
};

class CrateSummarySelectIterator: public SqlSelectIterator {
public:
    CrateSummarySelectIterator(CrateSummarySelectIterator&& other)
        : SqlSelectIterator(std::move(other)),
          m_queryFields(std::move(other.m_queryFields)) {
    }
    ~CrateSummarySelectIterator() override {}

    bool readNext(CrateSummary* pCrateSummary) {
        if (next()) {
            m_queryFields.readValues(query(), pCrateSummary);
            return true;
        } else {
            return false;
        }
    }

private:
    friend class CrateStorage;
    CrateSummarySelectIterator() {}
    explicit CrateSummarySelectIterator(FwdSqlQuery query)
        : SqlSelectIterator(query),
          m_queryFields(SqlSelectIterator::query()) {
    }

    CrateSummaryQueryFields m_queryFields;
};

class CrateTrackQueryFields {
  public:
    CrateTrackQueryFields() {}
    explicit CrateTrackQueryFields(const FwdSqlQuery& query);
    virtual ~CrateTrackQueryFields() {}

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

class CrateTrackSelectIterator: public SqlSelectIterator {
public:
    CrateTrackSelectIterator(CrateTrackSelectIterator&& other)
        : SqlSelectIterator(std::move(other)),
          m_queryFields(std::move(other.m_queryFields)) {
    }
    ~CrateTrackSelectIterator() override {}

    CrateId crateId() const {
        return m_queryFields.crateId(query());
    }
    TrackId trackId() const {
        return m_queryFields.trackId(query());
    }

private:
    friend class CrateStorage;
    CrateTrackSelectIterator() {}
    explicit CrateTrackSelectIterator(FwdSqlQuery query)
        : SqlSelectIterator(query),
          m_queryFields(SqlSelectIterator::query()) {
    }

    CrateTrackQueryFields m_queryFields;
};

class CrateStorage: public SqlStorage {
  public:
    CrateStorage();
    ~CrateStorage() final;


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
    void afterInsertingCrate(
            CrateId crateId);

    bool onUpdatingCrate(
            SqlTransaction& transaction,
            const Crate& crate);
    void afterUpdatingCrate(
            CrateId crateId);

    bool onDeletingCrate(
            SqlTransaction& transaction,
            CrateId crateId);
    void afterDeletingCrate(
            CrateId crateId);

    bool onAddingCrateTracks(
            SqlTransaction& transaction,
            CrateId crateId,
            const QList<TrackId>& trackIds);
    void afterAddingCrateTracks(
            CrateId crateId,
            const QList<TrackId>& trackIds);

    bool onRemovingCrateTracks(
            SqlTransaction& transaction,
            CrateId crateId,
            const QList<TrackId>& trackIds);
    void afterRemovingCrateTracks(
            CrateId crateId,
            const QList<TrackId>& trackIds);

    // Returns the set of crate ids for which the crate
    // summaries might have changed.
    QSet<CrateId> afterHidingOrUnhidingTracks(
            const QList<TrackId>& trackIds);

    bool onPurgingTracks(
            SqlTransaction& transaction,
            const QList<TrackId>& trackIds);
    QMap<CrateId, QList<TrackId>> afterPurgingTracks(
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
    CrateSelectIterator selectCrates() const; // all crates
    CrateSelectIterator selectCratesByIds( // subset of crates
            const QString& subselectForCrateIds,
            SqlSubselectMode subselectMode) const;

    // TODO(XXX): Move this function into the Auto DJ component after
    // fixing the database design flaw (see also: crateschema.h).
    // Auto DJ needs to use the function selectCratesByIds() from
    // this class for the actual implementation.
    CrateSelectIterator selectAutoDjCrates(bool autoDjSource = true) const;

    // Crate content, i.e. the crate's tracks referenced by id
    uint countCrateTracks(CrateId crateId) const;
    bool isCrateTrack(CrateId crateId, TrackId trackId) const; // cached, no db access

    // Format a subselect query for the tracks contained in crate.
    QString formatSubselectQueryForCrateTracks(
            CrateId crateId,
            const QString& trackIdColumn) const; // no db access

    CrateTrackSelectIterator selectCrateTracks(CrateId crateId) const;


    /////////////////////////////////////////////////////////////////////////
    // CrateSummary view operations (read-only, const)
    /////////////////////////////////////////////////////////////////////////

    // Track summaries of all crates:
    //  - Hidden tracks are excluded from the crate summary statistics
    //  - The result list is ordered by crate name:
    //     - case-insensitive
    //     - locale-aware
    CrateSummarySelectIterator selectCrateSummaries() const; // all crates

    // Omit the pCrate parameter for checking if the corresponding crate exists.
    bool readCrateSummaryById(CrateId id, CrateSummary* pCrateSummary = nullptr) const;

  private:
    void createViews();

    void clearCaches();
    void refreshCaches();

    QSqlDatabase m_database;

    QSet<CrateId> m_deletedCratesCache;
    QMultiHash<TrackId, CrateId> m_trackCratesCache;
};


#endif // MIXXX_CRATESTORAGE_H
