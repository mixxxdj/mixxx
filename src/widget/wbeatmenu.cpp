#include "wbeatmenu.h"

#include "preferences/beatgridmode.h"

namespace {
constexpr int kDefaultBeatsPerMeasure = 4;
}

WBeatMenu::WBeatMenu(UserSettingsPointer pConfig, QWidget* parent)
        : QMenu(parent), m_pConfig(pConfig) {
}

void WBeatMenu::update() {
    BeatGridMode beatMode = BeatGridMode(m_pConfig->getValue(
            ConfigKey("[Waveform]", "beatGridMode"), (int)BeatGridMode::BEATS));
    clear();
    if (beatMode == BeatGridMode::BEATS_DOWNBEATS) {
        m_pSetAsDownbeat = make_parented<QAction>(tr("Set as Downbeat"), this);
        connect(m_pSetAsDownbeat, &QAction::triggered, this, &WBeatMenu::slotDownbeatUpdated);
        addAction(m_pSetAsDownbeat);
    }
}

void WBeatMenu::slotDownbeatUpdated() {
    if (m_pBeats) {
        m_pBeats->setDownbeatStartIndex(
                m_beat.getIndex() % kDefaultBeatsPerMeasure);
    }
}
