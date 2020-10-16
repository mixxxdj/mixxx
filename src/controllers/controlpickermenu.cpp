#include "controllers/controlpickermenu.h"

#include "vinylcontrol/defs_vinylcontrol.h"
#include "mixer/playermanager.h"
#include "engine/controls/cuecontrol.h"
#include "engine/controls/loopingcontrol.h"
#include "effects/effectrack.h"
#include "effects/effectchainslot.h"
#include "effects/effectslot.h"
#include "effects/effectparameterslot.h"

ControlPickerMenu::ControlPickerMenu(QWidget* pParent)
        : QMenu(pParent) {
    m_effectMasterOutputStr = tr("Master Output");
    m_effectHeadphoneOutputStr = tr("Headphone Output");
    m_deckStr = tr("Deck %1");
    m_samplerStr = tr("Sampler %1");
    m_previewdeckStr = tr("Preview Deck %1");
    m_microphoneStr = tr("Microphone %1");
    m_auxStr = tr("Auxiliary %1");
    m_resetStr = tr("Reset to default");
    m_effectRackStr = tr("Effect Rack %1");
    m_effectUnitStr = tr("Unit %1");
    m_effectStr = tr("Slot %1");
    m_parameterStr = tr("Parameter %1");
    m_libraryStr = tr("Library");

    // Master Controls
    QMenu* mixerMenu = addSubmenu(tr("Mixer"));
    addControl("[Master]", "crossfader", tr("Crossfader"), tr("Master crossfader"), mixerMenu, true);
    addControl("[Master]", "gain", tr("Master Gain"), tr("Master gain"), mixerMenu, true);
    addControl("[Master]", "balance", tr("Master Balance"), tr("Master balance"), mixerMenu, true);
    addControl("[Master]", "delay", tr("Master Delay"), tr("Master delay"), mixerMenu, true);
    addControl("[Master]", "headGain", tr("Headphone Gain"), tr("Headphone gain"), mixerMenu, true);
    addControl("[Master]", "headMix", tr("Headphone Mix"), tr("Headphone mix (pre/main)"), mixerMenu, true);
    addControl("[Master]", "headSplit", tr("Headphone Split Cue"), tr("Toggle headphone split cueing"), mixerMenu);
    addControl("[Master]", "headDelay", tr("Headphone Delay"), tr("Headphone delay"), mixerMenu, true);
    addDeckAndSamplerControl("orientation", tr("Orientation"),
                             tr("Mix orientation (e.g. left, right, center)"), mixerMenu);
    addDeckAndSamplerControl("orientation_left", tr("Orient Left"),
                             tr("Set mix orientation to left"), mixerMenu);
    addDeckAndSamplerControl("orientation_center", tr("Orient Center"),
                             tr("Set mix orientation to center"), mixerMenu);
    addDeckAndSamplerControl("orientation_right", tr("Orient Right"),
                             tr("Set mix orientation to right"), mixerMenu);

    // Transport
    QMenu* transportMenu = addSubmenu(tr("Transport"));
    addDeckAndSamplerAndPreviewDeckControl("play", tr("Play"), tr("Play button"), transportMenu);
    // Preview deck does not go to master so volume does not matter.
    addDeckAndSamplerAndPreviewDeckControl("back", tr("Fast Rewind"), tr("Fast Rewind button"), transportMenu);
    addDeckAndSamplerAndPreviewDeckControl("fwd", tr("Fast Forward"), tr("Fast Forward button"), transportMenu);
    addDeckAndSamplerAndPreviewDeckControl("playposition", tr("Strip Search"),
                                           tr("Strip-search through track"), transportMenu);
    addDeckAndSamplerAndPreviewDeckControl("reverse", tr("Play Reverse"), tr("Play Reverse button"), transportMenu);
    addDeckAndSamplerAndPreviewDeckControl("reverseroll", tr("Reverse Roll (Censor)"),
                                           tr("Reverse roll (Censor) button"), transportMenu);
    addDeckAndSamplerAndPreviewDeckControl("start", tr("Jump To Start"), tr("Jumps to start of track"), transportMenu);
    addDeckAndSamplerAndPreviewDeckControl("start_play", tr("Play From Start"),
                                           tr("Jump to start of track and play"), transportMenu);
    addDeckAndSamplerAndPreviewDeckControl("stop", tr("Stop"), tr("Stop button"), transportMenu);
    addDeckAndSamplerAndPreviewDeckControl("start_stop", tr("Stop And Jump To Start"),
                                           tr("Stop playback and jump to start of track"), transportMenu);
    addDeckAndSamplerAndPreviewDeckControl("end", tr("Jump To End"), tr("Jump to end of track"), transportMenu);
    addDeckAndSamplerControl("volume", tr("Volume"), tr("Volume Fader"), transportMenu, true);
    addDeckAndSamplerControl("volume_set_one", tr("Full Volume"), tr("Set to full volume"), transportMenu);
    addDeckAndSamplerControl("volume_set_zero", tr("Zero Volume"), tr("Set to zero volume"), transportMenu);
    addDeckAndSamplerAndPreviewDeckControl("pregain", tr("Track Gain"), tr("Track Gain knob"), transportMenu, true);
    addDeckAndSamplerControl("mute", tr("Mute"), tr("Mute button"), transportMenu);
    addDeckAndSamplerAndPreviewDeckControl("eject", tr("Eject"), tr("Eject track"), transportMenu);
    addDeckAndSamplerControl("pfl", tr("Headphone Listen"), tr("Headphone listen (pfl) button"), transportMenu);
    addDeckAndSamplerControl("repeat", tr("Repeat Mode"), tr("Toggle repeat mode"), transportMenu);
    addDeckAndSamplerControl("slip_enabled", tr("Slip Mode"), tr("Toggle slip mode"), transportMenu);

    // BPM & Sync
    QMenu* bpmMenu = addSubmenu(tr("BPM"));
    addDeckAndSamplerControl("bpm", tr("BPM"), tr("BPM"), bpmMenu, true);
    addDeckAndSamplerControl("bpm_up", tr("BPM +1"), tr("Increase BPM by 1"), bpmMenu);
    addDeckAndSamplerControl("bpm_down", tr("BPM -1"), tr("Decrease BPM by 1"), bpmMenu);
    addDeckAndSamplerControl("bpm_up_small", tr("BPM +0.1"), tr("Increase BPM by 0.1"), bpmMenu);
    addDeckAndSamplerControl("bpm_down_small", tr("BPM -0.1"), tr("Decrease BPM by 0.1"), bpmMenu);
    addDeckAndSamplerControl("bpm_tap", tr("BPM Tap"), tr("BPM tap button"), bpmMenu);
    addDeckAndSamplerControl("beats_adjust_faster", tr("Adjust Beatgrid Faster +.01"), tr("Increase track's average BPM by 0.01"), bpmMenu);
    addDeckAndSamplerControl("beats_adjust_slower", tr("Adjust Beatgrid Slower -.01"), tr("Decrease track's average BPM by 0.01"), bpmMenu);
    addDeckAndSamplerControl("beats_translate_earlier", tr("Move Beatgrid Earlier"), tr("Adjust the beatgrid to the left"), bpmMenu);
    addDeckAndSamplerControl("beats_translate_later", tr("Move Beatgrid Later"), tr("Adjust the beatgrid to the right"), bpmMenu);
    addDeckControl("beats_translate_curpos", tr("Adjust Beatgrid"),
                   tr("Align beatgrid to current position"), bpmMenu);
    addDeckControl("beats_translate_match_alignment", tr("Adjust Beatgrid - Match Alignment"),
                   tr("Adjust beatgrid to match another playing deck."), bpmMenu);
    addDeckAndSamplerControl("quantize", tr("Quantize Mode"), tr("Toggle quantize mode"), bpmMenu);

    QMenu* syncMenu = addSubmenu(tr("Sync"));
    addDeckAndSamplerControl("sync_enabled", tr("Sync Mode"),
                             tr("Tap to sync tempo (and phase with quantize enabled), hold to enable permanent sync"), syncMenu);
    addControl("[InternalClock]", "sync_master", tr("Internal Sync Master"),
               tr("Toggle Internal Sync Master"), syncMenu);
    addControl("[InternalClock]", "bpm", tr("Internal Master BPM"),
               tr("Internal Master BPM"), syncMenu);
    addControl("[InternalClock]", "bpm_up", tr("Internal Master BPM +1"),
               tr("Increase internal master BPM by 1"), syncMenu);

    addControl("[InternalClock]", "bpm_down", tr("Internal Master BPM -1"),
               tr("Decrease internal master BPM by 1"), syncMenu);

    addControl("[InternalClock]", "bpm_up_small", tr("Internal Master BPM +0.1"),
               tr("Increase internal master BPM by 0.1"), syncMenu);
    addControl("[InternalClock]", "bpm_down_small", tr("Internal Master BPM -0.1"),
               tr("Decrease internal master BPM by 0.1"), syncMenu);
    addDeckAndSamplerControl("sync_master", tr("Sync Master"), tr("Toggle sync master"), syncMenu);
    addDeckAndSamplerControl("sync_mode", tr("Sync Mode"),
                             tr("Sync mode 3-state toggle (OFF, FOLLOWER, MASTER)"), syncMenu);
    addDeckAndSamplerControl("beatsync", tr("Beat Sync One-Shot"),
                             tr("One-time beat sync tempo (and phase with quantize enabled)"), syncMenu);
    addDeckAndSamplerControl("beatsync_tempo", tr("Sync Tempo One-Shot"),
                             tr("One-time beat sync (tempo only)"), syncMenu);
    addDeckAndSamplerControl("beatsync_phase", tr("Sync Phase One-Shot"),
                             tr("One-time beat sync (phase only)"), syncMenu);

    // Speed
    QMenu* speedMenu = addSubmenu(tr("Speed (Pitch/Tempo)"));
    addDeckAndSamplerControl("keylock", tr("Keylock Mode"),
                             tr("Toggle keylock mode"), speedMenu);
    addDeckAndSamplerControl("rate", tr("Playback Speed"),
                             tr("Playback speed control (Vinyl \"Pitch\" slider)"), speedMenu, true);
    addDeckAndSamplerControl("pitch", tr("Pitch (Musical key)"),
                             tr("Pitch control (does not affect tempo), center is original pitch"), speedMenu, true);
    addDeckAndSamplerControl("pitch_up", tr("Increase Pitch"),
                            tr("Increases the pitch by one semitone"), speedMenu);
    addDeckAndSamplerControl("pitch_up_small", tr("Increase Pitch (Fine)"),
                            tr("Increases the pitch by 10 cents"), speedMenu);
    addDeckAndSamplerControl("pitch_down", tr("Decrease Pitch"),
                            tr("Decreases the pitch by one semitone"), speedMenu);
    addDeckAndSamplerControl("pitch_down_small", tr("Decrease Pitch (Fine)"),
                            tr("Decreases the pitch by 10 cents"), speedMenu);
    addDeckAndSamplerControl("pitch_adjust", tr("Pitch Adjust"),
                             tr("Adjust pitch from speed slider pitch"), speedMenu, true);
    addDeckAndSamplerControl("sync_key", tr("Match Key"), tr("Match musical key"), speedMenu);
    addDeckAndSamplerControl("reset_key", tr("Reset Key"), tr("Resets key to original"), speedMenu);
    addDeckAndSamplerControl("rate_perm_up", tr("Increase Speed"),
                             tr("Adjust speed faster (coarse)"), speedMenu);
    addDeckAndSamplerControl("rate_perm_up_small", tr("Increase Speed (Fine)"),
                             tr("Adjust speed faster (fine)"), speedMenu);
    addDeckAndSamplerControl("rate_perm_down", tr("Decrease Speed"),
                             tr("Adjust speed slower (coarse)"), speedMenu);
    addDeckAndSamplerControl("rate_perm_down_small", tr("Increase Speed (Fine)"),
                             tr("Adjust speed slower (fine)"), speedMenu);
    addDeckAndSamplerControl("rate_temp_up", tr("Temporarily Increase Speed"),
                             tr("Temporarily increase speed (coarse)"), speedMenu);
    addDeckAndSamplerControl("rate_temp_up_small", tr("Temporarily Increase Speed (Fine)"),
                             tr("Temporarily increase speed (fine)"), speedMenu);
    addDeckAndSamplerControl("rate_temp_down", tr("Temporarily Decrease Speed"),
                             tr("Temporarily decrease speed (coarse)"), speedMenu);
    addDeckAndSamplerControl("rate_temp_down_small", tr("Temporarily Decrease Speed (Fine)"),
                             tr("Temporarily decrease speed (fine)"), speedMenu);

    // EQs
    QMenu* eqMenu = addSubmenu(tr("Equalizers"));
    constexpr int kNumEqRacks = 1;
    const int iNumDecks = static_cast<int>(ControlObject::get(ConfigKey("[Master]", "num_decks")));
    for (int iRackNumber = 0; iRackNumber < kNumEqRacks; ++iRackNumber) {
        // TODO: Although there is a mode with 4-band EQs, it's not feasible
        // right now to add support for learning both it and regular 3-band eqs.
        // Since 3-band is by far the most common, stick with that.
        const int kMaxEqs = 3;
        QList<QString> eqNames;
        eqNames.append(tr("Low EQ"));
        eqNames.append(tr("Mid EQ"));
        eqNames.append(tr("High EQ"));
        for (int deck = 1; deck <= iNumDecks; ++deck) {
            QMenu* deckMenu = addSubmenu(QString("Deck %1").arg(deck), eqMenu);
            for (int effect = kMaxEqs - 1; effect >= 0; --effect) {
                const QString group = EqualizerRack::formatEffectSlotGroupString(
                        iRackNumber, 0, QString("[Channel%1]").arg(deck));
                QMenu* bandMenu = addSubmenu(eqNames[effect], deckMenu);
                QString control = "parameter%1";
                addControl(group, control.arg(effect+1),
                           tr("Adjust %1").arg(eqNames[effect]),
                           tr("Adjust %1").arg(eqNames[effect]),
                           bandMenu, true, tr("Deck %1").arg(deck));

                control = "button_parameter%1";
                addControl(group, control.arg(effect+1),
                           tr("Kill %1").arg(eqNames[effect]),
                           tr("Kill %1").arg(eqNames[effect]),
                           bandMenu, false, tr("Deck %1").arg(deck));
            }
        }
    }

    // Vinyl Control
    QMenu* vinylControlMenu = addSubmenu(tr("Vinyl Control"));
    addDeckControl("vinylcontrol_enabled", tr("Toggle Vinyl Control"),
                   tr("Toggle Vinyl Control (ON/OFF)"), vinylControlMenu);
    addDeckControl("vinylcontrol_mode", tr("Vinyl Control Mode"),
                   tr("Toggle vinyl-control mode (ABS/REL/CONST)"), vinylControlMenu);
    addDeckControl("vinylcontrol_cueing", tr("Vinyl Control Cueing Mode"),
                   tr("Toggle vinyl-control cueing mode (OFF/ONE/HOT)"), vinylControlMenu);
    addDeckControl("passthrough", tr("Vinyl Control Passthrough"),
                   tr("Pass through external audio into the internal mixer"), vinylControlMenu);
    addControl(VINYL_PREF_KEY, "Toggle", tr("Vinyl Control Next Deck"),
               tr("Single deck mode - Switch vinyl control to next deck"), vinylControlMenu);

    // Cues
    QMenu* cueMenu = addSubmenu(tr("Cues"));
    addDeckControl("cue_default", tr("Cue"), tr("Cue button"), cueMenu);
    addDeckControl("cue_set", tr("Set Cue"), tr("Set cue point"), cueMenu);
    addDeckControl("cue_goto", tr("Go-To Cue"), tr("Go to cue point"), cueMenu);
    addDeckAndSamplerAndPreviewDeckControl("cue_gotoandplay", tr("Go-To Cue And Play"),
                   tr("Go to cue point and play"), cueMenu);
    addDeckControl("cue_gotoandstop", tr("Go-To Cue And Stop"),
                   tr("Go to cue point and stop"), cueMenu);
    addDeckControl("cue_preview", tr("Preview Cue"),
                   tr("Preview from cue point"), cueMenu);
    addDeckControl("cue_cdj", tr("Cue (CDJ Mode)"),
                   tr("Cue button (CDJ mode)"), cueMenu);
    addDeckControl("play_stutter", tr("Stutter Cue"),
                   tr("Stutter cue"), cueMenu);
    addDeckControl("cue_play", tr("CUP (Cue + Play)"),
                       tr("Go to cue point and play after release"), cueMenu);

    // Hotcues
    QMenu* hotcueMenu = addSubmenu(tr("Hotcues"));
    QString hotcueActivateTitle = tr("Hotcue %1");
    QString hotcueClearTitle = tr("Clear Hotcue %1");
    QString hotcueSetTitle = tr("Set Hotcue %1");
    QString hotcueGotoTitle = tr("Jump To Hotcue %1");
    QString hotcueGotoAndStopTitle = tr("Jump To Hotcue %1 And Stop");
    QString hotcueGotoAndPlayTitle = tr("Jump To Hotcue %1 And Play");
    QString hotcuePreviewTitle = tr("Preview Hotcue %1");
    QString hotcueActivateDescription = tr("Set, preview from or jump to hotcue %1");
    QString hotcueClearDescription = tr("Clear hotcue %1");
    QString hotcueSetDescription = tr("Set hotcue %1");
    QString hotcueGotoDescription = tr("Jump to hotcue %1");
    QString hotcueGotoAndStopDescription = tr("Jump to hotcue %1 and stop");
    QString hotcueGotoAndPlayDescription = tr("Jump to hotcue %1 and play");
    QString hotcuePreviewDescription = tr("Preview from hotcue %1");
    for (int i = 1; i <= NUM_HOT_CUES; ++i) {
        QMenu* hotcueSubMenu = addSubmenu(tr("Hotcue %1").arg(QString::number(i)), hotcueMenu);
        addDeckAndSamplerControl(QString("hotcue_%1_activate").arg(i),
                                 hotcueActivateTitle.arg(QString::number(i)),
                                 hotcueActivateDescription.arg(QString::number(i)),
                                 hotcueSubMenu);
        addDeckAndSamplerControl(QString("hotcue_%1_clear").arg(i),
                                 hotcueClearTitle.arg(QString::number(i)),
                                 hotcueClearDescription.arg(QString::number(i)),
                                 hotcueSubMenu);
        addDeckAndSamplerControl(QString("hotcue_%1_set").arg(i),
                                 hotcueSetTitle.arg(QString::number(i)),
                                 hotcueSetDescription.arg(QString::number(i)),
                                 hotcueSubMenu);
        addDeckAndSamplerControl(QString("hotcue_%1_goto").arg(i),
                                 hotcueGotoTitle.arg(QString::number(i)),
                                 hotcueGotoDescription.arg(QString::number(i)),
                                 hotcueSubMenu);
        addDeckAndSamplerControl(QString("hotcue_%1_gotoandstop").arg(i),
                                 hotcueGotoAndStopTitle.arg(QString::number(i)),
                                 hotcueGotoAndStopDescription.arg(QString::number(i)),
                                 hotcueSubMenu);
        addDeckAndSamplerControl(QString("hotcue_%1_gotoandplay").arg(i),
                                 hotcueGotoAndPlayTitle.arg(QString::number(i)),
                                 hotcueGotoAndPlayDescription.arg(QString::number(i)),
                                 hotcueSubMenu);
        addDeckAndSamplerControl(QString("hotcue_%1_activate_preview").arg(i),
                                 hotcuePreviewTitle.arg(QString::number(i)),
                                 hotcuePreviewDescription.arg(QString::number(i)),
                                 hotcueSubMenu);
    }

    // Loops
    QMenu* loopMenu = addSubmenu(tr("Looping"));
    // add beatloop_activate and beatlooproll_activate to both the
    // Loop and Beat-Loop menus to make sure users can find them.
    QString beatloopActivateTitle = tr("Loop Selected Beats");
    QString beatloopActivateDescription = tr("Create a beat loop of selected beat size");
    QString beatloopRollActivateTitle = tr("Loop Roll Selected Beats");
    QString beatloopRollActivateDescription = tr("Create a rolling beat loop of selected beat size");

    addDeckControl("loop_in", tr("Loop In"), tr("Loop In button"), loopMenu);
    addDeckControl("loop_out", tr("Loop Out"), tr("Loop Out button"), loopMenu);
    addDeckControl("loop_exit", tr("Loop Exit"), tr("Loop Exit button"), loopMenu);
    addDeckControl("reloop_toggle", tr("Reloop/Exit Loop"), tr("Toggle loop on/off and jump to Loop In point if loop is behind play position"), loopMenu);
    addDeckControl("reloop_andstop", tr("Reloop And Stop"), tr("Enable loop, jump to Loop In point, and stop"), loopMenu);
    addDeckControl("loop_halve", tr("Loop Halve"), tr("Halve the loop length"), loopMenu);
    addDeckControl("loop_double", tr("Loop Double"), tr("Double the loop length"), loopMenu);
    addDeckControl("beatloop_activate", beatloopActivateTitle, beatloopActivateDescription, loopMenu);
    addDeckControl("beatlooproll_activate", beatloopRollActivateTitle, beatloopRollActivateDescription, loopMenu);

    QList<double> beatSizes = LoopingControl::getBeatSizes();

    QMap<double, QString> humanBeatSizes;
    humanBeatSizes[0.03125] = tr("1/32");
    humanBeatSizes[0.0625] = tr("1/16");
    humanBeatSizes[0.125] = tr("1/8");
    humanBeatSizes[0.25] = tr("1/4");
    humanBeatSizes[0.5] = tr("1/2");
    humanBeatSizes[1] = tr("1");
    humanBeatSizes[2] = tr("2");
    humanBeatSizes[4] = tr("4");
    humanBeatSizes[8] = tr("8");
    humanBeatSizes[16] = tr("16");
    humanBeatSizes[32] = tr("32");
    humanBeatSizes[64] = tr("64");

    // Loop moving
    QString loopMoveForwardTitle = tr("Move Loop +%1 Beats");
    QString loopMoveBackwardTitle = tr("Move Loop -%1 Beats");
    QString loopMoveForwardDescription = tr("Move loop forward by %1 beats");
    QString loopMoveBackwardDescription = tr("Move loop backward by %1 beats");

    foreach (double beats, beatSizes) {
        QString humanBeats = humanBeatSizes.value(beats, QString::number(beats));
        addDeckControl(QString("loop_move_%1_forward").arg(beats),
                       loopMoveForwardTitle.arg(humanBeats),
                       loopMoveForwardDescription.arg(humanBeats), loopMenu);
    }

    foreach (double beats, beatSizes) {
        QString humanBeats = humanBeatSizes.value(beats, QString::number(beats));
        addDeckControl(QString("loop_move_%1_backward").arg(beats),
                       loopMoveBackwardTitle.arg(humanBeats),
                       loopMoveBackwardDescription.arg(humanBeats), loopMenu);
    }

    // Beatloops
    QMenu* beatLoopMenu = addSubmenu(tr("Beat-Looping"));
    QString beatLoopTitle = tr("Loop %1 Beats");
    QString beatLoopRollTitle = tr("Loop Roll %1 Beats");
    QString beatLoopDescription = tr("Create %1-beat loop");
    QString beatLoopRollDescription = tr("Create temporary %1-beat loop roll");

    addDeckControl("beatloop_activate", beatloopActivateTitle, beatloopActivateDescription, beatLoopMenu);
    addDeckControl("beatlooproll_activate", beatloopRollActivateTitle, beatloopRollActivateDescription, beatLoopMenu);
    foreach (double beats, beatSizes) {
        QString humanBeats = humanBeatSizes.value(beats, QString::number(beats));
        addDeckControl(QString("beatloop_%1_toggle").arg(beats),
                       beatLoopTitle.arg(humanBeats),
                       beatLoopDescription.arg(humanBeats), beatLoopMenu);
    }

    foreach (double beats, beatSizes) {
        QString humanBeats = humanBeatSizes.value(beats, QString::number(beats));
        addDeckControl(QString("beatlooproll_%1_activate").arg(beats),
                       beatLoopRollTitle.arg(humanBeats),
                       beatLoopRollDescription.arg(humanBeats), beatLoopMenu);
    }

    // Beat jumping
    QMenu* beatJumpMenu = addSubmenu(tr("Beat Jump / Loop Move"));
    QString beatJumpForwardTitle = tr("Jump / Move Loop Forward %1 Beats");
    QString beatJumpBackwardTitle = tr("Jump / Move Loop Backward %1 Beats");
    QString beatJumpForwardDescription = tr("Jump forward by %1 beats, or if a loop is enabled, move the loop forward %1 beats");
    QString beatJumpBackwardDescription = tr("Jump backward by %1 beats, or if a loop is enabled, move the loop backward %1 beats");
    addDeckControl("beatjump_forward", tr("Beat Jump / Loop Move Forward Selected Beats"), tr("Jump forward by the selected number of beats, or if a loop is enabled, move the loop forward by the selected number of beats"), beatJumpMenu);
    addDeckControl("beatjump_backward", tr("Beat Jump / Loop Move Backward Selected Beats"), tr("Jump backward by the selected number of beats, or if a loop is enabled, move the loop backward by the selected number of beats"), beatJumpMenu);

    foreach (double beats, beatSizes) {
        QString humanBeats = humanBeatSizes.value(beats, QString::number(beats));
        addDeckControl(QString("beatjump_%1_forward").arg(beats),
                       beatJumpForwardTitle.arg(humanBeats),
                       beatJumpForwardDescription.arg(humanBeats), beatJumpMenu);
    }

    foreach (double beats, beatSizes) {
        QString humanBeats = humanBeatSizes.value(beats, QString::number(beats));
        addDeckControl(QString("beatjump_%1_backward").arg(beats),
                       beatJumpBackwardTitle.arg(humanBeats),
                       beatJumpBackwardDescription.arg(humanBeats), beatJumpMenu);
    }

    // Library Controls
    QMenu* libraryMenu = addSubmenu(tr("Library"));
    addControl("[Library]", "MoveUp",
               tr("Move up"),
               tr("Equivalent to pressing the UP key on the keyboard"),
               libraryMenu, false, m_libraryStr);
    addControl("[Library]", "MoveDown",
               tr("Move down"),
               tr("Equivalent to pressing the DOWN key on the keyboard"),
               libraryMenu, false, m_libraryStr);
    addControl("[Library]", "MoveVertical",
               tr("Move up/down"),
               tr("Move vertically in either direction using a knob, as if pressing UP/DOWN keys"),
               libraryMenu, false, m_libraryStr);
    addControl("[Library]", "ScrollUp",
               tr("Scroll Up"),
               tr("Equivalent to pressing the PAGE UP key on the keyboard"),
               libraryMenu, false, m_libraryStr);
    addControl("[Library]", "ScrollDown",
               tr("Scroll Down"),
               tr("Equivalent to pressing the PAGE DOWN key on the keyboard"),
               libraryMenu, false, m_libraryStr);
    addControl("[Library]", "ScrollVertical",
               tr("Scroll up/down"),
               tr("Scroll vertically in either direction using a knob, as if pressing PGUP/PGDOWN keys"),
               libraryMenu, false, m_libraryStr);
    addControl("[Library]", "MoveLeft",
               tr("Move left"),
               tr("Equivalent to pressing the LEFT key on the keyboard"),
               libraryMenu, false, m_libraryStr);
    addControl("[Library]", "MoveRight",
               tr("Move right"),
               tr("Equivalent to pressing the RIGHT key on the keyboard"),
               libraryMenu, false, m_libraryStr);
    addControl("[Library]", "MoveHorizontal",
               tr("Move left/right"),
               tr("Move horizontally in either direction using a knob, as if pressing LEFT/RIGHT keys"),
               libraryMenu, false, m_libraryStr);
    addControl("[Library]", "MoveFocusForward",
               tr("Move focus to right pane"),
               tr("Equivalent to pressing the TAB key on the keyboard"),
               libraryMenu, false, m_libraryStr);
    addControl("[Library]", "MoveFocusBackward",
               tr("Move focus to left pane"),
               tr("Equivalent to pressing the SHIFT+TAB key on the keyboard"),
               libraryMenu, false, m_libraryStr);
    addControl("[Library]", "MoveFocus",
               tr("Move focus to right/left pane"),
               tr("Move focus one pane to right or left using a knob, as if pressing TAB/SHIFT+TAB keys"),
               libraryMenu, false, m_libraryStr);
    addControl("[Library]", "GoToItem",
               tr("Go to the currently selected item"),
               tr("Choose the currently selected item and advance forward one pane if appropriate"),
               libraryMenu, false, m_libraryStr);
    addControl("[Library]", "AutoDjAddBottom",
               tr("Add to Auto DJ Queue (bottom)"),
               tr("Append the selected track to the Auto DJ Queue"),
               libraryMenu, false, m_libraryStr);
    addControl("[Library]", "AutoDjAddTop",
               tr("Add to Auto DJ Queue (top)"),
               tr("Prepend selected track to the Auto DJ Queue"),
               libraryMenu, false, m_libraryStr);
    addControl("[Library]", "AutoDjAddReplace",
               tr("Add to Auto DJ Queue (replace)"),
               tr("Replace Auto DJ Queue with selected tracks"),
               libraryMenu, false, m_libraryStr);


    // Load track (these can be loaded into any channel)
    addDeckAndSamplerControl("LoadSelectedTrack",
                             tr("Load Track"),
                             tr("Load selected track"), libraryMenu);
    addDeckAndSamplerAndPreviewDeckControl("LoadSelectedTrackAndPlay", tr("Track Load and Play"),
                                           tr("Load selected track and play"), libraryMenu);

    addControl("[Recording]", "toggle_recording",
               tr("Record Mix"),
               tr("Toggle mix recording"),
               libraryMenu, false, m_libraryStr);

    // Effect Controls
    QMenu* effectsMenu = addSubmenu(tr("Effects"));

    // Quick Effect Rack COs
    QMenu* quickEffectMenu = addSubmenu(tr("Quick Effects"), effectsMenu);
    for (int i = 1; i <= iNumDecks; ++i) {
        addControl(QString("[QuickEffectRack1_[Channel%1]]").arg(i), "super1",
                   tr("Deck %1 Quick Effect Super Knob").arg(i),
                   tr("Quick Effect Super Knob (control linked effect parameters)"),
                   quickEffectMenu, false, tr("Quick Effect"));
        addControl(QString("[QuickEffectRack1_[Channel%1]_Effect1]").arg(i), "enabled",
                   tr("Deck %1 Quick Effect Enable Button").arg(i),
                   tr("Quick Effect Enable Button"),
                   quickEffectMenu, false, tr("Quick Effect"));
    }

    const int kNumEffectRacks = 1;
    for (int iRackNumber = 1; iRackNumber <= kNumEffectRacks; ++iRackNumber) {
        const QString rackGroup = StandardEffectRack::formatGroupString(
                iRackNumber - 1);
        QMenu* rackMenu = addSubmenu(m_effectRackStr.arg(iRackNumber), effectsMenu);
        QString descriptionPrefix = m_effectRackStr.arg(iRackNumber);

        addControl(rackGroup, "clear",
                   tr("Clear Effect Rack"), tr("Clear effect rack"),
                   rackMenu, false, descriptionPrefix);

        const int numEffectUnits = static_cast<int>(ControlObject::get(
                ConfigKey(rackGroup, "num_effectunits")));
        for (int iEffectUnitNumber = 1; iEffectUnitNumber <= numEffectUnits;
             ++iEffectUnitNumber) {
            const QString effectUnitGroup =
                    StandardEffectRack::formatEffectChainSlotGroupString(
                        iRackNumber - 1, iEffectUnitNumber - 1);

            descriptionPrefix = QString("%1, %2").arg(m_effectRackStr.arg(iRackNumber),
                                                      m_effectUnitStr.arg(iEffectUnitNumber));

            QMenu* effectUnitMenu = addSubmenu(m_effectUnitStr.arg(iEffectUnitNumber),
                                               rackMenu);
            addControl(effectUnitGroup, "clear",
                       tr("Clear Unit"),
                       tr("Clear effect unit"),
                       effectUnitMenu, false, descriptionPrefix);
            addControl(effectUnitGroup, "enabled",
                       tr("Toggle Unit"),
                       tr("Enable or disable effect processing"),
                       effectUnitMenu, false, descriptionPrefix);
            addControl(effectUnitGroup, "mix",
                       tr("Dry/Wet"),
                       tr("Adjust the balance between the original (dry) and processed (wet) signal."),
                       effectUnitMenu, true, descriptionPrefix);
            addControl(effectUnitGroup, "super1",
                       tr("Super Knob"),
                       tr("Super Knob (control effects' Meta Knobs)"),
                       effectUnitMenu, true, descriptionPrefix);
            addControl(effectUnitGroup, "Mix Mode",
                       tr("Mix Mode Toggle"),
                       tr("Toggle effect unit between D/W and D+W modes"),
                       effectUnitMenu, false, descriptionPrefix);
            addControl(effectUnitGroup, "next_chain",
                       tr("Next Chain"),
                       tr("Next chain preset"),
                       effectUnitMenu, false, descriptionPrefix);
            addControl(effectUnitGroup, "prev_chain",
                       tr("Previous Chain"),
                       tr("Previous chain preset"),
                       effectUnitMenu, false, descriptionPrefix);
            addControl(effectUnitGroup, "chain_selector",
                       tr("Next/Previous Chain"),
                       tr("Next or previous chain preset"),
                       effectUnitMenu, false, descriptionPrefix);
            addControl(effectUnitGroup, "show_parameters",
                       tr("Show Effect Parameters"),
                       tr("Show Effect Parameters"),
                       effectUnitMenu, false, descriptionPrefix);

            QString enableOn = tr("Toggle Effect Unit");
            QMenu* effectUnitGroups = addSubmenu(enableOn,
                                                 effectUnitMenu);

            QString groupDescriptionPrefix = QString("%1, %2 %3").arg(
                    m_effectRackStr.arg(iRackNumber),
                    m_effectUnitStr.arg(iEffectUnitNumber),
                    enableOn);

            addControl(effectUnitGroup, "group_[Master]_enable",
                       m_effectMasterOutputStr,
                       m_effectMasterOutputStr,
                       effectUnitGroups, false, groupDescriptionPrefix);
            addControl(effectUnitGroup, "group_[Headphone]_enable",
                       m_effectHeadphoneOutputStr,
                       m_effectHeadphoneOutputStr,
                       effectUnitGroups, false, groupDescriptionPrefix);

            for (int iDeckNumber = 1; iDeckNumber <= iNumDecks; ++iDeckNumber) {
                // PlayerManager::groupForDeck is 0-indexed.
                QString playerGroup = PlayerManager::groupForDeck(iDeckNumber - 1);
                // TODO(owen): Fix bad i18n here.
                addControl(effectUnitGroup,
                           QString("group_%1_enable").arg(playerGroup),
                           tr("Assign ") + m_deckStr.arg(iDeckNumber),
                           tr("Assign ") + m_deckStr.arg(iDeckNumber),
                           effectUnitGroups, false, groupDescriptionPrefix);
            }

            const int iNumSamplers = static_cast<int>(ControlObject::get(
                    ConfigKey("[Master]", "num_samplers")));
            for (int iSamplerNumber = 1; iSamplerNumber <= iNumSamplers;
                 ++iSamplerNumber) {
                // PlayerManager::groupForSampler is 0-indexed.
                QString playerGroup = PlayerManager::groupForSampler(iSamplerNumber - 1);
                // TODO(owen): Fix bad i18n here.
                addControl(effectUnitGroup,
                           QString("group_%1_enable").arg(playerGroup),
                           tr("Assign ") + m_samplerStr.arg(iSamplerNumber),
                           tr("Assign ") + m_samplerStr.arg(iSamplerNumber),
                           effectUnitGroups, false, groupDescriptionPrefix);

            }

            const int iNumMicrophones = static_cast<int>(ControlObject::get(
                    ConfigKey("[Master]", "num_microphones")));
            for (int iMicrophoneNumber = 1; iMicrophoneNumber <= iNumMicrophones;
                 ++iMicrophoneNumber) {
                QString micGroup = PlayerManager::groupForMicrophone(iMicrophoneNumber - 1);
                // TODO(owen): Fix bad i18n here.
                addControl(effectUnitGroup,
                           QString("group_%1_enable").arg(micGroup),
                           tr("Assign ") + m_microphoneStr.arg(iMicrophoneNumber),
                           tr("Assign ") + m_microphoneStr.arg(iMicrophoneNumber),
                           effectUnitGroups, false, groupDescriptionPrefix);
            }

            const int iNumAuxiliaries = static_cast<int>(ControlObject::get(
                    ConfigKey("[Master]", "num_auxiliaries")));
            for (int iAuxiliaryNumber = 1; iAuxiliaryNumber <= iNumAuxiliaries;
                 ++iAuxiliaryNumber) {
                QString auxGroup = PlayerManager::groupForAuxiliary(iAuxiliaryNumber - 1);
                // TODO(owen): Fix bad i18n here.
                addControl(effectUnitGroup,
                           QString("group_%1_enable").arg(auxGroup),
                           tr("Assign ") + m_auxStr.arg(iAuxiliaryNumber),
                           tr("Assign ") + m_auxStr.arg(iAuxiliaryNumber),
                           effectUnitGroups, false, groupDescriptionPrefix);
            }

            const int numEffectSlots = static_cast<int>(ControlObject::get(
                    ConfigKey(effectUnitGroup, "num_effectslots")));
            for (int iEffectSlotNumber = 1; iEffectSlotNumber <= numEffectSlots;
                     ++iEffectSlotNumber) {
                const QString effectSlotGroup =
                        StandardEffectRack::formatEffectSlotGroupString(
                            iRackNumber - 1, iEffectUnitNumber - 1,
                            iEffectSlotNumber - 1);

                QMenu* effectSlotMenu = addSubmenu(m_effectStr.arg(iEffectSlotNumber),
                                                   effectUnitMenu);

                QString slotDescriptionPrefix =
                        QString("%1, %2").arg(descriptionPrefix,
                                              m_effectStr.arg(iEffectSlotNumber));

                addControl(effectSlotGroup, "clear",
                           tr("Clear"), tr("Clear the current effect"),
                           effectSlotMenu, false, slotDescriptionPrefix);
                addControl(effectSlotGroup, "meta",
                           tr("Meta Knob"), tr("Effect Meta Knob (control linked effect parameters)"),
                           effectSlotMenu, false, slotDescriptionPrefix);
                addControl(effectSlotGroup, "enabled",
                           tr("Toggle"), tr("Toggle the current effect"),
                           effectSlotMenu, false, slotDescriptionPrefix);
                addControl(effectSlotGroup, "next_effect",
                           tr("Next"), tr("Switch to next effect"),
                           effectSlotMenu, false, slotDescriptionPrefix);
                addControl(effectSlotGroup, "prev_effect",
                           tr("Previous"), tr("Switch to the previous effect"),
                           effectSlotMenu, false, slotDescriptionPrefix);
                addControl(effectSlotGroup, "effect_selector",
                           tr("Next or Previous"),
                           tr("Switch to either next or previous effect"),
                           effectSlotMenu, false, slotDescriptionPrefix);

                const int numParameterSlots = static_cast<int>(ControlObject::get(
                        ConfigKey(effectSlotGroup, "num_parameterslots")));
                for (int iParameterSlotNumber = 1; iParameterSlotNumber <= numParameterSlots;
                     ++iParameterSlotNumber) {
                    // The parameter slot group is the same as the effect slot
                    // group on a standard effect rack.
                    const QString parameterSlotGroup =
                            StandardEffectRack::formatEffectSlotGroupString(
                                iRackNumber - 1, iEffectUnitNumber - 1,
                                iEffectSlotNumber - 1);
                    const QString parameterSlotItemPrefix = EffectParameterSlot::formatItemPrefix(
                            iParameterSlotNumber - 1);
                    QMenu* parameterSlotMenu = addSubmenu(
                        m_parameterStr.arg(iParameterSlotNumber),
                        effectSlotMenu);

                    QString parameterDescriptionPrefix =
                            QString("%1, %2").arg(slotDescriptionPrefix,
                                                  m_parameterStr.arg(iParameterSlotNumber));

                    // Likely to change soon.
                    addControl(parameterSlotGroup, parameterSlotItemPrefix,
                               tr("Parameter Value"),
                               tr("Parameter Value"),
                               parameterSlotMenu, true,
                               parameterDescriptionPrefix);
                    addControl(parameterSlotGroup, parameterSlotItemPrefix + "_link_type",
                               tr("Meta Knob Mode"),
                               tr("Set how linked effect parameters change when turning the Meta Knob."),
                               parameterSlotMenu, false,
                               parameterDescriptionPrefix);
                    addControl(parameterSlotGroup, parameterSlotItemPrefix + "_link_inverse",
                               tr("Meta Knob Mode Invert"),
                               tr("Invert how linked effect parameters change when turning the Meta Knob."),
                               parameterSlotMenu, false,
                               parameterDescriptionPrefix);

                }
            }
        }
    }

    // Microphone Controls
    QMenu* microphoneMenu = addSubmenu(tr("Microphone / Auxiliary"));

    addMicrophoneAndAuxControl("talkover",
                               tr("Microphone On/Off"),
                               tr("Microphone on/off"), microphoneMenu,
                               true, false);
    addControl("[Master]", "duckStrength",
               tr("Microphone Ducking Strength"),
               tr("Microphone Ducking Strength"),
               microphoneMenu, true);
    addControl("[Master]", "talkoverDucking",
               tr("Microphone Ducking Mode"),
               tr("Toggle microphone ducking mode (OFF, AUTO, MANUAL)"),
               microphoneMenu);
    addMicrophoneAndAuxControl("passthrough",
                               tr("Auxiliary On/Off"),
                               tr("Auxiliary on/off"),
                               microphoneMenu,
                               false, true);
    addMicrophoneAndAuxControl("pregain",
                               tr("Gain"),
                               tr("Gain knob"), microphoneMenu,
                               true, true, true);
    addMicrophoneAndAuxControl("volume",
                               tr("Volume Fader"),
                               tr("Volume Fader"), microphoneMenu,
                               true, true, true);
    addMicrophoneAndAuxControl("volume_set_one",
                               tr("Full Volume"),
                               tr("Set to full volume"), microphoneMenu,
                               true, true);
    addMicrophoneAndAuxControl("volume_set_zero",
                               tr("Zero Volume"),
                               tr("Set to zero volume"), microphoneMenu,
                               true, true);
    addMicrophoneAndAuxControl("mute",
                               tr("Mute"),
                               tr("Mute button"), microphoneMenu,
                               true, true);
    addMicrophoneAndAuxControl("orientation",
                               tr("Orientation"),
                               tr("Mix orientation (e.g. left, right, center)"),
                               microphoneMenu, true, true);
    addMicrophoneAndAuxControl("orientation_left",
                               tr("Orient Left"),
                               tr("Set mix orientation to left"),
                               microphoneMenu,
                               true, true);
    addMicrophoneAndAuxControl("orientation_center",
                               tr("Orient Center"),
                               tr("Set mix orientation to center"), microphoneMenu,
                               true, true);
    addMicrophoneAndAuxControl("orientation_right",
                               tr("Orient Right"),
                               tr("Set mix orientation to right"), microphoneMenu,
                               true, true);
    addMicrophoneAndAuxControl("pfl",
                               tr("Headphone Listen"),
                               tr("Headphone listen button"), microphoneMenu,
                               true, true);

    // AutoDJ Controls
    QMenu* autodjMenu = addSubmenu(tr("Auto DJ"));
    addControl("[AutoDJ]", "shuffle_playlist",
               tr("Auto DJ Shuffle"),
               tr("Shuffle the content of the Auto DJ queue"), autodjMenu);
    addControl("[AutoDJ]", "skip_next",
               tr("Auto DJ Skip Next"),
               tr("Skip the next track in the Auto DJ queue"), autodjMenu);
    addControl("[AutoDJ]", "fade_now",
               tr("Auto DJ Fade To Next"),
               tr("Trigger the transition to the next track"), autodjMenu);
    addControl("[AutoDJ]", "enabled",
               tr("Auto DJ Toggle"),
               tr("Toggle Auto DJ On/Off"), autodjMenu);

    // Skin Controls
    QMenu* guiMenu = addSubmenu(tr("User Interface"));
    addControl("[Samplers]", "show_samplers",
               tr("Samplers Show/Hide"),
               tr("Show/hide the sampler section"), guiMenu);
    addControl("[Microphone]", "show_microphone",
               tr("Microphone Show/Hide"),
               tr("Show/hide the microphone section"), guiMenu);
    addControl(VINYL_PREF_KEY, "show_vinylcontrol",
               tr("Vinyl Control Show/Hide"),
               tr("Show/hide the vinyl control section"), guiMenu);
    addControl("[PreviewDeck]", "show_previewdeck",
               tr("Preview Deck Show/Hide"),
               tr("Show/hide the preview deck"), guiMenu);
    addControl("[EffectRack1]", "show",
               tr("Effect Rack Show/Hide"),
               tr("Show/hide the effect rack"), guiMenu);
    addControl("[Library]", "show_coverart",
               tr("Cover Art Show/Hide"),
               tr("Show/hide cover art"), guiMenu);
    addControl("[Master]", "maximize_library",
               tr("Library Maximize/Restore"),
               tr("Maximize the track library to take up all the available screen space."), guiMenu);

    QString spinnyTitle = tr("Vinyl Spinner Show/Hide");
    QString spinnyDescription = tr("Show/hide spinning vinyl widget");
    for (int i = 1; i <= iNumDecks; ++i) {
        addControl(QString("[Spinny%1]").arg(i), "show_spinny",
                   QString("%1: %2").arg(m_deckStr.arg(i), spinnyTitle),
                   QString("%1: %2").arg(m_deckStr.arg(i), spinnyDescription), guiMenu);

    }

    addDeckControl("waveform_zoom", tr("Waveform Zoom"), tr("Waveform zoom"), guiMenu);
    addDeckControl("waveform_zoom_down", tr("Waveform Zoom In"), tr("Zoom waveform in"), guiMenu);
    addDeckControl("waveform_zoom_up", tr("Waveform Zoom Out"), tr("Zoom waveform out"), guiMenu);

    // Controls to change a deck's star rating
    addDeckAndPreviewDeckControl("stars_up", tr("Star Rating Up"),
        tr("Increase the track rating by one star"), guiMenu);
    addDeckAndPreviewDeckControl("stars_down", tr("Star Rating Down"),
        tr("Decrease the track rating by one star"), guiMenu);
}

