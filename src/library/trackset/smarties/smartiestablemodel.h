#pragma once

#include "library/trackset/smarties/smartiesid.h"
#include "library/trackset/tracksettablemodel.h"

class SmartiesTableModel final : public TrackSetTableModel {
    Q_OBJECT

  public:
    SmartiesTableModel(QObject* parent, TrackCollectionManager* pTrackCollectionManager);
    ~SmartiesTableModel() final = default;

    void selectSmarties(SmartiesId smartiesId = SmartiesId());
    SmartiesId selectedSmarties() const {
        return m_selectedSmarties;
    }

    bool addTrack(const QModelIndex& index, const QString& location);

    void removeTracks(const QModelIndexList& indices) final;
    /// Returns the number of unsuccessful additions.
    int addTracksWithTrackIds(const QModelIndex& index,
            const QList<TrackId>& tracks,
            int* pOutInsertionPos) final;
    bool isLocked() final;

    Capabilities getCapabilities() const final;
    QString modelKey(bool noSearch) const override;

  private:
    SmartiesId m_selectedSmarties;
    QHash<SmartiesId, QString> m_searchTexts;
};
