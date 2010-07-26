// itunestrackmodel.h
// Created 12/19/2009 by RJ Ryan (rryan@mit.edu)
// Adapted from Phillip Whelan's RhythmboxPlaylistModel

#ifndef ITUNESPLAYLISTMODEL_H
#define ITUNESPLAYLISTMODEL_H

#include <QtSql>
#include <QtXml>
#include "trackmodel.h"

class ITunesTrackModel;

class ITunesPlaylistModel : public QAbstractTableModel, public TrackModel {

    enum Columns {
        COLUMN_ARTIST = 0,
        COLUMN_TITLE,
        COLUMN_ALBUM,
        COLUMN_DATE,
        COLUMN_BPM,
        COLUMN_GENRE,
        COLUMN_LOCATION,
        COLUMN_DURATION,
        NUM_COLUMNS
    };

    Q_OBJECT

  public:
    ITunesPlaylistModel(ITunesTrackModel* pTrackModel);
    virtual ~ITunesPlaylistModel();

    //QAbstractTableModel stuff
    virtual Qt::ItemFlags flags(const QModelIndex& index) const;
    virtual QVariant data(const QModelIndex & index,
                          int role = Qt::DisplayRole) const;
    virtual QVariant headerData(int section,
                                Qt::Orientation orientation,
                                int role = Qt::DisplayRole) const;
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex& parent) const;

    //Playlist Model stuff
    virtual TrackPointer getTrack(const QModelIndex& index) const;
    virtual QString getTrackLocation(const QModelIndex& index) const;
    virtual void search(const QString& searchText);
    virtual const QString currentSearch();
    virtual const QList<int>& searchColumns() const;
    virtual bool isColumnInternal(int column);
    virtual void removeTrack(const QModelIndex& index);
    virtual bool addTrack(const QModelIndex& index, QString location);
    virtual void moveTrack(const QModelIndex& sourceIndex,
                           const QModelIndex& destIndex);
    QItemDelegate* delegateForColumn(const int i);

    virtual QList<QString> getPlaylists();
    virtual void setPlaylist(QString playlist);

    int numPlaylists();
    QString playlistTitle(int n);

  signals:
    void startedLoading();
    void progressLoading(QString path);
    void finishedLoading();

  private:
    ITunesTrackModel* m_pTrackModel;
    QString m_sCurrentPlaylist;
    QString m_currentSearch;
};

#endif /* ITUNESPLAYLISTMODEL_H */
