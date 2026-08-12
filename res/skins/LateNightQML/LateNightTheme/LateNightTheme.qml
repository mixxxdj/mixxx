pragma Singleton
import QtQuick
import "."

QtObject {
    readonly property color accentColor: ColorScheme.accentColor
    readonly property color activePlayCueColor: isClassic ? "#db0000" : "#b24c12"
    readonly property url assetAuxMainMixButton: lateNightAsset("buttons", "btn__aux_play.svg")
    readonly property url assetAuxXfaderLeft: lateNightAsset("buttons", isClassic ? "btn__xfader_aux_left.svg" : "btn__xfader_deck_left_default.svg")
    readonly property url assetAuxXfaderMain: lateNightAsset("buttons", isClassic ? "btn__xfader_aux_mid.svg" : "btn__xfader_deck_mid_default.svg")
    readonly property url assetAuxXfaderRight: lateNightAsset("buttons", isClassic ? "btn__xfader_aux_right.svg" : "btn__xfader_deck_right_default.svg")
    readonly property url assetDeckArrowLeftUpButton: lateNightAsset("buttons", "btn__arrow_left_up.svg")
    readonly property url assetDeckArrowRightDownButton: lateNightAsset("buttons", "btn__arrow_right_down.svg")
    readonly property url assetDeckBeatCurposButton: lateNightAsset("buttons", "btn__beat_curpos.svg")
    readonly property url assetDeckBeatSpinBoxBorder: isClassic ? lateNightAsset("buttons", "spinbox_elevated_border.svg") : lateNightSubRegionButton("wide")
    readonly property url assetDeckBeatSpinBoxDownButton: isClassic ? lateNightAsset("buttons", "spinbox_down.svg") : lateNightAsset("buttons", "btn__spinbox_down.svg")
    readonly property url assetDeckBeatSpinBoxUpButton: isClassic ? lateNightAsset("buttons", "spinbox_up.svg") : lateNightAsset("buttons", "btn__spinbox_up.svg")
    readonly property url assetDeckBeatjumpLeftButton: lateNightAsset("buttons", "btn__beatjump_left.svg")
    readonly property url assetDeckBeatjumpRightButton: lateNightAsset("buttons", "btn__beatjump_right.svg")
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
    readonly property url assetDeckPlusButton: lateNightAsset("buttons", "btn__plus.svg")
    readonly property url assetDeckQuantizeButton: lateNightAsset("buttons", "btn__quantize.svg")
    readonly property url assetDeckRateSliderBackground: lateNightAsset("sliders", "slider_pitch_deck.svg")
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
    readonly property url assetDeckVinylControl0: lateNightAsset("style", "vinyl_control_0.svg")
    readonly property url assetDeckVinylControl1: lateNightAsset("style", "vinyl_control_1.svg")
    readonly property url assetDeckVinylControl2: lateNightAsset("style", "vinyl_control_2.svg")
    readonly property url assetDeckVinylControl3: lateNightAsset("style", "vinyl_control_3.svg")
    readonly property url assetDeckVolumeSliderBackground: lateNightAsset("sliders", "slider_volume_deck.svg")
    readonly property url assetDeckVolumeSliderHandle: lateNightAsset("sliders", "knob_volume_deck.svg")
    readonly property url assetFxKnobBackground: lateNightAsset("knobs", "knob_bg_fx.svg")
    readonly property url assetMainKnobBackground: lateNightAsset("knobs", "knob_bg_main.svg")
    readonly property url assetMicAuxAddButton: lateNightAsset("buttons", "btn__plus_flat.svg")
    readonly property url assetMicAuxGainKnobBackground: lateNightAsset("knobs", "knob_bg_small.svg")
    readonly property url assetMicAuxUnconfiguredBackground: lateNightAsset("buttons", "btn_flat_square.svg")
    readonly property url assetMicDuckAutoButton: lateNightAsset("buttons", "btn__mic_duck_auto.svg")
    readonly property url assetMicDuckManualButton: lateNightAsset("buttons", "btn__mic_duck_manual.svg")
    readonly property url assetMicDuckOffButton: lateNightAsset("buttons", "btn__mic_duck_off.svg")
    readonly property url assetMicTalkButton: lateNightAsset("buttons", "btn__mic_talk.svg")
    readonly property url assetRegularKnobBackground: lateNightAsset("knobs", "knob_bg_regular.svg")
    readonly property url assetSamplerCollapseButton: lateNightAsset("buttons", isClassic ? "btn__collapse.svg" : "btn__collapse_dim.svg")
    readonly property url assetSamplerExpandButton: lateNightAsset("buttons", isClassic ? "btn__expand.svg" : "btn__expand_dim.svg")
    readonly property url assetSamplerPauseButton: lateNightAsset("buttons", "btn__pause_sampler.svg")
    readonly property url assetSamplerPitchSliderBackground: lateNightAsset("sliders", "slider_pitch_sampler.svg")
    readonly property url assetSamplerPitchSliderHandle: lateNightAsset("sliders", "knob_pitch_sampler.svg")
    readonly property url assetSamplerPlayButton: lateNightAsset("buttons", "btn__play_sampler.svg")
    readonly property url assetSamplerSyncButton: lateNightAsset("buttons", "btn__sync_sampler.svg")
    readonly property url assetSamplerXfaderLeft: lateNightAsset("buttons", "btn__xfader_sampler_left.svg")
    readonly property url assetSamplerXfaderMain: lateNightAsset("buttons", "btn__xfader_sampler_main.svg")
    readonly property url assetSamplerXfaderRight: lateNightAsset("buttons", "btn__xfader_sampler_right.svg")
    readonly property url assetSmallKnobBackground: lateNightAsset("knobs", "knob_bg_small.svg")
    readonly property url assetToolbarDropdownIcon: lateNightAsset("buttons", "btn__fx_selector_down.svg")
    readonly property url assetToolbarMenuIcon: lateNightAsset("buttons", "btn__menu.svg")
    readonly property color backgroundColor: "#1e1e1e"
    readonly property color buttonActiveColor: white
    readonly property color buttonNormalColor: "#696969"
    readonly property color buttonPressedColor: white
    readonly property color darkGray: "#0f0f0f"
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
    readonly property bool isClassic: ColorScheme.name === "classic"
    readonly property bool isPaleMoon: ColorScheme.name === "palemoon"
    readonly property color keyControlsPressedColor: isPaleMoon ? "#7d350d" : "#db0000"
    readonly property string keyControlsPressedIconSuffix: isPaleMoon ? "active" : ""
    readonly property color libraryPanelSplitterBackground: "#1e1e1e"
    readonly property color libraryPanelSplitterHandle: "#5f5f5f"
    readonly property color libraryPanelSplitterHandleActive: "#7a7a7a"
    readonly property color micAuxPanelColor: isClassic ? "#171717" : "#151517"
    readonly property color micAuxUnconfiguredTextColor: isClassic ? "#444444" : "#555555"
    readonly property url optionalDeckControlsBackgroundTile: isClassic ? lateNightAsset("style", "background_tile.png") : ""
    readonly property url optionalDeckRateCenterActive: isPaleMoon ? lateNightAsset("buttons", "btn__rate_center_cyan.svg") : ""
    readonly property url optionalDeckRateCenterInactive: isPaleMoon ? lateNightAsset("buttons", "btn__rate_center_off.svg") : ""
    readonly property color overviewBorderBottomColor: "#2a2a2a"
    readonly property color overviewBorderLeftColor: "#121212"
    readonly property color overviewBorderRightColor: "#252525"
    readonly property color overviewBorderTopColor: "#0d0d0d"
    readonly property color overviewSettingsBackgroundColor: isClassic ? "#151515" : "#19191a"
    readonly property string playCueActiveIconSuffix: isPaleMoon ? "active" : ""
    readonly property color primaryDeckTextColor: isClassic ? "#f0bb2b" : "#c2b3a5"
    readonly property color primaryOverviewBackgroundColor: isClassic ? "#0f0f0f" : "#19191a"
    readonly property color primaryWaveformSignalColor: isClassic ? "#e7c413" : "#d9b28c"
    readonly property color samplerBpmColor: isClassic ? "#f0bb2b" : "#988f86"
    readonly property color samplerColor: accentColor
    readonly property color samplerGainColor: isClassic ? "#db7700" : "#b24c12"
    readonly property color samplerMutedColor: isClassic ? "#696969" : "#777777"
    readonly property color samplerPanelColor: isClassic ? "#171717" : "#151517"
    readonly property color samplerTitleColor: isClassic ? "#d2d2d1" : "#c2b3a5"
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
    readonly property color toolbarStatusErrorColor: "#f856e7"
    readonly property color toolbarStatusOkColor: "#54c76a"
    readonly property color toolbarStatusWarnColor: "#d89124"
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
    function sharedImage(fileName) {
        return Qt.resolvedUrl("../../../qml/images/" + fileName);
    }
}
