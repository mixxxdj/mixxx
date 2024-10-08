#include "controllers/controlpickermenu.h"

#include "control/controlobject.h"
#include "effects/chains/equalizereffectchain.h"
#include "effects/chains/standardeffectchain.h"
#include "effects/defs.h"
#include "effects/effectbuttonparameterslot.h"
#include "effects/effectknobparameterslot.h"
#include "engine/controls/cuecontrol.h"
#include "engine/controls/loopingcontrol.h"
#include "mixer/playermanager.h"
#include "moc_controlpickermenu.cpp"
#include "recording/defs_recording.h"
#include "vinylcontrol/defs_vinylcontrol.h"

namespace {
const QString kAppGroup = QStringLiteral("[App]");
} // namespace

ControlPickerMenu::ControlPickerMenu(QWidget* pParent)
        : QMenu(pParent) {
    m_effectMainOutputStr = tr("Main Output");
    m_effectHeadphoneOutputStr = tr("Headphone Output");
    m_deckStr = tr("Deck %1");
    m_samplerStr = tr("Sampler %1");
    m_previewdeckStr = tr("Preview Deck %1");
    m_microphoneStr = tr("Microphone %1");
    m_auxStr = tr("Auxiliary %1");
    m_resetStr = tr("Reset to default");
    m_effectRackStr = tr("Effect Rack %1");
    m_effectUnitStr = tr("Effect Unit %1");
    m_effectStr = tr("Slot %1");
    m_parameterStr = tr("Parameter %1");
    m_buttonParameterStr = tr("Button Parameter %1");
    m_libraryStr = tr("Library");

    m_numGroupsTrMap.insert("Channel", m_deckStr);
    m_numGroupsTrMap.insert("Sampler", m_samplerStr);
    m_numGroupsTrMap.insert("PreviewDeck", m_previewdeckStr);
    m_numGroupsTrMap.insert("Microphone", m_microphoneStr);
    m_numGroupsTrMap.insert("Auxiliary", m_auxStr);
    m_numGroupsTrMap.insert("EffectRack", m_effectRackStr);

    m_otherGroupsTrMap.insert("Skin", tr("Skin"));
    m_otherGroupsTrMap.insert("Library", m_libraryStr);
    m_otherGroupsTrMap.insert("Controller", tr("Controller"));
    // TODO(ronso0) "translate" legacy 'Master' to 'Main' in main branch?
    m_otherGroupsTrMap.insert("Master", "Master");

    // Mixer Controls
    QMenu* pMixerMenu = addSubmenu(tr("Mixer"));
    // Crossfader / Orientation
    QMenu* pCrossfaderMenu = addSubmenu(tr("Crossfader / Orientation"), pMixerMenu);
    addControl("[Master]",
            "crossfader",
            tr("Crossfader"),
            tr("Crossfader"),
            pCrossfaderMenu,
            true);
    addDeckAndSamplerControl("orientation",
            tr("Orientation"),
            tr("Mix orientation (e.g. left, right, center)"),
            pCrossfaderMenu);
    addDeckAndSamplerControl("orientation_left",
            tr("Orient Left"),
            tr("Set mix orientation to left"),
            pCrossfaderMenu);
    addDeckAndSamplerControl("orientation_center",
            tr("Orient Center"),
            tr("Set mix orientation to center"),
            pCrossfaderMenu);
    addDeckAndSamplerControl("orientation_right",
            tr("Orient Right"),
            tr("Set mix orientation to right"),
            pCrossfaderMenu);
    // Main Output
    QMenu* pMainOutputMenu = addSubmenu(tr("Main Output"), pMixerMenu);
    addControl("[Master]",
            "gain",
            tr("Main Output Gain"),
            tr("Main Output gain"),
            pMainOutputMenu,
            true);
    addControl("[Master]",
            "balance",
            tr("Main Output Balance"),
            tr("Main Output balance"),
            pMainOutputMenu,
            true);
    addControl("[Master]",
            "delay",
            tr("Main Output Delay"),
            tr("Main Output delay"),
            pMainOutputMenu,
            true);
    // Headphone
    QMenu* pHeadphoneMenu = addSubmenu(tr("Headphone"), pMixerMenu);
    addControl("[Master]",
            "headGain",
            tr("Headphone Gain"),
            tr("Headphone gain"),
            pHeadphoneMenu,
            true);
    addControl("[Master]",
            "headMix",
            tr("Headphone Mix"),
            tr("Headphone mix (pre/main)"),
            pHeadphoneMenu,
            true);
    addControl("[Master]",
            "headSplit",
            tr("Headphone Split Cue"),
            tr("Toggle headphone split cueing"),
            pHeadphoneMenu);
    addControl("[Master]",
            "headDelay",
            tr("Headphone Delay"),
            tr("Headphone delay"),
            pHeadphoneMenu,
            true);
    pMixerMenu->addSeparator();
    // EQs
    QMenu* pEqMenu = addSubmenu(tr("Equalizers"), pMixerMenu);
    constexpr int kNumEqRacks = 1;
    const int iNumDecks = static_cast<int>(ControlObject::get(
            ConfigKey(kAppGroup, QStringLiteral("num_decks"))));
    QList<QString> eqNames = {tr("Low EQ"), tr("Mid EQ"), tr("High EQ")};
    for (int iRackNumber = 0; iRackNumber < kNumEqRacks; ++iRackNumber) {
        // TODO: Although there is a mode with 4-band EQs, it's not feasible
        // right now to add support for learning both it and regular 3-band eqs.
        // Since 3-band is by far the most common, stick with that.
        const int kMaxEqs = 3;
        for (int deck = 1; deck <= iNumDecks; ++deck) {
            QMenu* pDeckMenu = addSubmenu(QString("Deck %1").arg(deck), pEqMenu);
            for (int effect = kMaxEqs - 1; effect >= 0; --effect) {
                const QString group = EqualizerEffectChain::formatEffectSlotGroup(
                        QString("[Channel%1]").arg(deck));
                QMenu* pBandMenu = addSubmenu(eqNames[effect], pDeckMenu);
                QString control = "parameter%1";
                addControl(group,
                        control.arg(effect + 1),
                        tr("Adjust %1").arg(eqNames[effect]),
                        tr("Adjust %1").arg(eqNames[effect]),
                        pBandMenu,
                        true,
                        tr("Deck %1").arg(deck));

                control = "button_parameter%1";
                addControl(group,
                        control.arg(effect + 1),
                        tr("Kill %1").arg(eqNames[effect]),
                        tr("Kill %1").arg(eqNames[effect]),
                        pBandMenu,
                        false,
                        tr("Deck %1").arg(deck));
            }
        }
    }
    pMixerMenu->addSeparator();
    // Volume / Pfl controls
    addDeckAndSamplerControl("volume", tr("Volume"), tr("Volume Fader"), pMixerMenu, true);
    addDeckAndSamplerControl("volume_set_one",
            tr("Full Volume"),
            tr("Set to full volume"),
            pMixerMenu);
    addDeckAndSamplerControl("volume_set_zero",
            tr("Zero Volume"),
            tr("Set to zero volume"),
            pMixerMenu);
    addDeckAndSamplerAndPreviewDeckControl("pregain",
            tr("Track Gain"),
            tr("Track Gain knob"),
            pMixerMenu,
            true);
    addDeckAndSamplerControl("mute", tr("Mute"), tr("Mute button"), pMixerMenu);
    pMixerMenu->addSeparator();
    addDeckAndSamplerControl("pfl",
            tr("Headphone Listen"),
            tr("Headphone listen (pfl) button"),
            pMixerMenu);

    addSeparator();

    // Transport
    QMenu* pTransportMenu = addSubmenu(tr("Transport"));
    addDeckAndSamplerAndPreviewDeckControl("play", tr("Play"), tr("Play button"), pTransportMenu);
    addDeckAndSamplerAndPreviewDeckControl("back",
            tr("Fast Rewind"),
            tr("Fast Rewind button"),
            pTransportMenu);
    addDeckAndSamplerAndPreviewDeckControl("fwd",
            tr("Fast Forward"),
            tr("Fast Forward button"),
            pTransportMenu);
    addDeckAndSamplerAndPreviewDeckControl("playposition",
            tr("Strip Search"),
            tr("Strip-search through track"),
            pTransportMenu);
    addDeckAndSamplerAndPreviewDeckControl("reverse",
            tr("Play Reverse"),
            tr("Play Reverse button"),
            pTransportMenu);
    addDeckAndSamplerAndPreviewDeckControl("reverseroll",
            tr("Reverse Roll (Censor)"),
            tr("Reverse roll (Censor) button"),
            pTransportMenu);
    addDeckAndSamplerAndPreviewDeckControl("start",
            tr("Jump To Start"),
            tr("Jumps to start of track"),
            pTransportMenu);
    addDeckAndSamplerAndPreviewDeckControl("start_play",
            tr("Play From Start"),
            tr("Jump to start of track and play"),
            pTransportMenu);
    addDeckAndSamplerAndPreviewDeckControl("stop", tr("Stop"), tr("Stop button"), pTransportMenu);
    addDeckAndSamplerAndPreviewDeckControl("start_stop",
            tr("Stop And Jump To Start"),
            tr("Stop playback and jump to start of track"),
            pTransportMenu);
    addDeckAndSamplerAndPreviewDeckControl("end",
            tr("Jump To End"),
            tr("Jump to end of track"),
            pTransportMenu);
    pTransportMenu->addSeparator();
    addDeckAndSamplerAndPreviewDeckControl("eject",
            tr("Eject"),
            tr("Eject or un-eject track, i.e. reload the last-ejected track "
               "(of any deck)<br>"
               "Double-press to reload the last replaced track. In empty decks "
               "it reloads the second-last ejected track."),
            pTransportMenu);
    addDeckAndSamplerControl("repeat", tr("Repeat Mode"), tr("Toggle repeat mode"), pTransportMenu);
    addDeckAndSamplerControl("slip_enabled",
            tr("Slip Mode"),
            tr("Toggle slip mode"),
            pTransportMenu);

    // BPM / Beatgrid
    QMenu* pBpmMenu = addSubmenu(tr("BPM / Beatgrid"));
    addDeckAndSamplerControl("bpm",
            tr("BPM"),
            tr("BPM"),
            pBpmMenu,
            true);
    addDeckAndSamplerControl("bpm_up",
            tr("BPM +1"),
            tr("Increase BPM by 1"),
            pBpmMenu);
    addDeckAndSamplerControl("bpm_down",
            tr("BPM -1"),
            tr("Decrease BPM by 1"),
            pBpmMenu);
    addDeckAndSamplerControl("bpm_up_small",
            tr("BPM +0.1"),
            tr("Increase BPM by 0.1"),
            pBpmMenu);
    addDeckAndSamplerControl("bpm_down_small",
            tr("BPM -0.1"),
            tr("Decrease BPM by 0.1"),
            pBpmMenu);
    pBpmMenu->addSeparator();
    addDeckAndSamplerControl("beats_set_halve",
            tr("Halve BPM"),
            tr("Multiply current BPM by 0.5"),
            pBpmMenu);
    addDeckAndSamplerControl("beats_set_twothirds",
            tr("2/3 BPM"),
            tr("Multiply current BPM by 0.666"),
            pBpmMenu);
    addDeckAndSamplerControl("beats_set_threefourths",
            tr("3/4 BPM"),
            tr("Multiply current BPM by 0.75"),
            pBpmMenu);
    addDeckAndSamplerControl("beats_set_fourthirds",
            tr("4/3 BPM"),
            tr("Multiply current BPM by 1.333"),
            pBpmMenu);
    addDeckAndSamplerControl("beats_set_threehalves",
            tr("3/2 BPM"),
            tr("Multiply current BPM by 1.5"),
            pBpmMenu);
    addDeckAndSamplerControl("beats_set_double",
            tr("Double BPM"),
            tr("Multiply current BPM by 2"),
            pBpmMenu);
    pBpmMenu->addSeparator();
    addDeckAndSamplerControl("bpm_tap",
            tr("BPM Tap"),
            tr("BPM tap button"),
            pBpmMenu);
    addDeckAndSamplerControl("tempo_tap",
            tr("Tempo Tap"),
            tr("Tempo tap button"),
            pBpmMenu);
    pBpmMenu->addSeparator();
    addDeckAndSamplerControl("beats_adjust_faster",
            tr("Adjust Beatgrid Faster +.01"),
            tr("Increase track's average BPM by 0.01"),
            pBpmMenu);
    addDeckAndSamplerControl("beats_adjust_slower",
            tr("Adjust Beatgrid Slower -.01"),
            tr("Decrease track's average BPM by 0.01"),
            pBpmMenu);
    addDeckAndSamplerControl("beats_translate_earlier",
            tr("Move Beatgrid Earlier"),
            tr("Adjust the beatgrid to the left"),
            pBpmMenu);
    addDeckAndSamplerControl("beats_translate_later",
            tr("Move Beatgrid Later"),
            tr("Adjust the beatgrid to the right"),
            pBpmMenu);
    addDeckAndSamplerControl("beats_translate_move",
            tr("Move Beatgrid"),
            tr("Adjust the beatgrid to the left or right"),
            pBpmMenu);
    addDeckControl("beats_translate_curpos",
            tr("Adjust Beatgrid"),
            tr("Align beatgrid to current position"),
            pBpmMenu);
    addDeckControl("beats_translate_match_alignment",
            tr("Adjust Beatgrid - Match Alignment"),
            tr("Adjust beatgrid to match another playing deck."),
            pBpmMenu);
    addDeckAndSamplerControl("bpmlock",
            tr("Toggle the BPM/beatgrid lock"),
            tr("Toggle the BPM/beatgrid lock"),
            pBpmMenu);
    addDeckControl("beats_undo_adjustment",
            tr("Revert last BPM/Beatgrid Change"),
            tr("Revert last BPM/Beatgrid Change of the loaded track."),
            pBpmMenu);
    pBpmMenu->addSeparator();
    addDeckAndSamplerControl("quantize", tr("Quantize Mode"), tr("Toggle quantize mode"), pBpmMenu);

    QMenu* pSyncMenu = addSubmenu(tr("Sync"));
    addDeckAndSamplerControl("sync_enabled",
            tr("Sync / Sync Lock"),
            tr("Tap to sync tempo (and phase with quantize enabled), hold to "
               "enable permanent sync"),
            pSyncMenu);
    addDeckAndSamplerControl("beatsync",
            tr("Beat Sync One-Shot"),
            tr("One-time beat sync tempo (and phase with quantize enabled)"),
            pSyncMenu);
    addDeckAndSamplerControl("beatsync_tempo",
            tr("Sync Tempo One-Shot"),
            tr("One-time beat sync (tempo only)"),
            pSyncMenu);
    addDeckAndSamplerControl("beatsync_phase",
            tr("Sync Phase One-Shot"),
            tr("One-time beat sync (phase only)"),
            pSyncMenu);
    pSyncMenu->addSeparator();
    addControl("[InternalClock]",
            "sync_leader",
            tr("Internal Sync Leader"),
            tr("Toggle Internal Sync Leader"),
            pSyncMenu);
    addControl("[InternalClock]",
            "bpm",
            tr("Internal Leader BPM"),
            tr("Internal Leader BPM"),
            pSyncMenu);
    addControl("[InternalClock]",
            "bpm_up",
            tr("Internal Leader BPM +1"),
            tr("Increase internal Leader BPM by 1"),
            pSyncMenu);

    addControl("[InternalClock]",
            "bpm_down",
            tr("Internal Leader BPM -1"),
            tr("Decrease internal Leader BPM by 1"),
            pSyncMenu);

    addControl("[InternalClock]",
            "bpm_up_small",
            tr("Internal Leader BPM +0.1"),
            tr("Increase internal Leader BPM by 0.1"),
            pSyncMenu);
    addControl("[InternalClock]",
            "bpm_down_small",
            tr("Internal Leader BPM -0.1"),
            tr("Decrease internal Leader BPM by 0.1"),
            pSyncMenu);
    pSyncMenu->addSeparator();
    addDeckAndSamplerControl("sync_leader",
            tr("Sync Leader"),
            tr("Sync mode 3-state toggle / indicator (Off, Soft Leader, "
               "Explicit Leader)"),
            pSyncMenu);

    // Speed
    QMenu* pSpeedMenu = addSubmenu(tr("Speed"));
    addDeckAndSamplerControl("rate",
            tr("Playback Speed"),
            tr("Playback speed control (Vinyl \"Pitch\" slider)"),
            pSpeedMenu,
            true);
    pSpeedMenu->addSeparator();
    addDeckAndSamplerControl("rate_perm_up",
            tr("Increase Speed"),
            tr("Adjust speed faster (coarse)"),
            pSpeedMenu);
    addDeckAndSamplerControl("rate_perm_up_small",
            tr("Increase Speed (Fine)"),
            tr("Adjust speed faster (fine)"),
            pSpeedMenu);
    addDeckAndSamplerControl("rate_perm_down",
            tr("Decrease Speed"),
            tr("Adjust speed slower (coarse)"),
            pSpeedMenu);
    addDeckAndSamplerControl("rate_perm_down_small",
            tr("Decrease Speed (Fine)"),
            tr("Adjust speed slower (fine)"),
            pSpeedMenu);
    pSpeedMenu->addSeparator();
    addDeckAndSamplerControl("rate_temp_up",
            tr("Temporarily Increase Speed"),
            tr("Temporarily increase speed (coarse)"),
            pSpeedMenu);
    addDeckAndSamplerControl("rate_temp_up_small",
            tr("Temporarily Increase Speed (Fine)"),
            tr("Temporarily increase speed (fine)"),
            pSpeedMenu);
    addDeckAndSamplerControl("rate_temp_down",
            tr("Temporarily Decrease Speed"),
            tr("Temporarily decrease speed (coarse)"),
            pSpeedMenu);
    addDeckAndSamplerControl("rate_temp_down_small",
            tr("Temporarily Decrease Speed (Fine)"),
            tr("Temporarily decrease speed (fine)"),
            pSpeedMenu);
    // Pitch (Musical Key)
    QMenu* pPitchMenu = addSubmenu(tr("Pitch (Musical Key)"));
    addDeckAndSamplerControl("pitch",
            tr("Pitch (Musical key)"),
            tr("Pitch control (does not affect tempo), center is original "
               "pitch"),
            pPitchMenu,
            true);
    addDeckAndSamplerControl("pitch_up",
            tr("Increase Pitch"),
            tr("Increases the pitch by one semitone"),
            pPitchMenu);
    addDeckAndSamplerControl("pitch_up_small",
            tr("Increase Pitch (Fine)"),
            tr("Increases the pitch by 10 cents"),
            pPitchMenu);
    addDeckAndSamplerControl("pitch_down",
            tr("Decrease Pitch"),
            tr("Decreases the pitch by one semitone"),
            pPitchMenu);
    addDeckAndSamplerControl("pitch_down_small",
            tr("Decrease Pitch (Fine)"),
            tr("Decreases the pitch by 10 cents"),
            pPitchMenu);
    addDeckAndSamplerControl("pitch_adjust",
            tr("Pitch Adjust"),
            tr("Adjust pitch from speed slider pitch"),
            pPitchMenu,
            true);
    pPitchMenu->addSeparator();
    addDeckAndSamplerControl("sync_key", tr("Match Key"), tr("Match musical key"), pPitchMenu);
    addDeckAndSamplerControl("reset_key",
            tr("Reset Key"),
            tr("Resets key to original"),
            pPitchMenu);
    addDeckAndSamplerControl("keylock", tr("Keylock"), tr("Toggle keylock mode"), pPitchMenu);

    // Vinyl Control
    QMenu* pVinylControlMenu = addSubmenu(tr("Vinyl Control"));
    addDeckControl("vinylcontrol_enabled",
            tr("Toggle Vinyl Control"),
            tr("Toggle Vinyl Control (ON/OFF)"),
            pVinylControlMenu);
    addDeckControl("vinylcontrol_mode",
            tr("Vinyl Control Mode"),
            tr("Toggle vinyl-control mode (ABS/REL/CONST)"),
            pVinylControlMenu);
    addDeckControl("vinylcontrol_cueing",
            tr("Vinyl Control Cueing Mode"),
            tr("Toggle vinyl-control cueing mode (OFF/ONE/HOT)"),
            pVinylControlMenu);
    addDeckControl("passthrough",
            tr("Vinyl Control Passthrough"),
            tr("Pass through external audio into the internal mixer"),
            pVinylControlMenu);
    addControl(VINYL_PREF_KEY,
            "Toggle",
            tr("Vinyl Control Next Deck"),
            tr("Single deck mode - Switch vinyl control to next deck"),
            pVinylControlMenu);

    // Cues
    QMenu* pCueMenu = addSubmenu(tr("Cues"));
    addDeckControl("cue_default", tr("Cue"), tr("Cue button"), pCueMenu);
    addDeckControl("cue_set", tr("Set Cue"), tr("Set cue point"), pCueMenu);
    addDeckControl("cue_goto", tr("Go-To Cue"), tr("Go to cue point"), pCueMenu);
    addDeckAndSamplerAndPreviewDeckControl("cue_gotoandplay",
            tr("Go-To Cue And Play"),
            tr("Go to cue point and play"),
            pCueMenu);
    addDeckControl("cue_gotoandstop",
            tr("Go-To Cue And Stop"),
            tr("Go to cue point and stop"),
            pCueMenu);
    addDeckControl("cue_preview", tr("Preview Cue"), tr("Preview from cue point"), pCueMenu);
    addDeckControl("cue_cdj", tr("Cue (CDJ Mode)"), tr("Cue button (CDJ mode)"), pCueMenu);
    addDeckControl("play_stutter", tr("Stutter Cue"), tr("Stutter cue"), pCueMenu);
    addDeckControl("cue_play",
            tr("CUP (Cue + Play)"),
            tr("Go to cue point and play after release"),
            pCueMenu);

    // Hotcues
    QMenu* pHotcueMainMenu = addSubmenu(tr("Hotcues"));
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
    addDeckControl("shift_cues_earlier",
            tr("Shift cue points earlier"),
            tr("Shift cue points 10 milliseconds earlier"),
            pHotcueMainMenu);
    addDeckControl("shift_cues_earlier_small",
            tr("Shift cue points earlier (fine)"),
            tr("Shift cue points 1 millisecond earlier"),
            pHotcueMainMenu);
    addDeckControl("shift_cues_later",
            tr("Shift cue points later"),
            tr("Shift cue points 10 milliseconds later"),
            pHotcueMainMenu);
    addDeckControl("shift_cues_later_small",
            tr("Shift cue points later (fine)"),
            tr("Shift cue points 1 millisecond later"),
            pHotcueMainMenu);
    // add menus for hotcues 1-16.
    // though, keep the menu small put additional hotcues in a separate menu,
    // but don't create that submenu for less than 4 additional hotcues.
    int preferredHotcuesVisible = 16;
    int moreMenuThreshold = 4;
    QMenu* pParentMenu = pHotcueMainMenu;
    QMenu* pHotcueMoreMenu = nullptr;
    bool moreHotcues = NUM_HOT_CUES >= preferredHotcuesVisible + moreMenuThreshold;
    if (moreHotcues) {
        // populate menu here, add it below #preferredHotcuesVisible
        pHotcueMoreMenu = new QMenu(
                tr("Hotcues %1-%2").arg(preferredHotcuesVisible + 1).arg(NUM_HOT_CUES),
                pHotcueMainMenu);
    }
    for (int i = 1; i <= NUM_HOT_CUES; ++i) {
        if (moreHotcues && i > preferredHotcuesVisible) {
            pParentMenu = pHotcueMoreMenu;
        }
        QMenu* pHotcueSubMenu = addSubmenu(tr("Hotcue %1").arg(QString::number(i)), pParentMenu);
        addDeckAndSamplerControl(QString("hotcue_%1_activate").arg(i),
                hotcueActivateTitle.arg(QString::number(i)),
                hotcueActivateDescription.arg(QString::number(i)),
                pHotcueSubMenu);
        addDeckAndSamplerControl(QString("hotcue_%1_clear").arg(i),
                hotcueClearTitle.arg(QString::number(i)),
                hotcueClearDescription.arg(QString::number(i)),
                pHotcueSubMenu);
        addDeckAndSamplerControl(QString("hotcue_%1_set").arg(i),
                hotcueSetTitle.arg(QString::number(i)),
                hotcueSetDescription.arg(QString::number(i)),
                pHotcueSubMenu);
        addDeckAndSamplerControl(QString("hotcue_%1_goto").arg(i),
                hotcueGotoTitle.arg(QString::number(i)),
                hotcueGotoDescription.arg(QString::number(i)),
                pHotcueSubMenu);
        addDeckAndSamplerControl(QString("hotcue_%1_gotoandstop").arg(i),
                hotcueGotoAndStopTitle.arg(QString::number(i)),
                hotcueGotoAndStopDescription.arg(QString::number(i)),
                pHotcueSubMenu);
        addDeckAndSamplerControl(QString("hotcue_%1_gotoandplay").arg(i),
                hotcueGotoAndPlayTitle.arg(QString::number(i)),
                hotcueGotoAndPlayDescription.arg(QString::number(i)),
                pHotcueSubMenu);
        addDeckAndSamplerControl(QString("hotcue_%1_activate_preview").arg(i),
                hotcuePreviewTitle.arg(QString::number(i)),
                hotcuePreviewDescription.arg(QString::number(i)),
                pHotcueSubMenu);
    }
    if (moreHotcues) {
        pHotcueMainMenu->addSeparator();
        pHotcueMainMenu->addMenu(pHotcueMoreMenu);
    }

    // Intro/outro range markers
    QMenu* pIntroOutroMenu = addSubmenu(tr("Intro / Outro Markers"));
    const QStringList markerTitles = {
            tr("Intro Start Marker"),
            tr("Intro End Marker"),
            tr("Outro Start Marker"),
            tr("Outro End Marker")};
    const QStringList markerNames = {
            tr("intro start marker"),
            tr("intro end marker"),
            tr("outro start marker"),
            tr("outro end marker")};
    const QStringList markerCOs = {
            "intro_start",
            "intro_end",
            "outro_start",
            "outro_end"};

    for (int i = 0; i < markerTitles.size(); ++i) {
        QMenu* pTempMenu = addSubmenu(markerTitles[i], pIntroOutroMenu);
        addDeckAndSamplerAndPreviewDeckControl(
                QString("%1_activate").arg(markerCOs[i]),
                tr("Activate %1", "[intro/outro marker").arg(markerTitles[i]),
                tr("Jump to or set the %1", "[intro/outro marker").arg(markerNames[i]),
                pTempMenu);
        addDeckAndSamplerAndPreviewDeckControl(
                QString("%1_set").arg(markerCOs[i]),
                tr("Set %1", "[intro/outro marker").arg(markerTitles[i]),
                tr("Set or jump to the %1", "[intro/outro marker").arg(markerNames[i]),
                pTempMenu);
        addDeckAndSamplerAndPreviewDeckControl(
                QString("%1_clear").arg(markerCOs[i]),
                tr("Clear %1", "[intro/outro marker").arg(markerTitles[i]),
                tr("Clear the %1", "[intro/outro marker").arg(markerNames[i]),
                pTempMenu);
    }

    // Loops
    QMenu* pLoopMenu = addSubmenu(tr("Looping"));
    // add beatloop_activate and beatlooproll_activate to both the
    // Loop and Beat-Loop menus to make sure users can find them.
    QString noBeatsSeconds = QChar('(') +
            tr("if the track has no beats the unit is seconds") + QChar(')');
    QString beatloopActivateTitle = tr("Loop Selected Beats");
    QString beatloopActivateDescription =
            tr("Create a beat loop of selected beat size") + noBeatsSeconds;
    QString beatloopRollActivateTitle = tr("Loop Roll Selected Beats");
    QString beatloopRollActivateDescription =
            tr("Create a rolling beat loop of selected beat size") + noBeatsSeconds;
    QString beatLoopTitle = tr("Loop %1 Beats");
    QString reverseBeatLoopTitle = tr("Loop %1 Beats set from its end point");
    QString beatLoopRollTitle = tr("Loop Roll %1 Beats");
    QString reverseBeatLoopRollTitle = tr("Loop Roll %1 Beats set from its end point");
    QString beatLoopDescription = tr("Create %1-beat loop");
    QString reverseBeatLoopDescription = tr(
            "Create %1-beat loop with the current play position as loop end");
    QString beatLoopRollDescription =
            tr("Create temporary %1-beat loop roll") + noBeatsSeconds;
    QString reverseBeatLoopRollDescription =
            tr("Create temporary %1-beat loop roll with the current play "
               "position as loop end") +
            noBeatsSeconds;

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

    // Beatloops
    addDeckControl("beatloop_activate",
            beatloopActivateTitle,
            beatloopActivateDescription,
            pLoopMenu);
    QMenu* pLoopActivateMenu = addSubmenu(tr("Loop Beats"), pLoopMenu);
    foreach (double beats, beatSizes) {
        QString humanBeats = humanBeatSizes.value(beats, QString::number(beats));
        addDeckControl(QString("beatloop_%1_toggle").arg(beats),
                beatLoopTitle.arg(humanBeats),
                beatLoopDescription.arg(humanBeats),
                pLoopActivateMenu);
    }
    for (double beats : beatSizes) {
        QString humanBeats = humanBeatSizes.value(beats, QString::number(beats));
        addDeckControl(QString("beatloop_r%1_toggle").arg(beats),
                reverseBeatLoopTitle.arg(humanBeats),
                reverseBeatLoopDescription.arg(humanBeats),
                pLoopActivateMenu);
    }
    pLoopMenu->addSeparator();

    addDeckControl("beatlooproll_activate",
            beatloopRollActivateTitle,
            beatloopRollActivateDescription,
            pLoopMenu);
    QMenu* pLooprollActivateMenu = addSubmenu(tr("Loop Roll Beats"), pLoopMenu);
    foreach (double beats, beatSizes) {
        QString humanBeats = humanBeatSizes.value(beats, QString::number(beats));
        addDeckControl(QString("beatlooproll_%1_activate").arg(beats),
                beatLoopRollTitle.arg(humanBeats),
                beatLoopRollDescription.arg(humanBeats),
                pLooprollActivateMenu);
    }
    for (double beats : beatSizes) {
        QString humanBeats = humanBeatSizes.value(beats, QString::number(beats));
        addDeckControl(QString("beatlooproll_r%1_activate").arg(beats),
                reverseBeatLoopRollTitle.arg(humanBeats),
                reverseBeatLoopRollDescription.arg(humanBeats),
                pLooprollActivateMenu);
    }
    pLoopMenu->addSeparator();

    addDeckControl("loop_in", tr("Loop In"), tr("Loop In button"), pLoopMenu);
    addDeckControl("loop_in_goto", tr("Go To Loop In"), tr("Go to Loop In button"), pLoopMenu);
    addDeckControl("loop_out", tr("Loop Out"), tr("Loop Out button"), pLoopMenu);
    addDeckControl("loop_out_goto", tr("Go To Loop Out"), tr("Go to Loop Out button"), pLoopMenu);
    addDeckControl("loop_exit", tr("Loop Exit"), tr("Loop Exit button"), pLoopMenu);
    addDeckControl("reloop_toggle",
            tr("Reloop/Exit Loop"),
            tr("Toggle loop on/off and jump to Loop In point if loop is behind "
               "play position"),
            pLoopMenu);
    addDeckControl("reloop_andstop",
            tr("Reloop And Stop"),
            tr("Enable loop, jump to Loop In point, and stop"),
            pLoopMenu);
    addDeckControl("loop_halve", tr("Loop Halve"), tr("Halve the loop length"), pLoopMenu);
    addDeckControl("loop_double", tr("Loop Double"), tr("Double the loop length"), pLoopMenu);

    // Beat Jump / Loop Move
    QMenu* pBeatJumpMenu = addSubmenu(tr("Beat Jump / Loop Move"));
    QString beatJumpForwardTitle = tr("Jump / Move Loop Forward %1 Beats");
    QString beatJumpBackwardTitle = tr("Jump / Move Loop Backward %1 Beats");
    QString beatJumpForwardDescription =
            tr("Jump forward by %1 beats, or if a loop is enabled, move the "
               "loop forward %1 beats") +
            noBeatsSeconds;
    QString beatJumpBackwardDescription =
            tr("Jump backward by %1 beats, or if a loop is enabled, move the "
               "loop backward %1 beats") +
            noBeatsSeconds;
    addDeckControl("beatjump_forward",
            tr("Beat Jump / Loop Move Forward Selected Beats"),
            tr("Jump forward by the selected number of beats, or if a loop is "
               "enabled, move the loop forward by the selected number of "
               "beats"),
            pBeatJumpMenu);
    addDeckControl("beatjump_backward",
            tr("Beat Jump / Loop Move Backward Selected Beats"),
            tr("Jump backward by the selected number of beats, or if a loop is "
               "enabled, move the loop backward by the selected number of "
               "beats"),
            pBeatJumpMenu);
    addDeckControl("loop_anchor",
            tr("Beat Jump"),
            tr("Indicate which loop marker remain static when adjusting the "
               "size or is inherited from the current position"),
            pBeatJumpMenu);
    pBeatJumpMenu->addSeparator();

    QMenu* beatjumpFwdSubmenu = addSubmenu(tr("Beat Jump / Loop Move Forward"), pBeatJumpMenu);
    foreach (double beats, beatSizes) {
        QString humanBeats = humanBeatSizes.value(beats, QString::number(beats));
        addDeckControl(QString("beatjump_%1_forward").arg(beats),
                beatJumpForwardTitle.arg(humanBeats),
                beatJumpForwardDescription.arg(humanBeats),
                beatjumpFwdSubmenu);
    }

    QMenu* beatjumpBwdSubmenu = addSubmenu(tr("Beat Jump / Loop Move Backward"), pBeatJumpMenu);
    foreach (double beats, beatSizes) {
        QString humanBeats = humanBeatSizes.value(beats, QString::number(beats));
        addDeckControl(QString("beatjump_%1_backward").arg(beats),
                beatJumpBackwardTitle.arg(humanBeats),
                beatJumpBackwardDescription.arg(humanBeats),
                beatjumpBwdSubmenu);
    }

    // Loop moving
    QString loopMoveForwardTitle = tr("Move Loop +%1 Beats");
    QString loopMoveBackwardTitle = tr("Move Loop -%1 Beats");
    QString loopMoveForwardDescription = tr("Move loop forward by %1 beats") +
            noBeatsSeconds;
    QString loopMoveBackwardDescription = tr("Move loop backward by %1 beats") +
            noBeatsSeconds;

    QMenu* pLoopmoveFwdSubmenu = addSubmenu(tr("Loop Move Forward"), pBeatJumpMenu);
    foreach (double beats, beatSizes) {
        QString humanBeats = humanBeatSizes.value(beats, QString::number(beats));
        addDeckControl(QString("loop_move_%1_forward").arg(beats),
                loopMoveForwardTitle.arg(humanBeats),
                loopMoveForwardDescription.arg(humanBeats),
                pLoopmoveFwdSubmenu);
    }

    QMenu* pLoopmoveBwdSubmenu = addSubmenu(tr("Loop Move Backward"), pBeatJumpMenu);
    foreach (double beats, beatSizes) {
        QString humanBeats = humanBeatSizes.value(beats, QString::number(beats));
        addDeckControl(QString("loop_move_%1_backward").arg(beats),
                loopMoveBackwardTitle.arg(humanBeats),
                loopMoveBackwardDescription.arg(humanBeats),
                pLoopmoveBwdSubmenu);
    }

    addDeckControl("loop_remove",
            tr("Remove Temporary Loop"),
            tr("Remove the temporary loop"),
            pLoopMenu);

    addSeparator();

    // Library Controls
    QMenu* pLibraryMenu = addSubmenu(m_libraryStr);
    QMenu* pNavigationMenu = addSubmenu(tr("Navigation"), pLibraryMenu);
    addLibraryControl("MoveUp",
            tr("Move up"),
            tr("Equivalent to pressing the UP key on the keyboard"),
            pNavigationMenu);
    addLibraryControl("MoveDown",
            tr("Move down"),
            tr("Equivalent to pressing the DOWN key on the keyboard"),
            pNavigationMenu);
    addLibraryControl("MoveVertical",
            tr("Move up/down"),
            tr("Move vertically in either direction using a knob, as if "
               "pressing UP/DOWN keys"),
            pNavigationMenu);
    addLibraryControl("ScrollUp",
            tr("Scroll Up"),
            tr("Equivalent to pressing the PAGE UP key on the keyboard"),
            pNavigationMenu);
    addLibraryControl("ScrollDown",
            tr("Scroll Down"),
            tr("Equivalent to pressing the PAGE DOWN key on the keyboard"),
            pNavigationMenu);
    addLibraryControl("ScrollVertical",
            tr("Scroll up/down"),
            tr("Scroll vertically in either direction using a knob, as if "
               "pressing PGUP/PGDOWN keys"),
            pNavigationMenu);
    addLibraryControl("MoveLeft",
            tr("Move left"),
            tr("Equivalent to pressing the LEFT key on the keyboard"),
            pNavigationMenu);
    addLibraryControl("MoveRight",
            tr("Move right"),
            tr("Equivalent to pressing the RIGHT key on the keyboard"),
            pNavigationMenu);
    addLibraryControl("MoveHorizontal",
            tr("Move left/right"),
            tr("Move horizontally in either direction using a knob, as if "
               "pressing LEFT/RIGHT keys"),
            pNavigationMenu);
    pNavigationMenu->addSeparator();
    addLibraryControl("MoveFocusForward",
            tr("Move focus to right pane"),
            tr("Equivalent to pressing the TAB key on the keyboard"),
            pNavigationMenu);
    addLibraryControl("MoveFocusBackward",
            tr("Move focus to left pane"),
            tr("Equivalent to pressing the SHIFT+TAB key on the keyboard"),
            pNavigationMenu);
    addLibraryControl("MoveFocus",
            tr("Move focus to right/left pane"),
            tr("Move focus one pane to right or left using a knob, as if "
               "pressing TAB/SHIFT+TAB keys"),
            pNavigationMenu);
    addLibraryControl("sort_focused_column",
            tr("Sort focused column"),
            tr("Sort the column of the cell that is currently focused, "
               "equivalent to clicking on its header"),
            pNavigationMenu);

    pLibraryMenu->addSeparator();
    addLibraryControl("GoToItem",
            tr("Go to the currently selected item"),
            tr("Choose the currently selected item and advance forward one "
               "pane if appropriate"),
            pLibraryMenu);
    // Load track (these can be loaded into any channel)
    addDeckAndSamplerControl("LoadSelectedTrack",
            tr("Load Track"),
            tr("Load selected track"),
            pLibraryMenu);
    addDeckAndSamplerAndPreviewDeckControl("LoadSelectedTrackAndPlay",
            tr("Load Track and Play"),
            tr("Load selected track and play"),
            pLibraryMenu);
    pLibraryMenu->addSeparator();
    // Auto DJ
    addLibraryControl("AutoDjAddBottom",
            tr("Add to Auto DJ Queue (bottom)"),
            tr("Append the selected track to the Auto DJ Queue"),
            pLibraryMenu);
    addLibraryControl("AutoDjAddTop",
            tr("Add to Auto DJ Queue (top)"),
            tr("Prepend selected track to the Auto DJ Queue"),
            pLibraryMenu);
    addLibraryControl("AutoDjAddReplace",
            tr("Add to Auto DJ Queue (replace)"),
            tr("Replace Auto DJ Queue with selected tracks"),
            pLibraryMenu);
    pLibraryMenu->addSeparator();
    // Search box
    addLibraryControl("search_history_next",
            tr("Select next search history"),
            tr("Selects the next search history entry"),
            pLibraryMenu);
    addLibraryControl("search_history_prev",
            tr("Select previous search history"),
            tr("Selects the previous search history entry"),
            pLibraryMenu);
    addLibraryControl("search_history_selector",
            tr("Move selected search entry"),
            tr("Moves the selected search history item into given direction "
               "and steps"),
            pLibraryMenu);
    addLibraryControl("clear_search",
            tr("Clear search"),
            tr("Clears the search query"),
            pLibraryMenu);

    // Color selection
    addLibraryControl("track_color_next",
            tr("Select Next Color Available"),
            tr("Select the next color in the color palette"
               " for the first selected track"),
            pLibraryMenu);
    addLibraryControl("track_color_prev",
            tr("Select Previous Color Available"),
            tr("Select the previous color in the color palette"
               " for the first selected track"),
            pLibraryMenu);
    addLibraryControl("track_color_selector",
            tr("Navigate Through Track Colors"),
            tr("Select either next or previous color in the"
               " palette for the first selected track."),
            pLibraryMenu);

    pLibraryMenu->addSeparator();
    addControl("[Recording]",
            "toggle_recording",
            tr("Record Mix"),
            tr("Toggle mix recording"),
            pLibraryMenu,
            false,
            m_libraryStr);

    // Effect Controls
    QMenu* pEffectsMenu = addSubmenu(tr("Effects"));

    // Quick Effect Rack COs
    QMenu* pQuickEffectMenu = addSubmenu(tr("Quick Effects"), pEffectsMenu);
    for (int i = 1; i <= iNumDecks; ++i) {
        addControl(QString("[QuickEffectRack1_[Channel%1]]").arg(i),
                "super1",
                tr("Deck %1 Quick Effect Super Knob").arg(i),
                tr("Quick Effect Super Knob (control linked effect "
                   "parameters)"),
                pQuickEffectMenu,
                false,
                tr("Quick Effect"));
        addControl(QString("[QuickEffectRack1_[Channel%1]_Effect1]").arg(i),
                "enabled",
                tr("Deck %1 Quick Effect Enable Button").arg(i),
                tr("Quick Effect Enable Button"),
                pQuickEffectMenu,
                false,
                tr("Quick Effect"));
    }

    pEffectsMenu->addSeparator();

    for (int iEffectUnitNumber = 1; iEffectUnitNumber <= kNumStandardEffectUnits;
            ++iEffectUnitNumber) {
        const QString effectUnitGroup =
                StandardEffectChain::formatEffectChainGroup(iEffectUnitNumber - 1);

        const QString descriptionPrefix = QString("%1").arg(m_effectUnitStr.arg(iEffectUnitNumber));

        QMenu* pEffectUnitMenu = addSubmenu(m_effectUnitStr.arg(iEffectUnitNumber),
                pEffectsMenu);
        addControl(effectUnitGroup,
                "clear",
                tr("Clear Unit"),
                tr("Clear effect unit"),
                pEffectUnitMenu,
                false,
                descriptionPrefix);
        addControl(effectUnitGroup,
                "enabled",
                tr("Toggle Unit"),
                tr("Enable or disable effect processing"),
                pEffectUnitMenu,
                false,
                descriptionPrefix);
        addControl(effectUnitGroup,
                "mix",
                tr("Dry/Wet"),
                tr("Adjust the balance between the original (dry) and "
                   "processed (wet) signal."),
                pEffectUnitMenu,
                true,
                descriptionPrefix);
        addControl(effectUnitGroup,
                "super1",
                tr("Super Knob"),
                tr("Super Knob (control effects' Meta Knobs)"),
                pEffectUnitMenu,
                true,
                descriptionPrefix);
        addControl(effectUnitGroup,
                "mix_mode",
                tr("Mix Mode Toggle"),
                tr("Toggle effect unit between D/W and D+W modes"),
                pEffectUnitMenu,
                false,
                descriptionPrefix);
        addControl(effectUnitGroup,
                "next_chain",
                tr("Next Chain"),
                tr("Next chain preset"),
                pEffectUnitMenu,
                false,
                descriptionPrefix);
        addControl(effectUnitGroup,
                "prev_chain",
                tr("Previous Chain"),
                tr("Previous chain preset"),
                pEffectUnitMenu,
                false,
                descriptionPrefix);
        addControl(effectUnitGroup,
                "chain_selector",
                tr("Next/Previous Chain"),
                tr("Next or previous chain preset"),
                pEffectUnitMenu,
                false,
                descriptionPrefix);
        addControl(effectUnitGroup,
                "show_parameters",
                tr("Show Effect Parameters"),
                tr("Show Effect Parameters"),
                pEffectUnitMenu,
                false,
                descriptionPrefix);

        QString assignMenuTitle = tr("Effect Unit Assignment");
        QString assignString = tr("Assign ");
        QMenu* pEffectUnitGroupsMenu = addSubmenu(assignMenuTitle,
                pEffectUnitMenu);

        QString groupDescriptionPrefix = QString("%1").arg(
                m_effectUnitStr.arg(iEffectUnitNumber));

        addControl(effectUnitGroup, "group_[Master]_enable",
                assignString + m_effectMainOutputStr, // in ComboBox
                assignString + m_effectMainOutputStr, // description below
                pEffectUnitGroupsMenu,
                false,
                groupDescriptionPrefix);
        addControl(effectUnitGroup,
                "group_[Headphone]_enable",
                assignString + m_effectHeadphoneOutputStr,
                assignString + m_effectHeadphoneOutputStr,
                pEffectUnitGroupsMenu,
                false,
                groupDescriptionPrefix);

        for (int iDeckNumber = 1; iDeckNumber <= iNumDecks; ++iDeckNumber) {
            // PlayerManager::groupForDeck is 0-indexed.
            QString playerGroup = PlayerManager::groupForDeck(iDeckNumber - 1);
            // TODO(owen): Fix bad i18n here.
            addControl(effectUnitGroup,
                    QString("group_%1_enable").arg(playerGroup),
                    assignString + m_deckStr.arg(iDeckNumber),
                    assignString + m_deckStr.arg(iDeckNumber),
                    pEffectUnitGroupsMenu,
                    false,
                    groupDescriptionPrefix);
        }

        const int iNumSamplers = static_cast<int>(ControlObject::get(
                ConfigKey(kAppGroup, QStringLiteral("num_samplers"))));
        for (int iSamplerNumber = 1; iSamplerNumber <= iNumSamplers;
                ++iSamplerNumber) {
            // PlayerManager::groupForSampler is 0-indexed.
            QString playerGroup = PlayerManager::groupForSampler(iSamplerNumber - 1);
            // TODO(owen): Fix bad i18n here.
            addControl(effectUnitGroup,
                    QString("group_%1_enable").arg(playerGroup),
                    assignString + m_samplerStr.arg(iSamplerNumber),
                    assignString + m_samplerStr.arg(iSamplerNumber),
                    pEffectUnitGroupsMenu,
                    false,
                    groupDescriptionPrefix);
        }

        const int iNumMicrophones = static_cast<int>(ControlObject::get(
                ConfigKey(kAppGroup, QStringLiteral("num_microphones"))));
        for (int iMicrophoneNumber = 1; iMicrophoneNumber <= iNumMicrophones;
                ++iMicrophoneNumber) {
            QString micGroup = PlayerManager::groupForMicrophone(iMicrophoneNumber - 1);
            // TODO(owen): Fix bad i18n here.
            addControl(effectUnitGroup,
                    QString("group_%1_enable").arg(micGroup),
                    assignString + m_microphoneStr.arg(iMicrophoneNumber),
                    assignString + m_microphoneStr.arg(iMicrophoneNumber),
                    pEffectUnitGroupsMenu,
                    false,
                    groupDescriptionPrefix);
        }

        const int iNumAuxiliaries = static_cast<int>(ControlObject::get(
                ConfigKey(kAppGroup, QStringLiteral("num_auxiliaries"))));
        for (int iAuxiliaryNumber = 1; iAuxiliaryNumber <= iNumAuxiliaries;
                ++iAuxiliaryNumber) {
            QString auxGroup = PlayerManager::groupForAuxiliary(iAuxiliaryNumber - 1);
            // TODO(owen): Fix bad i18n here.
            addControl(effectUnitGroup,
                    QString("group_%1_enable").arg(auxGroup),
                    assignString + m_auxStr.arg(iAuxiliaryNumber),
                    assignString + m_auxStr.arg(iAuxiliaryNumber),
                    pEffectUnitGroupsMenu,
                    false,
                    groupDescriptionPrefix);
        }

        const int numEffectSlots = static_cast<int>(ControlObject::get(
                ConfigKey(effectUnitGroup, "num_effectslots")));
        for (int iEffectSlotNumber = 1; iEffectSlotNumber <= numEffectSlots;
                ++iEffectSlotNumber) {
            const QString effectSlotGroup =
                    StandardEffectChain::formatEffectSlotGroup(
                            iEffectUnitNumber - 1, iEffectSlotNumber - 1);

            QMenu* pEffectSlotMenu = addSubmenu(m_effectStr.arg(iEffectSlotNumber),
                    pEffectUnitMenu);

            QString slotDescriptionPrefix =
                    QString("%1, %2").arg(descriptionPrefix,
                            m_effectStr.arg(iEffectSlotNumber));

            addControl(effectSlotGroup,
                    "clear",
                    tr("Clear"),
                    tr("Clear the current effect"),
                    pEffectSlotMenu,
                    false,
                    slotDescriptionPrefix);
            addControl(effectSlotGroup,
                    "meta",
                    tr("Meta Knob"),
                    tr("Effect Meta Knob (control linked effect parameters)"),
                    pEffectSlotMenu,
                    false,
                    slotDescriptionPrefix);
            addControl(effectSlotGroup,
                    "enabled",
                    tr("Toggle"),
                    tr("Toggle the current effect"),
                    pEffectSlotMenu,
                    false,
                    slotDescriptionPrefix);
            addControl(effectSlotGroup,
                    "next_effect",
                    tr("Next"),
                    tr("Switch to next effect"),
                    pEffectSlotMenu,
                    false,
                    slotDescriptionPrefix);
            addControl(effectSlotGroup,
                    "prev_effect",
                    tr("Previous"),
                    tr("Switch to the previous effect"),
                    pEffectSlotMenu,
                    false,
                    slotDescriptionPrefix);
            addControl(effectSlotGroup,
                    "effect_selector",
                    tr("Next or Previous"),
                    tr("Switch to either next or previous effect"),
                    pEffectSlotMenu,
                    false,
                    slotDescriptionPrefix);

            // Effect parameter knobs
            const int numParameterSlots = static_cast<int>(ControlObject::get(
                    ConfigKey(effectSlotGroup, "num_parameterslots")));
            for (int iParameterSlotNumber = 1; iParameterSlotNumber <= numParameterSlots;
                    ++iParameterSlotNumber) {
                // The parameter slot group is the same as the effect slot
                // group on a standard effect rack.
                const QString parameterSlotGroup =
                        StandardEffectChain::formatEffectSlotGroup(
                                iEffectUnitNumber - 1, iEffectSlotNumber - 1);
                const QString parameterSlotItemPrefix = EffectKnobParameterSlot::formatItemPrefix(
                        iParameterSlotNumber - 1);
                QMenu* pParameterSlotMenu = addSubmenu(
                        m_parameterStr.arg(iParameterSlotNumber),
                        pEffectSlotMenu);

                QString parameterDescriptionPrefix =
                        QString("%1, %2").arg(slotDescriptionPrefix,
                                m_parameterStr.arg(iParameterSlotNumber));

                // Likely to change soon.
                addControl(parameterSlotGroup,
                        parameterSlotItemPrefix,
                        tr("Parameter Value"),
                        tr("Parameter Value"),
                        pParameterSlotMenu,
                        true,
                        parameterDescriptionPrefix);
                addControl(parameterSlotGroup,
                        parameterSlotItemPrefix + "_link_type",
                        tr("Meta Knob Mode"),
                        tr("Set how linked effect parameters change when "
                           "turning the Meta Knob."),
                        pParameterSlotMenu,
                        false,
                        parameterDescriptionPrefix);
                addControl(parameterSlotGroup,
                        parameterSlotItemPrefix + "_link_inverse",
                        tr("Meta Knob Mode Invert"),
                        tr("Invert how linked effect parameters change when "
                           "turning the Meta Knob."),
                        pParameterSlotMenu,
                        false,
                        parameterDescriptionPrefix);
            }

            // Effect parameter buttons
            const int numButtonParameterSlots = static_cast<int>(ControlObject::get(
                    ConfigKey(effectSlotGroup, "num_button_parameterslots")));
            for (int iParameterSlotNumber = 1; iParameterSlotNumber <= numButtonParameterSlots;
                    ++iParameterSlotNumber) {
                // The parameter slot group is the same as the effect slot
                // group on a standard effect rack.
                const QString parameterSlotGroup =
                        StandardEffectChain::formatEffectSlotGroup(
                                iEffectUnitNumber - 1, iEffectSlotNumber - 1);
                const QString parameterSlotItemPrefix =
                        EffectButtonParameterSlot::formatItemPrefix(
                                iParameterSlotNumber - 1);
                QMenu* pParameterSlotMenu = addSubmenu(
                        m_buttonParameterStr.arg(iParameterSlotNumber),
                        pEffectSlotMenu);

                QString parameterDescriptionPrefix =
                        QString("%1, %2").arg(slotDescriptionPrefix,
                                m_buttonParameterStr.arg(iParameterSlotNumber));

                // Likely to change soon.
                addControl(parameterSlotGroup,
                        parameterSlotItemPrefix,
                        tr("Button Parameter Value"),
                        tr("Button Parameter Value"),
                        pParameterSlotMenu,
                        true,
                        parameterDescriptionPrefix);
            }
        }
    }

    // Microphone Controls
    QMenu* pMicrophoneMenu = addSubmenu(tr("Microphone / Auxiliary"));

    addMicrophoneAndAuxControl("talkover",
            tr("Microphone On/Off"),
            tr("Microphone on/off"),
            pMicrophoneMenu,
            true,
            false);
    addControl("[Master]",
            "duckStrength",
            tr("Microphone Ducking Strength"),
            tr("Microphone Ducking Strength"),
            pMicrophoneMenu,
            true);
    addControl("[Master]",
            "talkoverDucking",
            tr("Microphone Ducking Mode"),
            tr("Toggle microphone ducking mode (OFF, AUTO, MANUAL)"),
            pMicrophoneMenu);
    addMicrophoneAndAuxControl("passthrough",
            tr("Auxiliary On/Off"),
            tr("Auxiliary on/off"),
            pMicrophoneMenu,
            false,
            true);
    pMicrophoneMenu->addSeparator();
    addMicrophoneAndAuxControl("pregain",
            tr("Gain"),
            tr("Gain knob"),
            pMicrophoneMenu,
            true,
            true,
            true);
    addMicrophoneAndAuxControl("volume",
            tr("Volume Fader"),
            tr("Volume Fader"),
            pMicrophoneMenu,
            true,
            true,
            true);
    addMicrophoneAndAuxControl("volume_set_one",
            tr("Full Volume"),
            tr("Set to full volume"),
            pMicrophoneMenu,
            true,
            true);
    addMicrophoneAndAuxControl("volume_set_zero",
            tr("Zero Volume"),
            tr("Set to zero volume"),
            pMicrophoneMenu,
            true,
            true);
    addMicrophoneAndAuxControl("mute",
            tr("Mute"),
            tr("Mute button"),
            pMicrophoneMenu,
            true,
            true);
    addMicrophoneAndAuxControl("pfl",
            tr("Headphone Listen"),
            tr("Headphone listen button"),
            pMicrophoneMenu,
            true,
            true);
    pMicrophoneMenu->addSeparator();
    addMicrophoneAndAuxControl("orientation",
            tr("Orientation"),
            tr("Mix orientation (e.g. left, right, center)"),
            pMicrophoneMenu,
            true,
            true);
    addMicrophoneAndAuxControl("orientation_left",
            tr("Orient Left"),
            tr("Set mix orientation to left"),
            pMicrophoneMenu,
            true,
            true);
    addMicrophoneAndAuxControl("orientation_center",
            tr("Orient Center"),
            tr("Set mix orientation to center"),
            pMicrophoneMenu,
            true,
            true);
    addMicrophoneAndAuxControl("orientation_right",
            tr("Orient Right"),
            tr("Set mix orientation to right"),
            pMicrophoneMenu,
            true,
            true);

    // AutoDJ Controls
    QMenu* pAutodjMenu = addSubmenu(tr("Auto DJ"));
    addControl("[AutoDJ]",
            "shuffle_playlist",
            tr("Auto DJ Shuffle"),
            tr("Shuffle the content of the Auto DJ queue"),
            pAutodjMenu);
    addControl("[AutoDJ]",
            "skip_next",
            tr("Auto DJ Skip Next"),
            tr("Skip the next track in the Auto DJ queue"),
            pAutodjMenu);
    addControl("[AutoDJ]",
            "add_random_track",
            tr("Auto DJ Add Random Track"),
            tr("Add a random track to the Auto DJ queue"),
            pAutodjMenu);
    addControl("[AutoDJ]",
            "fade_now",
            tr("Auto DJ Fade To Next"),
            tr("Trigger the transition to the next track"),
            pAutodjMenu);
    addControl("[AutoDJ]",
            "enabled",
            tr("Auto DJ Toggle"),
            tr("Toggle Auto DJ On/Off"),
            pAutodjMenu);

    // Skin Controls
    QMenu* pGuiMenu = addSubmenu(tr("User Interface"));
    addControl("[Samplers]",
            "show_samplers",
            tr("Samplers Show/Hide"),
            tr("Show/hide the sampler section"),
            pGuiMenu);
    addControl("[Microphone]",
            "show_microphone",
            tr("Microphone & Auxiliary Show/Hide"),
            tr("Show/hide the microphone & auxiliary section"),
            pGuiMenu);
    addControl("[PreviewDeck]",
            "show_previewdeck",
            tr("Preview Deck Show/Hide"),
            tr("Show/hide the preview deck"),
            pGuiMenu);
    addControl("[EffectRack1]",
            "show",
            tr("Effect Rack Show/Hide"),
            tr("Show/hide the effect rack"),
            pGuiMenu);
    addControl("[Skin]",
            "show_4effectunits",
            tr("4 Effect Units Show/Hide"),
            tr("Switches between showing 2 and 4 effect units"),
            pGuiMenu);
    addControl("[Skin]",
            "show_mixer",
            tr("Mixer Show/Hide"),
            tr("Show or hide the mixer."),
            pGuiMenu);
    addControl("[Library]",
            "show_coverart",
            tr("Cover Art Show/Hide (Library)"),
            tr("Show/hide cover art in the library"),
            pGuiMenu);
    addControl("[Skin]",
            "show_maximized_library",
            tr("Library Maximize/Restore"),
            tr("Maximize the track library to take up all the available screen "
               "space."),
            pGuiMenu);

    pGuiMenu->addSeparator();

    addControl("[Skin]",
            "show_4decks",
            tr("Toggle 4 Decks"),
            tr("Switches between showing 2 decks and 4 decks."),
            pGuiMenu);
    addControl("[Skin]",
            "show_coverart",
            tr("Cover Art Show/Hide (Decks)"),
            tr("Show/hide cover art in the main decks"),
            pGuiMenu);
    addControl(VINYL_PREF_KEY,
            "show_vinylcontrol",
            tr("Vinyl Control Show/Hide"),
            tr("Show/hide the vinyl control section"),
            pGuiMenu);

    QString spinnyTitle = tr("Vinyl Spinner Show/Hide");
    QString spinnyDescription = tr("Show/hide spinning vinyl widget");
    QMenu* pSpinnyMenu = addSubmenu(spinnyTitle, pGuiMenu);
    pGuiMenu->addSeparator();
    addControl("[Skin]",
            "show_spinnies",
            tr("Vinyl Spinners Show/Hide (All Decks)"),
            tr("Show/Hide all spinnies"),
            pSpinnyMenu);
    // TODO(ronso0) Add hint that this currently only affects the Shade skin
    for (int i = 1; i <= iNumDecks; ++i) {
        addControl(QString("[Spinny%1]").arg(i),
                "show_spinny",
                QString("%1: %2").arg(m_deckStr.arg(i), spinnyTitle),
                QString("%1: %2").arg(m_deckStr.arg(i), spinnyDescription),
                pSpinnyMenu);
    }

    pGuiMenu->addSeparator();

    addControl("[Skin]",
            "show_waveforms",
            tr("Toggle Waveforms"),
            tr("Show/hide the scrolling waveforms."),
            pGuiMenu);
    addDeckControl("waveform_zoom", tr("Waveform Zoom"), tr("Waveform zoom"), pGuiMenu);
    addDeckControl("waveform_zoom_down", tr("Waveform Zoom In"), tr("Zoom waveform in"), pGuiMenu);
    addDeckControl("waveform_zoom_up", tr("Waveform Zoom Out"), tr("Zoom waveform out"), pGuiMenu);
    addDeckControl("waveform_zoom_set_default",
            tr("Waveform Zoom Reset To Default"),
            tr("Reset the waveform zoom level to the default value selected in "
               "Preferences -> Waveforms"),
            pGuiMenu);

    pGuiMenu->addSeparator();

    // Controls to change a deck's star rating
    addDeckAndPreviewDeckControl("stars_up",
            tr("Star Rating Up"),
            tr("Increase the track rating by one star"),
            pGuiMenu);
    addDeckAndPreviewDeckControl("stars_down",
            tr("Star Rating Down"),
            tr("Decrease the track rating by one star"),
            pGuiMenu);

    // Controls to change a deck's loaded track color
    addDeckAndPreviewDeckControl("track_color_next",
            tr("Select Next Color Available"),
            tr("Select the next color in the color palette for the loaded track."),
            pGuiMenu);
    addDeckAndPreviewDeckControl("track_color_prev",
            tr("Select Previous Color Available"),
            tr("Select previous color in the color palette for the loaded track."),
            pGuiMenu);
    addDeckAndPreviewDeckControl("track_color_selector",
            tr("Navigate Through Track Colors"),
            tr("Select either next or previous color in the palette for the loaded track."),
            pGuiMenu);

    // Misc. controls
    addControl("[Shoutcast]",
            "enabled",
            tr("Start/Stop Live Broadcasting"),
            tr("Stream your mix over the Internet."),
            pGuiMenu);
    addControl(RECORDING_PREF_KEY,
            "toggle_recording",
            tr("Record Mix"),
            tr("Start/stop recording your mix."),
            pGuiMenu);
}

