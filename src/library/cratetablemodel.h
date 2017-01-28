// cratetablemodel.h
// Created 10/25/2009 by RJ Ryan (rryan@mit.edu)

#ifndef CRATETABLEMODEL_H
#define CRATETABLEMODEL_H

#include "library/basesqltablemodel.h"
#include "library/dao/cratedao.h"

class CrateTableModel : public BaseSqlTableModel {
    Q_OBJECT
  public:
    CrateTableModel(QObject* parent, TrackCollection* pTrackCollection);
    ~CrateTableModel() final;

    void setTableModel(int crateId=-1);
    int getCrate() const {
        return m_iCrateId;
    }

    bool addTrack(const QModelIndex &index, QString location);

    // From TrackModel
    bool isColumnInternal(int column) final;
    void removeTracks(const QModelIndexList& indices) final;
    // Returns the number of unsuccessful track additions
    int addTracks(const QModelIndex& index, const QList<QString>& locations) final;
    CapabilitiesFlags getCapabilities() const final;

  private:
    int m_iCrateId;
};

#endif /* CRATETABLEMODEL_H */
