#pragma once

#include "library/trackset/searchcrate/searchcrateid.h"
#include "library/trackset/searchcrate/searchcratestorage.h"
#include "library/trackset/tracksettablemodel.h"

class TrackCollection;

class SearchCrateTableModel final : public TrackSetTableModel {
    Q_OBJECT

  public:
    SearchCrateTableModel(
            QObject* parent,
            TrackCollectionManager* pTrackCollectionManager,
            TrackCollection* pTrackCollection,
            UserSettingsPointer pConfig);
    ~SearchCrateTableModel() final = default;

    void selectSearchCrate(SearchCrateId searchCrateId = SearchCrateId());
    SearchCrateId selectedSearchCrate() const {
        return m_selectedSearchCrate;
    }

    void selectSearchCrateGroup(const QString& groupName);
    QList<QVariantMap> getGroupedSearchCrates();

    void loadSearchCrateList();

    void selectPlaylistsCrates2QVL(QVariantList& playlistsCratesData);

    void selectSearchCrate2QVL(SearchCrateId searchCrateId, QVariantList& searchCrateData);
    void saveQVL2SearchCrate(SearchCrateId searchCrateId, const QVariantList& searchCrateData);

    QString buildConditionUpdateQuery(
            const QVariantList& searchCrateData, int startIdx, int endIdx);

    void getWhereClauseForSearchCrate(SearchCrateId searchCrateId);
    QString buildWhereClause(const QVariantList& searchCrateData);

    QVariant getPreviousRecordId(SearchCrateId currentId);
    QVariant getNextRecordId(SearchCrateId currentId);
    QVariantList searchCrateData;

    Capabilities getCapabilities() const final;
    QString modelKey(bool noSearch) const override;

  private:
    QList<QVariantList> m_searchCrateList;
    SearchCrateId m_selectedSearchCrate;
    QHash<SearchCrateId, QString> m_searchTexts;
    TrackCollection* m_pTrackCollection;

    UserSettingsPointer m_pConfig;
};
