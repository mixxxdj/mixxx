#pragma once

#include <QList>
#include <QSet>

#include "library/trackset/searchcrate/searchcrateid.h"
#include "track/trackid.h"
#include "util/db/fwdsqlqueryselectresult.h"
#include "util/db/sqlstorage.h"
#include "util/db/sqlsubselectmode.h"

class SearchCrate;
class SearchCrateSummary;

class SearchCrateQueryFields {
  public:
    SearchCrateQueryFields() {
    }
    explicit SearchCrateQueryFields(const FwdSqlQuery& query);
    virtual ~SearchCrateQueryFields() = default;

    void bindConditionField(const QString& fieldName, const SearchCrate& searchCrate, int index);
    void bindConditionOperator(const QString& operatorName,
            const SearchCrate& searchCrate,
            int index);
    void bindConditionValue(const QString& valueName, const SearchCrate& searchCrate, int index);
    void bindConditionCombiner(const QString& combinerName,
            const SearchCrate& searchCrate,
            int index);

    void populateFromQuery(const FwdSqlQuery& query, SearchCrate* pSearchCrate) const;

  private slots:
    SearchCrateId getId(const FwdSqlQuery& query) const {
        return SearchCrateId(query.fieldValue(m_iId));
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
    //  Conditions
    QString getCondition1Field(const FwdSqlQuery& query) const {
        return query.fieldValue(m_iCondition1Field).toString();
    }
    QString getCondition1Operator(const FwdSqlQuery& query) const {
        return query.fieldValue(m_iCondition1Operator).toString();
    }
    QString getCondition1Value(const FwdSqlQuery& query) const {
        return query.fieldValue(m_iCondition1Value).toString();
    }
    QString getCondition1Combiner(const FwdSqlQuery& query) const {
        return query.fieldValue(m_iCondition1Combiner).toString();
    }
    QString getCondition2Field(const FwdSqlQuery& query) const {
        return query.fieldValue(m_iCondition2Field).toString();
    }
    QString getCondition2Operator(const FwdSqlQuery& query) const {
        return query.fieldValue(m_iCondition2Operator).toString();
    }
    QString getCondition2Value(const FwdSqlQuery& query) const {
        return query.fieldValue(m_iCondition2Value).toString();
    }
    QString getCondition2Combiner(const FwdSqlQuery& query) const {
        return query.fieldValue(m_iCondition2Combiner).toString();
    }
    QString getCondition3Field(const FwdSqlQuery& query) const {
        return query.fieldValue(m_iCondition3Field).toString();
    }
    QString getCondition3Operator(const FwdSqlQuery& query) const {
        return query.fieldValue(m_iCondition3Operator).toString();
    }
    QString getCondition3Value(const FwdSqlQuery& query) const {
        return query.fieldValue(m_iCondition3Value).toString();
    }
    QString getCondition3Combiner(const FwdSqlQuery& query) const {
        return query.fieldValue(m_iCondition3Combiner).toString();
    }
    QString getCondition4Field(const FwdSqlQuery& query) const {
        return query.fieldValue(m_iCondition4Field).toString();
    }
    QString getCondition4Operator(const FwdSqlQuery& query) const {
        return query.fieldValue(m_iCondition4Operator).toString();
    }
    QString getCondition4Value(const FwdSqlQuery& query) const {
        return query.fieldValue(m_iCondition4Value).toString();
    }
    QString getCondition4Combiner(const FwdSqlQuery& query) const {
        return query.fieldValue(m_iCondition4Combiner).toString();
    }
    QString getCondition5Field(const FwdSqlQuery& query) const {
        return query.fieldValue(m_iCondition5Field).toString();
    }
    QString getCondition5Operator(const FwdSqlQuery& query) const {
        return query.fieldValue(m_iCondition5Operator).toString();
    }
    QString getCondition5Value(const FwdSqlQuery& query) const {
        return query.fieldValue(m_iCondition5Value).toString();
    }
    QString getCondition5Combiner(const FwdSqlQuery& query) const {
        return query.fieldValue(m_iCondition5Combiner).toString();
    }
    QString getCondition6Field(const FwdSqlQuery& query) const {
        return query.fieldValue(m_iCondition6Field).toString();
    }
    QString getCondition6Operator(const FwdSqlQuery& query) const {
        return query.fieldValue(m_iCondition6Operator).toString();
    }
    QString getCondition6Value(const FwdSqlQuery& query) const {
        return query.fieldValue(m_iCondition6Value).toString();
    }
    QString getCondition6Combiner(const FwdSqlQuery& query) const {
        return query.fieldValue(m_iCondition6Combiner).toString();
    }
    QString getCondition7Field(const FwdSqlQuery& query) const {
        return query.fieldValue(m_iCondition7Field).toString();
    }
    QString getCondition7Operator(const FwdSqlQuery& query) const {
        return query.fieldValue(m_iCondition7Operator).toString();
    }
    QString getCondition7Value(const FwdSqlQuery& query) const {
        return query.fieldValue(m_iCondition7Value).toString();
    }
    QString getCondition7Combiner(const FwdSqlQuery& query) const {
        return query.fieldValue(m_iCondition7Combiner).toString();
    }
    QString getCondition8Field(const FwdSqlQuery& query) const {
        return query.fieldValue(m_iCondition8Field).toString();
    }
    QString getCondition8Operator(const FwdSqlQuery& query) const {
        return query.fieldValue(m_iCondition8Operator).toString();
    }
    QString getCondition8Value(const FwdSqlQuery& query) const {
        return query.fieldValue(m_iCondition8Value).toString();
    }
    QString getCondition8Combiner(const FwdSqlQuery& query) const {
        return query.fieldValue(m_iCondition8Combiner).toString();
    }
    QString getCondition9Field(const FwdSqlQuery& query) const {
        return query.fieldValue(m_iCondition9Field).toString();
    }
    QString getCondition9Operator(const FwdSqlQuery& query) const {
        return query.fieldValue(m_iCondition9Operator).toString();
    }
    QString getCondition9Value(const FwdSqlQuery& query) const {
        return query.fieldValue(m_iCondition9Value).toString();
    }
    QString getCondition9Combiner(const FwdSqlQuery& query) const {
        return query.fieldValue(m_iCondition9Combiner).toString();
    }
    QString getCondition10Field(const FwdSqlQuery& query) const {
        return query.fieldValue(m_iCondition10Field).toString();
    }
    QString getCondition10Operator(const FwdSqlQuery& query) const {
        return query.fieldValue(m_iCondition10Operator).toString();
    }
    QString getCondition10Value(const FwdSqlQuery& query) const {
        return query.fieldValue(m_iCondition10Value).toString();
    }
    QString getCondition10Combiner(const FwdSqlQuery& query) const {
        return query.fieldValue(m_iCondition10Combiner).toString();
    }
    QString getCondition11Field(const FwdSqlQuery& query) const {
        return query.fieldValue(m_iCondition11Field).toString();
    }
    QString getCondition11Operator(const FwdSqlQuery& query) const {
        return query.fieldValue(m_iCondition11Operator).toString();
    }
    QString getCondition11Value(const FwdSqlQuery& query) const {
        return query.fieldValue(m_iCondition11Value).toString();
    }
    QString getCondition11Combiner(const FwdSqlQuery& query) const {
        return query.fieldValue(m_iCondition11Combiner).toString();
    }
    QString getCondition12Field(const FwdSqlQuery& query) const {
        return query.fieldValue(m_iCondition12Field).toString();
    }
    QString getCondition12Operator(const FwdSqlQuery& query) const {
        return query.fieldValue(m_iCondition12Operator).toString();
    }
    QString getCondition12Value(const FwdSqlQuery& query) const {
        return query.fieldValue(m_iCondition12Value).toString();
    }
    QString getCondition12Combiner(const FwdSqlQuery& query) const {
        return query.fieldValue(m_iCondition12Combiner).toString();
    }

  private:
    DbFieldIndex m_iId;
    DbFieldIndex m_iName;
    DbFieldIndex m_iLocked;
    DbFieldIndex m_iAutoDjSource;
    DbFieldIndex m_iSearchInput;
    DbFieldIndex m_iSearchSql;
    DbFieldIndex m_iCondition1Field;
    DbFieldIndex m_iCondition1Operator;
    DbFieldIndex m_iCondition1Value;
    DbFieldIndex m_iCondition1Combiner;
    DbFieldIndex m_iCondition2Field;
    DbFieldIndex m_iCondition2Operator;
    DbFieldIndex m_iCondition2Value;
    DbFieldIndex m_iCondition2Combiner;
    DbFieldIndex m_iCondition3Field;
    DbFieldIndex m_iCondition3Operator;
    DbFieldIndex m_iCondition3Value;
    DbFieldIndex m_iCondition3Combiner;
    DbFieldIndex m_iCondition4Field;
    DbFieldIndex m_iCondition4Operator;
    DbFieldIndex m_iCondition4Value;
    DbFieldIndex m_iCondition4Combiner;
    DbFieldIndex m_iCondition5Field;
    DbFieldIndex m_iCondition5Operator;
    DbFieldIndex m_iCondition5Value;
    DbFieldIndex m_iCondition5Combiner;
    DbFieldIndex m_iCondition6Field;
    DbFieldIndex m_iCondition6Operator;
    DbFieldIndex m_iCondition6Value;
    DbFieldIndex m_iCondition6Combiner;
    DbFieldIndex m_iCondition7Field;
    DbFieldIndex m_iCondition7Operator;
    DbFieldIndex m_iCondition7Value;
    DbFieldIndex m_iCondition7Combiner;
    DbFieldIndex m_iCondition8Field;
    DbFieldIndex m_iCondition8Operator;
    DbFieldIndex m_iCondition8Value;
    DbFieldIndex m_iCondition8Combiner;
    DbFieldIndex m_iCondition9Field;
    DbFieldIndex m_iCondition9Operator;
    DbFieldIndex m_iCondition9Value;
    DbFieldIndex m_iCondition9Combiner;
    DbFieldIndex m_iCondition10Field;
    DbFieldIndex m_iCondition10Operator;
    DbFieldIndex m_iCondition10Value;
    DbFieldIndex m_iCondition10Combiner;
    DbFieldIndex m_iCondition11Field;
    DbFieldIndex m_iCondition11Operator;
    DbFieldIndex m_iCondition11Value;
    DbFieldIndex m_iCondition11Combiner;
    DbFieldIndex m_iCondition12Field;
    DbFieldIndex m_iCondition12Operator;
    DbFieldIndex m_iCondition12Value;
    DbFieldIndex m_iCondition12Combiner;
};

class SearchCrateSelectResult : public FwdSqlQuerySelectResult {
  public:
    SearchCrateSelectResult(SearchCrateSelectResult&& other)
            : FwdSqlQuerySelectResult(std::move(other)),
              m_queryFields(std::move(other.m_queryFields)) {
    }
    ~SearchCrateSelectResult() override = default;

