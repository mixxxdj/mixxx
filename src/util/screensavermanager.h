#pragma once
#include <QObject>

#include "preferences/constants.h"
#include "preferences/usersettings.h"
#include "util/screensaver.h"

namespace mixxx {

class ScreensaverManager : public QObject {
    Q_OBJECT
  public:
    ScreensaverManager(UserSettingsPointer pConfig, QObject* pParent = nullptr);
    ~ScreensaverManager();

    void setStatus(mixxx::ScreenSaverPreference status);

    mixxx::ScreenSaverPreference status() {
        return m_inhibitScreensaver;
    }
  public slots:
    void slotCurrentPlayingDeckChanged(int deck);

  private:
    const UserSettingsPointer m_pConfig;
    mixxx::ScreenSaverPreference m_inhibitScreensaver;
};

} // namespace mixxx
