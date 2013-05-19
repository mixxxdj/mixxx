#ifndef HIDDENTABLEMODEL_H
#define HIDDENTABLEMODEL_H

#include "library/basesqltablemodel.h"

class TrackCollection;

class HiddenTableModel : public BaseSqlTableModel {
    Q_OBJECT
  public:
    HiddenTableModel(QObject* parent, TrackCollection* pTrackCollection);
    virtual ~HiddenTableModel();
    TrackPointer getTrack(const QModelIndex& index) const;
    void search(const QString& searchText);
    bool isColumnInternal(int column);
    bool isColumnHiddenByDefault(int column);
    void purgeTracks(const QModelIndexList& indices);
    void unhideTracks(const QModelIndexList& indices);
    bool addTrack(const QModelIndex& index, QString location);

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
