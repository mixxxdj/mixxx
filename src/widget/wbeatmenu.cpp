#include "wbeatmenu.h"

WBeatMenu::WBeatMenu(UserSettingsPointer pConfig, QWidget* parent)
        : QMenu(parent), m_pConfig(pConfig) {
    update();
}

WBeatMenu::~WBeatMenu() {
}

void WBeatMenu::update() {
    int beatMode = m_pConfig->getValue(ConfigKey("[Waveform]", "beatGridMode"), 0);
    clear();
    if (beatMode == 1) {
        m_pSetAsDownbeat = new QAction(tr("Set as Downbeat"), this);
        connect(m_pSetAsDownbeat, &QAction::triggered, this, &WBeatMenu::updateDownbeat);
        addAction(m_pSetAsDownbeat);
    }
}