ControlPickerMenu::~ControlPickerMenu() {
}

void ControlPickerMenu::addSingleControl(QString group, QString control,
                                         QString title, QString description,
                                         QMenu* pMenu, QString prefix,
                                         QString actionTitle) {
    int controlIndex;

    if (prefix.isEmpty()) {
        controlIndex = addAvailableControl(ConfigKey(group, control), title, description);
    } else {
        QString prefixedTitle = QString("%1: %2").arg(prefix, title);
        QString prefixedDescription = QString("%1: %2").arg(prefix, description);
        controlIndex = addAvailableControl(ConfigKey(group, control), prefixedTitle, prefixedDescription);
    }

    if (actionTitle.isEmpty()) {
        actionTitle = title;
    }

    auto pAction = make_parented<QAction>(actionTitle, pMenu);
    connect(pAction, &QAction::triggered,
            this, [this, controlIndex] { controlChosen(controlIndex); });
    pMenu->addAction(pAction);
}

void ControlPickerMenu::addControl(QString group, QString control, QString title,
                                   QString description,
                                   QMenu* pMenu,
                                   bool addReset,
                                   QString prefix) {
    addSingleControl(group, control, title, description, pMenu, prefix);

    if (addReset) {
        QString resetTitle = QString("%1 (%2)").arg(title, m_resetStr);
        QString resetDescription = QString("%1 (%2)").arg(description, m_resetStr);
        QString resetControl = QString("%1_set_default").arg(control);

        addSingleControl(group, resetControl, resetTitle, resetDescription, pMenu, prefix);
    }
}

