#include "util/screensavermanager.h"

#include "mixer/playerinfo.h"
#include "moc_screensavermanager.cpp"

namespace mixxx {

ScreensaverManager::ScreensaverManager(UserSettingsPointer pConfig, QObject* pParent)
        : QObject(pParent), m_pConfig(pConfig) {
    m_inhibitScreensaver =
            pConfig->getValue(ConfigKey("[Config]", "InhibitScreensaver"),
                    mixxx::preferences::constants::ScreenSaver::On);
    pConfig->setValue(ConfigKey("[Config]", "InhibitScreensaver"), m_inhibitScreensaver);
    if (m_inhibitScreensaver == mixxx::preferences::constants::ScreenSaver::On) {
        mixxx::ScreenSaverHelper::inhibit();
    }
}

ScreensaverManager::~ScreensaverManager() {
    if (m_inhibitScreensaver != mixxx::preferences::constants::ScreenSaver::Off) {
        mixxx::ScreenSaverHelper::uninhibit();
    }
}

void ScreensaverManager::setStatus(mixxx::preferences::constants::ScreenSaver newInhibit) {
    if (m_inhibitScreensaver != mixxx::preferences::constants::ScreenSaver::Off) {
        mixxx::ScreenSaverHelper::uninhibit();
    }

    if (newInhibit == mixxx::preferences::constants::ScreenSaver::On ||
            (newInhibit == mixxx::preferences::constants::ScreenSaver::OnPlay &&
                    PlayerInfo::instance().getCurrentPlayingDeck() != -1)) {
        mixxx::ScreenSaverHelper::inhibit();
    }
    m_pConfig->setValue(ConfigKey("[Config]", "InhibitScreensaver"), newInhibit);
    m_inhibitScreensaver = newInhibit;
}

void ScreensaverManager::slotCurrentPlayingDeckChanged(int deck) {
    if (m_inhibitScreensaver != mixxx::preferences::constants::ScreenSaver::OnPlay) {
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
