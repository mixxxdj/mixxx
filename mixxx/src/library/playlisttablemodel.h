#ifndef PLAYLISTTABLEMODEL_H
#define PLAYLISTTABLEMODEL_H

#include <QtSql>
#include <QItemDelegate>
#include <QtCore>

#include "library/basesqltablemodel.h"
#include "library/dao/playlistdao.h"
#include "library/dao/trackdao.h"
#include "library/librarytablemodel.h"

class TrackCollection;

class PlaylistTableModel : public BaseSqlTableModel {
    Q_OBJECT
  public:
    PlaylistTableModel(QObject* parent, TrackCollection* pTrackCollection,
                       QString settingsNamespace);
    virtual ~PlaylistTableModel();
    void setPlaylist(int playlistId);
    int getPlaylist() const {
        return m_iPlaylistId;
    }
    virtual TrackPointer getTrack(const QModelIndex& index) const;

    virtual void search(const QString& searchText);
    virtual bool isColumnInternal(int column);
    virtual bool isColumnHiddenByDefault(int column);
    virtual void removeTrack(const QModelIndex& index);
    virtual void removeTracks(const QModelIndexList& indices);
    virtual bool addTrack(const QModelIndex& index, QString location);
    virtual bool appendTrack(int trackId);
    // Adding multiple tracks at one to a playlist. Returns the number of
    // successful additions.
    virtual int addTracks(const QModelIndex& index, QList<QString> locations);
    virtual void moveTrack(const QModelIndex& sourceIndex, const QModelIndex& destIndex);
    virtual void shuffleTracks(const QModelIndex& currentIndex);

    QAbstractItemDelegate* delegateForColumn(const int i, QObject* pObject);
    TrackModel::CapabilitiesFlags getCapabilities() const;

  private slots:
    void slotSearch(const QString& searchText);

  signals:
    void doSearch(const QString& searchText);

  private:
    TrackCollection* m_pTrackCollection;
    PlaylistDAO& m_playlistDao;
    TrackDAO& m_trackDao;
    int m_iPlaylistId;
};

#endif
