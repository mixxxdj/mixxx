#include "util/screensavermanager.h"

#include "mixer/playerinfo.h"

namespace mixxx {

ScreensaverManager::ScreensaverManager(UserSettingsPointer pConfig, QObject* pParent)
        : QObject(pParent), m_pConfig(pConfig) {
    int inhibit = pConfig->getValue<int>(ConfigKey("[Config]", "InhibitScreensaver"), -1);
    if (inhibit == -1) {
        inhibit = static_cast<int>(mixxx::ScreenSaverPreference::PREVENT_ON);
        pConfig->setValue<int>(ConfigKey("[Config]", "InhibitScreensaver"), inhibit);
    }
    m_inhibitScreensaver = static_cast<mixxx::ScreenSaverPreference>(inhibit);
    if (m_inhibitScreensaver == mixxx::ScreenSaverPreference::PREVENT_ON) {
        mixxx::ScreenSaverHelper::inhibit();
    }
}

ScreensaverManager::~ScreensaverManager() {
    if (m_inhibitScreensaver != mixxx::ScreenSaverPreference::PREVENT_OFF) {
        mixxx::ScreenSaverHelper::uninhibit();
    }
}

void ScreensaverManager::setStatus(mixxx::ScreenSaverPreference newInhibit) {
    if (m_inhibitScreensaver != mixxx::ScreenSaverPreference::PREVENT_OFF) {
        mixxx::ScreenSaverHelper::uninhibit();
    }

    if (newInhibit == mixxx::ScreenSaverPreference::PREVENT_ON) {
        mixxx::ScreenSaverHelper::inhibit();
    } else if (newInhibit ==
                    mixxx::ScreenSaverPreference::PREVENT_ON_PLAY &&
            PlayerInfo::instance().getCurrentPlayingDeck() != -1) {
        mixxx::ScreenSaverHelper::inhibit();
    }
    int inhibit_int = static_cast<int>(newInhibit);
    m_pConfig->setValue<int>(ConfigKey("[Config]", "InhibitScreensaver"), inhibit_int);
    m_inhibitScreensaver = newInhibit;
}

void ScreensaverManager::slotCurrentPlayingDeckChanged(int deck) {
    if (m_inhibitScreensaver != mixxx::ScreenSaverPreference::PREVENT_ON_PLAY) {
        return;
    }
    if (deck == -1) {
        // If no deck is playing, allow the screensaver to run.
        mixxx::ScreenSaverHelper::uninhibit();
    } else {
        mixxx::ScreenSaverHelper::inhibit();
    }
}

} // namespace mixxx
