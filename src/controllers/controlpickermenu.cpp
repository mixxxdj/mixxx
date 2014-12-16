#include "controllers/controlpickermenu.h"

#include "vinylcontrol/defs_vinylcontrol.h"
#include "playermanager.h"
#include "engine/cuecontrol.h"
#include "engine/loopingcontrol.h"
#include "effects/effectrack.h"
#include "effects/effectchainslot.h"
#include "effects/effectslot.h"
#include "effects/effectparameterslot.h"

ControlPickerMenu::ControlPickerMenu(QWidget* pParent)
        : QMenu(pParent) {
    connect(&m_actionMapper, SIGNAL(mapped(int)),
            this, SLOT(controlChosen(int)));

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
    addDeckAndSamplerControl("volume_set_one", tr("Full Volume"), tr("Sets volume to full"), transportMenu);
    addDeckAndSamplerControl("volume_set_zero", tr("Zero Volume"), tr("Sets volume to zero"), transportMenu);
    addDeckAndSamplerAndPreviewDeckControl("pregain", tr("Track Gain"), tr("Track Gain knob"), transportMenu, true);
    addDeckAndSamplerControl("mute", tr("Mute"), tr("Mute button"), transportMenu);
    addDeckAndSamplerAndPreviewDeckControl("eject", tr("Eject"), tr("Eject track"), transportMenu);
    addDeckAndSamplerControl("pfl", tr("Headphone Listen"), tr("Headphone listen (pfl) button"), transportMenu);
    addDeckAndSamplerControl("repeat", tr("Repeat Mode"), tr("Toggle repeat mode"), transportMenu);
    addDeckAndSamplerControl("slip_enabled", tr("Slip Mode"), tr("Toggle slip mode"), transportMenu);
    addDeckAndSamplerControl("orientation", tr("Orientation"),
                             tr("Mix orientation (e.g. left, right, center)"), transportMenu);
    addDeckAndSamplerControl("orientation_left", tr("Orient Left"),
                             tr("Set mix orientation to left"), transportMenu);
    addDeckAndSamplerControl("orientation_center", tr("Orient Center"),
                             tr("Set mix orientation to center"), transportMenu);
    addDeckAndSamplerControl("orientation_right", tr("Orient Right"),
                             tr("Set mix orientation to right"), transportMenu);

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
                             tr("Tap to sync, hold to enable sync mode"), syncMenu);
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
                             tr("One-time beat sync (tempo and phase)"), syncMenu);
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
                             tr("Pitch control (does not affect tempo)"), speedMenu, true);
    addDeckAndSamplerControl("sync_key", tr("Sync Key"), tr("Match musical key"), speedMenu, true);
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
    addDeckControl("filterHigh", tr("High EQ"), tr("High EQ knob"), eqMenu, true);
    addDeckControl("filterMid", tr("Mid EQ"), tr("Mid EQ knob"), eqMenu, true);
    addDeckControl("filterLow", tr("Low EQ"), tr("Low EQ knob"), eqMenu, true);
    addDeckControl("filterHighKill", tr("Kill High EQ"), tr("Kill High EQ"), eqMenu);
    addDeckControl("filterMidKill", tr("Kill Mid EQ"), tr("Kill Mid EQ"), eqMenu);
    addDeckControl("filterLowKill", tr("Kill Low EQ"), tr("Kill Low EQ"), eqMenu);

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
    addDeckControl("cue_gotoandplay", tr("Go-To Cue And Play"),
                   tr("Go to cue point and play"), cueMenu);
    addDeckControl("cue_gotoandstop", tr("Go-To Cue And Stop"),
                   tr("Go to cue point and stop"), cueMenu);
    addDeckControl("cue_preview", tr("Preview Cue"),
                   tr("Preview from cue point"), cueMenu);
    addDeckControl("cue_cdj", tr("Cue (CDJ Mode)"),
                   tr("Cue button (CDJ mode)"), cueMenu);
    addDeckControl("play_stutter", tr("Stutter Cue"),
                   tr("Stutter cue"), cueMenu);

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
    addDeckControl("loop_in", tr("Loop In"), tr("Loop In button"), loopMenu);
    addDeckControl("loop_out", tr("Loop Out"), tr("Loop Out button"), loopMenu);
    addDeckControl("loop_exit", tr("Loop Exit"), tr("Loop Exit button"), loopMenu);
    addDeckControl("reloop_exit", tr("Reloop/Exit Loop"), tr("Reloop/Exit button"), loopMenu);
    addDeckControl("loop_halve", tr("Loop Halve"), tr("Halve the current loop's length"), loopMenu);
    addDeckControl("loop_double", tr("Loop Double"),
                   tr("Double the current loop's length"), loopMenu);

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
    QMenu* beatJumpMenu = addSubmenu(tr("Beat-Jump"));
    QString beatJumpForwardTitle = tr("Jump Forward %1 Beats");
    QString beatJumpBackwardTitle = tr("Jump Back %1 Beats");
    QString beatJumpForwardDescription = tr("Jump forward by %1 beats");
    QString beatJumpBackwardDescription = tr("Jump backward by %1 beats");

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
    addPrefixedControl("[Playlist]", "ToggleSelectedSidebarItem",
                       tr("Expand/Collapse View"),
                       tr("Expand/collapse the selected view (library, playlist..)"),
                       m_libraryStr, libraryMenu);

    addPrefixedControl("[Playlist]", "SelectPlaylist",
                       tr("Switch Next/Previous View"),
                       tr("Switch to the next or previous view (library, playlist..)"),
                       m_libraryStr, libraryMenu);
    addPrefixedControl("[Playlist]", "SelectNextPlaylist",
                       tr("Switch To Next View"),
                       tr("Switch to the next view (library, playlist..)"),
                       m_libraryStr, libraryMenu);
    addPrefixedControl("[Playlist]", "SelectPrevPlaylist",
                       tr("Switch To Previous View"),
                       tr("Switch to the previous view (library, playlist..)"),
                       m_libraryStr, libraryMenu);
    addPrefixedControl("[Playlist]", "SelectTrackKnob",
                       tr("Scroll To Next/Previous Track"),
                       tr("Scroll up or down in library/playlist"),
                       m_libraryStr, libraryMenu);
    addPrefixedControl("[Playlist]", "SelectNextTrack",
                       tr("Scroll To Next Track"),
                       tr("Scroll to next track in library/playlist"),
                       m_libraryStr, libraryMenu);
    addPrefixedControl("[Playlist]", "SelectPrevTrack",
                       tr("Scroll To Previous Track"),
                       tr("Scroll to previous track in library/playlist"),
                       m_libraryStr, libraryMenu);
    addPrefixedControl("[Playlist]", "LoadSelectedIntoFirstStopped",
                       tr("Load Track Into Stopped Deck"),
                       tr("Load selected track into first stopped deck"),
                       m_libraryStr, libraryMenu);
    addDeckAndSamplerControl("LoadSelectedTrack",
                             tr("Load Track"),
                             tr("Load selected track"), libraryMenu);
    addDeckAndSamplerAndPreviewDeckControl("LoadSelectedTrackAndPlay", tr("Track Load and Play"),
                                           tr("Load selected track and play"), libraryMenu);

    addPrefixedControl("[Recording]", "toggle_recording",
                       tr("Record Mix"),
                       tr("Toggle mix recording"),
                       m_libraryStr, libraryMenu);

    // Effect Controls
    QMenu* effectsMenu = addSubmenu(tr("Effects"));

    const int kNumEffectRacks = 1;
    for (int iRackNumber = 1; iRackNumber <= kNumEffectRacks; ++iRackNumber) {
        const QString rackGroup = StandardEffectRack::formatGroupString(
                iRackNumber - 1);
        QMenu* rackMenu = addSubmenu(m_effectRackStr.arg(iRackNumber), effectsMenu);
        QString descriptionPrefix = m_effectRackStr.arg(iRackNumber);

        addPrefixedControl(rackGroup, "clear",
                           tr("Clear Effect Rack"), tr("Clear effect rack"),
                           descriptionPrefix, rackMenu);

        const int numEffectUnits = ControlObject::get(
            ConfigKey(rackGroup, "num_effectunits"));
        for (int iEffectUnitNumber = 1; iEffectUnitNumber <= numEffectUnits;
             ++iEffectUnitNumber) {
            const QString effectUnitGroup =
                    StandardEffectRack::formatEffectChainSlotGroupString(
                        iRackNumber - 1, iEffectUnitNumber - 1);

            descriptionPrefix = QString("%1, %2").arg(m_effectRackStr.arg(iRackNumber),
                                                      m_effectUnitStr.arg(iEffectUnitNumber));

            QMenu* effectUnitMenu = addSubmenu(m_effectUnitStr.arg(iEffectUnitNumber),
                                               rackMenu);
            addPrefixedControl(effectUnitGroup, "clear",
                               tr("Clear Unit"),
                               tr("Clear effect unit"), descriptionPrefix,
                               effectUnitMenu);
            addPrefixedControl(effectUnitGroup, "enabled",
                               tr("Toggle Unit"),
                               tr("Toggle effect unit"), descriptionPrefix,
                               effectUnitMenu, false);
            addPrefixedControl(effectUnitGroup, "mix",
                               tr("Dry/Wet"),
                               tr("Dry/Wet"), descriptionPrefix,
                               effectUnitMenu, true);
            addPrefixedControl(effectUnitGroup, "super1",
                               tr("Super Knob"),
                               tr("Super Knob (control linked effect parameters)"),
                               descriptionPrefix,
                               effectUnitMenu, true);
            addPrefixedControl(effectUnitGroup, "insertion_type",
                               tr("Insert/Send Toggle"),
                               tr("Insert/Send Toggle"),
                               descriptionPrefix,
                               effectUnitMenu);
            addPrefixedControl(effectUnitGroup, "next_chain",
                               tr("Next Chain"),
                               tr("Next chain preset"), descriptionPrefix,
                               effectUnitMenu);
            addPrefixedControl(effectUnitGroup, "prev_chain",
                               tr("Previous Chain"),
                               tr("Previous chain preset"), descriptionPrefix,
                               effectUnitMenu);
            addPrefixedControl(effectUnitGroup, "chain_selector",
                               tr("Next/Previous Chain"),
                               tr("Next or previous chain preset"), descriptionPrefix,
                               effectUnitMenu);

            QString enableOn = tr("Toggle Effect Unit");
            QMenu* effectUnitGroups = addSubmenu(enableOn,
                                                 effectUnitMenu);

            QString groupDescriptionPrefix = QString("%1, %2 %3").arg(
                    m_effectRackStr.arg(iRackNumber),
                    m_effectUnitStr.arg(iEffectUnitNumber),
                    enableOn);

            addPrefixedControl(effectUnitGroup, "group_[Master]_enable",
                               m_effectMasterOutputStr,
                               m_effectMasterOutputStr, groupDescriptionPrefix,
                               effectUnitGroups);
            addPrefixedControl(effectUnitGroup, "group_[Headphone]_enable",
                               m_effectHeadphoneOutputStr,
                               m_effectHeadphoneOutputStr, groupDescriptionPrefix,
                               effectUnitGroups);

            const int iNumDecks = ControlObject::get(
                ConfigKey("[Master]", "num_decks"));
            for (int iDeckNumber = 1; iDeckNumber <= iNumDecks; ++iDeckNumber) {
                // PlayerManager::groupForDeck is 0-indexed.
                QString playerGroup = PlayerManager::groupForDeck(iDeckNumber - 1);
                // TODO(owen): Fix bad i18n here.
                addPrefixedControl(effectUnitGroup,
                                   QString("group_%1_enable").arg(playerGroup),
                                   tr("Assign ") + m_deckStr.arg(iDeckNumber),
                                   tr("Assign ") + m_deckStr.arg(iDeckNumber),
                                   groupDescriptionPrefix,
                                   effectUnitGroups);

            }

            const int iNumSamplers = ControlObject::get(
                ConfigKey("[Master]", "num_samplers"));
            for (int iSamplerNumber = 1; iSamplerNumber <= iNumSamplers;
                 ++iSamplerNumber) {
                // PlayerManager::groupForSampler is 0-indexed.
                QString playerGroup = PlayerManager::groupForSampler(iSamplerNumber - 1);
                // TODO(owen): Fix bad i18n here.
                addPrefixedControl(effectUnitGroup,
                                   QString("group_%1_enable").arg(playerGroup),
                                   tr("Assign ") + m_samplerStr.arg(iSamplerNumber),
                                   tr("Assign ") + m_samplerStr.arg(iSamplerNumber),
                                   groupDescriptionPrefix,
                                   effectUnitGroups);

            }

            const int iNumMicrophones = ControlObject::get(
                ConfigKey("[Master]", "num_microphones"));
            for (int iMicrophoneNumber = 1; iMicrophoneNumber <= iNumMicrophones;
                 ++iMicrophoneNumber) {
                QString micGroup = "[Microphone]";
                if (iMicrophoneNumber > 1) {
                    micGroup = QString("[Microphone%1]").arg(iMicrophoneNumber);
                }
                // TODO(owen): Fix bad i18n here.
                addPrefixedControl(effectUnitGroup,
                                   QString("group_%1_enable").arg(micGroup),
                                   tr("Assign ") + m_microphoneStr.arg(iMicrophoneNumber),
                                   tr("Assign ") + m_microphoneStr.arg(iMicrophoneNumber),
                                   groupDescriptionPrefix,
                                   effectUnitGroups);
            }

            const int iNumAuxiliaries = ControlObject::get(
                ConfigKey("[Master]", "num_auxiliaries"));
            for (int iAuxiliaryNumber = 1; iAuxiliaryNumber <= iNumAuxiliaries;
                 ++iAuxiliaryNumber) {
                QString auxGroup = QString("[Auxiliary%1]").arg(iAuxiliaryNumber);
                // TODO(owen): Fix bad i18n here.
                addPrefixedControl(effectUnitGroup,
                                   QString("group_%1_enable").arg(auxGroup),
                                   tr("Assign ") + m_auxStr.arg(iAuxiliaryNumber),
                                   tr("Assign ") + m_auxStr.arg(iAuxiliaryNumber),
                                   groupDescriptionPrefix,
                                   effectUnitGroups);
            }

            const int numEffectSlots = ControlObject::get(
                    ConfigKey(effectUnitGroup, "num_effectslots"));
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

                addPrefixedControl(effectSlotGroup, "clear",
                                   tr("Clear"), tr("Clear the current effect"),
                                   slotDescriptionPrefix,
                                   effectSlotMenu);
                addPrefixedControl(effectSlotGroup, "enabled",
                                   tr("Toggle"), tr("Toggle the current effect"),
                                   slotDescriptionPrefix,
                                   effectSlotMenu);
                addPrefixedControl(effectSlotGroup, "next_effect",
                                   tr("Next"), tr("Switch to next effect"),
                                   slotDescriptionPrefix,
                                   effectSlotMenu);
                addPrefixedControl(effectSlotGroup, "prev_effect",
                                   tr("Previous"), tr("Switch to the previous effect"),
                                   slotDescriptionPrefix,
                                   effectSlotMenu);
                addPrefixedControl(effectSlotGroup, "effect_selector",
                                   tr("Next or Previous"),
                                   tr("Switch to either next or previous effect"),
                                   slotDescriptionPrefix,
                                   effectSlotMenu);

                const int numParameterSlots = ControlObject::get(
                        ConfigKey(effectSlotGroup, "num_parameterslots"));
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
                    addPrefixedControl(parameterSlotGroup, parameterSlotItemPrefix,
                                       tr("Parameter Value"),
                                       tr("Parameter Value"),
                                       parameterDescriptionPrefix,
                                       parameterSlotMenu, true);

                    addPrefixedControl(parameterSlotGroup, parameterSlotItemPrefix + "_link_type",
                                       tr("Super Knob Mode"),
                                       tr("3-state Super Knob Link Toggle (unlinked, linear, inverse)"),
                                       parameterDescriptionPrefix,
                                       parameterSlotMenu);

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
                               tr("Volume Full"),
                               tr("Set to full volume"), microphoneMenu,
                               true, true);
    addMicrophoneAndAuxControl("volume_set_zero",
                               tr("Volume Zero"),
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
               tr("Shuffle the content of the Auto DJ playlist"),
               autodjMenu);
    addControl("[AutoDJ]", "skip_next",
               tr("Auto DJ Skip Next"),
               tr("Skip the next track in the Auto DJ playlist"), autodjMenu);
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
    addControl("[Library]", "show_coverart",
               tr("Cover Art Show/Hide"),
               tr("Show/hide cover art"), guiMenu);

    const int iNumDecks = ControlObject::get(ConfigKey("[Master]", "num_decks"));
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
}

