#pragma once

#include <QList>
#include <QSet>

#include "library/trackset/genre/genreid.h"
#include "track/trackid.h"
#include "util/db/fwdsqlqueryselectresult.h"
#include "util/db/sqlstorage.h"
#include "util/db/sqlsubselectmode.h"

class Genre;
class GenreSummary;

class GenreQueryFields {
  public:
    GenreQueryFields() {
    }
    explicit GenreQueryFields(const FwdSqlQuery& query);
    virtual ~GenreQueryFields() = default;

    GenreId getId(const FwdSqlQuery& query) const {
        return GenreId(query.fieldValue(m_iId));
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
    QString getNameLevel1(const FwdSqlQuery& query) const {
        return query.fieldValue(m_iNameLevel1).toString();
    }
    QString getNameLevel2(const FwdSqlQuery& query) const {
        return query.fieldValue(m_iNameLevel2).toString();
    }
    QString getNameLevel3(const FwdSqlQuery& query) const {
        return query.fieldValue(m_iNameLevel3).toString();
    }
    QString getNameLevel4(const FwdSqlQuery& query) const {
        return query.fieldValue(m_iNameLevel4).toString();
    }
    QString getNameLevel5(const FwdSqlQuery& query) const {
        return query.fieldValue(m_iNameLevel5).toString();
    }
    QString getDisplayGroup(const FwdSqlQuery& query) const {
        return query.fieldValue(m_iDisplayGroup).toString();
    }
    bool isModelDefined(const FwdSqlQuery& query) const {
        return query.fieldValueBoolean(m_iIsModelDefined);
    }
    bool isVisible(const FwdSqlQuery& query) const {
        return query.fieldValueBoolean(m_iIsVisible);
    }
    int getCount(const FwdSqlQuery& query) const {
        return query.fieldValue(m_iCount).toInt();
    }
    int getShow(const FwdSqlQuery& query) const {
        return query.fieldValueBoolean(m_iShow);
    }
    int getDisplayOrder(const FwdSqlQuery& query) const {
        return query.fieldValue(m_iDisplayOrder).toInt();
    }

    void populateFromQuery(
            const FwdSqlQuery& query,
            Genre* pGenre) const;

  private:
    DbFieldIndex m_iId;
    DbFieldIndex m_iName;
    DbFieldIndex m_iLocked;
    DbFieldIndex m_iAutoDjSource;
    DbFieldIndex m_iNameLevel1;
    DbFieldIndex m_iNameLevel2;
    DbFieldIndex m_iNameLevel3;
    DbFieldIndex m_iNameLevel4;
    DbFieldIndex m_iNameLevel5;
    DbFieldIndex m_iDisplayGroup;
    DbFieldIndex m_iIsModelDefined;
    DbFieldIndex m_iIsVisible;
    DbFieldIndex m_iCount;
    DbFieldIndex m_iShow;
    DbFieldIndex m_iDisplayOrder;
};

class GenreSelectResult : public FwdSqlQuerySelectResult {
  public:
    GenreSelectResult(GenreSelectResult&& other)
            : FwdSqlQuerySelectResult(std::move(other)),
              m_queryFields(std::move(other.m_queryFields)) {
    }
    ~GenreSelectResult() override = default;

    bool populateNext(Genre* pGenre) {
        if (next()) {
            m_queryFields.populateFromQuery(query(), pGenre);
            return true;
        } else {
            return false;
        }
    }

  private:
    friend class GenreStorage;
    GenreSelectResult() = default;
    explicit GenreSelectResult(FwdSqlQuery&& query)
            : FwdSqlQuerySelectResult(std::move(query)),
              m_queryFields(FwdSqlQuerySelectResult::query()) {
    }

    GenreQueryFields m_queryFields;
};

class GenreSummaryQueryFields : public GenreQueryFields {
  public:
    GenreSummaryQueryFields() = default;
    explicit GenreSummaryQueryFields(const FwdSqlQuery& query);
    ~GenreSummaryQueryFields() override = default;

