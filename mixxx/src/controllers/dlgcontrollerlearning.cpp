/**
* @file dlgcontrollerlearning.cpp
* @author Sean M. Pappalardo  spappalardo@mixxx.org
* @date Thu 12 Apr 2012
* @brief The controller mapping learning wizard
*
*/

#include "controllers/dlgcontrollerlearning.h"

#include "engine/cuecontrol.h"

DlgControllerLearning::DlgControllerLearning(QWidget * parent,
                                             Controller* controller)
        : QDialog(parent),
          m_pController(controller),
          m_controlPickerMenu(this) {
    qRegisterMetaType<MixxxControl>("MixxxControl");

    setupUi(this);
    labelMappedTo->setText("");
    // Ensure the first page is always shown regardless of the last page shown
    // when the .ui file was saved.
    stackedWidget->setCurrentIndex(0);

    // Delete this dialog when its closed. We don't want any persistence.
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint);

    connect(pushButtonChooseControl, SIGNAL(clicked()), this, SLOT(showControlMenu()));
    connect(pushButtonDone, SIGNAL(clicked()), this, SLOT(close()));
    connect(&m_actionMapper, SIGNAL(mapped(int)), this, SLOT(controlChosen(int)));

    connect(m_pController, SIGNAL(learnedMessage(QString)), this, SLOT(controlMapped(QString)));

    connect(this, SIGNAL(cancelLearning()), m_pController, SLOT(cancelLearn()));
    connect(this, SIGNAL(learn(MixxxControl)), m_pController, SLOT(learn(MixxxControl)));

    m_deckStr = tr("Deck %1");
    m_samplerStr = tr("Sampler %1");
    m_resetStr = tr("Reset to default");

    // Master Controls
    QMenu* mixerMenu = addSubmenu(tr("Mixer"));
    addControl("[Master]", "crossfader", tr("Crossfader"), mixerMenu, true);
    addControl("[Master]", "volume", tr("Master volume"), mixerMenu, true);
    addControl("[Master]", "balance", tr("Master balance"), mixerMenu, true);
    addControl("[Master]", "headVolume", tr("Headphone volume"), mixerMenu, true);
    addControl("[Master]", "headMix", tr("Headphone mix (pre/main)"), mixerMenu, true);

    // Transport
    QMenu* transportMenu = addSubmenu(tr("Transport"));
    addDeckControl("playposition", tr("Strip-search through track"), transportMenu);
    addDeckAndSamplerControl("play", tr("Play button"), transportMenu);
    addDeckControl("volume", tr("Volume fader"), transportMenu, true);
    addDeckControl("back", tr("Fast rewind button"), transportMenu);
    addDeckControl("fwd", tr("Fast forward button"), transportMenu);
    addDeckControl("start", tr("Jump to start of track"), transportMenu);
    addDeckControl("end", tr("Jump to end of track"), transportMenu);
    addDeckControl("reverse", tr("Play reverse button"), transportMenu);
    addDeckAndSamplerControl("pregain", tr("Gain knob"), transportMenu, true);
    addDeckAndSamplerControl("pfl", tr("Headphone listen button"), transportMenu);
    addDeckAndSamplerControl("repeat", tr("Toggle repeat mode"), transportMenu);
    addDeckAndSamplerControl("eject", tr("Eject track"), transportMenu);
    addSamplerControl("orientation", tr("Mix orientation (e.g. left, right, center)"), transportMenu);

    // BPM & Sync
    QMenu* bpmMenu = addSubmenu(tr("BPM and Sync"));
    addDeckAndSamplerControl("bpm_tap", tr("BPM tap button"), bpmMenu);
    addDeckControl("beats_translate_curpos", tr("Adjust beatgrid"), bpmMenu);
    addDeckAndSamplerControl("quantize", tr("Toggle quantize mode"), bpmMenu);
    addDeckAndSamplerControl("beatsync", tr("Beat sync (tempo and phase)"), bpmMenu);
    addDeckAndSamplerControl("beatsync_tempo", tr("Beat sync (tempo only)"), bpmMenu);
    addDeckAndSamplerControl("beatsync_phase", tr("Beat sync (phase only)"), bpmMenu);

    // Rate
    QMenu* rateMenu = addSubmenu(tr("Pitch and Rate"));
    addDeckAndSamplerControl("keylock", tr("Toggle keylock mode"), rateMenu);
    addDeckAndSamplerControl("rate", tr("Pitch control slider"), rateMenu, true);
    addDeckControl("rate_perm_up", tr("Adjust rate up (coarse)"), rateMenu);
    addDeckControl("rate_perm_up_small", tr("Adjust rate up (fine)"), rateMenu);
    addDeckControl("rate_perm_down", tr("Adjust rate down (coarse)"), rateMenu);
    addDeckControl("rate_perm_down_small", tr("Adjust rate down (fine)"), rateMenu);
    addDeckControl("rate_temp_up", tr("Pitch-bend rate up (coarse)"), rateMenu);
    addDeckControl("rate_temp_up_small", tr("Pitch-bend rate up (fine)"), rateMenu);
    addDeckControl("rate_temp_down", tr("Pitch-bend rate down (coarse)"), rateMenu);
    addDeckControl("rate_temp_down_small", tr("Pitch-bend rate down (fine)"), rateMenu);

    // EQs
    QMenu* eqMenu = addSubmenu(tr("Equalizers"));
    addDeckControl("filterHigh", tr("High EQ knob"), eqMenu, true);
    addDeckControl("filterMid", tr("Mid EQ knob"), eqMenu, true);
    addDeckControl("filterLow", tr("Low EQ knob"), eqMenu, true);
    addDeckControl("filterHighKill", tr("High EQ kill"), eqMenu);
    addDeckControl("filterMidKill", tr("Mid EQ kill"), eqMenu);
    addDeckControl("filterLowKill", tr("Low EQ kill"), eqMenu);

    // Vinyl Control
    QMenu* vinylControlMenu = addSubmenu(tr("Vinyl Control"));
    addDeckControl("vinylcontrol_enabled", tr("Toggle vinyl-control (ON/OFF)"), vinylControlMenu);
    addDeckControl("vinylcontrol_cueing", tr("Toggle vinyl-control cueing mode (OFF/ONE/HOT)"), vinylControlMenu);
    addDeckControl("vinylcontrol_mode", tr("Toggle vinyl-control mode (ABS/REL/CONST)"), vinylControlMenu);

    // Cues
    QMenu* cueMenu = addSubmenu(tr("Cues"));
    addDeckControl("cue_default", tr("Cue button"), cueMenu);
    addDeckControl("cue_set", tr("Set cue point"), cueMenu);
    addDeckControl("cue_gotoandstop", tr("Go to cue point and stop"), cueMenu);

    // Hotcues
    QMenu* hotcueMenu = addSubmenu(tr("Hotcues"));
    QString hotcueActivate = tr("Set or jump to hotcue %1");
    QString hotcueClear = tr("Clear hotcue %1");
    QString hotcueGoto = tr("Jump to hotcue %1");
    QString hotcueGotoAndStop = tr("Jump to hotcue %1 and stop");
    for (int i = 1; i <= NUM_HOT_CUES; ++i) {
        QMenu* hotcueSubMenu = addSubmenu(tr("Hotcue %1").arg(QString::number(i)), hotcueMenu);
        addDeckAndSamplerControl(QString("hotcue_%1_activate").arg(i),
                                 hotcueActivate.arg(QString::number(i)), hotcueSubMenu);
        addDeckAndSamplerControl(QString("hotcue_%1_clear").arg(i),
                                 hotcueClear.arg(QString::number(i)), hotcueSubMenu);
        addDeckAndSamplerControl(QString("hotcue_%1_goto").arg(i),
                                 hotcueGoto.arg(QString::number(i)), hotcueSubMenu);
        addDeckAndSamplerControl(QString("hotcue_%1_gotoandstop").arg(i),
                                 hotcueGotoAndStop.arg(QString::number(i)), hotcueSubMenu);
    }

    // Loops
    QMenu* loopMenu = addSubmenu(tr("Looping"));
    addDeckControl("loop_in", tr("Loop In button"), loopMenu);
    addDeckControl("loop_out", tr("Loop Out button"), loopMenu);
    addDeckControl("reloop_exit", tr("Reloop / Exit button"), loopMenu);
    addDeckControl("loop_halve", tr("Halve the current loop's length"), loopMenu);
    addDeckControl("loop_double", tr("Double the current loop's length"), loopMenu);
    addDeckControl("slip_enabled", tr("Toggle slip mode"), loopMenu);

    // Beatloops
    QMenu* beatLoopMenu = addSubmenu(tr("Beat-Looping"));
    QString beatLoop = tr("Create %1-beat loop");
    QString beatLoopRoll = tr("Create temporary %1-beat loop roll");
    addDeckControl("beatloop_0.03125_toggle",  beatLoop.arg(tr("1/32th")), beatLoopMenu);
    addDeckControl("beatloop_0.0625_toggle",  beatLoop.arg(tr("1/16th")), beatLoopMenu);
    addDeckControl("beatloop_0.125_toggle", beatLoop.arg(tr("1/8th")), beatLoopMenu);
    addDeckControl("beatloop_0.25_toggle", beatLoop.arg(tr("1/4th")), beatLoopMenu);
    addDeckControl("beatloop_0.5_toggle", beatLoop.arg("1/2"), beatLoopMenu);
    addDeckControl("beatloop_1_toggle", beatLoop.arg("1"), beatLoopMenu);
    addDeckControl("beatloop_2_toggle", beatLoop.arg("2"), beatLoopMenu);
    addDeckControl("beatloop_4_toggle", beatLoop.arg("4"), beatLoopMenu);
    addDeckControl("beatloop_8_toggle", beatLoop.arg("8"), beatLoopMenu);
    addDeckControl("beatloop_16_toggle", beatLoop.arg("16"), beatLoopMenu);
    addDeckControl("beatloop_32_toggle", beatLoop.arg("32"), beatLoopMenu);
    addDeckControl("beatloop_64_toggle", beatLoop.arg("64"), beatLoopMenu);
    addDeckControl("beatlooproll_0.03125_activate",  beatLoopRoll.arg(tr("1/32th")), beatLoopMenu);
    addDeckControl("beatlooproll_0.0625_activate",  beatLoopRoll.arg(tr("1/16th")), beatLoopMenu);
    addDeckControl("beatlooproll_0.125_activate", beatLoopRoll.arg(tr("1/8th")), beatLoopMenu);
    addDeckControl("beatlooproll_0.25_activate", beatLoopRoll.arg(tr("1/4th")), beatLoopMenu);
    addDeckControl("beatlooproll_0.5_activate", beatLoopRoll.arg("1/2"), beatLoopMenu);
    addDeckControl("beatlooproll_1_activate", beatLoopRoll.arg("1"), beatLoopMenu);
    addDeckControl("beatlooproll_2_activate", beatLoopRoll.arg("2"), beatLoopMenu);
    addDeckControl("beatlooproll_4_activate", beatLoopRoll.arg("4"), beatLoopMenu);
    addDeckControl("beatlooproll_8_activate", beatLoopRoll.arg("8"), beatLoopMenu);
    addDeckControl("beatlooproll_16_activate", beatLoopRoll.arg("16"), beatLoopMenu);
    addDeckControl("beatlooproll_32_activate", beatLoopRoll.arg("32"), beatLoopMenu);
    addDeckControl("beatlooproll_64_activate", beatLoopRoll.arg("64"), beatLoopMenu);

    // Library Controls
    QMenu* libraryMenu = addSubmenu(tr("Library"));
    addControl("[Playlist]", "SelectNextPlaylist", tr("Switch to the next view (library, playlist..)"),
               libraryMenu);
    addControl("[Playlist]", "SelectPrevPlaylist", tr("Switch to the previous view (library, playlist..)"),
               libraryMenu);
    addControl("[Playlist]", "SelectNextTrack", tr("Scroll to next track in library/playlist"),
               libraryMenu);
    addControl("[Playlist]", "SelectPrevTrack", tr("Scroll to previous track in library/playlist"),
               libraryMenu);
    addControl("[Playlist]", "LoadSelectedIntoFirstStopped", tr("Load selected track into first stopped deck"),
               libraryMenu);
    addDeckAndSamplerControl("LoadSelectedTrack", tr("Load selected track"), libraryMenu);

    // Flanger Controls
    QMenu* effectsMenu = addSubmenu(tr("Effects"));
    addDeckControl("flanger", tr("Toggle flange effect"), effectsMenu);
    addControl("[Flanger]", "lfoPeriod", tr("Flange effect: Wavelength/period"), effectsMenu, true);
    addControl("[Flanger]", "lfoDepth", tr("Flange effect: Intensity"), effectsMenu, true);
    addControl("[Flanger]", "lfoDelay", tr("Flange effect: Phase delay"), effectsMenu, true);

    // Microphone Controls
    QMenu* microphoneMenu = addSubmenu(tr("Microphone"));
    addControl("[Microphone]", "talkover", tr("Microphone on/off"), microphoneMenu);
    addControl("[Microphone]", "volume", tr("Microphone volume"), microphoneMenu, true);
    addControl("[Microphone]", "orientation", tr("Microphone channel orientation (e.g. left, right, center)"), microphoneMenu);

    // Skin Controls
    QMenu* guiMenu = addSubmenu(tr("User Interface"));
    addControl("[Samplers]", "show_samplers", tr("Show/hide the sampler section"), guiMenu);
    addControl("[Microphone]", "show_microphone", tr("Show/hide the microphone section"), guiMenu);
    addControl("[Vinylcontrol]", "show_vinylcontrol", tr("Show/hide the vinyl control section"), guiMenu);

    ControlObject *co = ControlObject::getControl(ConfigKey("[Master]", "num_decks"));
    const int iNumDecks = co->get();
    QString spinnyText = tr("Show/hide spinning vinyl widget");
    for (int i = 1; i <= iNumDecks; ++i) {
        addControl(QString("[Spinny%1]").arg(i), "show_spinny",
                   QString("%1: %2").arg(m_deckStr.arg(i), spinnyText), guiMenu);
    }

    emit(listenForClicks());
    labelMappedTo->setText("");
    labelNextHelp->hide();
    controlToMapMessage->setText("");
    stackedWidget->setCurrentIndex(0);
    m_currentControl = MixxxControl();
}