void ControlPickerMenu::addPlayerControl(QString control, QString controlTitle,
                                         QString controlDescription,
                                         QMenu* pMenu,
                                         bool deckControls, bool samplerControls,
                                         bool previewdeckControls,
                                         bool addReset) {
    const int iNumSamplers = static_cast<int>(
            ControlObject::get(ConfigKey("[Master]", "num_samplers")));
    const int iNumDecks = static_cast<int>(ControlObject::get(ConfigKey("[Master]", "num_decks")));
    const int iNumPreviewDecks = static_cast<int>(
            ControlObject::get(ConfigKey("[Master]", "num_preview_decks")));

    parented_ptr<QMenu> controlMenu = make_parented<QMenu>(controlTitle, pMenu);
    pMenu->addMenu(controlMenu);

    parented_ptr<QMenu> resetControlMenu = nullptr;
    QString resetControl = QString("%1_set_default").arg(control);
    if (addReset) {
        QString resetMenuTitle = QString("%1 (%2)").arg(controlTitle, m_resetStr);
        resetControlMenu = make_parented<QMenu>(resetMenuTitle, pMenu);
        pMenu->addMenu(resetControlMenu);
    }

    for (int i = 1; deckControls && i <= iNumDecks; ++i) {
        // PlayerManager::groupForDeck is 0-indexed.
        QString prefix = m_deckStr.arg(i);
        QString group = PlayerManager::groupForDeck(i - 1);
        addSingleControl(group, control, controlTitle, controlDescription,
                         controlMenu, prefix, prefix);

        if (resetControlMenu) {
            QString resetTitle = QString("%1 (%2)").arg(controlTitle, m_resetStr);
            QString resetDescription = QString("%1 (%2)").arg(controlDescription, m_resetStr);
            addSingleControl(group, resetControl, resetTitle, resetDescription,
                    resetControlMenu, prefix, prefix);
        }
    }

    for (int i = 1; previewdeckControls && i <= iNumPreviewDecks; ++i) {
        // PlayerManager::groupForPreviewDeck is 0-indexed.
        QString prefix = m_previewdeckStr.arg(i);
        QString group = PlayerManager::groupForPreviewDeck(i - 1);
        addSingleControl(group, control, controlTitle, controlDescription,
                         controlMenu, prefix, prefix);

        if (resetControlMenu) {
            QString resetTitle = QString("%1 (%2)").arg(controlTitle, m_resetStr);
            QString resetDescription = QString("%1 (%2)").arg(controlDescription, m_resetStr);
            addSingleControl(group, resetControl, resetTitle, resetDescription,
                    resetControlMenu, prefix, prefix);
        }
    }

    for (int i = 1; samplerControls && i <= iNumSamplers; ++i) {
        // PlayerManager::groupForSampler is 0-indexed.
        QString prefix = m_samplerStr.arg(i);
        QString group = PlayerManager::groupForSampler(i - 1);
        addSingleControl(group, control, controlTitle, controlDescription,
                         controlMenu, prefix, prefix);

        if (resetControlMenu) {
            QString resetTitle = QString("%1 (%2)").arg(controlTitle, m_resetStr);
            QString resetDescription = QString("%1 (%2)").arg(controlDescription, m_resetStr);
            addSingleControl(group, resetControl, resetTitle, resetDescription,
                    resetControlMenu, prefix, prefix);
        }
    }
}

