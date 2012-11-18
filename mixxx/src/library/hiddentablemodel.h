#ifndef HIDDENTABLEMODEL_H
#define HIDDENTABLEMODEL_H

#include <QtSql>
#include <QItemDelegate>
#include <QtCore>

#include "trackmodel.h"
#include "library/dao/trackdao.h"
#include "library/basesqltablemodel.h"

class TrackCollection;

class HiddenTableModel : public BaseSqlTableModel {
    Q_OBJECT
  public:
    HiddenTableModel(QObject* parent, TrackCollection* pTrackCollection);
    virtual ~HiddenTableModel();
    virtual TrackPointer getTrack(const QModelIndex& index) const;
    virtual void search(const QString& searchText);
    virtual bool isColumnInternal(int column);
    virtual bool isColumnHiddenByDefault(int column);
    virtual void purgeTracks(const QModelIndexList& indices);
    virtual void unhideTracks(const QModelIndexList& indices);
    virtual bool addTrack(const QModelIndex& index, QString location);

    Qt::ItemFlags flags(const QModelIndex &index) const;
    TrackModel::CapabilitiesFlags getCapabilities() const;

  private slots:
    void slotSearch(const QString& searchText);

  signals:
    void doSearch(const QString& searchText);

  private:
    TrackCollection* m_pTrackCollection;
    TrackDAO& m_trackDao;
};

#endif