DlgControllerLearning::~DlgControllerLearning() {
    //If there was any ongoing learning, cancel it (benign if there wasn't).
    emit(cancelLearning());
    emit(stopListeningForClicks());
}

void DlgControllerLearning::addControl(QString group, QString control, QString description,
                                       QMenu* pMenu,
                                       bool addReset) {
    QAction* pAction = pMenu->addAction(description, &m_actionMapper, SLOT(map()));
    m_actionMapper.setMapping(pAction, m_controlsAvailable.size());
    m_controlsAvailable.append(MixxxControl(group, control, description));

    if (addReset) {
        QString resetDescription = QString("%1 (%2)").arg(description, m_resetStr);
        QString resetControl = QString("%1_set_default").arg(control);
        QAction* pResetAction = pMenu->addAction(resetDescription, &m_actionMapper, SLOT(map()));
        m_actionMapper.setMapping(pResetAction, m_controlsAvailable.size());
        m_controlsAvailable.append(MixxxControl(group, resetControl, resetDescription));
    }
}

void DlgControllerLearning::addPlayerControl(
    QString control, QString controlDescription, QMenu* pMenu,
    bool deckControls, bool samplerControls, bool addReset) {
    ControlObject* numSamplers = ControlObject::getControl(ConfigKey("[Master]", "num_samplers"));
    const int iNumSamplers = numSamplers->get();
    ControlObject* numDecks = ControlObject::getControl(ConfigKey("[Master]", "num_decks"));
    const int iNumDecks = numDecks->get();

    QMenu* controlMenu = new QMenu(controlDescription, pMenu);
    pMenu->addMenu(controlMenu);

    QMenu* resetControlMenu = NULL;
    QString resetControl = QString("%1_set_default").arg(control);
    if (addReset) {
        QString resetHelpText = QString("%1 (%2)").arg(controlDescription, m_resetStr);
        resetControlMenu = new QMenu(resetHelpText, pMenu);
        pMenu->addMenu(resetControlMenu);
    }

    for (int i = 1; deckControls && i <= iNumDecks; ++i) {
        QString group = QString("[Channel%1]").arg(i);
        QString description = QString("%1: %2").arg(
            m_deckStr.arg(QString::number(i)), controlDescription);
        QAction* pAction = controlMenu->addAction(m_deckStr.arg(i), &m_actionMapper, SLOT(map()));
        m_actionMapper.setMapping(pAction, m_controlsAvailable.size());
        m_controlsAvailable.append(MixxxControl(group, control, description));

        if (resetControlMenu) {
            QString resetDescription = QString("%1 (%2)").arg(description, m_resetStr);
            QAction* pResetAction = resetControlMenu->addAction(m_deckStr.arg(i), &m_actionMapper, SLOT(map()));
            m_actionMapper.setMapping(pResetAction, m_controlsAvailable.size());
            m_controlsAvailable.append(MixxxControl(group, resetControl, resetDescription));
        }
    }

    for (int i = 1; samplerControls && i <= iNumSamplers; ++i) {
        QString group = QString("[Sampler%1]").arg(i);
        QString description = QString("%1: %2").arg(
            m_samplerStr.arg(QString::number(i)), controlDescription);
        QAction* pAction = controlMenu->addAction(m_samplerStr.arg(i), &m_actionMapper, SLOT(map()));
        m_actionMapper.setMapping(pAction, m_controlsAvailable.size());
        m_controlsAvailable.append(MixxxControl(group, control, description));

        if (resetControlMenu) {
            QString resetDescription = QString("%1 (%2)").arg(description, m_resetStr);
            QAction* pResetAction = resetControlMenu->addAction(m_samplerStr.arg(i), &m_actionMapper, SLOT(map()));
            m_actionMapper.setMapping(pResetAction, m_controlsAvailable.size());
            m_controlsAvailable.append(MixxxControl(group, resetControl, resetDescription));
        }
    }
}

