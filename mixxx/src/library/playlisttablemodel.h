#ifndef PLAYLISTTABLEMODEL_H
#define PLAYLISTTABLEMODEL_H

#include <QtSql>
#include <QItemDelegate>
#include <QtCore>
#include "trackmodel.h"
#include "library/dao/playlistdao.h"
#include "library/dao/trackdao.h"

class TrackCollection;

class PlaylistTableModel : public QSqlTableModel, public virtual TrackModel
{
public:
    PlaylistTableModel(QObject* parent, TrackCollection* pTrackCollection);
    virtual ~PlaylistTableModel();
    void setPlaylist(int playlistId);
    virtual TrackInfoObject* getTrack(const QModelIndex& index) const;
    virtual QString getTrackLocation(const QModelIndex& index) const;
    virtual void search(const QString& searchText);
    virtual const QString currentSearch();
    virtual bool isColumnInternal(int column);
    virtual void removeTrack(const QModelIndex& index);
    virtual void addTrack(const QModelIndex& index, QString location);
    virtual void moveTrack(const QModelIndex& sourceIndex, const QModelIndex& destIndex);
    virtual QVariant data(const QModelIndex& item, int role) const;
    QMimeData* mimeData(const QModelIndexList &indexes) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QItemDelegate* delegateForColumn(int i);
    TrackModel::CapabilitiesFlags getCapabilities() const;
private:
    TrackCollection* m_pTrackCollection;
    PlaylistDAO& m_playlistDao;
    TrackDAO& m_trackDao;
    int m_iPlaylistId;
    QString m_currentSearch;
};

#endif
