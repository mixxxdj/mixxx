#ifndef PLAYLISTDAO_H
#define PLAYLISTDAO_H

#include <QObject>
#include <QSqlDatabase>
#include "library/dao/dao.h"

const QString PLAYLISTTRACKSTABLE_POSITION = "position";
const QString PLAYLISTTRACKSTABLE_PLAYLISTID = "playlist_id";

class PlaylistDAO : public QObject, public virtual DAO {
    Q_OBJECT
  public:
	enum hidden_type {
		PLHT_NOT_HIDDEN = 0,
		PLHT_AUTO_DJ = 1,
		PLHT_SET_LOG = 2,
		PLHT_UNKNOWN = -1
	};
    PlaylistDAO(QSqlDatabase& database);
    virtual ~PlaylistDAO();

    void initialize();
    void setDatabase(QSqlDatabase& database) { m_database = database; };
    /** Create a playlist */
    bool createPlaylist(QString name, enum hidden_type = PLHT_NOT_HIDDEN);
    /** Delete a playlist */
    void deletePlaylist(int playlistId);
    /** Rename a playlist */
    void renamePlaylist(int playlistId, const QString& newName);
    /** Lock or unlock a playlist */
    bool setPlaylistLocked(int playlistId, bool locked);
    /** Find out the state of a playlist lock */
    bool isPlaylistLocked(int playlistId);
    /** Append a track to a playlist */
    void appendTrackToPlaylist(int trackId, int playlistId);
    /** Find out how many playlists exist. */
    unsigned int playlistCount();
    /** Get the name of the playlist at the given position */
    QString getPlaylistName(unsigned int position);
    // Get the playlist id by its name
    int getPlaylistIdFromName(QString name);
    /** Get the id of the playlist at position. Note that the position is the
     * natural position in the database table, not the display order position
     * column stored in the database. */
    int getPlaylistId(int position);
    // Returns true if the playlist with playlistId is hidden
    bool isHidden(int playlistId);
    // Returns cause of playlistId is hidden
    enum hidden_type getHiddenType(int playlistId);
    /** Remove a track from a playlist */
    void removeTrackFromPlaylist(int playlistId, int position);
    /** Insert a track into a specific position in a playlist */
    void insertTrackIntoPlaylist(int trackId, int playlistId, int position);
    /** Add a playlist to the Auto-DJ Queue */
    void addToAutoDJQueue(int playlistId, bool bTop);
  signals:
    void added(int playlistId);
    void deleted(int playlistId);
    void changed(int playlistId);
    void trackAdded(int playlistId, int trackId, int position);
    void trackRemoved(int playlistId, int trackId, int position);
  private:
    QSqlDatabase& m_database;
};

#endif //PLAYLISTDAO_H