    bool populateNext(SearchCrate* pSearchCrate) {
        if (next()) {
            m_queryFields.populateFromQuery(query(), pSearchCrate);
            return true;
        } else {
            return false;
        }
    }

    // private:
    friend class SearchCrateStorage;
    SearchCrateSelectResult() = default;
    explicit SearchCrateSelectResult(FwdSqlQuery&& query)
            : FwdSqlQuerySelectResult(std::move(query)),
              m_queryFields(FwdSqlQuerySelectResult::query()) {
    }

    SearchCrateQueryFields m_queryFields;

  private:
};

class SearchCrateSummaryQueryFields : public SearchCrateQueryFields {
  public:
    SearchCrateSummaryQueryFields() = default;
    explicit SearchCrateSummaryQueryFields(const FwdSqlQuery& query);
    ~SearchCrateSummaryQueryFields() override = default;

    uint getTrackCount(const FwdSqlQuery& query) const {
        QVariant varTrackCount = query.fieldValue(m_iTrackCount);
        if (varTrackCount.isNull()) {
            return 0; // searchCrate is empty
        } else {
            return varTrackCount.toUInt();
        }
    }
    double getTrackDuration(const FwdSqlQuery& query) const {
        QVariant varTrackDuration = query.fieldValue(m_iTrackDuration);
        if (varTrackDuration.isNull()) {
            return 0.0; // searchCrate is empty
        } else {
            return varTrackDuration.toDouble();
        }
    }

