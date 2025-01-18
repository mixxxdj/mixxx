#pragma once

#include <QObject>

#include "library/trackset/smarties/smartiesid.h"
#include "preferences/usersettings.h"

class TrackCollection;
class Smarties;

class SmartiesFeatureHelper : public QObject {
    Q_OBJECT

  public:
    SmartiesFeatureHelper(
            TrackCollection* pTrackCollection,
            UserSettingsPointer pConfig);
    ~SmartiesFeatureHelper() override = default;

    SmartiesId createEmptySmarties();
    SmartiesId createEmptySmartiesFromUI();
    SmartiesId createEmptySmartiesFromSearch(const QString& text);
    SmartiesId duplicateSmarties(const Smarties& oldSmarties);

  private:
    QString proposeNameForNewSmarties(
            const QString& initialName = QString()) const;

    TrackCollection* m_pTrackCollection;

    UserSettingsPointer m_pConfig;
};