ControlPickerMenu::~ControlPickerMenu() {
}

void ControlPickerMenu::addControl(QString group, QString control, QString title,
                                   QString description,
                                   QMenu* pMenu,
                                   bool addReset) {
    QAction* pAction = pMenu->addAction(title, &m_actionMapper,
                                        SLOT(map()));
    m_actionMapper.setMapping(pAction, m_controlsAvailable.size());
    addAvailableControl(ConfigKey(group, control), title, description);

    if (addReset) {
        QString resetDescription = QString("%1 (%2)").arg(title,
                                                          m_resetStr);
        QString resetControl = QString("%1_set_default").arg(control);
        QAction* pResetAction = pMenu->addAction(resetDescription,
                                                 &m_actionMapper, SLOT(map()));
        m_actionMapper.setMapping(pResetAction, m_controlsAvailable.size());
        addAvailableControl(ConfigKey(group, resetControl),
                            resetDescription, resetDescription);
    }
}

void ControlPickerMenu::addPrefixedControl(QString group, QString control,
                                           QString controlTitle,
                                           QString controlDescription, QString descriptionPrefix,
                                           QMenu* pMenu, bool addReset) {
    QAction* pAction = pMenu->addAction(controlTitle, &m_actionMapper, SLOT(map()));
    m_actionMapper.setMapping(pAction, m_controlsAvailable.size());

    QString title = QString("%1: %2").arg(descriptionPrefix, controlTitle);
    QString description = QString("%1: %2").arg(descriptionPrefix,
                                                controlDescription);
    addAvailableControl(ConfigKey(group, control), title, description);

    if (addReset) {
        QString resetMenuTitle = QString("%1 (%2)").arg(controlTitle, m_resetStr);
        QString resetMenuDescription = QString("%1 (%2)").arg(controlDescription, m_resetStr);
        QString resetControl = QString("%1_set_default").arg(control);
        QAction* pResetAction = pMenu->addAction(resetMenuTitle,
                                                 &m_actionMapper, SLOT(map()));
        QString resetTitle = QString("%1: %2").arg(descriptionPrefix,
                                                   resetMenuTitle);
        QString resetDescription = QString("%1: %2").arg(descriptionPrefix,
                                                         resetMenuDescription);
        m_actionMapper.setMapping(pResetAction, m_controlsAvailable.size());
        addAvailableControl(ConfigKey(group, resetControl),
                            resetTitle, resetDescription);
    }
}

