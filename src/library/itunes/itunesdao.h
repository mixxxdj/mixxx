#pragma once

#include <QHash>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QString>
#include <gsl/pointers>
#include <map>

#include "library/dao/dao.h"
#include "library/itunes/itunespathmapping.h"
#include "library/treeitem.h"

const int kRootITunesPlaylistId = -1;

struct ITunesTrack {
    int id;
    QString artist;
    QString title;
    QString album;
    QString albumArtist;
    QString genre;
    QString grouping;
    int year;
    int duration;
    QString location;
    int rating;
    QString comment;
    int trackNumber;
    int bpm;
    int bitrate;

    bool operator==(const ITunesTrack&) const = default;
    bool operator!=(const ITunesTrack&) const = default;
};

struct ITunesPlaylist {
    int id;
    QString name;

    bool operator==(const ITunesPlaylist&) const = default;
    bool operator!=(const ITunesPlaylist&) const = default;
};

std::ostream& operator<<(std::ostream& os, const ITunesTrack& track);
std::ostream& operator<<(std::ostream& os, const ITunesPlaylist& playlist);

/// A wrapper around the iTunes database tables. Keeps track of the
/// playlist tree, deals with duplicate disambiguation and can export
/// the tree afterwards.
class ITunesDAO : public DAO {
  public:
    ~ITunesDAO() override = default;

    void initialize(const QSqlDatabase& database) override;

    virtual bool importTrack(const ITunesTrack& track);
    virtual bool importPlaylist(const ITunesPlaylist& playlist);
    virtual bool importPlaylistRelation(int parentId, int childId);
    virtual bool importPlaylistTrack(int playlistId, int trackId, int position);
    virtual bool applyPathMapping(const ITunesPathMapping& pathMapping);

    virtual void appendPlaylistTree(gsl::not_null<TreeItem*> item,
            int playlistId = kRootITunesPlaylistId);

  private:
    QHash<QString, int> m_playlistDuplicatesByName;
    QHash<int, QString> m_playlistNameById;
    std::multimap<int, int> m_playlistIdsByParentId;

    // Keeps track of whether the database has been initialized.
    // In tests the database is not used, so the importer will
    // only be used to construct a TreeItem.
    bool m_isDatabaseInitialized = false;

    // Note that these queries reference the database, which is expected
    // to outlive the DAO.
    QSqlQuery m_insertTrackQuery;
    QSqlQuery m_insertPlaylistQuery;
    QSqlQuery m_insertPlaylistTrackQuery;
    QSqlQuery m_applyPathMappingQuery;

    QString uniquifyPlaylistName(QString name);
};
