/***************************************************************************
                           rhythmboxPlaylistsource.h
                              -------------------
     begin                : 8/17/2009
     copyright            : (C) 2009 Phillip Whelan
     email                : pwhelan@mixxx.org
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef RHYTHMBOXPLAYLISTMODEL_H
#define RHYTHMBOXPLAYLISTMODEL_H

#include <QtSql>
#include <QtXml>
#include "trackmodel.h"
#include "rhythmboxtrackmodel.h"

class TrackInfoObject;


/**
   @author Phillip Whelan
   Code copied originally from RhythmBoxTrackModel
*/
class RhythmboxPlaylistModel : public QAbstractTableModel, public TrackModel
{
    enum Columns {
        COLUMN_ARTIST = 0,
        COLUMN_TITLE,
        COLUMN_ALBUM,
        COLUMN_DATE,
        COLUMN_GENRE,
        COLUMN_LOCATION,
        COLUMN_DURATION,
        NUM_COLUMNS
    };

    Q_OBJECT
    public:
    RhythmboxPlaylistModel(RhythmboxTrackModel *);
    virtual ~RhythmboxPlaylistModel();

    //QAbstractTableModel stuff
    virtual Qt::ItemFlags flags ( const QModelIndex & index ) const;
    virtual QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
    virtual QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
    virtual int rowCount ( const QModelIndex & parent = QModelIndex() ) const;
    virtual int columnCount(const QModelIndex& parent) const;

    //Playlist Model stuff
	virtual TrackInfoObject* getTrack(const QModelIndex& index) const;
	virtual QString getTrackLocation(const QModelIndex& index) const;
	virtual void search(const QString& searchText);
	virtual void removeTrack(const QModelIndex& index);
	virtual void addTrack(const QModelIndex& index, QString location);
	virtual QList<QString> getPlaylists();
    virtual void setPlaylist(QString playlist);

    int numPlaylists();
    QString playlistTitle(int n);
    
	
/*
 	void scanPath(QString path);
 	bool trackExistsInDatabase(QString file_location);
 
 	QList<TrackInfoObject*> dumpDB();
 	
 	QSqlDatabase getDatabase();
 */
public slots:


signals:
 	void startedLoading();
 	void progressLoading(QString path);
 	void finishedLoading();
 
private:
    QDomNodeList m_playlistNodes;
    RhythmboxTrackModel *m_pRhythmbox;
    QMap<QString, QDomNodeList> m_mPlaylists;
    QString m_sCurrentPlaylist;
};

#endif