void ControlPickerMenu::addPlayerControl(QString control, QString controlTitle,
                                         QString controlDescription,
                                         QMenu* pMenu,
                                         bool deckControls, bool samplerControls,
                                         bool previewdeckControls,
                                         bool addReset) {
    const int iNumSamplers = ControlObject::get(ConfigKey("[Master]", "num_samplers"));
    const int iNumDecks = ControlObject::get(ConfigKey("[Master]", "num_decks"));
    const int iNumPreviewDecks = ControlObject::get(ConfigKey("[Master]", "num_preview_decks"));

    QMenu* controlMenu = new QMenu(controlTitle, pMenu);
    pMenu->addMenu(controlMenu);

    QMenu* resetControlMenu = NULL;
    QString resetControl = QString("%1_set_default").arg(control);
    if (addReset) {
        QString resetHelpText = QString("%1 (%2)").arg(controlTitle, m_resetStr);
        resetControlMenu = new QMenu(resetHelpText, pMenu);
        pMenu->addMenu(resetControlMenu);
    }

    for (int i = 1; deckControls && i <= iNumDecks; ++i) {
        // PlayerManager::groupForDeck is 0-indexed.
        QString group = PlayerManager::groupForDeck(i - 1);
        QString title = QString("%1: %2").arg(
            m_deckStr.arg(QString::number(i)), controlTitle);
        QString description = QString("%1: %2").arg(
            m_deckStr.arg(QString::number(i)), controlDescription);
        QAction* pAction = controlMenu->addAction(m_deckStr.arg(i), &m_actionMapper, SLOT(map()));
        m_actionMapper.setMapping(pAction, m_controlsAvailable.size());
        addAvailableControl(ConfigKey(group, control), title, description);

        if (resetControlMenu) {
            QString resetTitle = QString("%1 (%2)").arg(title, m_resetStr);
            QString resetDescription = QString("%1 (%2)").arg(description, m_resetStr);
            QAction* pResetAction = resetControlMenu->addAction(m_deckStr.arg(i), &m_actionMapper, SLOT(map()));
            m_actionMapper.setMapping(pResetAction, m_controlsAvailable.size());
            addAvailableControl(ConfigKey(group, resetControl), resetTitle, resetDescription);
        }
    }

    for (int i = 1; previewdeckControls && i <= iNumPreviewDecks; ++i) {
        // PlayerManager::groupForPreviewDeck is 0-indexed.
        QString group = PlayerManager::groupForPreviewDeck(i - 1);
        QString title = QString("%1: %2").arg(
            m_previewdeckStr.arg(QString::number(i)), controlTitle);
        QString description = QString("%1: %2").arg(
            m_previewdeckStr.arg(QString::number(i)), controlDescription);
        QAction* pAction = controlMenu->addAction(m_previewdeckStr.arg(i), &m_actionMapper, SLOT(map()));
        m_actionMapper.setMapping(pAction, m_controlsAvailable.size());
        addAvailableControl(ConfigKey(group, control), title, description);

        if (resetControlMenu) {
            QString resetTitle = QString("%1 (%2)").arg(title, m_resetStr);
            QString resetDescription = QString("%1 (%2)").arg(description, m_resetStr);
            QAction* pResetAction = resetControlMenu->addAction(m_previewdeckStr.arg(i), &m_actionMapper, SLOT(map()));
            m_actionMapper.setMapping(pResetAction, m_controlsAvailable.size());
            addAvailableControl(ConfigKey(group, resetControl), resetTitle, resetDescription);
        }
    }

    for (int i = 1; samplerControls && i <= iNumSamplers; ++i) {
        // PlayerManager::groupForSampler is 0-indexed.
        QString group = PlayerManager::groupForSampler(i - 1);
        QString title = QString("%1: %2").arg(
            m_samplerStr.arg(QString::number(i)), controlTitle);
        QString description = QString("%1: %2").arg(
            m_samplerStr.arg(QString::number(i)), controlDescription);
        QAction* pAction = controlMenu->addAction(m_samplerStr.arg(i), &m_actionMapper, SLOT(map()));
        m_actionMapper.setMapping(pAction, m_controlsAvailable.size());
        addAvailableControl(ConfigKey(group, control), title, description);

        if (resetControlMenu) {
            QString resetTitle = QString("%1 (%2)").arg(title, m_resetStr);
            QString resetDescription = QString("%1 (%2)").arg(description, m_resetStr);
            QAction* pResetAction = resetControlMenu->addAction(m_samplerStr.arg(i), &m_actionMapper, SLOT(map()));
            m_actionMapper.setMapping(pResetAction, m_controlsAvailable.size());
            addAvailableControl(ConfigKey(group, resetControl), resetTitle, resetDescription);
        }
    }
}

