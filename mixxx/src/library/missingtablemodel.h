#ifndef MISSINGTABLEMODEL_H
#define MISSINGTABLEMODEL_H

#include <QtSql>
#include <QItemDelegate>
#include <QtCore>

#include "trackmodel.h"
#include "library/dao/trackdao.h"
#include "library/basesqltablemodel.h"

class TrackCollection;

class MissingTableModel : public BaseSqlTableModel {
    Q_OBJECT
  public:
    MissingTableModel(QObject* parent, TrackCollection* pTrackCollection);
    virtual ~MissingTableModel();
    virtual TrackPointer getTrack(const QModelIndex& index) const;
    virtual void search(const QString& searchText);
    virtual bool isColumnInternal(int column);
    virtual bool isColumnHiddenByDefault(int column);
    virtual void removeTrack(const QModelIndex& index);
    virtual void removeTracks(const QModelIndexList& indices);
    virtual bool addTrack(const QModelIndex& index, QString location);
    virtual void moveTrack(const QModelIndex& sourceIndex, const QModelIndex& destIndex);

    Qt::ItemFlags flags(const QModelIndex &index) const;
    QItemDelegate* delegateForColumn(const int i);
    TrackModel::CapabilitiesFlags getCapabilities() const;

  private slots:
    void slotSearch(const QString& searchText);

  signals:
    void doSearch(const QString& searchText);

  private:
    TrackCollection* m_pTrackCollection;
    TrackDAO& m_trackDao;
    static const QString MISSINGFILTER;
};

#endif
