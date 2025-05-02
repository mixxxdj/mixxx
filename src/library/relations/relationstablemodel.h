#pragma once

#include "library/basesqltablemodel.h"

class Relation;

class RelationsTableModel final : public BaseSqlTableModel {
    Q_OBJECT

  public:
    RelationsTableModel(QObject* parent,
            TrackCollectionManager* pTrackCollectionManager);
    ~RelationsTableModel() final = default;

    bool isColumnInternal(int column) override;
    bool isColumnHiddenByDefault(int column) override;

    void showRelatedTracks(TrackPointer pTrack);

    Capabilities getCapabilities() const final;
    QString modelKey(bool noSearch) const override;

  public slots:
    void showAllRelations();

  private:
    TrackPointer m_pDeckTrack;
};
