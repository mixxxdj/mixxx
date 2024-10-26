#pragma once

#include <QList>
#include <QSet>

#include "library/trackset/smarties/smartiesid.h"
#include "track/trackid.h"
#include "util/db/fwdsqlqueryselectresult.h"
#include "util/db/sqlstorage.h"
#include "util/db/sqlsubselectmode.h"

class Smarties;
class SmartiesSummary;

class SmartiesQueryFields {
  public:
    SmartiesQueryFields() {
    }
    explicit SmartiesQueryFields(const FwdSqlQuery& query);
    virtual ~SmartiesQueryFields() = default;

    SmartiesId getId(const FwdSqlQuery& query) const {
        return SmartiesId(query.fieldValue(m_iId));
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
    QString getSearchInput(const FwdSqlQuery& query) const {
        return query.fieldValue(m_iSearchInput).toString();
    }
    QString getSearchSql(const FwdSqlQuery& query) const {
        return query.fieldValue(m_iSearchSql).toString();
    }

    void populateFromQuery(
            const FwdSqlQuery& query,
            Smarties* pSmarties) const;

  private:
    DbFieldIndex m_iId;
    DbFieldIndex m_iName;
    DbFieldIndex m_iLocked;
    DbFieldIndex m_iAutoDjSource;
    DbFieldIndex m_iSearchInput;
    DbFieldIndex m_iSearchSql;
};

class SmartiesSelectResult : public FwdSqlQuerySelectResult {
  public:
    SmartiesSelectResult(SmartiesSelectResult&& other)
            : FwdSqlQuerySelectResult(std::move(other)),
              m_queryFields(std::move(other.m_queryFields)) {
    }
    ~SmartiesSelectResult() override = default;

    bool populateNext(Smarties* pSmarties) {
        if (next()) {
            m_queryFields.populateFromQuery(query(), pSmarties);
            return true;
        } else {
            return false;
        }
    }

    // private:
    friend class SmartiesStorage;
    SmartiesSelectResult() = default;
    explicit SmartiesSelectResult(FwdSqlQuery&& query)
            : FwdSqlQuerySelectResult(std::move(query)),
              m_queryFields(FwdSqlQuerySelectResult::query()) {
    }

    SmartiesQueryFields m_queryFields;

  private:
};

class SmartiesSummaryQueryFields : public SmartiesQueryFields {
  public:
    SmartiesSummaryQueryFields() = default;
    explicit SmartiesSummaryQueryFields(const FwdSqlQuery& query);
    ~SmartiesSummaryQueryFields() override = default;

    uint getTrackCount(const FwdSqlQuery& query) const {
        QVariant varTrackCount = query.fieldValue(m_iTrackCount);
        if (varTrackCount.isNull()) {
            return 0; // smarties is empty
        } else {
            return varTrackCount.toUInt();
        }
    }
    double getTrackDuration(const FwdSqlQuery& query) const {
        QVariant varTrackDuration = query.fieldValue(m_iTrackDuration);
        if (varTrackDuration.isNull()) {
            return 0.0; // smarties is empty
        } else {
            return varTrackDuration.toDouble();
        }
    }

    void populateFromQuery(
            const FwdSqlQuery& query,
            SmartiesSummary* pSmartiesSummary) const;

  private:
    DbFieldIndex m_iTrackCount;
    DbFieldIndex m_iTrackDuration;
};

class SmartiesSummarySelectResult : public FwdSqlQuerySelectResult {
  public:
    SmartiesSummarySelectResult(SmartiesSummarySelectResult&& other)
            : FwdSqlQuerySelectResult(std::move(other)),
              m_queryFields(std::move(other.m_queryFields)) {
    }
    ~SmartiesSummarySelectResult() override = default;

    bool populateNext(SmartiesSummary* pSmartiesSummary) {
        if (next()) {
            m_queryFields.populateFromQuery(query(), pSmartiesSummary);
            return true;
        } else {
            return false;
        }
    }

