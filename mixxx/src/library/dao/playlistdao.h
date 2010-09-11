#ifndef PLAYLISTDAO_H
#define PLAYLISTDAO_H

#include <QSqlDatabase>
#include "library/dao/dao.h"

const QString PLAYLISTTRACKSTABLE_POSITION = "position";
const QString PLAYLISTTRACKSTABLE_PLAYLISTID = "playlist_id";

class PlaylistDAO : public virtual DAO {
  public:
    PlaylistDAO(QSqlDatabase& database);
    virtual ~PlaylistDAO();
    void initialize();
    void setDatabase(QSqlDatabase& database) { m_database = database; };
    /** Create a playlist */
    bool createPlaylist(QString name, bool hidden = false);
    /** Delete a playlist */
    void deletePlaylist(int playlistId);
    /** Append a track to a playlist */
    void appendTrackToPlaylist(int trackId, int playlistId);
    /** Find out how many playlists exist. */
    unsigned int playlistCount();
    /** Get the name of the playlist at the given position */
    QString getPlaylistName(unsigned int position);
    // Get the playlist id by its name
    int getPlaylistIdFromName(QString name);
    /** Get the id of the playlist at position */
    int getPlaylistId(int position);
    /** Remove a track from a playlist */
    void removeTrackFromPlaylist(int playlistId, int position);
    /** Insert a track into a specific position in a playlist */
    void insertTrackIntoPlaylist(int trackId, int playlistId, int position);
  private:
    QSqlDatabase& m_database;
};

#endif //PLAYLISTDAO_H
