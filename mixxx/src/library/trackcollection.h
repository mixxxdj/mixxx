/***************************************************************************
                           trackcollection.h
                              -------------------
     begin                : 10/27/2008
     copyright            : (C) 2008 Albert Santoni
     email                : gamegod \a\t users.sf.net
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef TRACKCOLLECTION_H
#define TRACKCOLLECTION_H

#include <QtSql>
#include <QSqlDatabase>

#include "library/dao/cratedao.h"

class TrackInfoObject;
class BpmDetector;

const QString LIBRARYTABLE_ID = "id";
const QString LIBRARYTABLE_ARTIST = "artist";
const QString LIBRARYTABLE_TITLE = "title";
const QString LIBRARYTABLE_ALBUM = "album";
const QString LIBRARYTABLE_YEAR = "year";
const QString LIBRARYTABLE_GENRE = "genre";
const QString LIBRARYTABLE_TRACKNUMBER = "tracknumber";
const QString LIBRARYTABLE_LOCATION = "location";
const QString LIBRARYTABLE_FILENAME = "filename";
const QString LIBRARYTABLE_COMMENT = "comment";
const QString LIBRARYTABLE_DURATION = "duration";
const QString LIBRARYTABLE_BITRATE = "bitrate";
const QString LIBRARYTABLE_BPM = "bpm";
const QString LIBRARYTABLE_LENGTHINBYTES = "length_in_bytes";
const QString LIBRARYTABLE_CUEPOINT = "cuepoint";
const QString LIBRARYTABLE_URL = "url";
const QString LIBRARYTABLE_SAMPLERATE = "samplerate";
const QString LIBRARYTABLE_WAVESUMMARYHEX = "wavesummaryhex";
const QString LIBRARYTABLE_CHANNELS = "channels";

const QString PLAYLISTTRACKSTABLE_POSITION = "position";
const QString PLAYLISTTRACKSTABLE_PLAYLISTID = "playlist_id";

/**
   @author Albert Santoni
*/
class TrackCollection : public QObject
{
    Q_OBJECT
    public:
    TrackCollection();
    ~TrackCollection();
    /** Add a track to the database */
    void addTrack(TrackInfoObject *pTrack);
    void addTrack(QString location);
    /** Removes a track from the library track collection. */
    void removeTrack(QString location);

    /** Get a track from the database, identified by id. Returns 0 if the track was
      * not found */
    //TrackInfoObject *getTrack(int id);

    /** Get a track from the database, identified by pathname. Returns 0 if
      * the track was not found. If the track is not in the database, a TIO is
      * created and added to the database */
    TrackInfoObject* getTrack(QString location);
	int getSize();

 	void scanPath(QString path);
 	bool trackExistsInDatabase(QString file_location);
 	int getTrackId(QString location);

 	QList<TrackInfoObject*> dumpDB();

 	QSqlDatabase& getDatabase();
 	/** Create a playlist */
    void createPlaylist(QString name);
    /** Delete a playlist */
    void deletePlaylist(int playlistId);
    /** Append a track to a playlist */
    void appendTrackToPlaylist(int trackId, int playlistId);
    void appendTrackToPlaylist(QString location, int playlistId);
    /** Find out how many playlists exist. */
    unsigned int playlistCount();
    /** Get the name of the playlist at the given position */
    QString getPlaylistName(unsigned int position);
    /** Get the id of the playlist at position */
    int getPlaylistId(int position);
    /** Remove a track from a playlist */
    void removeTrackFromPlaylist(int playlistId, int position);
    /** Insert a track into a specific position in a playlist */
    void insertTrackIntoPlaylist(QString location, int playlistId, int position);
public slots:
 	bool checkForTables();
 	/** Saves a track's info back to the database */
 	void updateTrackInDatabase(TrackInfoObject*);

 	TrackInfoObject *getTrackFromDB(QSqlQuery &query);

 	void slotCancelLibraryScan();

    CrateDAO& getCrateDAO();

signals:
 	void startedLoading();
 	void progressLoading(QString path);
 	void finishedLoading();

private:
    BpmDetector* m_pBpmDetector;
    QSqlDatabase m_db;
    CrateDAO m_crateDao;
    /** Flag to raise when library scan should be cancelled */
    int bCancelLibraryScan;
};

#endif