void ControlPickerMenu::addMicrophoneAndAuxControl(QString control,
                                                   QString controlTitle,
                                                   QString controlDescription,
                                                   QMenu* pMenu,
                                                   bool microphoneControls,
                                                   bool auxControls,
                                                   bool addReset) {
    parented_ptr<QMenu> controlMenu = make_parented<QMenu>(controlTitle, pMenu);
    pMenu->addMenu(controlMenu);

    parented_ptr<QMenu> resetControlMenu = nullptr;
    QString resetControl = QString("%1_set_default").arg(control);
    if (addReset) {
        QString resetHelpText = QString("%1 (%2)").arg(controlTitle, m_resetStr);
        resetControlMenu = make_parented<QMenu>(resetHelpText, pMenu);
        pMenu->addMenu(resetControlMenu);
    }

    if (microphoneControls) {
        const int kNumMicrophones = static_cast<int>(
                ControlObject::get(ConfigKey("[Master]", "num_microphones")));
        for (int i = 1; i <= kNumMicrophones; ++i) {
            QString prefix = m_microphoneStr.arg(i);
            QString group = PlayerManager::groupForMicrophone(i - 1);
            addSingleControl(group, control, controlTitle, controlDescription,
                             controlMenu, prefix, prefix);

            if (resetControlMenu) {
                QString resetTitle = QString("%1 (%2)").arg(controlTitle, m_resetStr);
                QString resetDescription = QString("%1 (%2)").arg(controlDescription, m_resetStr);
                addSingleControl(group, resetControl, resetTitle, resetDescription,
                        resetControlMenu, prefix, prefix);
            }
        }
    }

    const int kNumAuxiliaries = static_cast<int>(
            ControlObject::get(ConfigKey("[Master]", "num_auxiliaries")));
    if (auxControls) {
        for (int i = 1; i <= kNumAuxiliaries; ++i) {
            QString prefix = m_auxStr.arg(i);
            QString group = PlayerManager::groupForAuxiliary(i - 1);
            addSingleControl(group, control, controlTitle, controlDescription,
                             controlMenu, prefix, prefix);

            if (resetControlMenu) {
                QString resetTitle = QString("%1 (%2)").arg(controlTitle, m_resetStr);
                QString resetDescription = QString("%1 (%2)").arg(controlDescription, m_resetStr);
                addSingleControl(group, resetControl, resetTitle, resetDescription,
                        resetControlMenu, prefix, prefix);
            }
        }
    }
}

