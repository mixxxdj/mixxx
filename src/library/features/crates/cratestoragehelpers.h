#ifndef MIXXX_CRATESTORAGEHELPERS_H
#define MIXXX_CRATESTORAGEHELPERS_H

#include "library/crate/cratesummary.h"
#include "library/crate/crateschema.h"
#include "library/dao/trackschema.h"
#include "track/trackid.h"

#include "util/db/fwdsqlqueryselectresult.h"


const QString CRATETABLE_LOCKED = "locked";

const QString CRATE_SUMMARY_VIEW = "crate_summary";

const QString CRATESUMMARY_TRACK_COUNT = "track_count";
const QString CRATESUMMARY_TRACK_DURATION = "track_duration";

const QString kCrateTracksJoin = QString(
        "LEFT JOIN %3 ON %3.%4=%1.%2").arg(
                CRATE_TABLE,
                CRATETABLE_ID,
                CRATE_TRACKS_TABLE,
                CRATETRACKSTABLE_CRATEID);

const QString kLibraryTracksJoin = kCrateTracksJoin + QString(
        " LEFT JOIN %3 ON %3.%4=%1.%2").arg(
                CRATE_TRACKS_TABLE,
                CRATETRACKSTABLE_TRACKID,
                LIBRARY_TABLE,
                LIBRARYTABLE_ID);

const QString kCrateSummaryViewSelect = QString(
        "SELECT %1.*,"
            "COUNT(CASE %2.%4 WHEN 0 THEN 1 ELSE NULL END) AS %5,"
            "SUM(CASE %2.%4 WHEN 0 THEN %2.%3 ELSE 0 END) AS %6 "
            "FROM %1").arg(
                CRATE_TABLE,
                LIBRARY_TABLE,
                LIBRARYTABLE_DURATION,
                LIBRARYTABLE_MIXXXDELETED,
                CRATESUMMARY_TRACK_COUNT,
                CRATESUMMARY_TRACK_DURATION);

const QString kCrateSummaryViewQuery = QString(
            "CREATE TEMPORARY VIEW IF NOT EXISTS %1 AS %2 %3 GROUP BY %4.%5").arg(
                    CRATE_SUMMARY_VIEW,
                    kCrateSummaryViewSelect,
                    kLibraryTracksJoin,
                    CRATE_TABLE,
                    CRATETABLE_ID);

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

class CrateQueryBinder {
  public:
    explicit CrateQueryBinder(FwdSqlQuery& query)
        : m_query(query) {
    }
    virtual ~CrateQueryBinder() = default;

    void bindId(const QString& placeholder, const Crate& crate) const {
        m_query.bindValue(placeholder, crate.getId());
    }
    void bindName(const QString& placeholder, const Crate& crate) const {
        m_query.bindValue(placeholder, crate.getName());
    }
    void bindLocked(const QString& placeholder, const Crate& crate) const {
        m_query.bindValue(placeholder, crate.isLocked());
    }
    void bindAutoDjSource(const QString& placeholder, const Crate& crate) const {
        m_query.bindValue(placeholder, crate.isAutoDjSource());
    }

  protected:
    FwdSqlQuery& m_query;
};

#endif // MIXXX_CRATESTORAGEHELPERS_H