ControlPickerMenu::~ControlPickerMenu() {
}

void ControlPickerMenu::addSingleControl(const QString& group,
        const QString& control,
        const QString& title,
        const QString& description,
        QMenu* pMenu,
        const QString& prefix,
        const QString& actionTitle) {
    int controlIndex;

    if (prefix.isEmpty()) {
        controlIndex = addAvailableControl(ConfigKey(group, control), title, description);
    } else {
        QString prefixedTitle = QString("%1: %2").arg(prefix, title);
        QString prefixedDescription = QString("%1: %2").arg(prefix, description);
        controlIndex = addAvailableControl(ConfigKey(group, control), prefixedTitle, prefixedDescription);
    }

    auto pAction = make_parented<QAction>(actionTitle.isEmpty() ? title : actionTitle, pMenu);
    connect(pAction, &QAction::triggered, this, [this, controlIndex] {
        controlChosen(controlIndex);
    });
    pMenu->addAction(pAction);
}

void ControlPickerMenu::addControl(const QString& group,
        const QString& control,
        const QString& title,
        const QString& description,
        QMenu* pMenu,
        bool addReset,
        const QString& prefix) {
    addSingleControl(group, control, title, description, pMenu, prefix);

    if (addReset) {
        QString resetTitle = QString("%1 (%2)").arg(title, m_resetStr);
        QString resetDescription = QString("%1 (%2)").arg(description, m_resetStr);
        QString resetControl = QString("%1_set_default").arg(control);

        addSingleControl(group, resetControl, resetTitle, resetDescription, pMenu, prefix);
    }
}

