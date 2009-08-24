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
#include "abstractxmltrackmodel.h"


class TrackInfoObject;
class QSqlDatabase;


/**
   @author Albert Santoni
*/
class RhythmboxTrackModel : public AbstractXmlTrackModel
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
    virtual QItemDelegate* delegateForColumn(const int i);

protected:
    virtual TrackInfoObject *parseTrackNode(QDomNode node) const;
    /* Implemented by AbstractXmlTrackModel implementations to return the data for song columns */
    virtual QVariant getTrackColumnData(QDomNode node, const QModelIndex& index) const;
    /* Called by AbstractXmlTrackModel implementations to enumerate their columns */

public slots:

signals:
 	void startedLoading();
 	void progressLoading(QString path);
 	void finishedLoading();
};

#endif
