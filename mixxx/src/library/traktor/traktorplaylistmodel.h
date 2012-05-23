#ifndef TRAKTOR_PLAYLIST_MODEL_H
#define TRAKTOR_PLAYLIST_MODEL_H

#include <QtSql>
#include <QItemDelegate>
#include <QtCore>
#include "library/trackmodel.h"
#include "library/basesqltablemodel.h"
#include "library/librarytablemodel.h"
#include "library/dao/playlistdao.h"
#include "library/dao/trackdao.h"

class TrackCollection;

class TraktorPlaylistModel : public BaseSqlTableModel {
    Q_OBJECT
  public:
    TraktorPlaylistModel(QObject* parent, TrackCollection* pTrackCollection);
    TrackModel::CapabilitiesFlags getCapabilities() const;
    virtual ~TraktorPlaylistModel();

    virtual TrackPointer getTrack(const QModelIndex& index) const;
    virtual void search(const QString& searchText);
    virtual bool isColumnInternal(int column);
    virtual bool isColumnHiddenByDefault(int column);
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    void setPlaylist(QString path_name);
    QAbstractItemDelegate* delegateForColumn(const int i, QObject* pParent);

  private slots:
    void slotSearch(const QString& searchText);

  signals:
    void doSearch(const QString& searchText);

  private:
    TrackCollection* m_pTrackCollection;
    QSqlDatabase &m_database;
};

#endif /* TRAKTOR_TABLE_MODEL_H */
