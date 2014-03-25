#include "controllers/controlpickermenu.h"

#include "vinylcontrol/defs_vinylcontrol.h"
#include "playermanager.h"
#include "engine/cuecontrol.h"

ControlPickerMenu::ControlPickerMenu(QWidget* pParent)
        : QMenu(pParent) {
    connect(&m_actionMapper, SIGNAL(mapped(int)),
            this, SLOT(controlChosen(int)));

    m_deckStr = tr("Deck %1");
    m_samplerStr = tr("Sampler %1");
    m_previewdeckStr = tr("Preview Deck %1");
    m_microphoneStr = tr("Microphone %1");
    m_auxStr = tr("Auxiliary %1");
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
    addDeckAndPreviewDeckControl("playposition", tr("Strip-search through track"), transportMenu);
    addDeckAndSamplerAndPreviewDeckControl("play", tr("Play button"), transportMenu);
    addDeckControl("volume", tr("Volume fader"), transportMenu, true);
    addDeckControl("back", tr("Fast rewind button"), transportMenu);
    addDeckControl("fwd", tr("Fast forward button"), transportMenu);
    addDeckAndSamplerAndPreviewDeckControl("start", tr("Jump to start of track"), transportMenu);
    addDeckControl("end", tr("Jump to end of track"), transportMenu);
    addDeckControl("reverse", tr("Play reverse button"), transportMenu);
    addDeckControl("reverseroll", tr("Reverse roll (Censor) button"), transportMenu);
    addDeckAndSamplerAndPreviewDeckControl("pregain", tr("Gain knob"), transportMenu, true);
    addDeckAndSamplerControl("pfl", tr("Headphone listen button"), transportMenu);
    addDeckAndSamplerControl("mute", tr("Mute button"), transportMenu);
    addDeckAndSamplerControl("repeat", tr("Toggle repeat mode"), transportMenu);
    addDeckAndSamplerAndPreviewDeckControl("eject", tr("Eject track"), transportMenu);
    addSamplerControl("orientation", tr("Mix orientation (e.g. left, right, center)"), transportMenu);
    addDeckAndSamplerControl("slip_enabled", tr("Toggle slip mode"), transportMenu);

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
    addDeckAndSamplerControl("rate", tr("Speed control slider"), rateMenu, true);
    addDeckAndSamplerControl("pitch", tr("Pitch control slider"), rateMenu, true);
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
    addDeckControl("passthrough", tr("Pass through external audio into the internal mixer"), vinylControlMenu);
    addControl(VINYL_PREF_KEY, "Toggle", tr("Single deck mode - Toggle vinyl control to next deck"), vinylControlMenu);

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
    addDeckControl("loop_exit", tr("Loop Exit button"), loopMenu);
    addDeckControl("reloop_exit", tr("Reloop / Exit button"), loopMenu);
    addDeckControl("loop_halve", tr("Halve the current loop's length"), loopMenu);
    addDeckControl("loop_double", tr("Double the current loop's length"), loopMenu);

    // Loop moving
    QString loopMoveForward = tr("Move loop forward by %1 beats");
    QString loopMoveBackward = tr("Move loop backward by %1 beats");
    addDeckControl("loop_move_0.03125_forward",  loopMoveForward.arg(tr("1/32th")), loopMenu);
    addDeckControl("loop_move_0.0625_forward",  loopMoveForward.arg(tr("1/16th")), loopMenu);
    addDeckControl("loop_move_0.125_forward", loopMoveForward.arg(tr("1/8th")), loopMenu);
    addDeckControl("loop_move_0.25_forward", loopMoveForward.arg(tr("1/4th")), loopMenu);
    addDeckControl("loop_move_0.5_forward", loopMoveForward.arg("1/2"), loopMenu);
    addDeckControl("loop_move_1_forward", loopMoveForward.arg("1"), loopMenu);
    addDeckControl("loop_move_2_forward", loopMoveForward.arg("2"), loopMenu);
    addDeckControl("loop_move_4_forward", loopMoveForward.arg("4"), loopMenu);
    addDeckControl("loop_move_8_forward", loopMoveForward.arg("8"), loopMenu);
    addDeckControl("loop_move_16_forward", loopMoveForward.arg("16"), loopMenu);
    addDeckControl("loop_move_32_forward", loopMoveForward.arg("32"), loopMenu);
    addDeckControl("loop_move_64_forward", loopMoveForward.arg("64"), loopMenu);
    addDeckControl("loop_move_0.03125_backward",  loopMoveBackward.arg(tr("1/32th")), loopMenu);
    addDeckControl("loop_move_0.0625_backward",  loopMoveBackward.arg(tr("1/16th")), loopMenu);
    addDeckControl("loop_move_0.125_backward", loopMoveBackward.arg(tr("1/8th")), loopMenu);
    addDeckControl("loop_move_0.25_backward", loopMoveBackward.arg(tr("1/4th")), loopMenu);
    addDeckControl("loop_move_0.5_backward", loopMoveBackward.arg("1/2"), loopMenu);
    addDeckControl("loop_move_1_backward", loopMoveBackward.arg("1"), loopMenu);
    addDeckControl("loop_move_2_backward", loopMoveBackward.arg("2"), loopMenu);
    addDeckControl("loop_move_4_backward", loopMoveBackward.arg("4"), loopMenu);
    addDeckControl("loop_move_8_backward", loopMoveBackward.arg("8"), loopMenu);
    addDeckControl("loop_move_16_backward", loopMoveBackward.arg("16"), loopMenu);
    addDeckControl("loop_move_32_backward", loopMoveBackward.arg("32"), loopMenu);
    addDeckControl("loop_move_64_backward", loopMoveBackward.arg("64"), loopMenu);

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

    // Beat jumping
    QMenu* beatJumpMenu = addSubmenu(tr("Beat-Jump"));
    QString beatJumpForward = tr("Jump forward by %1 beats");
    QString beatJumpBackward = tr("Jump backward by %1 beats");
    addDeckControl("beatjump_0.03125_forward",  beatJumpForward.arg(tr("1/32th")), beatJumpMenu);
    addDeckControl("beatjump_0.0625_forward",  beatJumpForward.arg(tr("1/16th")), beatJumpMenu);
    addDeckControl("beatjump_0.125_forward", beatJumpForward.arg(tr("1/8th")), beatJumpMenu);
    addDeckControl("beatjump_0.25_forward", beatJumpForward.arg(tr("1/4th")), beatJumpMenu);
    addDeckControl("beatjump_0.5_forward", beatJumpForward.arg("1/2"), beatJumpMenu);
    addDeckControl("beatjump_1_forward", beatJumpForward.arg("1"), beatJumpMenu);
    addDeckControl("beatjump_2_forward", beatJumpForward.arg("2"), beatJumpMenu);
    addDeckControl("beatjump_4_forward", beatJumpForward.arg("4"), beatJumpMenu);
    addDeckControl("beatjump_8_forward", beatJumpForward.arg("8"), beatJumpMenu);
    addDeckControl("beatjump_16_forward", beatJumpForward.arg("16"), beatJumpMenu);
    addDeckControl("beatjump_32_forward", beatJumpForward.arg("32"), beatJumpMenu);
    addDeckControl("beatjump_64_forward", beatJumpForward.arg("64"), beatJumpMenu);
    addDeckControl("beatjump_0.03125_backward",  beatJumpBackward.arg(tr("1/32th")), beatJumpMenu);
    addDeckControl("beatjump_0.0625_backward",  beatJumpBackward.arg(tr("1/16th")), beatJumpMenu);
    addDeckControl("beatjump_0.125_backward", beatJumpBackward.arg(tr("1/8th")), beatJumpMenu);
    addDeckControl("beatjump_0.25_backward", beatJumpBackward.arg(tr("1/4th")), beatJumpMenu);
    addDeckControl("beatjump_0.5_backward", beatJumpBackward.arg("1/2"), beatJumpMenu);
    addDeckControl("beatjump_1_backward", beatJumpBackward.arg("1"), beatJumpMenu);
    addDeckControl("beatjump_2_backward", beatJumpBackward.arg("2"), beatJumpMenu);
    addDeckControl("beatjump_4_backward", beatJumpBackward.arg("4"), beatJumpMenu);
    addDeckControl("beatjump_8_backward", beatJumpBackward.arg("8"), beatJumpMenu);
    addDeckControl("beatjump_16_backward", beatJumpBackward.arg("16"), beatJumpMenu);
    addDeckControl("beatjump_32_backward", beatJumpBackward.arg("32"), beatJumpMenu);
    addDeckControl("beatjump_64_backward", beatJumpBackward.arg("64"), beatJumpMenu);

    // Library Controls
    QMenu* libraryMenu = addSubmenu(tr("Library"));
    addControl("[Playlist]", "ToggleSelectedSidebarItem", tr("Expand/collapse the selected view (library, playlist..)"),
               libraryMenu);
    addControl("[Playlist]", "SelectPlaylist", tr("Switch to the next or previous view (library, playlist..)"),
               libraryMenu);
    addControl("[Playlist]", "SelectNextPlaylist", tr("Switch to the next view (library, playlist..)"),
               libraryMenu);
    addControl("[Playlist]", "SelectPrevPlaylist", tr("Switch to the previous view (library, playlist..)"),
               libraryMenu);
    addControl("[Playlist]", "SelectTrackKnob", tr("Scroll up or down in library/playlist"),
               libraryMenu);
    addControl("[Playlist]", "SelectNextTrack", tr("Scroll to next track in library/playlist"),
               libraryMenu);
    addControl("[Playlist]", "SelectPrevTrack", tr("Scroll to previous track in library/playlist"),
               libraryMenu);
    addControl("[Playlist]", "LoadSelectedIntoFirstStopped", tr("Load selected track into first stopped deck"),
               libraryMenu);
    addDeckAndSamplerControl("LoadSelectedTrack", tr("Load selected track"), libraryMenu);
    addDeckAndSamplerAndPreviewDeckControl("LoadSelectedTrackAndPlay", tr("Load selected track and play"), libraryMenu);

    // Effect Controls
    QMenu* effectsMenu = addSubmenu(tr("Effects"));
    addDeckControl("flanger", tr("Toggle flange effect"), effectsMenu);
    addControl("[Flanger]", "lfoPeriod", tr("Flange effect: Wavelength/period"), effectsMenu, true);
    addControl("[Flanger]", "lfoDepth", tr("Flange effect: Intensity"), effectsMenu, true);
    addControl("[Flanger]", "lfoDelay", tr("Flange effect: Phase delay"), effectsMenu, true);
    addDeckControl("filter", tr("Toggle filter effect"), effectsMenu);
    addDeckControl("filterDepth", tr("Filter effect: Intensity"), effectsMenu, true);

    // Microphone Controls
    QMenu* microphoneMenu = addSubmenu(tr("Microphone / Auxiliary"));

    addMicrophoneAndAuxControl("talkover", tr("Microphone on/off"), microphoneMenu,
                               true, false);
    addMicrophoneAndAuxControl("passthrough", tr("Auxiliary on/off"), microphoneMenu,
                               false, true);
    addMicrophoneAndAuxControl("volume", tr("Volume fader"), microphoneMenu,
                               true, true, true);
    addMicrophoneAndAuxControl("mute", tr("Mute button"), microphoneMenu,
                               true, true);
    addMicrophoneAndAuxControl("orientation", tr("Mix orientation (e.g. left, right, center)"),
                               microphoneMenu, true, true);
    addMicrophoneAndAuxControl("pfl", tr("Headphone listen button"), microphoneMenu,
                               true, true);

    // AutoDJ Controls
    QMenu* autodjMenu = addSubmenu(tr("Auto DJ"));
    addControl("[AutoDJ]", "shuffle_playlist", tr("Shuffle the content of the Auto DJ playlist"), autodjMenu);
    addControl("[AutoDJ]", "skip_next", tr("Skip the next track in the Auto DJ playlist"), autodjMenu);
    addControl("[AutoDJ]", "fade_now", tr("Trigger the transition to the next track"), autodjMenu);
    addControl("[AutoDJ]", "enabled", tr("Toggle Auto DJ (ON/OFF)"), autodjMenu);

    // Skin Controls
    QMenu* guiMenu = addSubmenu(tr("User Interface"));
    addControl("[Samplers]", "show_samplers", tr("Show/hide the sampler section"), guiMenu);
    addControl("[Microphone]", "show_microphone", tr("Show/hide the microphone section"), guiMenu);
    addControl("[Vinylcontrol]", "show_vinylcontrol", tr("Show/hide the vinyl control section"), guiMenu);
    addControl("[PreviewDeck]", "show_previewdeck", tr("Show/hide the preview deck"), guiMenu);

    const int iNumDecks = ControlObject::get(ConfigKey("[Master]", "num_decks"));
    QString spinnyText = tr("Show/hide spinning vinyl widget");
    for (int i = 1; i <= iNumDecks; ++i) {
        addControl(QString("[Spinny%1]").arg(i), "show_spinny",
                   QString("%1: %2").arg(m_deckStr.arg(i), spinnyText), guiMenu);

    }
}