void ControlPickerMenu::addMicrophoneAndAuxControl(QString control,
                                                   QString controlTitle,
                                                   QString controlDescription,
                                                   QMenu* pMenu,
                                                   bool microhoneControls,
                                                   bool auxControls,
                                                   bool addReset) {
    QMenu* controlMenu = new QMenu(controlTitle, pMenu);
    pMenu->addMenu(controlMenu);

    QMenu* resetControlMenu = NULL;
    QString resetControl = QString("%1_set_default").arg(control);
    if (addReset) {
        QString resetHelpText = QString("%1 (%2)").arg(controlTitle, m_resetStr);
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

            QString title = QString("%1: %2").arg(
                m_microphoneStr.arg(QString::number(i)), controlTitle);
            QString description = QString("%1: %2").arg(
                m_microphoneStr.arg(QString::number(i)), controlDescription);
            QAction* pAction = controlMenu->addAction(m_microphoneStr.arg(i),
                                                      &m_actionMapper, SLOT(map()));
            m_actionMapper.setMapping(pAction, m_controlsAvailable.size());
            addAvailableControl(ConfigKey(group, control), title, description);

            if (addReset) {
                QString resetTitle = QString("%1 (%2)").arg(title, m_resetStr);
                QString resetDescription = QString("%1 (%2)").arg(controlDescription, m_resetStr);
                QAction* pResetAction = resetControlMenu->addAction(
                    m_microphoneStr.arg(i), &m_actionMapper, SLOT(map()));
                m_actionMapper.setMapping(pResetAction, m_controlsAvailable.size());
                addAvailableControl(ConfigKey(group, resetControl), resetTitle, resetDescription);
            }
        }
    }

    const int kNumAuxiliaries = ControlObject::get(ConfigKey("[Master]", "num_auxiliaries"));
    if (auxControls) {
        for (int i = 1; i <= kNumAuxiliaries; ++i) {
            QString group = QString("[Auxiliary%1]").arg(i);
            QString title = QString("%1: %2").arg(
                m_auxStr.arg(QString::number(i)), controlTitle);
            QString description = QString("%1: %2").arg(
                m_auxStr.arg(QString::number(i)), controlDescription);
            QAction* pAction = controlMenu->addAction(m_auxStr.arg(i),
                                                      &m_actionMapper, SLOT(map()));
            m_actionMapper.setMapping(pAction, m_controlsAvailable.size());
            addAvailableControl(ConfigKey(group, control), title, description);

            if (addReset) {
                QString resetTitle = QString("%1 (%2)").arg(title, m_resetStr);
                QString resetDescription = QString("%1 (%2)").arg(description, m_resetStr);
                QAction* pResetAction = resetControlMenu->addAction(
                    m_auxStr.arg(i), &m_actionMapper, SLOT(map()));
                m_actionMapper.setMapping(pResetAction, m_controlsAvailable.size());
                addAvailableControl(ConfigKey(group, resetControl), resetTitle, resetDescription);
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

void ControlPickerMenu::addAvailableControl(ConfigKey key,
                                            QString title,
                                            QString description) {
    m_controlsAvailable.append(key);
    m_descriptionsByKey.insert(key, description);
    m_titlesByKey.insert(key, title);
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
