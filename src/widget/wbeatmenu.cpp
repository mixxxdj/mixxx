#include "wbeatmenu.h"

WBeatMenu::WBeatMenu(UserSettingsPointer pConfig, QWidget* parent)
        : QMenu(parent),
          m_pConfig(pConfig),
          m_pTimeSignatureMenu(make_parented<WTimeSignatureMenu>(this)),
          m_pTempoMenu(make_parented<WTempoMenu>(this)),
          m_beat(mixxx::kInvalidFramePos) {
}

void WBeatMenu::updateMenu() {
    clear();
    if (!m_beat) {
        return;
    }
    // TODO(hacksdump): Only create menu for beats which are visible on the waveform.
    if (m_eSelectedOptions.testFlag(WBeatMenu::Option::SetDownbeat) &&
            m_beat->type() == mixxx::BeatType::Beat) {
        parented_ptr<QAction> pSetAsDownbeat;
        pSetAsDownbeat = make_parented<QAction>(tr("Set as Downbeat"), this);
        connect(pSetAsDownbeat.get(),
                &QAction::triggered,
                this,
                &WBeatMenu::slotDownbeatUpdated);
        addAction(pSetAsDownbeat.get());
    }
    if (m_eSelectedOptions.testFlag(WBeatMenu::Option::CueMenu)) {
        parented_ptr<QAction> pCueMenu;
        pCueMenu = make_parented<QAction>(tr("Edit Cue"), this);
        connect(pCueMenu.get(),
                &QAction::triggered,
                this,
                &WBeatMenu::cueButtonClicked);
        addAction(pCueMenu.get());
    }
    if (m_beat->type() == mixxx::BeatType::Downbeat) {
        parented_ptr<QAction> pTimeSignatureAction;
        pTimeSignatureAction =
                make_parented<QAction>(tr("Edit Time Signature"), this);
        connect(pTimeSignatureAction.get(),
                &QAction::triggered,
                this,
                &WBeatMenu::slotDisplayTimeSignatureMenu);
        addAction(pTimeSignatureAction.get());
    }

    parented_ptr<QAction> pTempoAction;
    pTempoAction =
            make_parented<QAction>(tr("Edit tempo ahead"), this);
    connect(pTempoAction.get(),
            &QAction::triggered,
            this,
            &WBeatMenu::slotDisplayTempoMenu);
    addAction(pTempoAction.get());
}

void WBeatMenu::slotDownbeatUpdated() {
    if (m_pBeats) {
        m_pBeats->setAsDownbeat(m_beat->beatIndex());
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

void WBeatMenu::slotDisplayTempoMenu() {
    m_pTempoMenu->popup(pos());
}

void WBeatMenu::setBeatsPointer(mixxx::BeatsPointer pBeats) {
    if (pBeats) {
        m_pBeats = pBeats;
        m_pTimeSignatureMenu->setBeatsPointer(pBeats);
        m_pTempoMenu->setBeatsPointer(pBeats);
    }
}

void WBeatMenu::setBeat(const std::optional<mixxx::Beat>& beat) {
    if (m_pBeats) {
        m_beat = beat;
        m_pTimeSignatureMenu->setBeat(beat);
        m_pTempoMenu->setBeat(beat);
    }
}

void WBeatMenu::slotBeatsUpdated() {
    setBeat(m_pBeats->getBeatAtIndex(m_beat->beatIndex()));
}
