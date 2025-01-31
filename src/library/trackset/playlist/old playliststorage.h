#pragma once

#include <QList>
#include <QSet>

#include "library/trackset/playlist/playlistid.h"
#include "track/trackid.h"
#include "util/db/fwdsqlqueryselectresult.h"
#include "util/db/sqlstorage.h"
#include "util/db/sqlsubselectmode.h"

class Playlist;
class PlaylistSummary;

class PlaylistQueryFields {
  public:
    PlaylistQueryFields() {
    }
    explicit PlaylistQueryFields(const FwdSqlQuery& query);
    virtual ~PlaylistQueryFields() = default;

    PlaylistId getId(const FwdSqlQuery& query) const {
        return PlaylistId(query.fieldValue(m_iId));
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
            Playlist* pPlaylist) const;

  private:
    DbFieldIndex m_iId;
    DbFieldIndex m_iName;
    DbFieldIndex m_iLocked;
    DbFieldIndex m_iAutoDjSource;
};

class PlaylistSelectResult : public FwdSqlQuerySelectResult {
  public:
    PlaylistSelectResult(PlaylistSelectResult&& other)
            : FwdSqlQuerySelectResult(std::move(other)),
              m_queryFields(std::move(other.m_queryFields)) {
    }
    ~PlaylistSelectResult() override = default;

    bool populateNext(Playlist* pPlaylist) {
        if (next()) {
            m_queryFields.populateFromQuery(query(), pPlaylist);
            return true;
        } else {
            return false;
        }
    }

  private:
    friend class PlaylistStorage;
    PlaylistSelectResult() = default;
    explicit PlaylistSelectResult(FwdSqlQuery&& query)
            : FwdSqlQuerySelectResult(std::move(query)),
              m_queryFields(FwdSqlQuerySelectResult::query()) {
    }

    PlaylistQueryFields m_queryFields;
};

class PlaylistSummaryQueryFields : public PlaylistQueryFields {
  public:
    PlaylistSummaryQueryFields() = default;
    explicit PlaylistSummaryQueryFields(const FwdSqlQuery& query);
    ~PlaylistSummaryQueryFields() override = default;

    uint getTrackCount(const FwdSqlQuery& query) const {
        QVariant varTrackCount = query.fieldValue(m_iTrackCount);
        if (varTrackCount.isNull()) {
            return 0; // playlist is empty
        } else {
            return varTrackCount.toUInt();
        }
    }
    double getTrackDuration(const FwdSqlQuery& query) const {
        QVariant varTrackDuration = query.fieldValue(m_iTrackDuration);
        if (varTrackDuration.isNull()) {
            return 0.0; // playlist is empty
        } else {
            return varTrackDuration.toDouble();
        }
    }

    void populateFromQuery(
            const FwdSqlQuery& query,
            PlaylistSummary* pPlaylistSummary) const;

  private:
    DbFieldIndex m_iTrackCount;
    DbFieldIndex m_iTrackDuration;
};

class PlaylistSummarySelectResult : public FwdSqlQuerySelectResult {
  public:
    PlaylistSummarySelectResult(PlaylistSummarySelectResult&& other)
            : FwdSqlQuerySelectResult(std::move(other)),
              m_queryFields(std::move(other.m_queryFields)) {
    }
    ~PlaylistSummarySelectResult() override = default;

    bool populateNext(PlaylistSummary* pPlaylistSummary) {
        if (next()) {
            m_queryFields.populateFromQuery(query(), pPlaylistSummary);
            return true;
        } else {
            return false;
        }
    }

  private:
    friend class PlaylistStorage;
    PlaylistSummarySelectResult() = default;
    explicit PlaylistSummarySelectResult(FwdSqlQuery&& query)
            : FwdSqlQuerySelectResult(std::move(query)),
              m_queryFields(FwdSqlQuerySelectResult::query()) {
    }

    PlaylistSummaryQueryFields m_queryFields;
};

class PlaylistTrackQueryFields {
  public:
    PlaylistTrackQueryFields() = default;
    explicit PlaylistTrackQueryFields(const FwdSqlQuery& query);
    virtual ~PlaylistTrackQueryFields() = default;

    PlaylistId playlistId(const FwdSqlQuery& query) const {
        return PlaylistId(query.fieldValue(m_iPlaylistId));
    }
    TrackId trackId(const FwdSqlQuery& query) const {
        return TrackId(query.fieldValue(m_iTrackId));
    }

  private:
    DbFieldIndex m_iPlaylistId;
    DbFieldIndex m_iTrackId;
};

// class TrackQueryFields {
//   public:
//     TrackQueryFields() = default;
//     explicit TrackQueryFields(const FwdSqlQuery& query);
//     virtual ~TrackQueryFields() = default;
//
//     TrackId trackId(const FwdSqlQuery& query) const {
//         return TrackId(query.fieldValue(m_iTrackId));
//     }
//
//   private:
//     DbFieldIndex m_iTrackId;
// };

class PlaylistTrackSelectResult : public FwdSqlQuerySelectResult {
  public:
    PlaylistTrackSelectResult(PlaylistTrackSelectResult&& other)
            : FwdSqlQuerySelectResult(std::move(other)),
              m_queryFields(std::move(other.m_queryFields)) {
    }
    ~PlaylistTrackSelectResult() override = default;

    PlaylistId playlistId() const {
        return m_queryFields.playlistId(query());
    }
    TrackId trackId() const {
        return m_queryFields.trackId(query());
    }

