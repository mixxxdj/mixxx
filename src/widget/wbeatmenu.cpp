#include "wbeatmenu.h"

#include "preferences/beatgridmode.h"

namespace {
constexpr int kDefaultBeatsPerMeasure = 4;
}

WBeatMenu::WBeatMenu(UserSettingsPointer pConfig, QWidget* parent)
        : QMenu(parent), m_pConfig(pConfig) {
}

void WBeatMenu::update() {
    clear();
    if (m_eSelectedOptions.testFlag(WBeatMenu::Option::SetDownbeat) &&
            m_beat.getType() == mixxx::track::io::BEAT) {
        m_pSetAsDownbeat = make_parented<QAction>(tr("Set as Downbeat"), this);
        connect(m_pSetAsDownbeat, &QAction::triggered, this, &WBeatMenu::slotDownbeatUpdated);
        addAction(m_pSetAsDownbeat);
    }
    if (m_eSelectedOptions.testFlag(WBeatMenu::Option::CueMenu)) {
        m_pCueMenu = make_parented<QAction>(tr("Cue Menu"), this);
        addAction(m_pCueMenu);
        connect(m_pCueMenu, &QAction::triggered, this, &WBeatMenu::cueButtonClicked);
    }
}

void WBeatMenu::slotDownbeatUpdated() {
    if (m_pBeats) {
        m_pBeats->setDownbeatStartIndex(
                m_beat.getIndex() % kDefaultBeatsPerMeasure);
    }
}

void WBeatMenu::addOptions(Options newOptions) {
    setOptions(m_eSelectedOptions | newOptions);
}

void WBeatMenu::removeOptions(Options removeOptions) {
    setOptions(WBeatMenu::Options(
            m_eSelectedOptions - (m_eSelectedOptions & removeOptions)));
}
