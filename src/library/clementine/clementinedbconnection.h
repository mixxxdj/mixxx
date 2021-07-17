#pragma once

#include <QFileInfo>
#include <QSqlDatabase>
#include <QUrl>

//Forward declare
class TrackCollectionManager;

struct ClementinePlaylist {
    QString playlistId;
    QString name;
};

struct ClementinePlaylistEntry {
    int trackId;
    QString title;
    QUrl uri;
    int duration;
    int year;
    int rating;
    QString genre;
    QString grouping;
    int tracknumber;
    int dateadded;
    double bpm;
    int bitrate;
    QString comment;
    int playcount;
    QString composer;
    QString artist;
    QString album;
    QString albumartist;
};

class ClementineDbConnection {
  public:
    ClementineDbConnection();
    ~ClementineDbConnection();

    static QFileInfo getDatabaseFile();

    bool open(const QFileInfo& databaseFile);
    QList<ClementinePlaylist> getPlaylists() const;
    QList<ClementinePlaylistEntry> getPlaylistEntries(int playlistId) const;

  private:
    QSqlDatabase m_database;
};