    void populateFromQuery(
            const FwdSqlQuery& query,
            SearchCrateSummary* pSearchCrateSummary) const;

  private:
    DbFieldIndex m_iTrackCount;
    DbFieldIndex m_iTrackDuration;
};

class SearchCrateSummarySelectResult : public FwdSqlQuerySelectResult {
  public:
    SearchCrateSummarySelectResult(SearchCrateSummarySelectResult&& other)
            : FwdSqlQuerySelectResult(std::move(other)),
              m_queryFields(std::move(other.m_queryFields)) {
    }
    ~SearchCrateSummarySelectResult() override = default;

    bool populateNext(SearchCrateSummary* pSearchCrateSummary) {
        if (next()) {
            m_queryFields.populateFromQuery(query(), pSearchCrateSummary);
            return true;
        } else {
            return false;
        }
    }

  private:
    friend class SearchCrateStorage;
    SearchCrateSummarySelectResult() = default;
    explicit SearchCrateSummarySelectResult(FwdSqlQuery&& query)
            : FwdSqlQuerySelectResult(std::move(query)),
              m_queryFields(FwdSqlQuerySelectResult::query()) {
    }

    SearchCrateSummaryQueryFields m_queryFields;
};

class SearchCrateTrackQueryFields {
  public:
    SearchCrateTrackQueryFields() = default;
    explicit SearchCrateTrackQueryFields(const FwdSqlQuery& query);
    virtual ~SearchCrateTrackQueryFields() = default;