    uint getTrackCount(const FwdSqlQuery& query) const {
        QVariant varTrackCount = query.fieldValue(m_iTrackCount);
        if (varTrackCount.isNull()) {
            return 0; // genre is empty
        } else {
            return varTrackCount.toUInt();
        }
    }
    double getTrackDuration(const FwdSqlQuery& query) const {
        QVariant varTrackDuration = query.fieldValue(m_iTrackDuration);
        if (varTrackDuration.isNull()) {
            return 0.0; // genre is empty
        } else {
            return varTrackDuration.toDouble();
        }
    }

    void populateFromQuery(
            const FwdSqlQuery& query,
            GenreSummary* pGenreSummary) const;

  private:
    DbFieldIndex m_iTrackCount;
    DbFieldIndex m_iTrackDuration;
};

class GenreSummarySelectResult : public FwdSqlQuerySelectResult {
  public:
    GenreSummarySelectResult(GenreSummarySelectResult&& other)
            : FwdSqlQuerySelectResult(std::move(other)),
              m_queryFields(std::move(other.m_queryFields)) {
    }
    ~GenreSummarySelectResult() override = default;

    bool populateNext(GenreSummary* pGenreSummary) {
        if (next()) {
            m_queryFields.populateFromQuery(query(), pGenreSummary);
            return true;
        } else {
            return false;
        }
    }

  private:
    friend class GenreStorage;
    GenreSummarySelectResult() = default;
    explicit GenreSummarySelectResult(FwdSqlQuery&& query)
            : FwdSqlQuerySelectResult(std::move(query)),
              m_queryFields(FwdSqlQuerySelectResult::query()) {
    }

    GenreSummaryQueryFields m_queryFields;
};

class GenreTrackQueryFields {
  public:
    GenreTrackQueryFields() = default;
    explicit GenreTrackQueryFields(const FwdSqlQuery& query);
    virtual ~GenreTrackQueryFields() = default;

    GenreId genreId(const FwdSqlQuery& query) const {
        return GenreId(query.fieldValue(m_iGenreId));
    }
    TrackId trackId(const FwdSqlQuery& query) const {
        return TrackId(query.fieldValue(m_iTrackId));
    }

  private:
    DbFieldIndex m_iGenreId;
    DbFieldIndex m_iTrackId;
};

// class GenreTrackQueryFields {
//   public:
//     GenreTrackQueryFields() = default;
//     explicit GenreTrackQueryFields(const FwdSqlQuery& query);
//     virtual ~GenreTrackQueryFields() = default;
//
//     TrackId trackId(const FwdSqlQuery& query) const {
//         return TrackId(query.fieldValue(m_iTrackId));
//     }
//
//   private:
//     DbFieldIndex m_iTrackId;
// };

class GenreTrackSelectResult : public FwdSqlQuerySelectResult {
  public:
    GenreTrackSelectResult(GenreTrackSelectResult&& other)
            : FwdSqlQuerySelectResult(std::move(other)),
              m_queryFields(std::move(other.m_queryFields)) {
    }
    ~GenreTrackSelectResult() override = default;

    GenreId genreId() const {
        return m_queryFields.genreId(query());
    }
    TrackId trackId() const {
        return m_queryFields.trackId(query());
    }

  private:
    friend class GenreStorage;
    GenreTrackSelectResult() = default;
    explicit GenreTrackSelectResult(FwdSqlQuery&& query)
            : FwdSqlQuerySelectResult(std::move(query)),
              m_queryFields(FwdSqlQuerySelectResult::query()) {
    }

    GenreTrackQueryFields m_queryFields;
};

class GenreStorage : public virtual /*implements*/ SqlStorage {
  public:
    GenreStorage() = default;
    ~GenreStorage() override = default;

    void repairDatabase(
            const QSqlDatabase& database) override;