void ControlPickerMenu::addDeckAndSamplerControl(QString control,
                                                 QString title,
                                                 QString controlDescription,
                                                 QMenu* pMenu,
                                                 bool addReset) {
    addPlayerControl(control, title, controlDescription, pMenu, true, true, false, addReset);
}

void ControlPickerMenu::addDeckAndPreviewDeckControl(QString control,
                                                     QString title,
                                                     QString controlDescription,
                                                     QMenu* pMenu,
                                                     bool addReset) {
    addPlayerControl(control, title, controlDescription, pMenu, true, false, true, addReset);
}

void ControlPickerMenu::addDeckAndSamplerAndPreviewDeckControl(QString control,
                                                               QString title,
                                                               QString controlDescription,
                                                               QMenu* pMenu,
                                                               bool addReset) {
    addPlayerControl(control, title, controlDescription, pMenu, true, true, true, addReset);
}

void ControlPickerMenu::addDeckControl(QString control,
                                       QString title,
                                       QString controlDescription,
                                       QMenu* pMenu,
                                       bool addReset) {
    addPlayerControl(control, title, controlDescription, pMenu, true, false, false, addReset);
}

void ControlPickerMenu::addSamplerControl(QString control,
                                          QString title,
                                          QString controlDescription,
                                          QMenu* pMenu,
                                          bool addReset) {
    addPlayerControl(control, title, controlDescription, pMenu, false, true, false, addReset);
}