    SearchCrateId searchCrateId(const FwdSqlQuery& query) const {
        return SearchCrateId(query.fieldValue(m_iSearchCrateId));
    }
    TrackId trackId(const FwdSqlQuery& query) const {
        return TrackId(query.fieldValue(m_iTrackId));
    }

  private:
    DbFieldIndex m_iSearchCrateId;
    DbFieldIndex m_iTrackId;
};

class SearchCrateTrackSelectResult : public FwdSqlQuerySelectResult {
  public:
    SearchCrateTrackSelectResult(SearchCrateTrackSelectResult&& other)
            : FwdSqlQuerySelectResult(std::move(other)),
              m_queryFields(std::move(other.m_queryFields)) {
    }
    ~SearchCrateTrackSelectResult() override = default;

    SearchCrateId searchCrateId() const {
        return m_queryFields.searchCrateId(query());
    }
    TrackId trackId() const {
        return m_queryFields.trackId(query());
    }

  private:
    friend class SearchCrateStorage;
    SearchCrateTrackSelectResult() = default;
    explicit SearchCrateTrackSelectResult(FwdSqlQuery&& query)
            : FwdSqlQuerySelectResult(std::move(query)),
              m_queryFields(FwdSqlQuerySelectResult::query()) {
    }

    SearchCrateTrackQueryFields m_queryFields;
};

class SearchCrateStorage : public virtual /*implements*/ SqlStorage {
  public:
    SearchCrateStorage() = default;
    ~SearchCrateStorage() override = default;

    void repairDatabase(
            const QSqlDatabase& database) override;

    void connectDatabase(
            const QSqlDatabase& database) override;
    void disconnectDatabase() override;

    /////////////////////////////////////////////////////////////////////////
    // SearchCrate write operations (transactional, non-const)
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

    bool onInsertingSearchCrate(const SearchCrate& searchCrate,
            SearchCrateId* pSearchCrateId = nullptr);

    bool onUpdatingSearchCrate(const SearchCrate& searchCrate);

    bool onDeletingSearchCrate(SearchCrateId searchCrateId);

    bool onAddingSearchCrateTracks(SearchCrateId searchCrateId, const QList<TrackId>& trackIds);

