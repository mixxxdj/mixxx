#ifndef TRAKTOR_TABLE_MODEL_H
#define TRAKTOR_TABLE_MODEL_H

#include <QtSql>
#include <QItemDelegate>
#include <QtCore>
#include "trackmodel.h"
#include "library/basesqltablemodel.h"
#include "library/librarytablemodel.h"
#include "library/dao/playlistdao.h"
#include "library/dao/trackdao.h"

class TrackCollection;

class TraktorTableModel : public BaseSqlTableModel, public virtual TrackModel
{
    Q_OBJECT
  public:
    TraktorTableModel(QObject* parent, TrackCollection* pTrackCollection);
    virtual ~TraktorTableModel();
    
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
	
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    QMimeData* mimeData(const QModelIndexList &indexes) const;
   
    QItemDelegate* delegateForColumn(const int i);
    TrackModel::CapabilitiesFlags getCapabilities() const;

  private slots:
    void slotSearch(const QString& searchText);

  signals:
    void doSearch(const QString& searchText);

  private:
    TrackCollection* m_pTrackCollection;
	QSqlDatabase &m_database;
    
    
    QString m_currentSearch;
};

#endif /* TRAKTOR_TABLE_MODEL_H */