void ControlPickerMenu::addPreviewDeckControl(QString control,
                                              QString title,
                                              QString controlDescription,
                                              QMenu* pMenu,
                                              bool addReset) {
    addPlayerControl(control, title, controlDescription, pMenu, false, false, true, addReset);
}

QMenu* ControlPickerMenu::addSubmenu(QString title, QMenu* pParent) {
    if (pParent == NULL) {
        pParent = this;
    }
    auto subMenu = make_parented<QMenu>(title, pParent);
    pParent->addMenu(subMenu);
    return subMenu;
}

void ControlPickerMenu::controlChosen(int controlIndex) {
    if (controlIndex < 0 || controlIndex >= m_controlsAvailable.size()) {
        return;
    }
    emit controlPicked(m_controlsAvailable[controlIndex]);
}

int ControlPickerMenu::addAvailableControl(ConfigKey key,
                                           QString title,
                                           QString description) {
    m_controlsAvailable.append(key);
    m_descriptionsByKey.insert(key, description);
    m_titlesByKey.insert(key, title);
    // return the index of the control which will be connected to the index
    // of the respective action in the menu
    return m_controlsAvailable.size() - 1;
}

bool ControlPickerMenu::controlExists(ConfigKey key) const {
    return m_titlesByKey.contains(key);
}

QString ControlPickerMenu::descriptionForConfigKey(ConfigKey key) const {
    return m_descriptionsByKey.value(key, QString());
}

QString ControlPickerMenu::controlTitleForConfigKey(ConfigKey key) const {
    return m_titlesByKey.value(key, QString());
}
