#include "wbeatmenu.h"

WBeatMenu::WBeatMenu(UserSettingsPointer pConfig, QWidget* parent)
        : QMenu(parent) {
    m_pSetAsDownbeat = new QAction(tr("Set as Downbeat"), this);
    connect(m_pSetAsDownbeat, &QAction::triggered, this, &WBeatMenu::updateDownbeat);
    addAction(m_pSetAsDownbeat);
}

WBeatMenu::~WBeatMenu() {
}
