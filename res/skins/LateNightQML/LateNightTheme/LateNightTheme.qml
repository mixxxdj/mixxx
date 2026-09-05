pragma Singleton
import QtQuick
import "."

QtObject {
    readonly property color accentColor: ColorScheme.accentColor
    readonly property color activePlayCueColor: isClassic ? "#db0000" : "#b24c12"
    readonly property url assetDeckArrowLeftUpButton: lateNightAsset("buttons", "btn__arrow_left_up.svg")
    readonly property url assetDeckArrowRightDownButton: lateNightAsset("buttons", "btn__arrow_right_down.svg")
    readonly property url assetDeckBeatCurposButton: lateNightAsset("buttons", "btn__beat_curpos.svg")
    readonly property url assetDeckBeatCurposLargeButton: lateNightAsset("buttons", "btn__beat_curpos_large.svg")
    readonly property url assetDeckBeatSpinBoxBorder: isClassic ? lateNightAsset("buttons", "spinbox_elevated_border.svg") : lateNightSubRegionButton("wide")
    readonly property url assetDeckBeatSpinBoxDownButton: isClassic ? lateNightAsset("buttons", "spinbox_down.svg") : lateNightAsset("buttons", "btn__spinbox_down.svg")
    readonly property url assetDeckBeatSpinBoxUpButton: isClassic ? lateNightAsset("buttons", "spinbox_up.svg") : lateNightAsset("buttons", "btn__spinbox_up.svg")
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
    readonly property color backgroundColor: "#1e1e1e"
    readonly property color beatgridDisabledCoverColor: "#b4151517"
    readonly property color bpmTapEditorBackgroundColor: "#0f0f0f"
    readonly property color bpmTapEditorButtonColor: "#171719"
    readonly property color bpmTapEditorEditBorderColor: isPaleMoon ? "#257b82" : "#d08e00"
    readonly property color bpmTapEditorSelectBackgroundColor: "#151517"
    readonly property color bpmTapEditorSelectBorderColor: isPaleMoon ? "#7d350d" : "#5E4507"
    readonly property color buttonActiveColor: white
    readonly property color buttonNormalColor: "#696969"
    readonly property color buttonPressedColor: white
    readonly property color darkGray: "#0f0f0f"
    readonly property color deckActiveButtonTextColor: "#000000"
    readonly property color deckBeatSpinBoxTextColor: isClassic ? "#888888" : "#a7998b"
    readonly property color deckButtonInactiveColor: isClassic ? "#262626" : "#121213"
    readonly property color deckDimButtonInactiveColor: isClassic ? "#262626" : "#171719"
    readonly property color deckEmbeddedButtonInactiveColor: isClassic ? "#262626" : "#1e1e20"
    readonly property color deckPanelBorderDark: "#0c0c0c"
    readonly property color deckPanelBorderLeft: "#282828"
    readonly property color deckPanelBorderLight: "#333333"
    readonly property color deckPanelBorderRight: deckTopRowBackgroundColor
    readonly property color deckPanelColor: "#1e1e20"
    readonly property color deckReadonlyTextColor: isClassic ? "#888888" : "#777777"
    readonly property color deckTimeTextColor: isClassic ? "#f0bb2b" : "#777777"
    readonly property color deckTopRowBackgroundColor: "#181818"
    readonly property int deckOuterMargin: isClassic ? 2 : 1
    readonly property int fullDeckHeight: 206
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
    readonly property color compactVuPanelColor: isClassic ? "#1e1e1e" : "#151517"
    readonly property color compactVuPanelBorderColor: isClassic ? "#333333" : "#212123"
    readonly property color compactVuGutterColor: isClassic ? "#040404" : "#080808"
    readonly property int compactVuMeterHeight: 96
    readonly property int compactVuDeckGroupWidth: isClassic ? 19 : 18
    readonly property int compactVuMainGroupWidth: 34
    readonly property int compactVuSlotWidth: compactVuDeckGroupWidth * 2 + compactVuMainGroupWidth
    readonly property int smallSpinnySize: 63
    readonly property int miniSpinnySize: 53
    readonly property int deckTransportHeight: 55
    readonly property color effectsAssignmentActiveTextColor: "#000000"
    readonly property color effectsAssignmentInactiveColor: isClassic ? "#262626" : "#1e1e20"
    readonly property color effectsAssignmentInactiveTextColor: isClassic ? "#d2d2d1" : "#666666"
    readonly property color effectsControlActiveColor: "#888888"
    readonly property color effectsControlInactiveColor: "#262626"
    readonly property color effectsControllerColor12: isClassic ? "#73b508" : "#518f00"
    readonly property color effectsControllerColor34: isClassic ? "#0795b5" : "#028392"
    readonly property color effectsFillerColor: isClassic ? "#171717" : "#151517"
    readonly property color effectsFocusBorderColor: isClassic ? "#d08e00" : "#257b82"
    readonly property color effectsHeaderColor: isClassic ? "#1e1e1e" : "#151517"
    readonly property color effectsMasterButtonInactiveColor: isClassic ? "#262626" : "#1e1e20"
    readonly property color effectsPanelColor: isClassic ? "#1e1e1e" : "#1e1e20"
    readonly property color effectsParameterActiveColor: "#888888"
    readonly property color effectsParameterArcColor: "#6d6d6d"
    readonly property color effectsParameterInactiveColor: isClassic ? "#333333" : "#2a2a2c"
    readonly property string effectsParameterIndicatorColor: isClassic ? "white" : "grey"
    readonly property color effectsParameterInverseActiveColor: "#9c0900"
    readonly property color effectsParameterLinkInactiveColor: isClassic ? "#4b4b4b" : "#333333"
    readonly property color effectsParameterPanelColor: isClassic ? "#151515" : "#1e1e20"
    readonly property color effectsParameterTextColor: "#666666"
    readonly property color effectsRackGutterColor: "#060606"
    readonly property color effectsSlotToggleInactiveColor: isClassic ? "#262626" : "#121213"
    readonly property color effectsUnitColor12: isClassic ? "#659f08" : "#438225"
    readonly property color effectsUnitColor34: isClassic ? "#0895bc" : "#257b82"
    readonly property color effectsUnitDimColor12: isClassic ? "#426b00" : "#236b00"
    readonly property color effectsUnitDimColor34: isClassic ? "#00696b" : "#146674"
    readonly property bool isClassic: ColorScheme.name === "classic"
    readonly property bool isPaleMoon: ColorScheme.name === "palemoon"
    readonly property color keyControlsPressedColor: isPaleMoon ? "#7d350d" : "#db0000"
    readonly property string keyControlsPressedIconSuffix: isPaleMoon ? "active" : ""
    readonly property color libraryPanelSplitterBackground: "#1e1e1e"
    readonly property color libraryPanelSplitterHandle: "#5f5f5f"
    readonly property color libraryPanelSplitterHandleActive: "#7a7a7a"
    readonly property color mixerAccentCyan: "#0bd9d1"
    readonly property color mixerAccentOrange: isClassic ? "#db7700" : "#b24c12"
    readonly property color mixerAccentRed: isClassic ? "#db0000" : "#a80000"
    readonly property color mixerArcEqColor: "#858585"
    readonly property color mixerArcGainColor: "#b96300"
    readonly property color mixerArcGainLowColor: "#8d3b11"
    readonly property color mixerArcMainBalanceColor: "#a00000"
    readonly property color mixerArcQuickEffectColor: "#518f00"
    readonly property real mixerArcRadiusBig: 14.5
    readonly property real mixerArcRadiusCompact: 12.5
    readonly property real mixerArcWidth: 2
    readonly property color mixerControlTextColor: isClassic ? "#d2d2d1" : "#a7998b"
    readonly property color mixerDimTextColor: "#696969"
    readonly property color mixerEqKillActiveColor: isClassic ? "#db0000" : "#a80000"
    readonly property color mixerFxAssignInactiveColor: isClassic ? deckEmbeddedButtonInactiveColor : "#151517"
    readonly property color mixerFxAssignInactiveTextColor: isClassic ? mixerDimTextColor : "#555555"
    readonly property color mixerMainSeparatorDarkColor: isPaleMoon ? "#0c0c0c" : mixerPanelBorderDark
    readonly property color mixerMainSeparatorLightColor: isPaleMoon ? "#222222" : mixerPanelBorderLight
    readonly property color mixerSplitActiveColor: isClassic ? "#888888" : "#555555"
    readonly property color mixerSplitInactiveColor: isClassic ? deckEmbeddedButtonInactiveColor : "#222222"
    readonly property color mixerPanelBorderBottom: isPaleMoon ? "#0c0c0c" : "#0a0a0a"
    readonly property color mixerPanelBorderDark: "#080808"
    readonly property color mixerPanelBorderLeft: isPaleMoon ? "#282828" : "#333333"
    readonly property color mixerPanelBorderLight: "#343434"
    readonly property color mixerPanelBorderRight: isPaleMoon ? "#181818" : "#0a0a0a"
    readonly property color mixerPanelBorderTop: "#333333"
    readonly property color mixerPanelColor: "#1d1d1f"
    readonly property color mixerPflActiveFillColor: isClassic ? "#db0000" : "#666666"
    readonly property color mixerQuickEffectActiveColor: isClassic ? "#659f08" : "#236b00"
    readonly property color mixerQuickEffectSelectorTextColor: "#918273"
    readonly property color mixerSliderBarColor: "#257b82"
    readonly property color mixerVuClipColor: mixerAccentRed
    readonly property color mixerVuLevelColor: mixerAccentRed
    readonly property url optionalDeckControlsBackgroundTile: isClassic ? lateNightAsset("style", "background_tile.png") : ""
    readonly property url optionalDeckRateCenterActive: isPaleMoon ? lateNightAsset("buttons", "btn__rate_center_cyan.svg") : ""
    readonly property url optionalDeckRateCenterInactive: isPaleMoon ? lateNightAsset("buttons", "btn__rate_center_off.svg") : ""
    readonly property url optionalMixerEqKillDotActiveGreen: isPaleMoon ? lateNightAsset("buttons", "btn__eq_kill_dot_active_green.svg") : ""
    readonly property url optionalMixerEqKillDotActiveRed: isPaleMoon ? lateNightAsset("buttons", "btn__eq_kill_dot_active_red.svg") : ""
    readonly property url optionalMixerEqKillDotOff: isPaleMoon ? lateNightAsset("buttons", "btn__eq_kill_dot_off.svg") : ""
    readonly property url optionalMixerQuickEffectActiveIcon: isPaleMoon ? lateNightAsset("buttons", "btn__star_active.svg") : ""
    readonly property color overviewHotcueBrightTextColor: "#000000"
    readonly property int overviewHotcueBrightnessThreshold: 127
    readonly property color overviewMarkerTextColor: "#ffffff"
    readonly property color overviewRgbHighColor: "#ff0000"
    readonly property color overviewRgbLowColor: "#0000ff"
    readonly property color overviewRgbMidColor: "#00ff00"
    readonly property color overviewBorderBottomColor: "#2a2a2a"
    readonly property color overviewBorderLeftColor: "#121212"
    readonly property color overviewBorderRightColor: "#252525"
    readonly property color overviewBorderTopColor: "#0d0d0d"
    readonly property color overviewSettingsBackgroundColor: isClassic ? "#151515" : "#19191a"
    readonly property string playCueActiveIconSuffix: isPaleMoon ? "active" : ""
    readonly property color primaryDeckTextColor: isClassic ? "#f0bb2b" : "#c2b3a5"
    readonly property color primaryOverviewBackgroundColor: isClassic ? "#0f0f0f" : "#19191a"
    readonly property color primaryWaveformSignalColor: isClassic ? "#e7c413" : "#d9b28c"
    readonly property color samplerBpmColor: isClassic ? "#f0bb2b" : "#766b65"
    readonly property color samplerBpmSeparatorLightColor: "#292929"
    readonly property color samplerColor: accentColor
    readonly property color samplerEffectAssignment12ActiveColor: isClassic ? "#659f08" : "#236b00"
    readonly property color samplerEffectAssignment34ActiveColor: isClassic ? "#0895bc" : "#146674"
    readonly property color samplerEffectAssignmentInactiveTextColor: isClassic ? "#696969" : "#666666"
    readonly property color samplerExpanderBottomBorderColor: isClassic ? "#111111" : "#020202"
    readonly property color samplerExpanderColor: isClassic ? "#171717" : "#151517"
    readonly property color samplerExpanderLeftBorderColor: isClassic ? "#222222" : "#191919"
    readonly property color samplerExpanderRightBorderColor: "#111111"
    readonly property color samplerExpanderTopBorderColor: isClassic ? "#222222" : "#212123"
    readonly property color samplerGainArcColor: isClassic ? "#db7700" : "#8d3b11"
    readonly property color samplerGainColor: isClassic ? "#db7700" : "#b24c12"
    readonly property color samplerOverviewBackgroundColor: isClassic ? "#151515" : "#19191a"
    readonly property color samplerOverviewBackgroundLoadedColor: isClassic ? "#080808" : "#151515"
    readonly property color samplerOverviewBorderBottomColor: isClassic ? "#333333" : "#2a2a2a"
    readonly property color samplerOverviewBorderLeftColor: isClassic ? "#0a0a0a" : "#121212"
    readonly property color samplerOverviewBorderRightColor: isClassic ? "#333333" : "#252525"
    readonly property color samplerOverviewBorderTopColor: isClassic ? "#0a0a0a" : "#0d0d0d"
    readonly property color samplerPanelColor: isClassic ? "#1e1e1e" : "#1e1e20"
    readonly property color samplerPflActiveColor: isClassic ? "#db0000" : "#666666"
    readonly property color samplerPitchSliderBarColor: "#888888"
    readonly property color samplerSettingsBorderBottomColor: isClassic ? "#0c0c0c" : "#2a2a2a"
    readonly property color samplerSettingsBorderTopColor: isClassic ? "#0c0c0c" : "#080808"
    readonly property color samplerTitleColor: isClassic ? "#d2d2d1" : "#c2b3a5"
    readonly property color samplerWaveformFilteredHighColor: isClassic ? "#f3f16f" : "#a17b35"
    readonly property color samplerWaveformFilteredLowColor: isClassic ? "#e7c413" : "#d9b28c"
    readonly property color samplerWaveformFilteredMidColor: isClassic ? "#edaf27" : "#d09271"
    readonly property color secondaryDeckTextColor: isClassic ? "#0bd9d1" : "#85bdbb"
    readonly property color secondaryOverviewBackgroundColor: "#001b23"
    readonly property color secondaryWaveformSignalColor: isClassic ? "#09b2ae" : "#7bc6c3"
    readonly property color starsColor12: isClassic ? "#f0bb2b" : "#988f86"
    readonly property color starsColor34: isClassic ? "#0bd9d1" : "#559b99"
    readonly property int syncButtonHorizontalPadding: isClassic ? 3 : 0
    readonly property color syncExplicitLeaderColor: activePlayCueColor
    readonly property color syncImplicitLeaderColor: isPaleMoon ? "#7d350d" : "#db7700"
    readonly property color syncInactiveBackgroundColor: "#1e1e1e"
    readonly property color textColor: white
    readonly property color textColorMuted: "#696969"
    readonly property color trackPropertyHighlightColor: "#151515"
    readonly property color trackPropertySelectedTextColor: "#111111"
    readonly property color trackPropertySelectionColor: white
    readonly property color toolbarActiveColor: white
    readonly property color toolbarBackgroundColor: "#242424"
    readonly property color toolbarBottomBorderColor: "#020202"
    readonly property color toolbarBroadcastOnColor: isClassic ? "#659f08" : "#438225"
    readonly property color toolbarButtonActiveBackgroundColor: isClassic ? "#d09300" : "#777777"
    readonly property color toolbarButtonActiveTextColor: "#000000"
    readonly property int toolbarButtonHeight: 26
    readonly property color toolbarButtonInactiveBackgroundColor: isClassic ? "#262626" : "#151517"
    readonly property color toolbarButtonInactiveTextColor: isClassic ? "#d2d2d1" : "#777777"
    readonly property int toolbarButtonWidth: 52
    readonly property color toolbarClockTextColor: isClassic ? "#f0bb2b" : "#c2b3a5"
    readonly property color toolbarLatencyBorderColor: "#040404"
    readonly property color toolbarLatencyLabelColor: "#444444"
    readonly property color toolbarLatencyOverloadColor: "#ffff00"
    readonly property color toolbarMenuDisabledTextColor: "#777777"
    readonly property color toolbarMenuHoverColor: isPaleMoon ? "#2c454f" : "#5e4507"
    readonly property color toolbarMenuHoverTextColor: "#ffffff"
    readonly property color toolbarMenuTextColor: "#c2b3a5"
    readonly property color toolbarMeterBackgroundColor: darkGray
    readonly property color toolbarPopupBackgroundColor: "#0f0f0f"
    readonly property color toolbarPopupBorderColor: "#585858"
    readonly property color toolbarRecordInitColor: "#d09300"
    readonly property color toolbarRecordOnColor: isClassic ? "#db0000" : "#a80000"
    readonly property color toolbarRecordingColor: "#db0000"
    readonly property color toolbarRecordingTextColor: "#ff7373"
    readonly property color toolbarRootBackgroundColor: "#151517"
    readonly property color toolbarSettingsTextColor: "#d2d2d2"
    readonly property color toolbarStatusErrorColor: "#f856e7"
    readonly property color toolbarStatusOkColor: "#54c76a"
    readonly property color toolbarStatusWarnColor: "#d89124"
    readonly property color passthroughActiveColor: vinylStatusSpeedColor
    readonly property color vinylCueingActiveColor: "#888888"
    readonly property color vinylStatusSignalAndSpeedColor: "#f856e7"
    readonly property color vinylStatusSignalColor: isClassic ? "#659f08" : "#438225"
    readonly property color vinylStatusSpeedColor: "#d09300"
    // Legacy #VinylControls buttons use light text in Classic and darker text
    // in PaleMoon when inactive.
    readonly property color vinylControlInactiveLabelColor: isClassic ? "#d2d2d1" : "#666666"
    readonly property color waveformBeatAxesColor: isPaleMoon ? "#999999" : "#ffffff"
    readonly property color waveformCueColor: isPaleMoon ? "#ff7a01" : "#ff001c"
    readonly property color waveformDefaultMarkColor: "#ff0000"
    readonly property color waveformDisabledMarkColor: "#ffffff"
    readonly property color waveformEndOfTrackWarningColor: "#ff8872"
    readonly property color waveformFilteredHighColor: "#d5c2a2"
    readonly property color waveformFilteredLowColor: "#2154d7"
    readonly property color waveformFilteredMidColor: "#97632d"
    readonly property color waveformIntroOutroColor: isPaleMoon ? "#2c5c9a" : "#0000ff"
    readonly property color waveformLoopColor: isPaleMoon ? "#00b400" : "#00ff00"
    readonly property color waveformMarkerTextColor: "#ffffff"
    readonly property color waveformPlayPositionColor: isPaleMoon ? "#00c6ff" : "#00c8ff"
    readonly property color waveformPrimaryBackgroundColor: "#0f0f0e"
    readonly property color waveformSecondaryBackgroundColor: "#001b23"
    readonly property color white: "#D9D9D9"

    function lateNightAsset(directory, fileName) {
        return Qt.resolvedUrl("../../LateNight/" + ColorScheme.name + "/" + directory + "/" + fileName);
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
        return brightness <= overviewHotcueBrightnessThreshold
                ? overviewMarkerTextColor
                : overviewHotcueBrightTextColor;
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
