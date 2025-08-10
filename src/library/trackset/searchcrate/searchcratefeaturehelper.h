#pragma once

#include <QObject>

#include "library/trackset/searchcrate/dlggroupedsearchcratesinfo.h"
#include "library/trackset/searchcrate/dlgsearchcrateinfo.h"
#include "library/trackset/searchcrate/searchcrateid.h"
#include "preferences/usersettings.h"

class TrackCollection;
class SearchCrate;
class BaseTrackSetFeature;

class SearchCrateFeatureHelper : public QObject {
    Q_OBJECT

  public:
    SearchCrateFeatureHelper(
            TrackCollection* pTrackCollection,
            UserSettingsPointer pConfig);
    ~SearchCrateFeatureHelper() override = default;

    SearchCrateId createEmptySearchCrate();
    SearchCrateId createEmptySearchCrateFromUI();
    SearchCrateId createEmptySearchCrateFromSearch(const QString& text);
    SearchCrateId duplicateSearchCrate(const SearchCrate& oldSearchCrate);

  private:
    QString proposeNameForNewSearchCrate(
            const QString& initialName = QString()) const;

    TrackCollection* m_pTrackCollection;

    UserSettingsPointer m_pConfig;
};