    void connectDatabase(
            const QSqlDatabase& database) override;
    void disconnectDatabase() override;

    /////////////////////////////////////////////////////////////////////////
    // Genre write operations (transactional, non-const)
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

    bool onInsertingGenre(
            const Genre& genre,
            GenreId* pGenreId = nullptr);

    bool onUpdatingGenre(
            const Genre& genre);

    bool onDeletingGenre(
            GenreId genreId);

    bool onAddingGenreTracks(
            GenreId genreId,
            const QList<TrackId>& trackIds);

    bool onRemovingGenreTracks(
            GenreId genreId,
            const QList<TrackId>& trackIds);

    bool onPurgingTracks(
            const QList<TrackId>& trackIds);

    /////////////////////////////////////////////////////////////////////////
    // Genre read operations (read-only, const)
    /////////////////////////////////////////////////////////////////////////

    uint countGenres() const;

    // Omit the pGenre parameter for checking if the corresponding genre exists.
    bool readGenreById(
            GenreId id,
            Genre* pGenre = nullptr) const;
    bool readGenreByName(
            const QString& name,
            Genre* pGenre = nullptr) const;

    // The following list results are ordered by genre name:
    //  - case-insensitive
    //  - locale-aware
    GenreSelectResult selectGenres() const; // all genres
    GenreSelectResult selectGenresByIds(    // subset of genres
            const QString& subselectForGenreIds,
            SqlSubselectMode subselectMode) const;

    // TODO(XXX): Move this function into the AutoDJ component after
    // fixing various database design flaws in AutoDJ itself (see also:
    // genreschema.h). AutoDJ should use the function selectGenresByIds()
    // from this class for the actual implementation.
    // This refactoring should be deferred until consensus on the
    // redesign of the AutoDJ feature has been reached. The main
    // ideas of the new design should be documented for verification
    // before starting to code.
    GenreSelectResult selectAutoDjGenres(bool autoDjSource = true) const;

    // Genre content, i.e. the genre's tracks referenced by id
    uint countGenreTracks(GenreId genreId) const;

    // Format a subselect query for the tracks contained in genre.
    static QString formatSubselectQueryForGenreTrackIds(
            GenreId genreId); // no db access

    QString formatQueryForTrackIdsByGenreNameLike(
            const QString& genreNameLike) const;      // no db access
    static QString formatQueryForTrackIdsWithGenre(); // no db access
    // Select the track ids of a genre or the genre ids of a track respectively.
    // The results are sorted (ascending) by the target id, i.e. the id that is
    // not provided for filtering. This enables the caller to perform efficient
    // binary searches on the result set after storing it in a list or vector.
    GenreTrackSelectResult selectGenreTracksSorted(
            GenreId genreId) const;
    GenreTrackSelectResult selectTrackGenresSorted(
            TrackId trackId) const;
    GenreSummarySelectResult selectGenresWithTrackCount(
            const QList<TrackId>& trackIds) const;
    GenreTrackSelectResult selectTracksSortedByGenreNameLike(
            const QString& genreNameLike) const;
    GenreTrackSelectResult selectAllTracksSorted() const;

    // Returns the set of genre ids for genres that contain any of the
    // provided track ids.
    QSet<GenreId> collectGenreIdsOfTracks(
            const QList<TrackId>& trackIds) const;

    /////////////////////////////////////////////////////////////////////////
    // GenreSummary view operations (read-only, const)
    /////////////////////////////////////////////////////////////////////////

    // Track summaries of all genres:
    //  - Hidden tracks are excluded from the genre summary statistics
    //  - The result list is ordered by genre name:
    //     - case-insensitive
    //     - locale-aware
    GenreSummarySelectResult selectGenreSummaries() const; // all genres

    // Omit the pGenre parameter for checking if the corresponding genre exists.
    bool readGenreSummaryById(GenreId id, GenreSummary* pGenreSummary = nullptr) const;

  private:
    void createViews();

    QSqlDatabase m_database;
};