void ControlPickerMenu::addPlayerControl(const QString& control,
        const QString& controlTitle,
        const QString& controlDescription,
        QMenu* pMenu,
        bool deckControls,
        bool samplerControls,
        bool previewdeckControls,
        bool addReset) {
    const int iNumSamplers = static_cast<int>(
            ControlObject::get(ConfigKey(kAppGroup, QStringLiteral("num_samplers"))));
    const int iNumDecks = static_cast<int>(ControlObject::get(
            ConfigKey(kAppGroup, QStringLiteral("num_decks"))));
    const int iNumPreviewDecks = static_cast<int>(
            ControlObject::get(ConfigKey(kAppGroup, QStringLiteral("num_preview_decks"))));

    parented_ptr<QMenu> pControlMenu = make_parented<QMenu>(controlTitle, pMenu);
    pMenu->addMenu(pControlMenu);

    parented_ptr<QMenu> pResetControlMenu = nullptr;
    QString resetControl = QString("%1_set_default").arg(control);
    if (addReset) {
        QString resetMenuTitle = QString("%1 (%2)").arg(controlTitle, m_resetStr);
        pResetControlMenu = make_parented<QMenu>(resetMenuTitle, pMenu);
        pMenu->addMenu(pResetControlMenu);
    }

    for (int i = 1; deckControls && i <= iNumDecks; ++i) {
        // PlayerManager::groupForDeck is 0-indexed.
        QString prefix = m_deckStr.arg(i);
        QString group = PlayerManager::groupForDeck(i - 1);
        addSingleControl(group,
                control,
                controlTitle,
                controlDescription,
                pControlMenu,
                prefix,
                prefix);

        if (pResetControlMenu) {
            QString resetTitle = QString("%1 (%2)").arg(controlTitle, m_resetStr);
            QString resetDescription = QString("%1 (%2)").arg(controlDescription, m_resetStr);
            addSingleControl(group,
                    resetControl,
                    resetTitle,
                    resetDescription,
                    pResetControlMenu,
                    prefix,
                    prefix);
        }
    }

    for (int i = 1; previewdeckControls && i <= iNumPreviewDecks; ++i) {
        // PlayerManager::groupForPreviewDeck is 0-indexed.
        QString prefix;
        if (iNumPreviewDecks == 1) {
            prefix = m_previewdeckStr.arg("");
        } else {
            prefix = m_previewdeckStr.arg(i);
        }
        QString group = PlayerManager::groupForPreviewDeck(i - 1);
        addSingleControl(group,
                control,
                controlTitle,
                controlDescription,
                pControlMenu,
                prefix,
                prefix);

        if (pResetControlMenu) {
            QString resetTitle = QString("%1 (%2)").arg(controlTitle, m_resetStr);
            QString resetDescription = QString("%1 (%2)").arg(controlDescription, m_resetStr);
            addSingleControl(group,
                    resetControl,
                    resetTitle,
                    resetDescription,
                    pResetControlMenu,
                    prefix,
                    prefix);
        }
    }

    if (samplerControls) {
        QMenu* pSamplerControlMainMenu = addSubmenu(tr("Samplers"), pControlMenu);
        QMenu* pSamplerControlMenu = pSamplerControlMainMenu;
        QMenu* pSamplerResetControlMainMenu = nullptr;
        QMenu* pSamplerResetControlMenu = nullptr;
        if (pResetControlMenu) {
            pSamplerResetControlMainMenu = addSubmenu(tr("Samplers"), pResetControlMenu);
            pSamplerResetControlMenu = pSamplerResetControlMainMenu;
        }
        const int maxSamplersPerMenu = 16;
        int samplersInMenu = 0;
        QString submenuLabel;
        for (int i = 1; i <= iNumSamplers; ++i) {
            if (samplersInMenu == maxSamplersPerMenu) {
                int limit = iNumSamplers > i + 15 ? i + 15 : iNumSamplers;
                submenuLabel = m_samplerStr.arg(i) + QStringLiteral("- %1").arg(limit);
                pSamplerControlMenu = addSubmenu(submenuLabel, pSamplerControlMainMenu);
            }
            // PlayerManager::groupForSampler is 0-indexed.
            QString prefix = m_samplerStr.arg(i);
            QString group = PlayerManager::groupForSampler(i - 1);
            addSingleControl(group,
                    control,
                    controlTitle,
                    controlDescription,
                    pSamplerControlMenu,
                    prefix,
                    prefix);

            if (pResetControlMenu) {
                if (samplersInMenu == maxSamplersPerMenu) {
                    pSamplerResetControlMenu = addSubmenu(
                            submenuLabel, pSamplerResetControlMainMenu);
                }
                QString resetTitle = QString("%1 (%2)").arg(controlTitle, m_resetStr);
                QString resetDescription = QString("%1 (%2)").arg(controlDescription, m_resetStr);
                addSingleControl(group,
                        resetControl,
                        resetTitle,
                        resetDescription,
                        pSamplerResetControlMenu,
                        prefix,
                        prefix);
            }
            if (samplersInMenu == maxSamplersPerMenu) {
                samplersInMenu = 0;
            }
            samplersInMenu++;
        }
    }
}

