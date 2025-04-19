#pragma once

#include "library/basesqltablemodel.h"

class RelationsTableModel final : public BaseSqlTableModel {
    Q_OBJECT

  public:
    RelationsTableModel(QObject* parent,
            TrackCollectionManager* pTrackCollectionManager);
    ~RelationsTableModel() final = default;

    bool isColumnInternal(int column) override;

    void showRelatedTracks(TrackPointer pTrack);

    TrackPointer selectedTrack() const {
        return m_pTrack;
    }

    Capabilities getCapabilities() const final;
    QString modelKey(bool noSearch) const override;

  public slots:
    void showAllRelations();

  private:
    TrackPointer m_pTrack;
};
