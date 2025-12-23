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

    void setStatus(mixxx::preferences::ScreenSaver status);

    mixxx::preferences::ScreenSaver status() {
        return m_inhibitScreensaver;
    }
  public slots:
    void slotCurrentPlayingDeckChanged(int deck);

  private:
    const UserSettingsPointer m_pConfig;
    mixxx::preferences::ScreenSaver m_inhibitScreensaver;
};

} // namespace mixxx