void ControlPickerMenu::addMicrophoneAndAuxControl(const QString& control,
        const QString& controlTitle,
        const QString& controlDescription,
        QMenu* pMenu,
        bool microphoneControls,
        bool auxControls,
        bool addReset) {
    parented_ptr<QMenu> pControlMenu = make_parented<QMenu>(controlTitle, pMenu);
    pMenu->addMenu(pControlMenu);

    parented_ptr<QMenu> pResetControlMenu = nullptr;
    QString resetControl = QString("%1_set_default").arg(control);
    if (addReset) {
        QString resetHelpText = QString("%1 (%2)").arg(controlTitle, m_resetStr);
        pResetControlMenu = make_parented<QMenu>(resetHelpText, pMenu);
        pMenu->addMenu(pResetControlMenu);
    }

    if (microphoneControls) {
        const int kNumMicrophones = static_cast<int>(
                ControlObject::get(ConfigKey(kAppGroup, QStringLiteral("num_microphones"))));
        for (int i = 1; i <= kNumMicrophones; ++i) {
            QString prefix = m_microphoneStr.arg(i);
            QString group = PlayerManager::groupForMicrophone(i - 1);
            addSingleControl(group,
                    control,
                    controlTitle,
                    controlDescription,
                    pControlMenu,
                    prefix,
                    prefix);

            if (pResetControlMenu) {
                QString resetTitle = QString("%1 (%2)").arg(controlTitle, m_resetStr);
                QString resetDescription = QString("%1 (%2)").arg(controlDescription, m_resetStr);
                addSingleControl(group,
                        resetControl,
                        resetTitle,
                        resetDescription,
                        pResetControlMenu,
                        prefix,
                        prefix);
            }
        }
    }

    const int kNumAuxiliaries = static_cast<int>(
            ControlObject::get(ConfigKey(kAppGroup, QStringLiteral("num_auxiliaries"))));
    if (auxControls) {
        for (int i = 1; i <= kNumAuxiliaries; ++i) {
            QString prefix = m_auxStr.arg(i);
            QString group = PlayerManager::groupForAuxiliary(i - 1);
            addSingleControl(group,
                    control,
                    controlTitle,
                    controlDescription,
                    pControlMenu,
                    prefix,
                    prefix);

            if (pResetControlMenu) {
                QString resetTitle = QString("%1 (%2)").arg(controlTitle, m_resetStr);
                QString resetDescription = QString("%1 (%2)").arg(controlDescription, m_resetStr);
                addSingleControl(group,
                        resetControl,
                        resetTitle,
                        resetDescription,
                        pResetControlMenu,
                        prefix,
                        prefix);
            }
        }
    }
}

