#pragma once

#include "library/trackset/smarties/smartiesid.h"
#include "library/trackset/smarties/smartiesstorage.h"
#include "library/trackset/tracksettablemodel.h"

class SmartiesTableModel final : public TrackSetTableModel {
    Q_OBJECT

  public:
    SmartiesTableModel(
            QObject* parent,
            TrackCollectionManager* pTrackCollectionManager);
    ~SmartiesTableModel() final = default;

    void selectSmarties(SmartiesId smartiesId = SmartiesId());
    SmartiesId selectedSmarties() const {
        return m_selectedSmarties;
    }

    void selectSmarties2QVL(SmartiesId smartiesId, QVariantList& smartiesData);
    //    void selectSmarties2QVL(SmartiesId smartiesId, const QVariantList& smartiesData);
    void saveQVL2Smarties(SmartiesId smartiesId, const QVariantList& smartiesData);
    //    void getNextSmartiesId(const QString& currentSmartiesId);
    //    void getPreviousSmartiesId(const QString& currentSmartiesId);

    bool executeSegmentedUpdate(const QString& queryStr, const QString& label);

    QString buildCoreUpdateQuery(const QVariantList& smartiesData);
    QString buildConditionUpdateQuery(const QVariantList& smartiesData, int startIdx, int endIdx);

    void getWhereClauseForSmarties(SmartiesId smartiesId);
    QString buildWhereClause(const QVariantList& smartiesData);

    QVariant getPreviousRecordId(SmartiesId currentId);
    QVariant getNextRecordId(SmartiesId currentId);
    //    bool saveSmartiesData(const QVariantList& smartiesData, int smartiesId);
    //    bool addTrack(const QModelIndex& index, const QString& location);

    //    void removeTracks(const QModelIndexList& indices) final;
    /// Returns the number of unsuccessful additions.
    //    int addTracksWithTrackIds(const QModelIndex& index,
    //            const QList<TrackId>& tracks,
    //            int* pOutInsertionPos) final;
    //    bool isLocked() final;
    QVariantList smartiesData;

    Capabilities getCapabilities() const final;
    QString modelKey(bool noSearch) const override;

  private:
    SmartiesId m_selectedSmarties;
    QHash<SmartiesId, QString> m_searchTexts;
};
