#ifndef ITUNES_TABLE_MODEL_H
#define ITUNES_TABLE_MODEL_H

#include <QtSql>
#include <QItemDelegate>
#include <QtCore>
#include "library/trackmodel.h"
#include "library/basesqltablemodel.h"
#include "library/librarytablemodel.h"
#include "library/dao/playlistdao.h"
#include "library/dao/trackdao.h"

class TrackCollection;

class ITunesTrackModel : public BaseSqlTableModel {
    Q_OBJECT
  public:
    ITunesTrackModel(QObject* parent, TrackCollection* pTrackCollection);
    TrackModel::CapabilitiesFlags getCapabilities() const;
    virtual ~ITunesTrackModel();

    virtual TrackPointer getTrack(const QModelIndex& index) const;
    virtual void search(const QString& searchText);
    virtual bool isColumnInternal(int column);
    virtual bool isColumnHiddenByDefault(int column);
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;

  private slots:
    void slotSearch(const QString& searchText);

  signals:
    void doSearch(const QString& searchText);

  private:
    TrackCollection* m_pTrackCollection;
    QSqlDatabase &m_database;
};

#endif /* ITUNES_TABLE_MODEL_H */
