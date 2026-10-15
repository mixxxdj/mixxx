pragma Singleton
import QtQuick
import "."

QtObject {
    property bool layoutAnimationsEnabled: true

    readonly property color accentColor: ColorScheme.accentColor
    readonly property color activePlayCueColor: isClassic ? "#ff2d55" : "#ff7a18"
    readonly property url assetDeckArrowLeftUpButton: lateNightAsset("buttons", "btn__arrow_left_up.svg")
    readonly property url assetDeckArrowRightDownButton: lateNightAsset("buttons", "btn__arrow_right_down.svg")
    readonly property url assetDeckBeatCurposButton: lateNightAsset("buttons", "btn__beat_curpos.svg")
    readonly property url assetDeckBeatCurposLargeButton: lateNightAsset("buttons", "btn__beat_curpos_large.svg")
    readonly property url assetDeckBeatSpinBoxBorder: isClassic ? lateNightAsset("buttons", "spinbox_elevated_border.svg") : lateNightSubRegionButton("wide")
    readonly property url assetDeckBeatSpinBoxFocusBorder: isClassic ? lateNightAsset("buttons", "spinbox_elevated_border_focus.svg") : lateNightAsset("buttons", "btn_embedded_spinbox_focus_blue.svg")
    readonly property url assetDeckBeatSpinBoxDownButton: isClassic ? lateNightAsset("buttons", "spinbox_down.svg") : lateNightAsset("buttons", "btn__spinbox_down.svg")
    readonly property url assetDeckBeatSpinBoxUpButton: isClassic ? lateNightAsset("buttons", "spinbox_up.svg") : lateNightAsset("buttons", "btn__spinbox_up.svg")
    readonly property url assetDeckBeatgridControlsCollapseButton: lateNightAsset("buttons", "btn__beatgrid_controls_collapse.svg")
    readonly property url assetDeckBeatgridControlsExpandButton: lateNightAsset("buttons", "btn__beatgrid_controls_expand.svg")
    readonly property url assetDeckBeatjumpLeftButton: lateNightAsset("buttons", "btn__beatjump_left.svg")
    readonly property url assetDeckBeatjumpRightButton: lateNightAsset("buttons", "btn__beatjump_right.svg")
    readonly property url assetDeckBeatsEarlierButton: lateNightAsset("buttons", "btn__beats_earlier.svg")
    readonly property url assetDeckBeatsFasterButton: lateNightAsset("buttons", "btn__beats_faster.svg")
    readonly property url assetDeckBeatsHotcuesEarlierButton: lateNightAsset("buttons", "btn__beats_hotcues_earlier.svg")
    readonly property url assetDeckBeatsHotcuesLaterButton: lateNightAsset("buttons", "btn__beats_hotcues_later.svg")
    readonly property url assetDeckBeatsLaterButton: lateNightAsset("buttons", "btn__beats_later.svg")
    readonly property url assetDeckBeatsSlowerButton: lateNightAsset("buttons", "btn__beats_slower.svg")
    readonly property url assetDeckBpmLockedButton: lateNightAsset("buttons", "btn__bpm_locked.svg")
    readonly property url assetDeckBpmSelectEditButton: lateNightAsset("buttons", "btn__bpm_select_edit.svg")
    readonly property url assetDeckBpmSelectTapButton: lateNightAsset("buttons", "btn__bpm_select_tap.svg")
    readonly property url assetDeckBpmSpinboxMinusButton: lateNightAsset("buttons", "btn__bpm_spinbox_minus.svg")
    readonly property url assetDeckBpmSpinboxMinusPressedButton: lateNightAsset("buttons", isClassic ? "btn__bpm_spinbox_minus_pressed.svg" : "btn__bpm_spinbox_minus.svg")
    readonly property url assetDeckBpmSpinboxPlusButton: lateNightAsset("buttons", "btn__bpm_spinbox_plus.svg")
    readonly property url assetDeckBpmSpinboxPlusPressedButton: lateNightAsset("buttons", isClassic ? "btn__bpm_spinbox_plus_pressed.svg" : "btn__bpm_spinbox_plus.svg")
    readonly property url assetDeckBpmUnlockedButton: lateNightAsset("buttons", "btn__bpm_unlocked.svg")
    readonly property url assetDeckCoverDefault: lateNightAsset("style", "cover_default.svg")
    readonly property url assetDeckCueButton: lateNightAsset("buttons", "btn__cue_deck.svg")
    readonly property url assetDeckEjectButton: lateNightAsset("buttons", "btn__eject.svg")
    readonly property url assetDeckIntroEndButton: lateNightAsset("buttons", "btn__intro_end.svg")
    readonly property url assetDeckIntroStartButton: lateNightAsset("buttons", "btn__intro_start.svg")
    readonly property url assetDeckKeyButtonBackground: lateNightAsset("buttons", "btn_embedded_library.svg")
    readonly property url assetDeckKeyDownButton: lateNightAsset("buttons", "btn__key_down.svg")
    readonly property url assetDeckKeyMatchButton: lateNightAsset("buttons", "btn__key_match.svg")
    readonly property url assetDeckKeyUpButton: lateNightAsset("buttons", "btn__key_up.svg")
    readonly property url assetDeckKeylockButton: lateNightAsset("buttons", "btn__keylock.svg")
    readonly property url assetDeckLeaderBackground: lateNightAsset("buttons", "btn_embedded_grid.svg")
    readonly property url assetDeckLeaderButton: lateNightAsset("buttons", "btn__sync_leader.svg")
    readonly property url assetDeckLeaderExplicitButton: isPaleMoon ? lateNightAsset("buttons", "btn__sync_leader_explicit.svg") : lateNightAsset("buttons", "btn__sync_leader_active.svg")
    readonly property url assetDeckLeaderImplicitButton: isPaleMoon ? lateNightAsset("buttons", "btn__sync_leader_implicit.svg") : lateNightAsset("buttons", "btn__sync_leader_active.svg")
    readonly property url assetDeckLoopAnchorEndButton: lateNightAsset("buttons", "btn__loop_anchor_end.svg")
    readonly property url assetDeckLoopAnchorStartButton: lateNightAsset("buttons", "btn__loop_anchor_start.svg")
    readonly property url assetDeckLoopButton: lateNightAsset("buttons", "btn__loop.svg")
    readonly property url assetDeckLoopInButton: lateNightAsset("buttons", "btn__loop_in.svg")
    readonly property url assetDeckLoopOutButton: lateNightAsset("buttons", "btn__loop_out.svg")
    readonly property url assetDeckMinusButton: lateNightAsset("buttons", "btn__minus.svg")
    readonly property url assetDeckOutroEndButton: lateNightAsset("buttons", "btn__outro_end.svg")
    readonly property url assetDeckOutroStartButton: lateNightAsset("buttons", "btn__outro_start.svg")
    readonly property url assetDeckPlayButton: lateNightAsset("buttons", "btn__play_deck.svg")
    readonly property url assetDeckPlayMiniButton: lateNightAsset("buttons", "btn__play_deck_mini.svg")
    readonly property url assetDeckPauseMiniButton: lateNightAsset("buttons", "btn__pause_deck_mini.svg")
    readonly property url assetDeckPlusButton: lateNightAsset("buttons", "btn__plus.svg")
    readonly property url assetDeckQuantizeButton: lateNightAsset("buttons", "btn__quantize.svg")
    readonly property url assetDeckRateSliderBackground: lateNightAsset("sliders", "slider_pitch_deck.svg")
    readonly property url assetDeckRateCompactSliderBackground: lateNightAsset("sliders", "slider_pitch_deck_compact.svg")
    readonly property url assetDeckRateCompactSyncSliderBackground: lateNightAsset("sliders", "slider_pitch_deck_compact_sync.svg")
    readonly property url assetDeckRateSliderHandle: lateNightAsset("sliders", "knob_pitch_deck.svg")
    readonly property url assetDeckReloopButton: lateNightAsset("buttons", "btn__reloop.svg")
    readonly property url assetDeckRepeatButton: lateNightAsset("buttons", "btn__repeat.svg")
    readonly property url assetDeckReverseButton: lateNightAsset("buttons", "btn__reverse.svg")
    readonly property url assetDeckSettingsOffButton: lateNightAsset("buttons", "btn__settings_off.svg")
    readonly property url assetDeckSettingsOnButton: lateNightAsset("buttons", "btn__settings_on.svg")
    readonly property url assetDeckSlipButton: lateNightAsset("buttons", "btn__slip.svg")
    readonly property url assetDeckSpinnyBackground: lateNightAsset("style", "spinny_bg.svg")
    readonly property url assetDeckSpinnyGhostIndicator: lateNightAsset("style", "spinny_indicator_ghost.svg")
    readonly property url assetDeckSpinnyIndicator: lateNightAsset("style", "spinny_indicator.svg")
    readonly property url assetDeckSpinnyMask12: lateNightAsset("style", "spinny_mask_12.svg")
    readonly property url assetDeckSpinnyMask34: lateNightAsset("style", "spinny_mask_34.svg")
    readonly property url assetDeckStemControlsCollapseButton: lateNightAsset("buttons", "btn__stem_controls_collapse.svg")
    readonly property url assetDeckStemControlsExpandButton: lateNightAsset("buttons", "btn__stem_controls_expand.svg")
    readonly property url assetWaveformSplitterHandle: lateNightAsset("style", isClassic ? "splitter_handle_horizontal.png" : "splitter_handle_horizontal.svg")
    readonly property url assetWaveformSplitterHandlePressed: lateNightAsset("style", isClassic ? "splitter_handle_horizontal_pressed.png" : "splitter_handle_horizontal_pressed.svg")
    readonly property url assetDeckSyncActiveButton: isPaleMoon ? lateNightAsset("buttons", "btn__sync_deck_active.svg") : lateNightAsset("buttons", "btn__sync_deck.svg")
    readonly property url assetDeckSyncBackground: lateNightAsset("buttons", "btn_embedded_library.svg")
    readonly property url assetDeckSyncButton: lateNightAsset("buttons", "btn__sync_deck.svg")
    readonly property url assetDeckUndoButton: lateNightAsset("buttons", "btn__undo.svg")
    readonly property url assetDeckVinylControl0: lateNightAsset("style", "vinyl_control_0.svg")
    readonly property url assetDeckVinylControl1: lateNightAsset("style", "vinyl_control_1.svg")
    readonly property url assetDeckVinylControl2: lateNightAsset("style", "vinyl_control_2.svg")
    readonly property url assetDeckVinylControl3: lateNightAsset("style", "vinyl_control_3.svg")
    readonly property url assetDeckVolumeSliderBackground: lateNightAsset("sliders", "slider_volume_deck.svg")
    readonly property url assetDeckVolumeSliderHandle: lateNightAsset("sliders", "knob_volume_deck.svg")
    readonly property url assetAuxMainMixButton: lateNightAsset("buttons", "btn__aux_play.svg")
    readonly property url assetAuxXfaderLeft: lateNightAsset("buttons", "btn__xfader_deck_left_default.svg")
    readonly property url assetAuxXfaderMain: lateNightAsset("buttons", "btn__xfader_deck_mid_default.svg")
    readonly property url assetAuxXfaderRight: lateNightAsset("buttons", "btn__xfader_deck_right_default.svg")
    readonly property url assetFxCollapseButton: lateNightAsset("buttons", isClassic ? "btn__collapse.svg" : "btn__collapse_dim.svg")
    readonly property url assetFxExpandButton: lateNightAsset("buttons", isClassic ? "btn__expand.svg" : "btn__expand_dim.svg")
    readonly property url assetFxFlowHorizontal: lateNightAsset("style", isClassic ? "fx_separator.svg" : "fx_flow_horizontal.svg")
    readonly property url assetFxFlowVertical: lateNightAsset("style", isClassic ? "fx_separator.svg" : "fx_flow_vertical.svg")
    readonly property url assetFxFocusActiveButton: lateNightAsset("buttons", "btn__fx_focus_active.svg")
    readonly property url assetFxFocusButton: lateNightAsset("buttons", "btn__fx_focus.svg")
    readonly property url assetFxKnobBackground: lateNightAsset("knobs", "knob_bg_fx.svg")
    readonly property url assetFxMixModeButton: lateNightAsset("buttons", "btn_embedded_mixmode.svg")
    readonly property url assetFxMixModeDryWetButton: lateNightAsset("buttons", "btn__fx_mixmode_d-w.svg")
    readonly property url assetFxMixModeDryWetSumButton: lateNightAsset("buttons", "btn__fx_mixmode_d+w.svg")
    readonly property url assetFxParameterActiveButton: lateNightAsset("buttons", "btn_embedded_fx_parameter_active.svg")
    readonly property url assetFxParameterButton: lateNightAsset("buttons", "btn_embedded_fx_parameter.svg")
    readonly property url assetFxSelectorActiveBorder: lateNightAsset("buttons", "btn_embedded_library_active.svg")
    readonly property url assetFxSelectorBorder: lateNightAsset("buttons", "btn_embedded_library.svg")
    readonly property url assetFxSelectorDownButton: lateNightAsset("buttons", "btn__fx_selector_down.svg")
    readonly property url assetFxSettingsButton: lateNightAsset("buttons", "btn__fx_settings.svg")
    readonly property url assetFxSlotButtonActiveBackground: lateNightAsset("buttons", "btn_embedded_square_active.svg")
    readonly property url assetFxSlotButtonBackground: lateNightAsset("buttons", "btn_embedded_square.svg")
    readonly property url assetFxToggleActiveButton: lateNightAsset("buttons", "btn__fx_toggle_active.svg")
    readonly property url assetFxToggleButton: lateNightAsset("buttons", "btn__fx_toggle.svg")
    readonly property url assetMainKnobBackground: lateNightAsset("knobs", "knob_bg_main.svg")
    readonly property url assetMicAuxAddButton: lateNightAsset("buttons", "btn__plus_flat.svg")
    readonly property url assetMicAuxGainKnobBackground: lateNightAsset("knobs", "knob_bg_small.svg")
    readonly property url assetMicAuxUnconfiguredBackground: lateNightAsset("buttons", "btn_flat_square.svg")
    readonly property url assetMicAuxVuClippingActive: lateNightAsset("style", "vu_micaux_clipping_active.png")
    readonly property url assetMicAuxVuClippingBackground: lateNightAsset("style", "vu_micaux_clipping_bg_.png")
    readonly property url assetMicAuxVuLevelActive: lateNightAsset("style", "vu_micaux_level_active.png")
    readonly property url assetMicAuxVuLevelBackground: lateNightAsset("style", "vu_micaux_level_bg_.png")
    readonly property url assetMicDuckAutoButton: lateNightAsset("buttons", "btn__mic_duck_auto.svg")
    readonly property url assetMicDuckManualButton: lateNightAsset("buttons", "btn__mic_duck_manual.svg")
    readonly property url assetMicDuckOffButton: lateNightAsset("buttons", "btn__mic_duck_off.svg")
    readonly property url assetMicTalkButton: lateNightAsset("buttons", "btn__mic_talk.svg")
    readonly property url assetMixerCrossfaderBackground: lateNightAsset("sliders", "slider_crossfader.svg")
    readonly property url assetMixerCrossfaderHandle: lateNightAsset("sliders", "knob_crossfader.svg")
    readonly property url assetMixerCrossfaderSmallBackground: lateNightAsset("sliders", "slider_crossfader_small.svg")
    readonly property url assetMixerEqKillButtonActiveBackground: lateNightAsset("buttons", "btn_embedded_eqkill_active.svg")
    readonly property url assetMixerEqKillButtonBackground: lateNightAsset("buttons", "btn_embedded_eqkill.svg")
    readonly property url assetMixerEqKillHighIcon: lateNightAsset("buttons", "btn__eq_kill_high.svg")
    readonly property url assetMixerEqKillLowIcon: lateNightAsset("buttons", "btn__eq_kill_low.svg")
    readonly property url assetMixerEqKillMidIcon: lateNightAsset("buttons", "btn__eq_kill_mid.svg")
    readonly property url assetMixerPflActiveIcon: isPaleMoon ? lateNightAsset("buttons", "btn__pfl_active.svg") : lateNightAsset("buttons", "btn__pfl.svg")
    readonly property url assetMixerPflBackground: lateNightTopRegionButton("square")
    readonly property url assetMixerPflIcon: lateNightAsset("buttons", "btn__pfl.svg")
    readonly property url assetMixerQuickEffectIcon: lateNightAsset("buttons", "btn__star.svg")
    readonly property url assetMixerSplitActiveIcon: lateNightAsset("buttons", isClassic ? "btn_elevated_headsplit_active.svg" : "btn_embedded_headsplit_active.svg")
    readonly property url assetMixerSplitIcon: lateNightAsset("buttons", isClassic ? "btn_elevated_headsplit.svg" : "btn_embedded_headsplit.svg")
    readonly property url assetMixerStemMuteIcon: lateNightAsset("buttons", "btn__stem_mute.svg")
    readonly property url assetMixerVolumeSliderBackground: lateNightAsset("sliders", "slider_volume_deck.svg")
    readonly property url assetMixerVolumeSliderHandle: lateNightAsset("sliders", "knob_volume_deck.svg")
    readonly property url assetRegularKnobBackground: lateNightAsset("knobs", "knob_bg_regular.svg")
    readonly property url assetSamplerCollapseButton: lateNightAsset("buttons", "btn__collapse_dim.svg")
    readonly property url assetSamplerExpandButton: lateNightAsset("buttons", "btn__expand_dim.svg")
    readonly property url assetSamplerPauseButton: lateNightAsset("buttons", "btn__pause_sampler.svg")
    readonly property url assetSamplerPitchSliderBackground: lateNightAsset("sliders", "slider_pitch_sampler.svg")
    readonly property url assetSamplerPitchSliderHandle: lateNightAsset("sliders", "knob_pitch_sampler.svg")
    readonly property url assetSamplerPlayButton: lateNightAsset("buttons", "btn__play_sampler.svg")
    readonly property url assetSamplerSyncButton: lateNightAsset("buttons", "btn__sync_sampler.svg")
    readonly property url assetSamplerVuClippingActive: lateNightAsset("style", "vu_sampler_clipping_active.png")
    readonly property url assetSamplerVuClippingBackground: lateNightAsset("style", "vu_sampler_clipping_bg_.png")
    readonly property url assetSamplerVuLevelActive: lateNightAsset("style", "vu_sampler_level_active.png")
    readonly property url assetSamplerVuLevelBackground: lateNightAsset("style", "vu_sampler_level_bg_.png")
    readonly property url assetSamplerXfaderLeft: lateNightAsset("buttons", "btn__xfader_sampler_left.svg")
    readonly property url assetSamplerXfaderMain: lateNightAsset("buttons", "btn__xfader_sampler_main.svg")
    readonly property url assetSamplerXfaderRight: lateNightAsset("buttons", "btn__xfader_sampler_right.svg")
    readonly property url assetSmallKnobBackground: lateNightAsset("knobs", "knob_bg_small.svg")
    readonly property url assetToolbarDropdownIcon: lateNightAsset("buttons", "btn__fx_selector_down.svg")
    readonly property url assetToolbarMenuIcon: lateNightAsset("buttons", "btn__menu.svg")
    readonly property color backgroundColor: "#0d0d0d"
    readonly property color beatgridDisabledCoverColor: "#b4151517"
    readonly property color bpmTapEditorBackgroundColor: "#0a0a0a"
    readonly property color bpmTapEditorButtonColor: "#80131313"
    readonly property color bpmTapEditorEditBorderColor: isPaleMoon ? "#257b82" : "#d08e00"
    readonly property color bpmTapEditorSelectBackgroundColor: "#111111"
    readonly property color bpmTapEditorSelectBorderColor: isPaleMoon ? "#7d350d" : "#5E4507"
    readonly property color buttonActiveColor: white
    readonly property color buttonNormalColor: "#8c8c8c"
    readonly property color buttonPressedColor: white
    readonly property color darkGray: "#0a0a0a"
    readonly property color deckActiveButtonTextColor: "#000000"
    readonly property color deckBeatSpinBoxFocusedSelectedTextColor: "#000000"
    readonly property color deckBeatSpinBoxFocusedSelectionColor: "#d2d2d2"
    readonly property color deckBeatSpinBoxSelectedTextColor: isClassic ? "#000000" : "#a8a8a8"
    // Serato style per-deck colours: odd channels (1 and 3) use the cyan "A"
    // colour, even channels (2 and 4) the red "B" colour, which is what
    // VirtualDJ uses to tell the two sides of the mixer apart. Only the
    // NeonDeck scheme uses them, Classic and PaleMoon keep their accent colour.
    readonly property color deckAColor: "#0096f3"
    readonly property color deckBColor: "#ff3b30"
    readonly property bool usePerDeckColors: ColorScheme.name === "neondeck"
    // VirtualDJ draws every waveform in tones of its deck colour: a dark body for
    // the lows, the deck colour for the mids and a light tint for the transients.
    // When enabled the skin uses the filtered renderer with those tones instead
    // of the multi coloured RGB renderer chosen in the preferences.
    readonly property bool deckColorWaveforms: usePerDeckColors

    function waveformLowColorForGroup(group) {
        return Qt.darker(deckColorForGroup(group), 2.4);
    }

    function waveformMidColorForGroup(group) {
        return Qt.darker(deckColorForGroup(group), 1.25);
    }

    function waveformHighColorForGroup(group) {
        return Qt.tint(deckColorForGroup(group), "#a6ffffff");
    }
    // Frosted glass surfaces: translucent white borders and highlights on top of
    // the panel colour, so controls read as glossy glass instead of flat blocks.
    // Active states use a translucent tint of the deck colour plus a bright
    // border instead of a solid saturated fill.
    readonly property color glassBorderColor: "#1fffffff"
    readonly property color glassBorderColorStrong: "#33ffffff"
    readonly property color glassHighlightColor: "#14ffffff"
    readonly property color glassShadeColor: "#40000000"
    readonly property color glassSurfaceColor: "#0dffffff"
    readonly property color glassSurfaceHoverColor: "#1affffff"
    // Type scale, so labels and readouts keep a consistent hierarchy.
    readonly property int fontSizeTiny: 9
    readonly property int fontSizeSmall: 11
    readonly property int fontSizeBody: 13
    readonly property int fontSizeSubtitle: 15
    readonly property int fontSizeTitle: 17
    readonly property int fontSizeReadout: 24
    readonly property real labelLetterSpacing: 0.6
    readonly property real activeFillOpacity: 0.32
    readonly property real activePressedFillOpacity: 0.48

    // Returns the same colour with another opacity, for translucent surfaces.
    function withAlpha(inputColor, alpha) {
        return Qt.rgba(inputColor.r, inputColor.g, inputColor.b, alpha);
    }
    readonly property color deckBeatSpinBoxSelectionColor: isClassic ? "#d2d2d2" : "#111111"
    readonly property color deckBeatSpinBoxTextColor: isClassic ? "#6b8399" : "#a8a8a8"
    readonly property color deckButtonInactiveColor: isClassic ? "#131e2a" : "#70101010"
    readonly property color deckDimButtonInactiveColor: isClassic ? "#131e2a" : "#80131313"
    readonly property color deckEmbeddedButtonInactiveColor: isClassic ? "#131e2a" : "#161616"
    readonly property color deckPanelBorderDark: "#40000000"
    readonly property color deckPanelBorderLeft: "#14ffffff"
    readonly property color deckPanelBorderLight: "#26ffffff"
    readonly property color deckPanelBorderRight: deckTopRowBackgroundColor
    readonly property color deckPanelColor: "#171717"
    readonly property color deckReadonlyTextColor: isClassic ? "#6b8399" : "#9a9a9a"
    readonly property color deckTimeTextColor: isClassic ? "#ffd60a" : "#9a9a9a"
    readonly property color deckTopRowBackgroundColor: "#111111"
    readonly property color layoutGutterColor: isClassic ? "#0a0a0a" : "#080808"
    readonly property int deckMixerGutter: isClassic ? 4 : 3
    readonly property int deckRowGutter: isClassic ? 4 : 3
    readonly property int deckOuterMargin: isClassic ? 2 : 1
    readonly property int fullDeckHeight: 230
    // Legacy Compact is a 110px overview row plus a 30px transport shell,
    // including the outer deck insets. Keep this separate from Full's 55px
    // transport so hidden optional controls do not leave a phantom half-row.
    readonly property int compactDeckHeight: 150
    readonly property int compactDeckTransportHeight: 30
    // Compact rate controls retain the legacy 62px sync row plus the
    // separator and horizontal insets. Keeping the outer width explicit
    // prevents the leader button from being clipped by the deck edge.
    readonly property int compactRateControlWidth: 66
    readonly property int miniDeckHeight: 53
    readonly property int fullBigSpinnySize: 114
    readonly property int compactBigSpinnySize: 110
    // Width of the deck settings column (rating stars and the slip/quantize/
    // curpos/eject/repeat/keylock buttons) that OverviewRow keeps on the right
    // edge of the overview strip. The jog of the Full deck reserves this width
    // so its ring cannot cover those buttons.
    readonly property int deckSettingsColumnWidth: 76
    readonly property color compactVuPanelColor: isClassic ? "#161616" : "#111111"
    readonly property color compactVuPanelBorderColor: isClassic ? "#262626" : "#212123"
    readonly property color compactVuPanelBorderBottomColor: isClassic ? "#04080b" : "#010304"
    readonly property color compactVuPanelBorderLeftColor: isClassic ? "#262626" : "#191919"
    readonly property color compactVuPanelBorderRightColor: isClassic ? "#04080b" : "#111111"
    readonly property color compactVuGutterColor: isClassic ? "#040404" : "#080808"
    readonly property int compactVuMeterHeight: 96
    readonly property int compactVuDeckGroupWidth: isClassic ? 19 : 18
    readonly property int compactVuMainGroupWidth: 34
    readonly property int compactVuSlotWidth: compactVuDeckGroupWidth * 2 + compactVuMainGroupWidth
    readonly property int smallSpinnySize: 63
    readonly property int miniSpinnySize: 53
    readonly property int deckTransportHeight: 55
    readonly property color effectsAssignmentActiveTextColor: "#000000"
    readonly property color effectsAssignmentInactiveColor: isClassic ? "#131e2a" : "#161616"
    readonly property color effectsAssignmentInactiveTextColor: isClassic ? "#cfe6f5" : "#4a5f73"
    readonly property color effectsControlActiveColor: "#6b8399"
    readonly property color effectsControlInactiveColor: "#131e2a"
    readonly property color effectsControllerColor12: isClassic ? "#73b508" : "#518f00"
    readonly property color effectsControllerColor34: isClassic ? "#0795b5" : "#028392"
    readonly property color effectsFillerColor: isClassic ? "#171717" : "#111111"
    readonly property color effectsFocusBorderColor: isClassic ? "#d08e00" : "#257b82"
    readonly property color effectsHeaderColor: isClassic ? "#161616" : "#111111"
    readonly property color effectsMasterButtonInactiveColor: isClassic ? "#131e2a" : "#161616"
    readonly property color effectsPanelColor: isClassic ? "#161616" : "#161616"
    readonly property color effectsParameterActiveColor: "#6b8399"
    readonly property color effectsParameterArcColor: "#6d6d6d"
    readonly property color effectsParameterInactiveColor: isClassic ? "#262626" : "#2a2a2c"
    readonly property string effectsParameterIndicatorColor: isClassic ? "white" : "grey"
    readonly property color effectsParameterInverseActiveColor: "#9c0900"
    readonly property color effectsParameterLinkInactiveColor: isClassic ? "#4b4b4b" : "#262626"
    readonly property color effectsParameterPanelColor: isClassic ? "#111111" : "#161616"
    readonly property color effectsParameterTextColor: "#4a5f73"
    readonly property color effectsRackGutterColor: layoutGutterColor
    readonly property color effectsSlotToggleInactiveColor: isClassic ? "#131e2a" : "#70101010"
    readonly property color effectsUnitColor12: isClassic ? "#39ff14" : "#438225"
    readonly property color effectsUnitColor34: isClassic ? "#0895bc" : "#257b82"
    readonly property color effectsUnitDimColor12: isClassic ? "#426b00" : "#236b00"
    readonly property color effectsUnitDimColor34: isClassic ? "#00696b" : "#146674"
    readonly property bool isClassic: ColorScheme.name === "classic"
    readonly property bool isPaleMoon: ColorScheme.name === "palemoon"
    readonly property color keyControlsPressedColor: isPaleMoon ? "#7d350d" : "#ff2d55"
    readonly property string keyControlsPressedIconSuffix: isPaleMoon ? "active" : ""
    readonly property color libraryPanelSplitterBackground: isPaleMoon ? "#080808" : "#0a0a0a"
    readonly property color libraryPanelSplitterHandle: "#5f5f5f"
    readonly property color libraryPanelSplitterHandleActive: "#7a7a7a"
    readonly property color mixerAccentCyan: "#0bd9d1"
    readonly property color mixerAccentOrange: isClassic ? "#00d5ff" : "#ff7a18"
    readonly property color mixerAccentRed: isClassic ? "#ff2d55" : "#a80000"
    // Neutral control arcs: VirtualDJ keeps them grey and lets the deck colour
    // appear only in the waveform, the jog ring and the pads.
    readonly property color mixerArcEqColor: "#8f8f8f"
    readonly property color mixerArcGainColor: "#a6a6a6"
    readonly property color mixerArcGainLowColor: "#7d7d7d"
    readonly property color mixerArcMainBalanceColor: "#9c9c9c"
    readonly property color mixerArcQuickEffectColor: "#8f8f8f"
    readonly property real mixerArcRadiusBig: 14.5
    readonly property real mixerArcRadiusCompact: 12.5
    readonly property real mixerArcWidth: 2
    readonly property color mixerControlTextColor: isClassic ? "#cfe6f5" : "#a8a8a8"
    readonly property color mixerDimTextColor: "#8c8c8c"
    readonly property color mixerEqKillActiveColor: isClassic ? "#ff2d55" : "#a80000"
    readonly property color mixerFxAssignInactiveColor: deckEmbeddedButtonInactiveColor
    readonly property color mixerFxAssignInactiveTextColor: isClassic ? effectsAssignmentInactiveTextColor : "#4a5f73"
    readonly property color mixerMainSeparatorDarkColor: isPaleMoon ? "#0a0a0a" : mixerPanelBorderDark
    readonly property color mixerMainSeparatorLightColor: isPaleMoon ? "#101a26" : mixerPanelBorderLight
    readonly property color mixerPanelBorderBottom: isPaleMoon ? "#0a0a0a" : "#04080b"
    readonly property color mixerPanelBorderDark: "#080808"
    readonly property color mixerPanelBorderLeft: isPaleMoon ? "#2e2e2e" : "#262626"
    readonly property color mixerPanelBorderLight: "#343434"
    readonly property color mixerPanelBorderRight: isPaleMoon ? "#141414" : "#04080b"
    readonly property color mixerPanelBorderTop: "#262626"
    readonly property color mixerPanelColor: "#1d1d1f"
    readonly property color mixerPflActiveFillColor: isClassic ? "#ff2d55" : "#4a5f73"
    readonly property color mixerQuickEffectActiveColor: isClassic ? "#39ff14" : "#236b00"
    readonly property color mixerQuickEffectSelectorHighlightColor: isClassic ? "#5e4507" : "#2c454f"
    readonly property color mixerQuickEffectSelectorPopupBackgroundColor: isClassic ? "#0a0a0a" : "#111111"
    readonly property color mixerQuickEffectSelectorPopupBorderColor: isClassic ? "#6b8399" : "#262626"
    readonly property color mixerQuickEffectSelectorHighlightTextColor: white
    readonly property color mixerQuickEffectSelectorTextColor: isClassic ? "#6b8399" : "#a8a8a8"
    readonly property color mixerSliderBarColor: "#8a8a8a"
    readonly property color mixerSplitActiveColor: isClassic ? "#6b8399" : "#555555"
    readonly property color mixerSplitInactiveColor: isClassic ? deckEmbeddedButtonInactiveColor : "#101a26"
    readonly property color mixerStemMuteInactiveColor: isClassic ? "#131e2a" : "#2a2a2c"
    readonly property color mixerVuClipColor: mixerAccentRed
    readonly property color mixerVuLevelColor: mixerAccentRed
    readonly property color micAuxDuckingArcColor: "#a00000"
    readonly property color micAuxGainColor: isClassic ? "#00d5ff" : "#ff7a18"
    readonly property color micAuxLabelTextColor: primaryDeckTextColor
    readonly property color micAuxPanelColor: isClassic ? "#161616" : "#161616"
    readonly property color micAuxRackGutterColor: isClassic ? "#0a0a0a" : "#080808"
    readonly property color micAuxUnconfiguredTextColor: isClassic ? "#4a5f73" : "#686666"
    readonly property string micTalkActiveIconSuffix: isPaleMoon ? "active" : ""
    readonly property url optionalDeckControlsBackgroundTile: isClassic ? lateNightAsset("style", "background_tile.png") : ""
    readonly property url optionalDeckRateCenterActive: isPaleMoon ? lateNightAsset("buttons", "btn__rate_center_cyan.svg") : ""
    readonly property url optionalDeckRateCenterInactive: isPaleMoon ? lateNightAsset("buttons", "btn__rate_center_off.svg") : ""
    readonly property url optionalMixerEqKillDotActiveGreen: isPaleMoon ? lateNightAsset("buttons", "btn__eq_kill_dot_active_green.svg") : ""
    readonly property url optionalMixerEqKillDotActiveRed: isPaleMoon ? lateNightAsset("buttons", "btn__eq_kill_dot_active_red.svg") : ""
    readonly property url optionalMixerEqKillDotOff: isPaleMoon ? lateNightAsset("buttons", "btn__eq_kill_dot_off.svg") : ""
    readonly property url optionalMixerQuickEffectActiveIcon: isPaleMoon ? lateNightAsset("buttons", "btn__star_active.svg") : ""
    readonly property color overviewBorderBottomColor: "#16202e"
    readonly property color overviewBorderLeftColor: "#121212"
    readonly property color overviewBorderRightColor: "#252525"
    readonly property color overviewBorderTopColor: "#0d0d0d"
    readonly property color overviewHotcueBrightTextColor: "#000000"
    readonly property int overviewHotcueBrightnessThreshold: 127
    readonly property color overviewMarkerTextColor: "#ffffff"
    readonly property color overviewRgbHighColor: "#ff0000"
    readonly property color overviewRgbLowColor: "#0000ff"
    readonly property color overviewRgbMidColor: "#00ff00"
    readonly property color overviewSettingsBackgroundColor: isClassic ? "#111111" : "#19191a"
    readonly property string playCueActiveIconSuffix: isPaleMoon ? "active" : ""
    readonly property color primaryDeckTextColor: isClassic ? "#ffd60a" : "#9fd0e0"
    readonly property color primaryOverviewBackgroundColor: isClassic ? "#0a0a0a" : "#19191a"
    readonly property color primaryWaveformSignalColor: isClassic ? "#00e5ff" : "#00e5ff"
    readonly property color rackFillerBorderColor: "#111111"
    readonly property color rackFillerBottomBorderColor: isClassic ? "#111111" : "#010304"
    readonly property color rackFillerLeftBorderColor: isClassic ? "#101a26" : "#191919"
    readonly property color rackFillerTopBorderColor: isClassic ? "#101a26" : "#1c1c1c"
    readonly property color samplerBpmColor: isClassic ? "#ffd60a" : "#766b65"
    readonly property color samplerBpmSeparatorLightColor: "#292929"
    readonly property color samplerColor: accentColor
    readonly property color samplerEffectAssignment12ActiveColor: isClassic ? "#39ff14" : "#236b00"
    readonly property color samplerEffectAssignment34ActiveColor: isClassic ? "#0895bc" : "#146674"
    readonly property color samplerEffectAssignmentInactiveTextColor: isClassic ? "#8c8c8c" : "#4a5f73"
    readonly property color samplerExpanderBottomBorderColor: isClassic ? "#111111" : "#010304"
    readonly property color samplerExpanderColor: isClassic ? "#171717" : "#111111"
    readonly property color samplerExpanderLeftBorderColor: isClassic ? "#101a26" : "#191919"
    readonly property color samplerExpanderRightBorderColor: "#111111"
    readonly property color samplerExpanderTopBorderColor: isClassic ? "#101a26" : "#212123"
    readonly property color samplerGainArcColor: isClassic ? "#00d5ff" : "#8d3b11"
    readonly property color samplerGainColor: isClassic ? "#00d5ff" : "#ff7a18"
    readonly property color samplerOverviewBackgroundColor: isClassic ? "#111111" : "#19191a"
    readonly property color samplerOverviewBackgroundLoadedColor: isClassic ? "#080808" : "#111111"
    readonly property color samplerOverviewBorderBottomColor: isClassic ? "#262626" : "#16202e"
    readonly property color samplerOverviewBorderLeftColor: isClassic ? "#04080b" : "#121212"
    readonly property color samplerOverviewBorderRightColor: isClassic ? "#262626" : "#252525"
    readonly property color samplerOverviewBorderTopColor: isClassic ? "#04080b" : "#0d0d0d"
    readonly property color samplerPanelColor: isClassic ? "#161616" : "#161616"
    readonly property color samplerPflActiveColor: isClassic ? "#ff2d55" : "#4a5f73"
    readonly property color samplerPitchSliderBarColor: "#6b8399"
    readonly property color samplerSettingsBorderBottomColor: isClassic ? "#0a0a0a" : "#16202e"
    readonly property color samplerSettingsBorderTopColor: isClassic ? "#0a0a0a" : "#080808"
    readonly property color samplerTitleColor: isClassic ? "#cfe6f5" : "#9fd0e0"
    readonly property color samplerWaveformFilteredHighColor: isClassic ? "#f3f16f" : "#a17b35"
    readonly property color samplerWaveformFilteredLowColor: isClassic ? "#00e5ff" : "#00e5ff"
    readonly property color samplerWaveformFilteredMidColor: isClassic ? "#edaf27" : "#d09271"
    readonly property color secondaryDeckTextColor: isClassic ? "#0bd9d1" : "#85bdbb"
    readonly property color secondaryOverviewBackgroundColor: "#001b23"
    readonly property color secondaryWaveformSignalColor: isClassic ? "#00e5ff" : "#6ef0ff"
    readonly property color starsColor12: isClassic ? "#ffd60a" : "#988f86"
    readonly property color starsColor34: isClassic ? "#0bd9d1" : "#559b99"
    readonly property int syncButtonHorizontalPadding: isClassic ? 3 : 0
    readonly property color syncExplicitLeaderColor: activePlayCueColor
    readonly property color syncImplicitLeaderColor: isPaleMoon ? "#7d350d" : "#00d5ff"
    readonly property color syncInactiveBackgroundColor: "#161616"
    readonly property color specialCueActiveColor: isPaleMoon ? "#385678" : "#2f4f73"
    readonly property color cueMenuCheckedColor: isClassic ? "#ff2d55" : "#ff7a18"
    readonly property color cueMenuDeleteHoverColor: "#6c2e2e"
    readonly property color cueMenuDeletePressedColor: "#dc4141"
    readonly property color textColor: "#f2f2f2"
    readonly property color textColorMuted: "#8c8c8c"
    readonly property color toolbarActiveColor: white
    readonly property color toolbarBackgroundColor: "#242424"
    readonly property color toolbarBottomBorderColor: "#010304"
    readonly property color toolbarBroadcastOnColor: isClassic ? "#39ff14" : "#438225"
    readonly property color toolbarButtonActiveBackgroundColor: isClassic ? "#ffb020" : "#9a9a9a"
    readonly property color toolbarButtonActiveTextColor: "#000000"
    readonly property int toolbarButtonHeight: 26
    readonly property color toolbarButtonInactiveBackgroundColor: isClassic ? "#131e2a" : "#111111"
    readonly property color toolbarButtonInactiveTextColor: isClassic ? "#cfe6f5" : "#9a9a9a"
    readonly property int toolbarButtonWidth: 52
    readonly property color toolbarClockTextColor: isClassic ? "#ffd60a" : "#9fd0e0"
    readonly property color toolbarLatencyBorderColor: "#040404"
    readonly property color toolbarLatencyLabelColor: isClassic ? "#ffd60a" : "#444444"
    readonly property color toolbarLatencyOverloadColor: "#ffff00"
    readonly property color toolbarMenuDisabledTextColor: "#9a9a9a"
    readonly property color toolbarMenuHoverColor: isPaleMoon ? "#2c454f" : "#5e4507"
    readonly property color toolbarMenuHoverTextColor: "#ffffff"
    readonly property color toolbarMenuTextColor: "#9fd0e0"
    readonly property color toolbarMeterBackgroundColor: darkGray
    readonly property color toolbarPopupBackgroundColor: "#0a0a0a"
    readonly property color toolbarPopupBorderColor: "#585858"
    readonly property color toolbarRecordInitColor: "#ffb020"
    readonly property color toolbarRecordOnColor: isClassic ? "#ff2d55" : "#a80000"
    readonly property color toolbarRecordingColor: "#ff2d55"
    readonly property color toolbarRecordingTextColor: "#ff7373"
    readonly property color toolbarRootBackgroundColor: "#111111"
    readonly property color toolbarSettingsTextColor: "#d2d2d2"
    readonly property color toolbarStatusErrorColor: "#f856e7"
    readonly property color toolbarStatusOkColor: "#54c76a"
    readonly property color toolbarStatusWarnColor: "#d89124"
    readonly property color trackPropertyHighlightColor: "#111111"
    readonly property color trackPropertySelectedTextColor: "#111111"
    readonly property color trackPropertySelectionColor: white
    readonly property color passthroughActiveColor: isClassic ? "#ffb020" : "#ff7a18"
    readonly property color passthroughLabelColor: passthroughActiveColor
    readonly property color passthroughOverlayColor: "#bb000000"
    readonly property color vinylCueingActiveColor: "#6b8399"
    readonly property color vinylStatusSignalAndSpeedColor: "#f856e7"
    readonly property color vinylStatusSignalColor: isClassic ? "#39ff14" : "#438225"
    readonly property color vinylStatusSpeedColor: "#ffb020"
    // Legacy #VinylControls buttons use light text in Classic and darker text
    // in PaleMoon when inactive.
    readonly property color vinylControlInactiveLabelColor: isClassic ? "#cfe6f5" : "#4a5f73"
    readonly property color waveformBeatAxesColor: isPaleMoon ? "#999999" : "#ffffff"
    readonly property color waveformCueColor: isPaleMoon ? "#ff7a01" : "#ff001c"
    readonly property color waveformDefaultMarkColor: "#ff0000"
    readonly property color waveformDisabledMarkColor: "#ffffff"
    readonly property color waveformEndOfTrackWarningColor: "#f856e7"
    readonly property color waveformFilteredHighColor: "#d5c2a2"
    readonly property color waveformFilteredLowColor: "#2154d7"
    readonly property color waveformFilteredMidColor: "#97632d"
    readonly property color waveformIntroOutroColor: isPaleMoon ? "#2c5c9a" : "#0000ff"
    readonly property color waveformLoopColor: isPaleMoon ? "#00b400" : "#00ff00"
    readonly property color waveformMarkerTextColor: "#ffffff"
    readonly property color waveformNoStemLabelColor: isPaleMoon ? "#444444" : "#ffd60a"
    readonly property color waveformContainerColor: isPaleMoon ? "#111111" : "#161616"
    readonly property color waveformContainerBottomBorderColor: isPaleMoon ? "#0a0a0a" : "#04080b"
    readonly property color waveformDeckBottomBorderColor: isPaleMoon ? "#16202e" : "#262626"
    readonly property color waveformOverlayColor: isPaleMoon ? "#111111" : "#171717"
    readonly property color waveformPlayPositionColor: isPaleMoon ? "#00e5ff" : "#00e5ff"
    readonly property color waveformPrimaryBackgroundColor: "#0a0a0a"
    readonly property color waveformPrimarySignalColor: isPaleMoon ? "#00e5ff" : "#00e5ff"
    readonly property color waveformSecondaryBackgroundColor: "#001b23"
    readonly property color waveformSecondarySignalColor: isPaleMoon ? "#6ef0ff" : "#00e5ff"
    readonly property color white: "#eaf6ff"

    // VirtualDJ and Serato use a narrow technical sans for their labels, so the
    // skin does too. The font ships in res/fonts and is registered at startup.
    readonly property string fontFamily: "Rajdhani"
    // Control icons come from Font Awesome Free (Solid), which ships in
    // res/fonts too. Attribution lives in res/fonts/FontAwesomeFree.LICENSE.txt.
    readonly property string iconFontFamily: "Font Awesome 6 Free"
    // The neon scheme recolours the LateNight controls, so it reuses the
    // palemoon artwork. Without this mapping the asset paths point at a scheme
    // directory that does not exist and every icon fails to resolve.
    readonly property string assetScheme: ColorScheme.name === "neondeck" ? "palemoon" : ColorScheme.name

    function deckNumberForGroup(group) {
        const match = /^\[Channel(\d+)\]$/.exec(String(group));
        return match ? parseInt(match[1], 10) : 1;
    }
    // Accent colour of a deck. Odd channels use deckAColor, even channels
    // deckBColor.
    function deckColorForGroup(group) {
        return deckNumberForGroup(group) % 2 === 1 ? deckAColor : deckBColor;
    }
    // Colour of the waveform or overview signal of a deck. NeonDeck gives each
    // side of the mixer its own colour, the legacy schemes keep the primary or
    // secondary colour the caller passes as legacyColor.
    function waveformSignalColorForGroup(group, legacyColor) {
        return usePerDeckColors ? deckColorForGroup(group) : legacyColor;
    }
    // Deck letter of a group, the way VirtualDJ labels its decks: A and B for
    // the main pair, C and D for the extras.
    function deckLetterForGroup(group) {
        const letters = ["A", "B", "C", "D"];
        const index = deckNumberForGroup(group) - 1;
        return index >= 0 && index < letters.length ? letters[index] : "A";
    }
    // Glyphs of the Font Awesome icons the skin uses, keyed by the Font Awesome
    // icon name so they are easy to look up on fontawesome.com.
    readonly property var iconGlyphs: ({
            "angles-left": "\uf100",
            "angles-right": "\uf101",
            "arrow-rotate-left": "\uf0e2",
            "arrow-rotate-right": "\uf01e",
            "arrows-rotate": "\uf021",
            "backward": "\uf04a",
            "backward-fast": "\uf049",
            "backward-step": "\uf048",
            "ban": "\uf05e",
            "bars": "\uf0c9",
            "bolt": "\uf0e7",
            "caret-left": "\uf0d9",
            "caret-right": "\uf0da",
            "chevron-down": "\uf078",
            "chevron-left": "\uf053",
            "chevron-right": "\uf054",
            "chevron-up": "\uf077",
            "circle": "\uf111",
            "circle-pause": "\uf28b",
            "circle-play": "\uf144",
            "clock": "\uf017",
            "compact-disc": "\uf51f",
            "compress": "\uf066",
            "crosshairs": "\uf05b",
            "eject": "\uf052",
            "ellipsis": "\uf141",
            "expand": "\uf065",
            "eye": "\uf06e",
            "eye-slash": "\uf070",
            "forward": "\uf04e",
            "forward-fast": "\uf050",
            "forward-step": "\uf051",
            "gauge": "\uf624",
            "gear": "\uf013",
            "headphones": "\uf025",
            "infinity": "\uf534",
            "key": "\uf084",
            "link": "\uf0c1",
            "list": "\uf03a",
            "lock": "\uf023",
            "magnet": "\uf076",
            "magnifying-glass": "\uf002",
            "microphone": "\uf130",
            "minus": "\uf068",
            "music": "\uf001",
            "pause": "\uf04c",
            "play": "\uf04b",
            "plus": "\u002b",
            "power-off": "\uf011",
            "record-vinyl": "\uf8d9",
            "repeat": "\uf363",
            "rotate": "\uf2f1",
            "shuffle": "\uf074",
            "sliders": "\uf1de",
            "star": "\uf005",
            "stop": "\uf04d",
            "table-columns": "\uf0db",
            "timeline": "\ue29c",
            "trash": "\uf1f8",
            "unlock": "\uf09c",
            "volume-high": "\uf028",
            "volume-low": "\uf027",
            "wand-magic-sparkles": "\ue2ca",
            "xmark": "\uf00d"
        })
    /// Glyph of a Font Awesome icon name. Empty when the name is unknown, so a
    /// typo shows nothing instead of a stray box.
    function iconGlyph(iconName) {
        const glyph = iconGlyphs[iconName];
        return glyph !== undefined ? glyph : "";
    }
    function lateNightAsset(directory, fileName) {
        return Qt.resolvedUrl("../../LateNight/" + assetScheme + "/" + directory + "/" + fileName);
    }
    function lateNightButton(fileName) {
        return lateNightAsset("buttons", fileName);
    }
    function lateNightRegionButton(regionButtonType, buttonSize) {
        return lateNightButton("btn_" + regionButtonType + "_" + buttonSize + ".svg");
    }
    function lateNightSubRegionButton(buttonSize) {
        return lateNightRegionButton(isClassic ? "elevated" : "embedded", buttonSize);
    }
    function lateNightTopRegionButton(buttonSize) {
        return lateNightRegionButton("embedded", buttonSize);
    }
    function mixerKnobIndicator(kind, colorName) {
        return lateNightAsset("knobs", "knob_indicator_" + kind + "_" + colorName + ".svg");
    }
    function mixerVuClipBackground(colorVariant) {
        return lateNightAsset("style", "vu_deck_clipping_bg_" + colorVariant + ".png");
    }
    function mixerVuLevelBackground(colorVariant) {
        return lateNightAsset("style", "vu_deck_level_bg_" + colorVariant + ".png");
    }
    function overviewHotcueTextColor(hotcueColor) {
        const red = hotcueColor.r * 255;
        const green = hotcueColor.g * 255;
        const blue = hotcueColor.b * 255;
        const brightness = Math.sqrt(red * red * 0.241 + green * green * 0.691 + blue * blue * 0.068);
        return brightness <= overviewHotcueBrightnessThreshold ? overviewMarkerTextColor : overviewHotcueBrightTextColor;
    }
    function sharedImage(fileName) {
        return Qt.resolvedUrl("../../../qml/images/" + fileName);
    }
    function vinylStatusColor(status) {
        switch (Math.round(status)) {
        case 1:
            return vinylStatusSignalColor;
        case 2:
            return vinylStatusSpeedColor;
        case 3:
            return vinylStatusSignalAndSpeedColor;
        default:
            return deckEmbeddedButtonInactiveColor;
        }
    }
}
