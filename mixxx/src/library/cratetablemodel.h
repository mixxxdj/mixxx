// cratetablemodel.h
// Created 10/25/2009 by RJ Ryan (rryan@mit.edu)

#ifndef CRATETABLEMODEL_H
#define CRATETABLEMODEL_H

#include <QItemDelegate>
#include <QSqlTableModel>

#include "library/basesqltablemodel.h"
#include "library/trackmodel.h"

class TrackCollection;

class CrateTableModel : public BaseSqlTableModel, public virtual TrackModel {
    Q_OBJECT
  public:
    CrateTableModel(QObject* parent, TrackCollection* pTrackCollection);
    virtual ~CrateTableModel();

    void setCrate(int crateId);

    virtual QVariant data(const QModelIndex& item, int role) const;
    QMimeData* mimeData(const QModelIndexList &indexes) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;

    // From TrackModel
    virtual TrackInfoObject* getTrack(const QModelIndex& index) const;
    virtual QString getTrackLocation(const QModelIndex& index) const;
    virtual void search(const QString& searchText);
    virtual const QString currentSearch();
    virtual bool isColumnInternal(int column);
    virtual void removeTrack(const QModelIndex& index);
    virtual void addTrack(const QModelIndex& index, QString location);
    virtual void moveTrack(const QModelIndex& sourceIndex,
                           const QModelIndex& destIndex);
    TrackModel::CapabilitiesFlags getCapabilities() const;
    virtual QItemDelegate* delegateForColumn(const int i);

  private slots:
    void slotSearch(const QString& searchText);
  signals:
    void doSearch(const QString& searchText);

  private:
    TrackCollection* m_pTrackCollection;
    int m_iCrateId;
    QString m_currentSearch;
};


#endif /* CRATETABLEMODEL_H */