ControlPickerMenu::~ControlPickerMenu() {
}

void ControlPickerMenu::addControl(QString group, QString control, QString description,
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

void ControlPickerMenu::addPlayerControl(QString control, QString controlDescription,
                                         QMenu* pMenu,
                                         bool deckControls, bool samplerControls,
                                         bool previewdeckControls,
                                         bool addReset) {
    const int iNumSamplers = ControlObject::get(ConfigKey("[Master]", "num_samplers"));
    const int iNumDecks = ControlObject::get(ConfigKey("[Master]", "num_decks"));
    const int iNumPreviewDecks = ControlObject::get(ConfigKey("[Master]", "num_preview_decks"));

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
        // PlayerManager::groupForDeck is 0-indexed.
        QString group = PlayerManager::groupForDeck(i - 1);
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

    for (int i = 1; previewdeckControls && i <= iNumPreviewDecks; ++i) {
        // PlayerManager::groupForPreviewDeck is 0-indexed.
        QString group = PlayerManager::groupForPreviewDeck(i - 1);
        QString description = QString("%1: %2").arg(
            m_previewdeckStr.arg(QString::number(i)), controlDescription);
        QAction* pAction = controlMenu->addAction(m_previewdeckStr.arg(i), &m_actionMapper, SLOT(map()));
        m_actionMapper.setMapping(pAction, m_controlsAvailable.size());
        m_controlsAvailable.append(MixxxControl(group, control, description));

        if (resetControlMenu) {
            QString resetDescription = QString("%1 (%2)").arg(description, m_resetStr);
            QAction* pResetAction = resetControlMenu->addAction(m_previewdeckStr.arg(i), &m_actionMapper, SLOT(map()));
            m_actionMapper.setMapping(pResetAction, m_controlsAvailable.size());
            m_controlsAvailable.append(MixxxControl(group, resetControl, resetDescription));
        }
    }

    for (int i = 1; samplerControls && i <= iNumSamplers; ++i) {
        // PlayerManager::groupForSampler is 0-indexed.
        QString group = PlayerManager::groupForSampler(i - 1);
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

void ControlPickerMenu::addMicrophoneAndAuxControl(QString control, QString controlDescription,
                                                   QMenu* pMenu,
                                                   bool microhoneControls,
                                                   bool auxControls,
                                                   bool addReset) {
    QMenu* controlMenu = new QMenu(controlDescription, pMenu);
    pMenu->addMenu(controlMenu);

    QMenu* resetControlMenu = NULL;
    QString resetControl = QString("%1_set_default").arg(control);
    if (addReset) {
        QString resetHelpText = QString("%1 (%2)").arg(controlDescription, m_resetStr);
        resetControlMenu = new QMenu(resetHelpText, pMenu);
        pMenu->addMenu(resetControlMenu);
    }

    if (microhoneControls) {
        const int kNumMicrophones = ControlObject::get(ConfigKey("[Master]", "num_microphones"));
        for (int i = 1; i <= kNumMicrophones; ++i) {
            QString group = "[Microphone]";
            if (i > 1) {
                group = QString("[Microphone%1]").arg(i);
            }

            QString description = QString("%1: %2").arg(
                m_microphoneStr.arg(QString::number(i)), controlDescription);
            QAction* pAction = controlMenu->addAction(m_microphoneStr.arg(i),
                                                      &m_actionMapper, SLOT(map()));
            m_actionMapper.setMapping(pAction, m_controlsAvailable.size());
            m_controlsAvailable.append(MixxxControl(group, control, description));

            if (addReset) {
                QString resetDescription = QString("%1 (%2)").arg(description, m_resetStr);
                QAction* pResetAction = resetControlMenu->addAction(
                    m_microphoneStr.arg(i), &m_actionMapper, SLOT(map()));
                m_actionMapper.setMapping(pResetAction, m_controlsAvailable.size());
                m_controlsAvailable.append(MixxxControl(group, resetControl, resetDescription));
            }
        }
    }

    const int kNumAuxiliaries = ControlObject::get(ConfigKey("[Master]", "num_auxiliaries"));
    if (auxControls) {
        for (int i = 1; i <= kNumAuxiliaries; ++i) {
            QString group = QString("[Auxiliary%1]").arg(i);
            QString description = QString("%1: %2").arg(
                m_auxStr.arg(QString::number(i)), controlDescription);
            QAction* pAction = controlMenu->addAction(m_auxStr.arg(i),
                                                      &m_actionMapper, SLOT(map()));
            m_actionMapper.setMapping(pAction, m_controlsAvailable.size());
            m_controlsAvailable.append(MixxxControl(group, control, description));

            if (addReset) {
                QString resetDescription = QString("%1 (%2)").arg(description, m_resetStr);
                QAction* pResetAction = resetControlMenu->addAction(
                    m_auxStr.arg(i), &m_actionMapper, SLOT(map()));
                m_actionMapper.setMapping(pResetAction, m_controlsAvailable.size());
                m_controlsAvailable.append(MixxxControl(group, resetControl, resetDescription));
            }
        }
    }
}

void ControlPickerMenu::addDeckAndSamplerControl(QString control, QString controlDescription,
                                                 QMenu* pMenu,
                                                 bool addReset) {
    addPlayerControl(control, controlDescription, pMenu, true, true, false, addReset);
}

void ControlPickerMenu::addDeckAndPreviewDeckControl(QString control, QString controlDescription,
                                                     QMenu* pMenu,
                                                     bool addReset) {
    addPlayerControl(control, controlDescription, pMenu, true, false, true, addReset);
}

void ControlPickerMenu::addDeckAndSamplerAndPreviewDeckControl(QString control,
                                                               QString controlDescription,
                                                               QMenu* pMenu,
                                                               bool addReset) {
    addPlayerControl(control, controlDescription, pMenu, true, true, true, addReset);
}

void ControlPickerMenu::addDeckControl(QString control, QString controlDescription,
                                       QMenu* pMenu,
                                       bool addReset) {
    addPlayerControl(control, controlDescription, pMenu, true, false, false, addReset);
}

void ControlPickerMenu::addSamplerControl(QString control, QString controlDescription,
                                          QMenu* pMenu,
                                          bool addReset) {
    addPlayerControl(control, controlDescription, pMenu, false, true, false, addReset);
}

void ControlPickerMenu::addPreviewDeckControl(QString control, QString controlDescription,
                                              QMenu* pMenu,
                                              bool addReset) {
    addPlayerControl(control, controlDescription, pMenu, false, false, true, addReset);
}

QMenu* ControlPickerMenu::addSubmenu(QString title, QMenu* pParent) {
    if (pParent == NULL) {
        pParent = this;
    }
    QMenu* subMenu = new QMenu(title, pParent);
    pParent->addMenu(subMenu);
    return subMenu;
}

void ControlPickerMenu::controlChosen(int controlIndex) {
    if (controlIndex < 0 || controlIndex >= m_controlsAvailable.size()) {
        return;
    }
    emit(controlPicked(m_controlsAvailable[controlIndex]));
}
