#pragma once

#include "library/trackset/crate/crateid.h"
#include "library/trackset/tracksettablemodel.h"

constexpr int kInvalidCrateId = -1;

class CrateTableModel final : public TrackSetTableModel {
    Q_OBJECT

  public:
    CrateTableModel(
            QObject* parent,
            TrackCollectionManager* pTrackCollectionManager,
            UserSettingsPointer pConfig);
    ~CrateTableModel() final = default;

    void selectCrate(CrateId crateId = CrateId());
    CrateId selectedCrate() const {
        return m_selectedCrate;
    }

    void selectCrateGroup(const QString& groupName);
    QList<QVariantMap> getGroupedCrates();
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
    CrateId m_selectedCrate;
    QHash<CrateId, QString> m_searchTexts;
    UserSettingsPointer m_pConfig;
};
