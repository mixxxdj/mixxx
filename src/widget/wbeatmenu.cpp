#include "wbeatmenu.h"

WBeatMenu::WBeatMenu(UserSettingsPointer pConfig, QWidget* parent)
        : QMenu(parent),
          m_pConfig(pConfig),
          m_pTimeSignatureMenu(make_parented<WTimeSignatureMenu>(this)),
          m_beat(mixxx::kInvalidFramePos) {
}

void WBeatMenu::updateMenu() {
    clear();
    if (m_eSelectedOptions.testFlag(WBeatMenu::Option::SetDownbeat) &&
            m_beat.getType() == mixxx::Beat::BEAT) {
        m_pSetAsDownbeat = make_parented<QAction>(tr("Set as Downbeat"), this);
        connect(m_pSetAsDownbeat,
                &QAction::triggered,
                this,
                &WBeatMenu::slotDownbeatUpdated);
        addAction(m_pSetAsDownbeat);
    }
    if (m_eSelectedOptions.testFlag(WBeatMenu::Option::CueMenu)) {
        m_pCueMenu = make_parented<QAction>(tr("Edit Cue"), this);
        addAction(m_pCueMenu);
        connect(m_pCueMenu,
                &QAction::triggered,
                this,
                &WBeatMenu::cueButtonClicked);
    }
    if (m_beat.getType() == mixxx::Beat::DOWNBEAT) {
        m_pTimeSignatureAction =
                make_parented<QAction>(tr("Edit Time Signature"), this);
        addAction(m_pTimeSignatureAction);
        connect(m_pTimeSignatureAction,
                &QAction::triggered,
                this,
                &WBeatMenu::slotDisplayTimeSignatureMenu);
    }
}

void WBeatMenu::slotDownbeatUpdated() {
    if (m_pBeats) {
        m_pBeats->setAsDownbeat(m_beat.getBeatIndex());
    }
}

void WBeatMenu::addOptions(Options newOptions) {
    setOptions(m_eSelectedOptions | newOptions);
}

void WBeatMenu::removeOptions(Options removeOptions) {
    setOptions(WBeatMenu::Options(
            m_eSelectedOptions - (m_eSelectedOptions & removeOptions)));
}

void WBeatMenu::slotDisplayTimeSignatureMenu() {
    m_pTimeSignatureMenu->popup(pos());
}