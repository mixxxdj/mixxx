/***************************************************************************
                           rhythmboxtracksource.h
                              -------------------
     begin                : 8/15/2009
     copyright            : (C) 2009 Albert Santoni
     email                : alberts@mixxx.org
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef RHYTHMBOXTRACKMODEL_H
#define RHYTHMBOXTRACKMODEL_H

#include <QtSql>
#include <QtXml>
#include "trackmodel.h"

class TrackInfoObject;
class QSqlDatabase;


/**
   @author Albert Santoni
*/
class RhythmboxTrackModel : public QAbstractTableModel, public TrackModel
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
    RhythmboxTrackModel();
    virtual ~RhythmboxTrackModel();

    //QAbstractTableModel stuff
    virtual Qt::ItemFlags flags ( const QModelIndex & index ) const;
    virtual QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
    virtual QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
    virtual int rowCount ( const QModelIndex & parent = QModelIndex() ) const;
    virtual int columnCount(const QModelIndex& parent) const;

    //Track Model stuff
	virtual TrackInfoObject* getTrack(const QModelIndex& index) const;
	virtual QString getTrackLocation(const QModelIndex& index) const;
    virtual TrackInfoObject *getTrackByLocation(const QString& location) const;
	virtual void search(const QString& searchText);
	virtual void removeTrack(const QModelIndex& index);
	virtual void addTrack(const QModelIndex& index, QString location);
	
	
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
    QDomNodeList m_entryNodes;
    QMap <QString, QDomNode> m_mTracksByLocation;
};

#endif
