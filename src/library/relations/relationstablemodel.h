#pragma once

#include "library/trackset/tracksettablemodel.h"

class RelationsTableModel final : public TrackSetTableModel {
    Q_OBJECT

  public:
    RelationsTableModel(QObject* parent,
            TrackCollectionManager* pTrackCollectionManager);
    ~RelationsTableModel() final = default;

    void displayRelatedTracks(TrackPointer pTrack);
    void displayAllRelations();
    TrackPointer selectedTrack() const {
        return m_pTrack;
    }

    Capabilities getCapabilities() const final;
    QString modelKey(bool noSearch) const override;

  private:
    TrackPointer m_pTrack;
};