  private:
    friend class PlaylistStorage;
    PlaylistTrackSelectResult() = default;
    explicit PlaylistTrackSelectResult(FwdSqlQuery&& query)
            : FwdSqlQuerySelectResult(std::move(query)),
              m_queryFields(FwdSqlQuerySelectResult::query()) {
    }

    PlaylistTrackQueryFields m_queryFields;
};

// class TrackSelectResult : public FwdSqlQuerySelectResult {
//   public:
//     TrackSelectResult(TrackSelectResult&& other)
//             : FwdSqlQuerySelectResult(std::move(other)),
//               m_queryFields(std::move(other.m_queryFields)) {
//     }
//     ~TrackSelectResult() override = default;
//
//     TrackId trackId() const {
//         return m_queryFields.trackId(query());
//     }
//
//   private:
//     friend class PlaylistStorage;
//     TrackSelectResult() = default;
//     explicit TrackSelectResult(FwdSqlQuery&& query)
//             : FwdSqlQuerySelectResult(std::move(query)),
//               m_queryFields(FwdSqlQuerySelectResult::query()) {
//     }
//
//     TrackQueryFields m_queryFields;
// };

class PlaylistStorage : public virtual /*implements*/ SqlStorage {
  public:
    PlaylistStorage() = default;
    ~PlaylistStorage() override = default;

    void repairDatabase(
            const QSqlDatabase& database) override;

    void connectDatabase(
            const QSqlDatabase& database) override;
    void disconnectDatabase() override;

    /////////////////////////////////////////////////////////////////////////
    // Playlist write operations (transactional, non-const)
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

    bool onInsertingPlaylist(
            const Playlist& playlist,
            PlaylistId* pPlaylistId = nullptr);

    bool onUpdatingPlaylist(
            const Playlist& playlist);

    bool onDeletingPlaylist(
            PlaylistId playlistId);

    bool onAddingPlaylistTracks(
            PlaylistId playlistId,
            const QList<TrackId>& trackIds);

    bool onRemovingPlaylistTracks(
            PlaylistId playlistId,
            const QList<TrackId>& trackIds);

    bool onPurgingTracks(
            const QList<TrackId>& trackIds);

    /////////////////////////////////////////////////////////////////////////
    // Playlist read operations (read-only, const)
    /////////////////////////////////////////////////////////////////////////

    uint countPlaylists() const;

    // Omit the pPlaylist parameter for checking if the corresponding playlist exists.
    bool readPlaylistById(
            PlaylistId id,
            Playlist* pPlaylist = nullptr) const;
    bool readPlaylistByName(
            const QString& name,
            Playlist* pPlaylist = nullptr) const;

    // The following list results are ordered by playlist name:
    //  - case-insensitive
    //  - locale-aware
    PlaylistSelectResult selectPlaylists() const; // all playlists
    PlaylistSelectResult selectPlaylistsByIds(    // subset of playlists
            const QString& subselectForPlaylistIds,
            SqlSubselectMode subselectMode) const;

    // TODO(XXX): Move this function into the AutoDJ component after
    // fixing various database design flaws in AutoDJ itself (see also:
    // playlistschema.h). AutoDJ should use the function selectPlaylistsByIds()
    // from this class for the actual implementation.
    // This refactoring should be deferred until consensus on the
    // redesign of the AutoDJ feature has been reached. The main
    // ideas of the new design should be documented for verification
    // before starting to code.
    PlaylistSelectResult selectAutoDjPlaylists(bool autoDjSource = true) const;

    // Playlist content, i.e. the playlist's tracks referenced by id
    uint countPlaylistTracks(PlaylistId playlistId) const;

    // Format a subselect query for the tracks contained in playlist.
    static QString formatSubselectQueryForPlaylistTrackIds(
            PlaylistId playlistId); // no db access

    QString formatQueryForTrackIdsByPlaylistNameLike(
            const QString& playlistNameLike) const;      // no db access
    static QString formatQueryForTrackIdsWithPlaylist(); // no db access
    // Select the track ids of a playlist or the playlist ids of a track respectively.
    // The results are sorted (ascending) by the target id, i.e. the id that is
    // not provided for filtering. This enables the caller to perform efficient
    // binary searches on the result set after storing it in a list or vector.
    PlaylistTrackSelectResult selectPlaylistTracksSorted(
            PlaylistId playlistId) const;
    PlaylistTrackSelectResult selectTrackPlaylistsSorted(
            TrackId trackId) const;
    PlaylistSummarySelectResult selectPlaylistsWithTrackCount(
            const QList<TrackId>& trackIds) const;
    PlaylistTrackSelectResult selectTracksSortedByPlaylistNameLike(
            const QString& playlistNameLike) const;
    TrackSelectResult selectAllTracksSorted() const;

    // Returns the set of playlist ids for playlists that contain any of the
    // provided track ids.
    QSet<PlaylistId> collectPlaylistIdsOfTracks(
            const QList<TrackId>& trackIds) const;

    /////////////////////////////////////////////////////////////////////////
    // PlaylistSummary view operations (read-only, const)
    /////////////////////////////////////////////////////////////////////////

    // Track summaries of all playlists:
    //  - Hidden tracks are excluded from the playlist summary statistics
    //  - The result list is ordered by playlist name:
    //     - case-insensitive
    //     - locale-aware
    PlaylistSummarySelectResult selectPlaylistSummaries() const; // all playlists

    // Omit the pPlaylist parameter for checking if the corresponding playlist exists.
    bool readPlaylistSummaryById(PlaylistId id, PlaylistSummary* pPlaylistSummary = nullptr) const;

  private:
    void createViews();

    QSqlDatabase m_database;
};
