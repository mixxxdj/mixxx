#include "util/screensavermanager.h"

#include "mixer/playerinfo.h"
#include "moc_screensavermanager.cpp"

namespace mixxx {

ScreensaverManager::ScreensaverManager(UserSettingsPointer pConfig, QObject* pParent)
        : QObject(pParent), m_pConfig(pConfig) {
    m_inhibitScreensaver =
            pConfig->getValue(ConfigKey("[Config]", "InhibitScreensaver"),
                    mixxx::preferences::ScreenSaver::On);
    pConfig->setValue(ConfigKey("[Config]", "InhibitScreensaver"), m_inhibitScreensaver);
    if (m_inhibitScreensaver == mixxx::preferences::ScreenSaver::On) {
        mixxx::ScreenSaverHelper::inhibit();
    }
}

ScreensaverManager::~ScreensaverManager() {
    if (m_inhibitScreensaver != mixxx::preferences::ScreenSaver::Off) {
        mixxx::ScreenSaverHelper::uninhibit();
    }
}

void ScreensaverManager::setStatus(mixxx::preferences::ScreenSaver newInhibit) {
    if (m_inhibitScreensaver != mixxx::preferences::ScreenSaver::Off) {
        mixxx::ScreenSaverHelper::uninhibit();
    }

    if (newInhibit == mixxx::preferences::ScreenSaver::On ||
            (newInhibit == mixxx::preferences::ScreenSaver::OnPlay &&
                    PlayerInfo::instance().getCurrentPlayingDeck() != -1)) {
        mixxx::ScreenSaverHelper::inhibit();
    }
    m_pConfig->setValue(ConfigKey("[Config]", "InhibitScreensaver"), newInhibit);
    m_inhibitScreensaver = newInhibit;
}

void ScreensaverManager::slotCurrentPlayingDeckChanged(int deck) {
    if (m_inhibitScreensaver != mixxx::preferences::ScreenSaver::OnPlay) {
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
