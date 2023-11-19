#pragma once

#include <QObject>

#include "library/trackset/crate/crateid.h"
#include "preferences/usersettings.h"

class TrackCollection;
class Crate;

class CrateFeatureHelper : public QObject {
    Q_OBJECT

  public:
    CrateFeatureHelper(
            TrackCollection* pTrackCollection,
            UserSettingsPointer pConfig);
    ~CrateFeatureHelper() override = default;

    CrateId createEmptyCrate();
    CrateId duplicateCrate(const Crate& oldCrate);

  private:
    QString proposeNameForNewCrate(
            const QString& initialName = QString()) const;

    TrackCollection* m_pTrackCollection;

    UserSettingsPointer m_pConfig;
};
