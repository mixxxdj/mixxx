#pragma once

#include "library/basesqltablemodel.h"
#include "library/crate/crateid.h"
#include "library/tracksettablemodel.h"

class CrateTableModel : public TrackSetTableModel {
    Q_OBJECT

  public:
    CrateTableModel(QObject* parent, TrackCollectionManager* pTrackCollectionManager);
    ~CrateTableModel() final = default;

    void selectCrate(CrateId crateId = CrateId());
    CrateId selectedCrate() const {
        return m_selectedCrate;
    }

    bool addTrack(const QModelIndex& index, QString location);

    void removeTracks(const QModelIndexList& indices) final;
    /// Returns the number of unsuccessful additions.
    int addTracks(const QModelIndex& index, const QList<QString>& locations) final;

    CapabilitiesFlags getCapabilities() const final;

  private:
    CrateId m_selectedCrate;
};
