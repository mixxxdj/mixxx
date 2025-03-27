#pragma once

#include "library/trackset/tracksettablemodel.h"

class RelationsTableModel final : public TrackSetTableModel {
    Q_OBJECT

  public:
    RelationsTableModel(QObject* parent,
            TrackCollectionManager* pTrackCollectionManager);
    ~RelationsTableModel() final = default;

    void displayTrackTargets(TrackPointer pTrack);
    TrackPointer selectedTrack() const {
        return m_pTrack;
    }

    Capabilities getCapabilities() const final;
    QString modelKey(bool noSearch) const override;

  private:
    TrackPointer m_pTrack;
};
