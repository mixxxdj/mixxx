/***************************************************************************
                           ituestrackmodel.h
                              -------------------
     begin                : 8/28/2009
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

#ifndef ITUNESTRACKMODEL_H
#define ITUNESTRACKMODEL_H

#include <QtSql>
#include <QtXml>
#include "trackmodel.h"
#include "abstractxmltrackmodel.h"
#include "itunesplaylistmodel.h"

class TrackInfoObject;
class QSqlDatabase;

/**
   @author Phillip Whelan
*/

// Darn Jobs and his capitalization!
class ITunesTrackModel : public AbstractXmlTrackModel
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
    ITunesTrackModel();
    virtual ~ITunesTrackModel();
    virtual QItemDelegate* delegateForColumn(const int i);
    virtual bool isColumnInternal(int column);
    static QString getiTunesMusicPath();

  public slots:

  signals:
    void startedLoading();
    void progressLoading(QString path);
    void finishedLoading();

  protected:
    virtual TrackInfoObject *parseTrackNode(QDomNode node) const;
    /* Implemented by AbstractXmlTrackModel implementations to return the data for song columns */
    virtual QVariant getTrackColumnData(QDomNode node, const QModelIndex& index) const;
    /* Called by AbstractXmlTrackModel implementations to enumerate their columns */

  private:
    QString findValueByKey(QDomNode dictNode, QString key) const;
    QDomElement findNodeByKey(QDomNode dictNode, QString key) const;
    TrackInfoObject* getTrackById(QString id);

    QMap<QString, QDomNode> m_mTracksById;
    QMap<QString, QList<QString> > m_mPlaylists;

    friend class ITunesPlaylistModel;
};

#endif