  private:
    friend class SmartiesStorage;
    SmartiesSummarySelectResult() = default;
    explicit SmartiesSummarySelectResult(FwdSqlQuery&& query)
            : FwdSqlQuerySelectResult(std::move(query)),
              m_queryFields(FwdSqlQuerySelectResult::query()) {
    }

    SmartiesSummaryQueryFields m_queryFields;
};

class SmartiesTrackQueryFields {
  public:
    SmartiesTrackQueryFields() = default;
    explicit SmartiesTrackQueryFields(const FwdSqlQuery& query);
    virtual ~SmartiesTrackQueryFields() = default;

    SmartiesId smartiesId(const FwdSqlQuery& query) const {
        return SmartiesId(query.fieldValue(m_iSmartiesId));
    }
    TrackId trackId(const FwdSqlQuery& query) const {
        return TrackId(query.fieldValue(m_iTrackId));
    }

  private:
    DbFieldIndex m_iSmartiesId;
    DbFieldIndex m_iTrackId;
};

// class TrackQueryFields {
//   public:
//     TrackQueryFields() = default;
//     explicit TrackQueryFields(const FwdSqlQuery& query);
//     virtual ~TrackQueryFields() = default;

//    TrackId trackId(const FwdSqlQuery& query) const {
//        return TrackId(query.fieldValue(m_iTrackId));
//    }

//  private:
//    DbFieldIndex m_iTrackId;
//};

class SmartiesTrackSelectResult : public FwdSqlQuerySelectResult {
  public:
    SmartiesTrackSelectResult(SmartiesTrackSelectResult&& other)
            : FwdSqlQuerySelectResult(std::move(other)),
              m_queryFields(std::move(other.m_queryFields)) {
    }
    ~SmartiesTrackSelectResult() override = default;

    SmartiesId smartiesId() const {
        return m_queryFields.smartiesId(query());
    }
    TrackId trackId() const {
        return m_queryFields.trackId(query());
    }

  private:
    friend class SmartiesStorage;
    SmartiesTrackSelectResult() = default;
    explicit SmartiesTrackSelectResult(FwdSqlQuery&& query)
            : FwdSqlQuerySelectResult(std::move(query)),
              m_queryFields(FwdSqlQuerySelectResult::query()) {
    }

    SmartiesTrackQueryFields m_queryFields;
};

// class TrackSelectResult : public FwdSqlQuerySelectResult {
//   public:
//     TrackSelectResult(TrackSelectResult&& other)
//             : FwdSqlQuerySelectResult(std::move(other)),
//               m_queryFields(std::move(other.m_queryFields)) {
//     }
//     ~TrackSelectResult() override = default;

//    TrackId trackId() const {
//        return m_queryFields.trackId(query());
//    }

//  private:
//    friend class SmartiesStorage;
//    TrackSelectResult() = default;
//    explicit TrackSelectResult(FwdSqlQuery&& query)
//            : FwdSqlQuerySelectResult(std::move(query)),
//              m_queryFields(FwdSqlQuerySelectResult::query()) {
//    }

//    TrackQueryFields m_queryFields;
//};

class SmartiesStorage : public virtual /*implements*/ SqlStorage {
  public:
    SmartiesStorage() = default;
    ~SmartiesStorage() override = default;

    void repairDatabase(
            const QSqlDatabase& database) override;

    void connectDatabase(
            const QSqlDatabase& database) override;
    void disconnectDatabase() override;

    /////////////////////////////////////////////////////////////////////////
    // Smarties write operations (transactional, non-const)
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

    bool onInsertingSmarties(
            const Smarties& smarties,
            SmartiesId* pSmartiesId = nullptr);

    bool onUpdatingSmarties(
            const Smarties& smarties);

    bool onDeletingSmarties(
            SmartiesId smartiesId);

