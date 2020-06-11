#include "wbeatmenu.h"

#include "preferences/beatgridmode.h"

WBeatMenu::WBeatMenu(UserSettingsPointer pConfig, QWidget* parent)
        : QMenu(parent), m_pConfig(pConfig) {
}

void WBeatMenu::update() {
    // TODO(hacksdump): Declare enum type for beat grid mode setting.
    BeatGridMode beatMode = BeatGridMode(m_pConfig->getValue(
            ConfigKey("[Waveform]", "beatGridMode"), (int)BeatGridMode::BEATS));
    clear();
    if (beatMode == BeatGridMode::BEATS_DOWNBEATS) {
        m_pSetAsDownbeat = make_parented<QAction>(tr("Set as Downbeat"), this);
        connect(m_pSetAsDownbeat, &QAction::triggered, this, &WBeatMenu::updateDownbeat);
        addAction(m_pSetAsDownbeat);
    }
}