void ControlPickerMenu::addDeckAndSamplerControl(const QString& control,
        const QString& title,
        const QString& controlDescription,
        QMenu* pMenu,
        bool addReset) {
    addPlayerControl(control, title, controlDescription, pMenu, true, true, false, addReset);
}

void ControlPickerMenu::addDeckAndPreviewDeckControl(const QString& control,
        const QString& title,
        const QString& controlDescription,
        QMenu* pMenu,
        bool addReset) {
    addPlayerControl(control, title, controlDescription, pMenu, true, false, true, addReset);
}

void ControlPickerMenu::addDeckAndSamplerAndPreviewDeckControl(const QString& control,
        const QString& title,
        const QString& controlDescription,
        QMenu* pMenu,
        bool addReset) {
    addPlayerControl(control, title, controlDescription, pMenu, true, true, true, addReset);
}

void ControlPickerMenu::addDeckControl(const QString& control,
        const QString& title,
        const QString& controlDescription,
        QMenu* pMenu,
        bool addReset) {
    addPlayerControl(control, title, controlDescription, pMenu, true, false, false, addReset);
}

void ControlPickerMenu::addSamplerControl(const QString& control,
        const QString& title,
        const QString& controlDescription,
        QMenu* pMenu,
        bool addReset) {
    addPlayerControl(control, title, controlDescription, pMenu, false, true, false, addReset);
}

