#ifndef MIXXX_CRATEFEATUREHELPER_H
#define MIXXX_CRATEFEATUREHELPER_H


#include <QObject>

#include "library/crate/crate.h"
#include "preferences/usersettings.h"

#include "library/features/crates/cratemanager.h"

// forward declaration(s)
class TrackCollection;

class CrateFeatureHelper: public QObject {
    Q_OBJECT
  public:
    CrateFeatureHelper(
            CrateManager* pCrates,
            UserSettingsPointer pConfig);
    ~CrateFeatureHelper() override {}

    CrateId createEmptyCrate(const Crate& parent = Crate());
    CrateId duplicateCrate(const Crate& oldCrate);

  private:
    QString proposeNameForNewCrate(
            const QString& initialName = QString()) const;

    CrateManager* m_pCrates;

    UserSettingsPointer m_pConfig;
};


#endif // MIXXX_CRATEFEATUREHELPER_H
