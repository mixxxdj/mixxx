#ifndef PLAYLISTTABLEMODEL_H
#define PLAYLISTTABLEMODEL_H

#include <QtSql>
#include <QItemDelegate>
#include <QtCore>
#include "trackmodel.h"
#include "library/basesqltablemodel.h"
#include "library/librarytablemodel.h"
#include "library/dao/playlistdao.h"
#include "library/dao/trackdao.h"

class TrackCollection;

class PlaylistTableModel : public BaseSqlTableModel, public virtual TrackModel
{
    Q_OBJECT
  public:
    PlaylistTableModel(QObject* parent, TrackCollection* pTrackCollection);
    virtual ~PlaylistTableModel();
    void setPlaylist(int playlistId);
    virtual TrackPointer getTrack(const QModelIndex& index) const;
    virtual QString getTrackLocation(const QModelIndex& index) const;
    virtual void search(const QString& searchText);
    virtual const QString currentSearch();
    virtual bool isColumnInternal(int column);
    virtual bool isColumnHiddenByDefault(int column);
    virtual void removeTrack(const QModelIndex& index);
    virtual void removeTracks(const QModelIndexList& indices);
    virtual bool addTrack(const QModelIndex& index, QString location);
    virtual void moveTrack(const QModelIndex& sourceIndex, const QModelIndex& destIndex);
    
    QMimeData* mimeData(const QModelIndexList &indexes) const;
   
    QItemDelegate* delegateForColumn(const int i);
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
    QString m_currentSearch;
};

#endif