    bool onAddingSmartiesTracks(
            SmartiesId smartiesId,
            const QList<TrackId>& trackIds);

    bool onRemovingSmartiesTracks(
            SmartiesId smartiesId,
            const QList<TrackId>& trackIds);

    bool onPurgingTracks(
            const QList<TrackId>& trackIds);

    /////////////////////////////////////////////////////////////////////////
    // Smarties read operations (read-only, const)
    /////////////////////////////////////////////////////////////////////////

    uint countSmarties() const;

    // Omit the pSmarties parameter for checking if the corresponding smarties exists.
    bool readSmartiesById(
            SmartiesId id,
            Smarties* pSmarties = nullptr) const;
    bool readSmartiesByName(
            const QString& name,
            Smarties* pSmarties = nullptr) const;

    // The following list results are ordered by smarties name:
    //  - case-insensitive
    //  - locale-aware
    SmartiesSelectResult selectSmarties()
            const; // all smarties
                   //    SmartiesSelectResult selectSmartiesByIds(    // subset
                   //    of smarties
                   //            const QString& subselectForSmartiesIds,
                   //            SqlSubselectMode subselectMode) const;

    // TODO(XXX): Move this function into the AutoDJ component after
    // fixing various database design flaws in AutoDJ itself (see also:
    // smartiesschema.h). AutoDJ should use the function selectSmartiesByIds()
    // from this class for the actual implementation.
    // This refactoring should be deferred until consensus on the
    // redesign of the AutoDJ feature has been reached. The main
    // ideas of the new design should be documented for verification
    // before starting to code.
    //    SmartiesSelectResult selectAutoDjSmarties(bool autoDjSource = true) const;

    // Smarties content, i.e. the smarties's tracks referenced by id
    uint countSmartiesTracks(SmartiesId smartiesId) const;

    // Format a subselect query for the tracks contained in smarties.
    static QString formatSubselectQueryForSmartiesTrackIds(
            SmartiesId smartiesId); // no db access

    static QString returnSearchSQLFieldFromTable(SmartiesId smartiesId);

    QString formatQueryForTrackIdsBySmartiesNameLike(
            const QString& smartiesNameLike) const;      // no db access
    static QString formatQueryForTrackIdsWithSmarties(); // no db access
    // Select the track ids of a smarties or the smarties ids of a track respectively.
    // The results are sorted (ascending) by the target id, i.e. the id that is
    // not provided for filtering. This enables the caller to perform efficient
    // binary searches on the result set after storing it in a list or vector.
    SmartiesTrackSelectResult selectSmartiesTracksSorted(
            SmartiesId smartiesId) const;
    SmartiesTrackSelectResult selectTrackSmartiesSorted(
            TrackId trackId) const;
    SmartiesSummarySelectResult selectSmartiesWithTrackCount(
            const QList<TrackId>& trackIds) const;
    SmartiesTrackSelectResult selectTracksSortedBySmartiesNameLike(
            const QString& smartiesNameLike) const;
    //    TrackSelectResult selectAllTracksSorted() const;

    // Returns the set of smarties ids for smarties that contain any of the
    // provided track ids.
    QSet<SmartiesId> collectSmartiesIdsOfTracks(
            const QList<TrackId>& trackIds) const;

    /////////////////////////////////////////////////////////////////////////
    // SmartiesSummary view operations (read-only, const)
    /////////////////////////////////////////////////////////////////////////

    // Track summaries of all smarties:
    //  - Hidden tracks are excluded from the smarties summary statistics
    //  - The result list is ordered by smarties name:
    //     - case-insensitive
    //     - locale-aware
    SmartiesSummarySelectResult selectSmartiesSummaries() const; // all smarties

    // Omit the pSmarties parameter for checking if the corresponding smarties exists.
    bool readSmartiesSummaryById(SmartiesId id, SmartiesSummary* pSmartiesSummary = nullptr) const;

  private:
    void createViews();

    QSqlDatabase m_database;
};
