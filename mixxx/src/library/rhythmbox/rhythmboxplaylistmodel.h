/***************************************************************************
                           rhythmboxplaylistmodel.h
                              -------------------
     begin                : 01/09/2011
     copyright            : (C) 2011 Tobias Rafreider

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
#include <QItemDelegate>
#include <QtCore>

#include "library/trackmodel.h"
#include "library/basesqltablemodel.h"
#include "library/librarytablemodel.h"
#include "library/dao/playlistdao.h"
#include "library/dao/trackdao.h"

class TrackCollection;

class RhythmboxPlaylistModel : public BaseSqlTableModel {
    Q_OBJECT
  public:
    RhythmboxPlaylistModel(QObject* parent, TrackCollection* pTrackCollection);
    virtual ~RhythmboxPlaylistModel();

    virtual TrackPointer getTrack(const QModelIndex& index) const;
    virtual void search(const QString& searchText);
    virtual bool isColumnInternal(int column);
    virtual bool isColumnHiddenByDefault(int column);
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    void setPlaylist(QString path_name);
    TrackModel::CapabilitiesFlags getCapabilities() const;

  private slots:
    void slotSearch(const QString& searchText);

  signals:
    void doSearch(const QString& searchText);

  private:
    TrackCollection* m_pTrackCollection;
    QSqlDatabase &m_database;
};

#endif
