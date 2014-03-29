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

    m_masterOutputStr = tr("Master Output");
    m_headphoneOutputStr = tr("Headphone Output");
    m_deckStr = tr("Deck %1");
    m_samplerStr = tr("Sampler %1");
    m_previewdeckStr = tr("Preview Deck %1");
    m_microphoneStr = tr("Microphone %1");
    m_auxStr = tr("Auxiliary %1");
    m_resetStr = tr("Reset to default");
    m_effectRackStr = tr("Effect Rack %1");
    m_effectUnitStr = tr("Effect Unit %1");
    m_effectStr = tr("Effect Slot %1");
    m_parameterStr = tr("Parameter %1");

    // Master Controls
    QMenu* mixerMenu = addSubmenu(tr("Mixer"));
    addControl("[Master]", "crossfader", tr("Crossfader"), mixerMenu, true);
    addControl("[Master]", "volume", tr("Master volume"), mixerMenu, true);
    addControl("[Master]", "balance", tr("Master balance"), mixerMenu, true);
    addControl("[Master]", "delay", tr("Master delay"), mixerMenu, true);
    addControl("[Master]", "headVolume", tr("Headphone volume"), mixerMenu, true);
    addControl("[Master]", "headMix", tr("Headphone mix (pre/main)"), mixerMenu, true);
    addControl("[Master]", "headSplit", tr("Toggle headphone split cueing"), mixerMenu);
    addControl("[Master]", "headDelay", tr("Headphone delay"), mixerMenu, true);

    // Transport
    QMenu* transportMenu = addSubmenu(tr("Transport"));
    addDeckAndSamplerAndPreviewDeckControl("playposition", tr("Strip-search through track"), transportMenu);
    addDeckAndSamplerAndPreviewDeckControl("play", tr("Play button"), transportMenu);
    // Preview deck does not go to master so volume does not matter.
    addDeckAndSamplerControl("volume", tr("Volume fader"), transportMenu, true);
    addDeckAndSamplerControl("volume_set_one", tr("Set to full volume"), transportMenu);
    addDeckAndSamplerControl("volume_set_zero", tr("Set to zero volume"), transportMenu);
    addDeckAndSamplerAndPreviewDeckControl("back", tr("Fast rewind button"), transportMenu);
    addDeckAndSamplerAndPreviewDeckControl("fwd", tr("Fast forward button"), transportMenu);
    addDeckAndSamplerAndPreviewDeckControl("stop", tr("Stop button"), transportMenu);
    addDeckAndSamplerAndPreviewDeckControl("start", tr("Jump to start of track"), transportMenu);
    addDeckAndSamplerAndPreviewDeckControl("start_play", tr("Jump to start of track and play"), transportMenu);
    addDeckAndSamplerAndPreviewDeckControl("start_stop", tr("Jump to start of track and stop"), transportMenu);

    addDeckAndSamplerAndPreviewDeckControl("end", tr("Jump to end of track"), transportMenu);
    addDeckAndSamplerAndPreviewDeckControl("reverse", tr("Play reverse button"), transportMenu);
    addDeckAndSamplerAndPreviewDeckControl("reverseroll", tr("Reverse roll (Censor) button"), transportMenu);
    addDeckAndSamplerAndPreviewDeckControl("pregain", tr("Gain knob"), transportMenu, true);
    addDeckAndSamplerControl("pfl", tr("Headphone listen button"), transportMenu);
    addDeckAndSamplerControl("mute", tr("Mute button"), transportMenu);
    addDeckAndSamplerControl("repeat", tr("Toggle repeat mode"), transportMenu);
    addDeckAndSamplerAndPreviewDeckControl("eject", tr("Eject track"), transportMenu);
    addDeckAndSamplerControl("orientation", tr("Mix orientation (e.g. left, right, center)"), transportMenu);
    addDeckAndSamplerControl("orientation_left", tr("Set mix orientation to left"), transportMenu);
    addDeckAndSamplerControl("orientation_center", tr("Set mix orientation to center"), transportMenu);
    addDeckAndSamplerControl("orientation_right", tr("Set mix orientation to right"), transportMenu);
    addDeckAndSamplerControl("slip_enabled", tr("Toggle slip mode"), transportMenu);

    // BPM & Sync
    QMenu* bpmMenu = addSubmenu(tr("BPM and Sync"));
    addDeckAndSamplerControl("bpm", tr("BPM"), bpmMenu, true);
    addDeckAndSamplerControl("bpm_up", tr("Increase BPM by 1"), bpmMenu);
    addDeckAndSamplerControl("bpm_down", tr("Decrease BPM by 1"), bpmMenu);
    addDeckAndSamplerControl("bpm_up_small", tr("Increase BPM by 0.1"), bpmMenu);
    addDeckAndSamplerControl("bpm_down_small", tr("Decrease BPM by 0.1"), bpmMenu);
    addDeckAndSamplerControl("bpm_tap", tr("BPM tap button"), bpmMenu);
    addDeckControl("beats_translate_curpos", tr("Adjust beatgrid"), bpmMenu);
    addDeckAndSamplerControl("quantize", tr("Toggle quantize mode"), bpmMenu);
    addDeckAndSamplerControl("sync_enabled", tr("Sync button. Tap to sync, hold to enable sync mode"), bpmMenu);
    addControl("[InternalClock]", "sync_master", tr("Toggle internal sync master"), bpmMenu);
    addControl("[InternalClock]", "bpm", tr("Internal master BPM"), bpmMenu);
    addControl("[InternalClock]", "bpm_up", tr("Increase internal master BPM by 1"), bpmMenu);

    addControl("[InternalClock]", "bpm_down", tr("Decrease internal master BPM by 1"), bpmMenu);

    addControl("[InternalClock]", "bpm_up_small", tr("Increase internal master BPM by 0.1"), bpmMenu);
    addControl("[InternalClock]", "bpm_down_small", tr("Decrease internal master BPM by 0.1"), bpmMenu);
    addDeckAndSamplerControl("sync_master", tr("Toggle sync master"), bpmMenu);
    addDeckAndSamplerControl("sync_mode", tr("Sync mode 3-state toggle (OFF, FOLLOWER, MASTER)"), bpmMenu);
    addDeckAndSamplerControl("beatsync", tr("One-time beat sync (tempo and phase)"), bpmMenu);
    addDeckAndSamplerControl("beatsync_tempo", tr("One-time beat sync (tempo only)"), bpmMenu);
    addDeckAndSamplerControl("beatsync_phase", tr("One-time beat sync (phase only)"), bpmMenu);

    // Rate
    QMenu* rateMenu = addSubmenu(tr("Pitch and Rate"));
    addDeckAndSamplerControl("keylock", tr("Toggle keylock mode"), rateMenu);
    addDeckAndSamplerControl("rate", tr("Speed control slider"), rateMenu, true);
    addDeckAndSamplerControl("pitch", tr("Pitch control slider"), rateMenu, true);
    addDeckAndSamplerControl("sync_key", tr("Sync key"), rateMenu, true);
    addDeckAndSamplerControl("rate_perm_up", tr("Adjust rate up (coarse)"), rateMenu);
    addDeckAndSamplerControl("rate_perm_up_small", tr("Adjust rate up (fine)"), rateMenu);
    addDeckAndSamplerControl("rate_perm_down", tr("Adjust rate down (coarse)"), rateMenu);
    addDeckAndSamplerControl("rate_perm_down_small", tr("Adjust rate down (fine)"), rateMenu);
    addDeckAndSamplerControl("rate_temp_up", tr("Pitch-bend rate up (coarse)"), rateMenu);
    addDeckAndSamplerControl("rate_temp_up_small", tr("Pitch-bend rate up (fine)"), rateMenu);
    addDeckAndSamplerControl("rate_temp_down", tr("Pitch-bend rate down (coarse)"), rateMenu);
    addDeckAndSamplerControl("rate_temp_down_small", tr("Pitch-bend rate down (fine)"), rateMenu);

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
    addDeckControl("cue_goto", tr("Go to cue point"), cueMenu);
    addDeckControl("cue_gotoandplay", tr("Go to cue point and play"), cueMenu);
    addDeckControl("cue_gotoandstop", tr("Go to cue point and stop"), cueMenu);
    addDeckControl("cue_preview", tr("Preview from cue point"), cueMenu);
    addDeckControl("cue_cdj", tr("Cue button (CDJ mode)"), cueMenu);
    addDeckControl("play_stutter", tr("Stutter cue"), cueMenu);

    // Hotcues
    QMenu* hotcueMenu = addSubmenu(tr("Hotcues"));
    QString hotcueActivate = tr("Set, preview from or jump to hotcue %1");
    QString hotcueClear = tr("Clear hotcue %1");
    QString hotcueSet = tr("Set hotcue %1");
    QString hotcueGoto = tr("Jump to hotcue %1");
    QString hotcueGotoAndStop = tr("Jump to hotcue %1 and stop");
    QString hotcueGotoAndPlay = tr("Jump to hotcue %1 and play");
    QString hotcuePreview = tr("Preview from hotcue %1");
    for (int i = 1; i <= NUM_HOT_CUES; ++i) {
        QMenu* hotcueSubMenu = addSubmenu(tr("Hotcue %1").arg(QString::number(i)), hotcueMenu);
        addDeckAndSamplerControl(QString("hotcue_%1_activate").arg(i),
                                 hotcueActivate.arg(QString::number(i)), hotcueSubMenu);
        addDeckAndSamplerControl(QString("hotcue_%1_clear").arg(i),
                                 hotcueClear.arg(QString::number(i)), hotcueSubMenu);
        addDeckAndSamplerControl(QString("hotcue_%1_set").arg(i),
                                 hotcueSet.arg(QString::number(i)), hotcueSubMenu);
        addDeckAndSamplerControl(QString("hotcue_%1_goto").arg(i),
                                 hotcueGoto.arg(QString::number(i)), hotcueSubMenu);
        addDeckAndSamplerControl(QString("hotcue_%1_gotoandstop").arg(i),
                                 hotcueGotoAndStop.arg(QString::number(i)), hotcueSubMenu);
        addDeckAndSamplerControl(QString("hotcue_%1_gotoandplay").arg(i),
                                 hotcueGotoAndPlay.arg(QString::number(i)), hotcueSubMenu);
        addDeckAndSamplerControl(QString("hotcue_%1_activate_preview").arg(i),
                                 hotcuePreview.arg(QString::number(i)), hotcueSubMenu);
    }

    // Loops
    QMenu* loopMenu = addSubmenu(tr("Looping"));
    addDeckControl("loop_in", tr("Loop In button"), loopMenu);
    addDeckControl("loop_out", tr("Loop Out button"), loopMenu);
    addDeckControl("loop_exit", tr("Loop Exit button"), loopMenu);
    addDeckControl("reloop_exit", tr("Reloop / Exit button"), loopMenu);
    addDeckControl("loop_halve", tr("Halve the current loop's length"), loopMenu);
    addDeckControl("loop_double", tr("Double the current loop's length"), loopMenu);

    QList<double> beatSizes = LoopingControl::getBeatSizes();

    QMap<double, QString> humanBeatSizes;
    humanBeatSizes[0.03125] = tr("1/32th");
    humanBeatSizes[0.0625] = tr("1/16th");
    humanBeatSizes[0.125] = tr("1/8th");
    humanBeatSizes[0.25] = tr("1/4th");
    humanBeatSizes[0.5] = tr("1/2");
    humanBeatSizes[1] = tr("1");
    humanBeatSizes[2] = tr("2");
    humanBeatSizes[4] = tr("4");
    humanBeatSizes[8] = tr("8");
    humanBeatSizes[16] = tr("16");
    humanBeatSizes[32] = tr("32");
    humanBeatSizes[64] = tr("64");

    // Loop moving
    QString loopMoveForward = tr("Move loop forward by %1 beats");
    QString loopMoveBackward = tr("Move loop backward by %1 beats");

    foreach (double beats, beatSizes) {
        QString humanBeats = humanBeatSizes.value(beats, QString::number(beats));
        addDeckControl(QString("loop_move_%1_forward").arg(beats),
                       loopMoveForward.arg(humanBeats), loopMenu);
    }

    foreach (double beats, beatSizes) {
        QString humanBeats = humanBeatSizes.value(beats, QString::number(beats));
        addDeckControl(QString("loop_move_%1_backward").arg(beats),
                       loopMoveBackward.arg(humanBeats), loopMenu);
    }

    // Beatloops
    QMenu* beatLoopMenu = addSubmenu(tr("Beat-Looping"));
    QString beatLoop = tr("Create %1-beat loop");
    QString beatLoopRoll = tr("Create temporary %1-beat loop roll");

    foreach (double beats, beatSizes) {
        QString humanBeats = humanBeatSizes.value(beats, QString::number(beats));
        addDeckControl(QString("beatloop_%1_toggle").arg(beats),
                       beatLoop.arg(humanBeats), beatLoopMenu);
    }

    foreach (double beats, beatSizes) {
        QString humanBeats = humanBeatSizes.value(beats, QString::number(beats));
        addDeckControl(QString("beatlooproll_%1_activate").arg(beats),
                       beatLoopRoll.arg(humanBeats), beatLoopMenu);
    }

    // Beat jumping
    QMenu* beatJumpMenu = addSubmenu(tr("Beat-Jump"));
    QString beatJumpForward = tr("Jump forward by %1 beats");
    QString beatJumpBackward = tr("Jump backward by %1 beats");

    foreach (double beats, beatSizes) {
        QString humanBeats = humanBeatSizes.value(beats, QString::number(beats));
        addDeckControl(QString("beatjump_%1_forward").arg(beats),
                       beatJumpForward.arg(humanBeats), beatJumpMenu);
    }

    foreach (double beats, beatSizes) {
        QString humanBeats = humanBeatSizes.value(beats, QString::number(beats));
        addDeckControl(QString("beatjump_%1_backward").arg(beats),
                       beatJumpBackward.arg(humanBeats), beatJumpMenu);
    }

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

    addControl("[Recording]", "toggle_recording", tr("Toggle mix recording"), libraryMenu);

    // Effect Controls
    QMenu* effectsMenu = addSubmenu(tr("Effects"));

    const int kNumEffectRacks = 1;
    for (int iRackNumber = 1; iRackNumber <= kNumEffectRacks; ++iRackNumber) {
        const QString rackGroup = EffectRack::formatGroupString(
                iRackNumber - 1);
        QMenu* rackMenu = addSubmenu(m_effectRackStr.arg(iRackNumber), effectsMenu);
        QString descriptionPrefix = m_effectRackStr.arg(iRackNumber);

        addEffectControl(rackGroup, "clear", tr("Clear effect rack"),
                         descriptionPrefix, rackMenu);

        const int numEffectUnits = ControlObject::get(
            ConfigKey(rackGroup, "num_effectunits"));
        for (int iEffectUnitNumber = 1; iEffectUnitNumber <= numEffectUnits;
             ++iEffectUnitNumber) {
            const QString effectUnitGroup = EffectChainSlot::formatGroupString(
                    iRackNumber - 1, iEffectUnitNumber - 1);

            descriptionPrefix = QString("%1, %2").arg(m_effectRackStr.arg(iRackNumber),
                                                      m_effectUnitStr.arg(iEffectUnitNumber));

            QMenu* effectUnitMenu = addSubmenu(m_effectUnitStr.arg(iEffectUnitNumber),
                                               rackMenu);
            addEffectControl(effectUnitGroup, "clear",
                             tr("Clear effect unit"), descriptionPrefix,
                             effectUnitMenu);
            addEffectControl(effectUnitGroup, "enabled",
                             tr("Toggle effect unit"), descriptionPrefix,
                             effectUnitMenu, false);
            addEffectControl(effectUnitGroup, "mix",
                             tr("Dry/Wet"), descriptionPrefix,
                             effectUnitMenu, true);
            addEffectControl(effectUnitGroup, "parameter",
                             tr("Super Knob (Control Linked Effect Parameters)"),
                             descriptionPrefix,
                             effectUnitMenu, true);
            addEffectControl(effectUnitGroup, "insertion_type",
                             tr("Insert / Send Toggle"), descriptionPrefix,
                             effectUnitMenu);
            addEffectControl(effectUnitGroup, "next_chain",
                             tr("Next chain preset"), descriptionPrefix,
                             effectUnitMenu);
            addEffectControl(effectUnitGroup, "prev_chain",
                             tr("Previous chain preset"), descriptionPrefix,
                             effectUnitMenu);
            addEffectControl(effectUnitGroup, "chain_selector",
                             tr("Next or previous chain preset"), descriptionPrefix,
                             effectUnitMenu);

            QString enableOn = tr("Toggle Effect Unit");
            QMenu* effectUnitGroups = addSubmenu(enableOn,
                                                 effectUnitMenu);

            QString groupDescriptionPrefix = QString("%1, %2 %3").arg(
                    m_effectRackStr.arg(iRackNumber),
                    m_effectUnitStr.arg(iEffectUnitNumber),
                    enableOn);

            addEffectControl(effectUnitGroup, "group_[Master]_enable",
                             m_masterOutputStr, groupDescriptionPrefix,
                             effectUnitGroups);
            addEffectControl(effectUnitGroup, "group_[Headphone]_enable",
                             m_headphoneOutputStr, groupDescriptionPrefix,
                             effectUnitGroups);

            const int iNumDecks = ControlObject::get(
                ConfigKey("[Master]", "num_decks"));
            for (int iDeckNumber = 1; iDeckNumber <= iNumDecks; ++iDeckNumber) {
                // PlayerManager::groupForDeck is 0-indexed.
                QString playerGroup = PlayerManager::groupForDeck(iDeckNumber - 1);
                addEffectControl(effectUnitGroup,
                                 QString("group_%1_enable").arg(playerGroup),
                                 m_deckStr.arg(iDeckNumber),
                                 groupDescriptionPrefix,
                                 effectUnitGroups);

            }

            const int iNumSamplers = ControlObject::get(
                ConfigKey("[Master]", "num_samplers"));
            for (int iSamplerNumber = 1; iSamplerNumber <= iNumSamplers;
                 ++iSamplerNumber) {
                // PlayerManager::groupForSampler is 0-indexed.
                QString playerGroup = PlayerManager::groupForSampler(iSamplerNumber - 1);
                addEffectControl(effectUnitGroup,
                                 QString("group_%1_enable").arg(playerGroup),
                                 m_samplerStr.arg(iSamplerNumber),
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
                addEffectControl(effectUnitGroup,
                                 QString("group_%1_enable").arg(micGroup),
                                 m_microphoneStr.arg(iMicrophoneNumber),
                                 groupDescriptionPrefix,
                                 effectUnitGroups);
            }

            const int iNumAuxiliaries = ControlObject::get(
                ConfigKey("[Master]", "num_auxiliaries"));
            for (int iAuxiliaryNumber = 1; iAuxiliaryNumber <= iNumAuxiliaries;
                 ++iAuxiliaryNumber) {
                QString auxGroup = QString("[Auxiliary%1]").arg(iAuxiliaryNumber);
                addEffectControl(effectUnitGroup,
                                 QString("group_%1_enable").arg(auxGroup),
                                 m_auxStr.arg(iAuxiliaryNumber),
                                 groupDescriptionPrefix,
                                 effectUnitGroups);
            }

            const int numEffectSlots = ControlObject::get(
                    ConfigKey(effectUnitGroup, "num_effectslots"));
            for (int iEffectSlotNumber = 1; iEffectSlotNumber <= numEffectSlots;
                     ++iEffectSlotNumber) {
                const QString effectSlotGroup = EffectSlot::formatGroupString(
                        iRackNumber - 1, iEffectUnitNumber - 1,
                        iEffectSlotNumber - 1);

                QMenu* effectSlotMenu = addSubmenu(m_effectStr.arg(iEffectSlotNumber),
                                                   effectUnitMenu);

                descriptionPrefix.append(QString(", %1").arg(
                    m_effectStr.arg(iEffectSlotNumber)));

                addEffectControl(effectSlotGroup, "clear",
                                 tr("Clear effect"), descriptionPrefix,
                                 effectSlotMenu);
                addEffectControl(effectSlotGroup, "enabled",
                                 tr("Toggle effect"), descriptionPrefix,
                                 effectSlotMenu);
                addEffectControl(effectSlotGroup, "next_effect",
                                 tr("Next effect"), descriptionPrefix,
                                 effectSlotMenu);
                addEffectControl(effectSlotGroup, "prev_effect",
                                 tr("Previous effect"), descriptionPrefix,
                                 effectSlotMenu);
                addEffectControl(effectSlotGroup, "effect_selector",
                                 tr("Next or previous effect"), descriptionPrefix,
                                 effectSlotMenu);

                const int numParameterSlots = ControlObject::get(
                        ConfigKey(effectSlotGroup, "num_parameterslots"));
                for (int iParameterSlotNumber = 1; iParameterSlotNumber <= numParameterSlots;
                     ++iParameterSlotNumber) {
                    const QString parameterSlotGroup = EffectParameterSlot::formatGroupString(
                            iRackNumber - 1, iEffectUnitNumber - 1,
                            iEffectSlotNumber - 1);
                    const QString parameterSlotItemPrefix = EffectParameterSlot::formatItemPrefix(
                            iParameterSlotNumber - 1);
                    QMenu* parameterSlotMenu = addSubmenu(
                        m_parameterStr.arg(iParameterSlotNumber),
                        effectSlotMenu);

                    descriptionPrefix.append(QString(", %1").arg(
                        m_parameterStr.arg(iParameterSlotNumber)));

                    // Likely to change soon.
                    addEffectControl(parameterSlotGroup, parameterSlotItemPrefix,
                                     tr("Parameter value"), descriptionPrefix,
                                     parameterSlotMenu, true);

                    addEffectControl(parameterSlotGroup, parameterSlotItemPrefix + "_link_type",
                                     tr("3-state Super Knob Link Toggle (unlinked, linear, inverse)"),
                                     descriptionPrefix, parameterSlotMenu);

                }
            }
        }
    }

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
    addControl("[Master]", "duckStrength",
               tr("Microphone ducking strength"), microphoneMenu, true);
    addControl("[Master]", "talkoverDucking",
               tr("Toggle microphone ducking mode (OFF, AUTO, MANUAL)"),
               microphoneMenu);
    addMicrophoneAndAuxControl("passthrough", tr("Auxiliary on/off"), microphoneMenu,
                               false, true);
    addMicrophoneAndAuxControl("volume", tr("Volume fader"), microphoneMenu,
                               true, true, true);
    addMicrophoneAndAuxControl("volume_set_one", tr("Set to full volume"), microphoneMenu,
                               true, true);
    addMicrophoneAndAuxControl("volume_set_zero", tr("Set to zero volume"), microphoneMenu,
                               true, true);
    addMicrophoneAndAuxControl("mute", tr("Mute button"), microphoneMenu,
                               true, true);
    addMicrophoneAndAuxControl("orientation", tr("Mix orientation (e.g. left, right, center)"),
                               microphoneMenu, true, true);
    addMicrophoneAndAuxControl("orientation_left", tr("Set mix orientation to left"), microphoneMenu,
                               true, true);
    addMicrophoneAndAuxControl("orientation_center", tr("Set mix orientation to center"), microphoneMenu,
                               true, true);
    addMicrophoneAndAuxControl("orientation_right", tr("Set mix orientation to right"), microphoneMenu,
                               true, true);
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
    addControl(VINYL_PREF_KEY, "show_vinylcontrol", tr("Show/hide the vinyl control section"), guiMenu);
    addControl("[PreviewDeck]", "show_previewdeck", tr("Show/hide the preview deck"), guiMenu);

    const int iNumDecks = ControlObject::get(ConfigKey("[Master]", "num_decks"));
    QString spinnyText = tr("Show/hide spinning vinyl widget");
    for (int i = 1; i <= iNumDecks; ++i) {
        addControl(QString("[Spinny%1]").arg(i), "show_spinny",
                   QString("%1: %2").arg(m_deckStr.arg(i), spinnyText), guiMenu);

    }

    addDeckControl("waveform_zoom", tr("Waveform zoom"), guiMenu);
    addDeckControl("waveform_zoom_down", tr("Zoom waveform in"), guiMenu);
    addDeckControl("waveform_zoom_up", tr("Zoom waveform out"), guiMenu);

}