void ControlPickerMenu::addPreviewDeckControl(const QString& control,
        const QString& title,
        const QString& controlDescription,
        QMenu* pMenu,
        bool addReset) {
    addPlayerControl(control, title, controlDescription, pMenu, false, false, true, addReset);
}

void ControlPickerMenu::addLibraryControl(const QString& control,
        const QString& title,
        const QString& description,
        QMenu* pMenu) {
    addSingleControl("[Library]", control, title, description, pMenu, m_libraryStr);
}

QMenu* ControlPickerMenu::addSubmenu(QString title, QMenu* pParent) {
    if (pParent == nullptr) {
        pParent = this;
    }
    auto pSubMenu = make_parented<QMenu>(title, pParent);
    pParent->addMenu(pSubMenu);
    return pSubMenu;
}

void ControlPickerMenu::controlChosen(int controlIndex) {
    if (controlIndex < 0 || controlIndex >= m_controlsAvailable.size()) {
        return;
    }
    emit controlPicked(m_controlsAvailable[controlIndex]);
}

int ControlPickerMenu::addAvailableControl(const ConfigKey& key,
        const QString& title,
        const QString& description) {
    m_controlsAvailable.append(key);
    m_descriptionsByKey.insert(key, description);
    m_titlesByKey.insert(key, title);
    // return the index of the control which will be connected to the index
    // of the respective action in the menu
    return m_controlsAvailable.size() - 1;
}

bool ControlPickerMenu::controlExists(const ConfigKey& key) const {
    return m_titlesByKey.contains(key);
}

QString ControlPickerMenu::descriptionForConfigKey(const ConfigKey& key) const {
    return m_descriptionsByKey.value(key, QString());
}

QString ControlPickerMenu::controlTitleForConfigKey(const ConfigKey& key) const {
    return m_titlesByKey.value(key, QString());
}