    bool onRemovingSearchCrateTracks(SearchCrateId searchCrateId, const QList<TrackId>& trackIds);

    bool onPurgingTracks(const QList<TrackId>& trackIds);

    /////////////////////////////////////////////////////////////////////////
    // SearchCrate read operations (read-only, const)
    /////////////////////////////////////////////////////////////////////////

    uint countSearchCrate() const;

    // Omit the pSearchCrate parameter for checking if the corresponding searchCrate exists.
    bool readSearchCrateById(
            SearchCrateId id,
            SearchCrate* pSearchCrate = nullptr) const;
    bool readSearchCrateByName(
            const QString& name,
            SearchCrate* pSearchCrate = nullptr) const;

    // The following list results are ordered by searchCrate name:
    //  - case-insensitive
    //  - locale-aware
    SearchCrateSelectResult selectSearchCrate()
            const; // all searchCrate
                   //    SearchCrateSelectResult selectSearchCrateByIds(    // subset
                   //    of searchCrate
                   //            const QString& subselectForSearchCrateIds,
                   //            SqlSubselectMode subselectMode) const;

    // TODO(XXX): Move this function into the AutoDJ component after
    // fixing various database design flaws in AutoDJ itself (see also:
    // searchCratechema.h). AutoDJ should use the function selectSearchCrateByIds()
    // from this class for the actual implementation.
    // This refactoring should be deferred until consensus on the
    // redesign of the AutoDJ feature has been reached. The main
    // ideas of the new design should be documented for verification
    // before starting to code.
    //    SearchCrateSelectResult selectAutoDjSearchCrate(bool autoDjSource = true) const;

    // SearchCrate content, i.e. the searchCrate's tracks referenced by id
    uint countSearchCrateTracks(SearchCrateId searchCrateId) const;

    // Format a subselect query for the tracks contained in searchCrate.
    static QString formatSubselectQueryForSearchCrateTrackIds(
            SearchCrateId searchCrateId); // no db access

    static QString returnSearchSQLFieldFromTable(SearchCrateId searchCrateId);

    QString formatQueryForTrackIdsBySearchCrateNameLike(
            const QString& searchCrateNameLike) const;      // no db access
    static QString formatQueryForTrackIdsWithSearchCrate(); // no db access
    // Select the track ids of a searchCrate or the searchCrate ids of a track respectively.
    // The results are sorted (ascending) by the target id, i.e. the id that is
    // not provided for filtering. This enables the caller to perform efficient
    // binary searches on the result set after storing it in a list or vector.
    SearchCrateTrackSelectResult selectSearchCrateTracksSorted(
            SearchCrateId searchCrateId) const;
    SearchCrateTrackSelectResult selectTrackSearchCrateSorted(
            TrackId trackId) const;
    SearchCrateSummarySelectResult selectSearchCrateWithTrackCount(
            const QList<TrackId>& trackIds) const;
    SearchCrateTrackSelectResult selectTracksSortedBySearchCrateNameLike(
            const QString& searchCrateNameLike) const;
    //    TrackSelectResult selectAllTracksSorted() const;

    // Returns the set of searchCrate ids for searchCrate that contain any of the
    // provided track ids.
    QSet<SearchCrateId> collectSearchCrateIdsOfTracks(
            const QList<TrackId>& trackIds) const;

    /////////////////////////////////////////////////////////////////////////
    // SearchCrateSummary view operations (read-only, const)
    /////////////////////////////////////////////////////////////////////////

    // Track summaries of all searchCrate:
    //  - Hidden tracks are excluded from the searchCrate summary statistics
    //  - The result list is ordered by searchCrate name:
    //     - case-insensitive
    //     - locale-aware
    SearchCrateSummarySelectResult selectSearchCrateSummaries() const; // all searchCrate

    // Omit the pSearchCrate parameter for checking if the corresponding searchCrate exists.
    bool readSearchCrateSummaryById(SearchCrateId id,
            SearchCrateSummary* pSearchCrateSummary = nullptr) const;

  private:
    void createViews();

    QSqlDatabase m_database;
};