ControlPickerMenu::~ControlPickerMenu() {
}

void ControlPickerMenu::addControl(QString group, QString control, QString description,
                                   QMenu* pMenu,
                                   bool addReset) {
    QAction* pAction = pMenu->addAction(description, &m_actionMapper,
                                        SLOT(map()));
    m_actionMapper.setMapping(pAction, m_controlsAvailable.size());
    addAvailableControl(ConfigKey(group, control), description);

    if (addReset) {
        QString resetDescription = QString("%1 (%2)").arg(description,
                                                          m_resetStr);
        QString resetControl = QString("%1_set_default").arg(control);
        QAction* pResetAction = pMenu->addAction(resetDescription,
                                                 &m_actionMapper, SLOT(map()));
        m_actionMapper.setMapping(pResetAction, m_controlsAvailable.size());
        addAvailableControl(ConfigKey(group, resetControl), resetDescription);
    }
}

void ControlPickerMenu::addEffectControl(QString group, QString control,
                                         QString menuDescription, QString descriptionPrefix,
                                         QMenu* pMenu, bool addReset) {
    QAction* pAction = pMenu->addAction(menuDescription, &m_actionMapper, SLOT(map()));
    m_actionMapper.setMapping(pAction, m_controlsAvailable.size());

    QString description = QString("%1: %2").arg(descriptionPrefix,
                                                menuDescription);
    addAvailableControl(ConfigKey(group, control), description);

    if (addReset) {
        QString resetMenuDescription = QString("%1 (%2)").arg(description, m_resetStr);
        QString resetControl = QString("%1_set_default").arg(control);
        QAction* pResetAction = pMenu->addAction(resetMenuDescription,
                                                 &m_actionMapper, SLOT(map()));
        QString resetDescription = QString("%1: %2").arg(descriptionPrefix,
                                                         resetMenuDescription);
        m_actionMapper.setMapping(pResetAction, m_controlsAvailable.size());
        addAvailableControl(ConfigKey(group, resetControl), resetDescription);
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
        addAvailableControl(ConfigKey(group, control), description);

        if (resetControlMenu) {
            QString resetDescription = QString("%1 (%2)").arg(description, m_resetStr);
            QAction* pResetAction = resetControlMenu->addAction(m_deckStr.arg(i), &m_actionMapper, SLOT(map()));
            m_actionMapper.setMapping(pResetAction, m_controlsAvailable.size());
            addAvailableControl(ConfigKey(group, resetControl), resetDescription);
        }
    }

    for (int i = 1; previewdeckControls && i <= iNumPreviewDecks; ++i) {
        // PlayerManager::groupForPreviewDeck is 0-indexed.
        QString group = PlayerManager::groupForPreviewDeck(i - 1);
        QString description = QString("%1: %2").arg(
            m_previewdeckStr.arg(QString::number(i)), controlDescription);
        QAction* pAction = controlMenu->addAction(m_previewdeckStr.arg(i), &m_actionMapper, SLOT(map()));
        m_actionMapper.setMapping(pAction, m_controlsAvailable.size());
        addAvailableControl(ConfigKey(group, control), description);

        if (resetControlMenu) {
            QString resetDescription = QString("%1 (%2)").arg(description, m_resetStr);
            QAction* pResetAction = resetControlMenu->addAction(m_previewdeckStr.arg(i), &m_actionMapper, SLOT(map()));
            m_actionMapper.setMapping(pResetAction, m_controlsAvailable.size());
            addAvailableControl(ConfigKey(group, resetControl), resetDescription);
        }
    }

    for (int i = 1; samplerControls && i <= iNumSamplers; ++i) {
        // PlayerManager::groupForSampler is 0-indexed.
        QString group = PlayerManager::groupForSampler(i - 1);
        QString description = QString("%1: %2").arg(
            m_samplerStr.arg(QString::number(i)), controlDescription);
        QAction* pAction = controlMenu->addAction(m_samplerStr.arg(i), &m_actionMapper, SLOT(map()));
        m_actionMapper.setMapping(pAction, m_controlsAvailable.size());
        addAvailableControl(ConfigKey(group, control), description);

        if (resetControlMenu) {
            QString resetDescription = QString("%1 (%2)").arg(description, m_resetStr);
            QAction* pResetAction = resetControlMenu->addAction(m_samplerStr.arg(i), &m_actionMapper, SLOT(map()));
            m_actionMapper.setMapping(pResetAction, m_controlsAvailable.size());
            addAvailableControl(ConfigKey(group, resetControl), resetDescription);
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
            addAvailableControl(ConfigKey(group, control), description);

            if (addReset) {
                QString resetDescription = QString("%1 (%2)").arg(description, m_resetStr);
                QAction* pResetAction = resetControlMenu->addAction(
                    m_microphoneStr.arg(i), &m_actionMapper, SLOT(map()));
                m_actionMapper.setMapping(pResetAction, m_controlsAvailable.size());
                addAvailableControl(ConfigKey(group, resetControl), resetDescription);
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
            addAvailableControl(ConfigKey(group, control), description);

            if (addReset) {
                QString resetDescription = QString("%1 (%2)").arg(description, m_resetStr);
                QAction* pResetAction = resetControlMenu->addAction(
                    m_auxStr.arg(i), &m_actionMapper, SLOT(map()));
                m_actionMapper.setMapping(pResetAction, m_controlsAvailable.size());
                addAvailableControl(ConfigKey(group, resetControl), resetDescription);
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

void ControlPickerMenu::addAvailableControl(ConfigKey key,
                                            QString description) {
    m_controlsAvailable.append(key);
    m_descriptionsByKey.insert(key, description);
}

QString ControlPickerMenu::descriptionForConfigKey(ConfigKey key) const {
    return m_descriptionsByKey.value(key, QString());
}