void DlgControllerLearning::addDeckAndSamplerControl(QString control, QString controlDescription,
                                                     QMenu* pMenu,
                                                     bool addReset) {
    addPlayerControl(control, controlDescription, pMenu, true, true, addReset);
}

void DlgControllerLearning::addDeckControl(QString control, QString controlDescription,
                                           QMenu* pMenu,
                                           bool addReset) {
    addPlayerControl(control, controlDescription, pMenu, true, false, addReset);
}

void DlgControllerLearning::addSamplerControl(QString control, QString controlDescription,
                                              QMenu* pMenu,
                                              bool addReset) {
    addPlayerControl(control, controlDescription, pMenu, false, true, addReset);
}

void DlgControllerLearning::showControlMenu() {
    m_controlPickerMenu.exec(pushButtonChooseControl->mapToGlobal(QPoint(0,0)));
}

void DlgControllerLearning::loadControl(const MixxxControl& control) {
    m_currentControl = control;
    QString message = tr("Ready to map: %1. Now move a control on your controller.")
            .arg(m_currentControl.description());
    controlToMapMessage->setText(message);
    labelMappedTo->setText("");
    labelNextHelp->hide();
    emit(learn(m_currentControl));
}

void DlgControllerLearning::controlChosen(int controlIndex) {
    if (controlIndex < 0 || controlIndex >= m_controlsAvailable.size()) {
        return;
    }
    loadControl(m_controlsAvailable[controlIndex]);
}

void DlgControllerLearning::controlClicked(ControlObject* pControl) {
    if (!pControl) {
        return;
    }
    ConfigKey key = pControl->getKey();
    // Lookup MixxxControl
    for (int i = 0; i < m_controlsAvailable.size(); ++i) {
        const MixxxControl& control = m_controlsAvailable[i];
        if (control.group() == key.group && control.item() == key.item) {
            loadControl(control);
            return;
        }
    }
}

void DlgControllerLearning::controlMapped(QString message) {
    QString mapMessage = tr("Successfully mapped to:") + " " + message;
    labelMappedTo->setText(mapMessage);
    labelNextHelp->show();
}

QMenu* DlgControllerLearning::addSubmenu(QString title, QMenu* pParent) {
    if (pParent == NULL) {
        pParent = &m_controlPickerMenu;
    }
    QMenu* subMenu = new QMenu(title, pParent);
    pParent->addMenu(subMenu);
    return subMenu;
}

