#pragma once

#include <QSqlDatabase>

#include "library/trackset/playlist/playlistsummary.h"
#include "util/db/fwdsqlqueryselectresult.h"
#include "util/db/sqlstorage.h"

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

    void populateFromQuery(const FwdSqlQuery& query, Playlist* pPlaylist) const;

  private:
    DbFieldIndex m_iId;
    DbFieldIndex m_iName;
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

    void populateFromQuery(const FwdSqlQuery& query, PlaylistSummary* pPlaylistSummary) const;

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

class PlaylistStorage : public virtual /*implements*/ SqlStorage {
  public:
    PlaylistStorage() = default;
    ~PlaylistStorage() override = default;

    void repairDatabase(const QSqlDatabase& database) override;

    void connectDatabase(const QSqlDatabase& database) override;
    void disconnectDatabase() override;

    /////////////////////////////////////////////////////////////////////////
    // Playlist read operations (read-only, const)
    /////////////////////////////////////////////////////////////////////////

    uint countPlaylists() const;

    /////////////////////////////////////////////////////////////////////////
    // PlaylistSummary view operations (read-only, const)
    /////////////////////////////////////////////////////////////////////////

    PlaylistSummarySelectResult selectPlaylistSummaries() const;

    // Omit the pPlaylistSummary parameter for checking if the corresponding crate exists.
    bool readPlaylistSummaryById(PlaylistId playlistId,
            PlaylistSummary* pPlaylistSummary = nullptr) const;

  private:
    void createViews();

    QSqlDatabase m_database;
};
