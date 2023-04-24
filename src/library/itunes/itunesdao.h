#pragma once

#include <QHash>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QString>
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
};

struct ITunesPlaylist {
    int id;
    QString name;
};

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

    virtual void appendPlaylistTree(TreeItem& item, int playlistId = kRootITunesPlaylistId);

  private:
    QHash<QString, int> m_playlistDuplicatesByName;
    QHash<int, QString> m_playlistNameById;
    std::multimap<int, int> m_playlistIdsByParentId;

    QSqlQuery m_insertTrackQuery;
    QSqlQuery m_insertPlaylistQuery;
    QSqlQuery m_insertPlaylistTrackQuery;
    QSqlQuery m_applyPathMappingQuery;

    QString uniquifyPlaylistName(QString name);
};
