pragma Singleton
import QtQuick
import "."

QtObject {
    readonly property bool isClassic: ColorScheme.name === "classic"
    readonly property bool isPaleMoon: ColorScheme.name === "palemoon"

    readonly property color backgroundColor: "#1e1e1e"
    readonly property color buttonActiveColor: white
    readonly property color buttonNormalColor: "#696969"
    readonly property color buttonPressedColor: white
    readonly property color darkGray: "#0f0f0f"
    readonly property color accentColor: ColorScheme.accentColor
    readonly property color activePlayCueColor: isClassic ? "#db0000" : "#b24c12"
    readonly property color deckTimeTextColor: isClassic ? "#f0bb2b" : "#777777"
    readonly property color deckButtonInactiveColor: isClassic ? "#262626" : "#121213"
    readonly property color deckDimButtonInactiveColor: isClassic ? "#262626" : "#171719"
    readonly property color deckEmbeddedButtonInactiveColor: isClassic ? "#262626" : "#1e1e20"
    readonly property color deckBeatSpinBoxTextColor: isClassic ? "#888888" : "#a7998b"
    readonly property color deckReadonlyTextColor: isClassic ? "#888888" : "#777777"
    readonly property color deckPanelColor: "#1e1e20"
    readonly property color deckPanelBorderDark: "#0c0c0c"
    readonly property color deckPanelBorderLight: "#333333"
    readonly property color libraryPanelSplitterBackground: "#1e1e1e"
    readonly property color libraryPanelSplitterHandle: "#5f5f5f"
    readonly property color libraryPanelSplitterHandleActive: "#7a7a7a"
    readonly property color textColor: white
    readonly property color textColorMuted: "#696969"
    readonly property color primaryDeckTextColor: isClassic ? "#f0bb2b" : "#c2b3a5"
    readonly property color primaryOverviewBackgroundColor: isClassic ? "#0f0f0f" : "#19191a"
    readonly property color primaryWaveformSignalColor: isClassic ? "#e7c413" : "#d9b28c"
    readonly property color secondaryDeckTextColor: isClassic ? "#0bd9d1" : "#85bdbb"
    readonly property color secondaryOverviewBackgroundColor: "#001b23"
    readonly property color secondaryWaveformSignalColor: isClassic ? "#09b2ae" : "#7bc6c3"
    readonly property color starsColor12: isClassic ? "#f0bb2b" : "#988f86"
    readonly property color starsColor34: isClassic ? "#0bd9d1" : "#559b99"
    readonly property color toolbarActiveColor: white
    readonly property color toolbarBackgroundColor: "#242424"
    readonly property int toolbarButtonHeight: 26
    readonly property int toolbarButtonWidth: 52
    readonly property color syncExplicitLeaderColor: activePlayCueColor
    readonly property color syncInactiveBackgroundColor: "#1e1e1e"
    readonly property color syncImplicitLeaderColor: isPaleMoon ? "#7d350d" : "#db7700"
    readonly property int syncButtonHorizontalPadding: isClassic ? 3 : 0
    readonly property color keyControlsPressedColor: isPaleMoon ? "#7d350d" : "#db0000"
    readonly property string keyControlsPressedIconSuffix: isPaleMoon ? "active" : ""
    readonly property string playCueActiveIconSuffix: isPaleMoon ? "active" : ""
    readonly property color white: "#D9D9D9"

    readonly property url assetDeckArrowLeftUpButton: legacyAsset("buttons", "btn__arrow_left_up.svg")
    readonly property url assetDeckArrowRightDownButton: legacyAsset("buttons", "btn__arrow_right_down.svg")
    readonly property url assetDeckBeatjumpLeftButton: legacyAsset("buttons", "btn__beatjump_left.svg")
    readonly property url assetDeckBeatjumpRightButton: legacyAsset("buttons", "btn__beatjump_right.svg")
    readonly property url assetDeckBeatSpinBoxBorder: isClassic ? legacyAsset("buttons", "spinbox_elevated_border.svg") : legacySubRegionButton("wide")
    readonly property url assetDeckBeatSpinBoxDownButton: isClassic ? legacyAsset("buttons", "spinbox_down.svg") : legacyAsset("buttons", "btn__spinbox_down.svg")
    readonly property url assetDeckBeatSpinBoxUpButton: isClassic ? legacyAsset("buttons", "spinbox_up.svg") : legacyAsset("buttons", "btn__spinbox_up.svg")
    readonly property url optionalDeckControlsBackgroundTile: isClassic ? legacyAsset("style", "background_tile.png") : ""
    readonly property url assetDeckCoverDefault: legacyAsset("style", "cover_default.svg")
    readonly property url assetDeckBeatCurposButton: legacyAsset("buttons", "btn__beat_curpos.svg")
    readonly property url assetDeckCueButton: legacyAsset("buttons", "btn__cue_deck.svg")
    readonly property url assetDeckEjectButton: legacyAsset("buttons", "btn__eject.svg")
    readonly property url assetDeckIntroEndButton: legacyAsset("buttons", "btn__intro_end.svg")
    readonly property url assetDeckIntroStartButton: legacyAsset("buttons", "btn__intro_start.svg")
    readonly property url assetDeckKeyDownButton: legacyAsset("buttons", "btn__key_down.svg")
    readonly property url assetDeckKeylockButton: legacyAsset("buttons", "btn__keylock.svg")
    readonly property url assetDeckKeyMatchButton: legacyAsset("buttons", "btn__key_match.svg")
    readonly property url assetDeckKeyUpButton: legacyAsset("buttons", "btn__key_up.svg")
    readonly property url assetDeckKeyButtonBackground: legacyAsset("buttons", "btn_embedded_library.svg")
    readonly property url assetDeckLeaderBackground: legacyAsset("buttons", "btn_embedded_grid.svg")
    readonly property url assetDeckLeaderButton: legacyAsset("buttons", "btn__sync_leader.svg")
    readonly property url assetDeckLeaderExplicitButton: isPaleMoon ? legacyAsset("buttons", "btn__sync_leader_explicit.svg") : legacyAsset("buttons", "btn__sync_leader_active.svg")
    readonly property url assetDeckLeaderImplicitButton: isPaleMoon ? legacyAsset("buttons", "btn__sync_leader_implicit.svg") : legacyAsset("buttons", "btn__sync_leader_active.svg")
    readonly property url assetDeckLoopButton: legacyAsset("buttons", "btn__loop.svg")
    readonly property url assetDeckLoopAnchorEndButton: legacyAsset("buttons", "btn__loop_anchor_end.svg")
    readonly property url assetDeckLoopAnchorStartButton: legacyAsset("buttons", "btn__loop_anchor_start.svg")
    readonly property url assetDeckLoopInButton: legacyAsset("buttons", "btn__loop_in.svg")
    readonly property url assetDeckLoopOutButton: legacyAsset("buttons", "btn__loop_out.svg")
    readonly property url assetDeckMinusButton: legacyAsset("buttons", "btn__minus.svg")
    readonly property url assetDeckOutroEndButton: legacyAsset("buttons", "btn__outro_end.svg")
    readonly property url assetDeckOutroStartButton: legacyAsset("buttons", "btn__outro_start.svg")
    readonly property url assetDeckPlayButton: legacyAsset("buttons", "btn__play_deck.svg")
    readonly property url assetDeckPlusButton: legacyAsset("buttons", "btn__plus.svg")
    readonly property url assetDeckQuantizeButton: legacyAsset("buttons", "btn__quantize.svg")
    readonly property url optionalDeckRateCenterActive: isPaleMoon ? legacyAsset("buttons", "btn__rate_center_cyan.svg") : ""
    readonly property url optionalDeckRateCenterInactive: isPaleMoon ? legacyAsset("buttons", "btn__rate_center_off.svg") : ""
    readonly property url assetDeckRateSliderBackground: legacyAsset("sliders", "slider_pitch_deck.svg")
    readonly property url assetDeckRateSliderHandle: legacyAsset("sliders", "knob_pitch_deck.svg")
    readonly property url assetDeckReloopButton: legacyAsset("buttons", "btn__reloop.svg")
    readonly property url assetDeckRepeatButton: legacyAsset("buttons", "btn__repeat.svg")
    readonly property url assetDeckReverseButton: legacyAsset("buttons", "btn__reverse.svg")
    readonly property url assetDeckSettingsOffButton: legacyAsset("buttons", "btn__settings_off.svg")
    readonly property url assetDeckSettingsOnButton: legacyAsset("buttons", "btn__settings_on.svg")
    readonly property url assetDeckSlipButton: legacyAsset("buttons", "btn__slip.svg")
    readonly property url assetDeckSpinnyBackground: legacyAsset("style", "spinny_bg.svg")
    readonly property url assetDeckSpinnyGhostIndicator: legacyAsset("style", "spinny_indicator_ghost.svg")
    readonly property url assetDeckSpinnyIndicator: legacyAsset("style", "spinny_indicator.svg")
    readonly property url assetDeckSpinnyMask12: legacyAsset("style", "spinny_mask_12.svg")
    readonly property url assetDeckSpinnyMask34: legacyAsset("style", "spinny_mask_34.svg")
    readonly property url assetDeckSyncBackground: legacyAsset("buttons", "btn_embedded_library.svg")
    readonly property url assetDeckSyncButton: legacyAsset("buttons", "btn__sync_deck.svg")
    readonly property url assetDeckSyncActiveButton: isPaleMoon ? legacyAsset("buttons", "btn__sync_deck_active.svg") : legacyAsset("buttons", "btn__sync_deck.svg")
    readonly property url assetDeckVolumeSliderBackground: legacyAsset("sliders", "slider_volume_deck.svg")
    readonly property url assetDeckVolumeSliderHandle: legacyAsset("sliders", "knob_volume_deck.svg")
    readonly property url assetDeckVinylControl0: legacyAsset("style", "vinyl_control_0.svg")
    readonly property url assetDeckVinylControl1: legacyAsset("style", "vinyl_control_1.svg")
    readonly property url assetDeckVinylControl2: legacyAsset("style", "vinyl_control_2.svg")
    readonly property url assetDeckVinylControl3: legacyAsset("style", "vinyl_control_3.svg")
    readonly property url assetFxKnobBackground: legacyAsset("knobs", "knob_bg_fx.svg")
    readonly property url assetMainKnobBackground: legacyAsset("knobs", "knob_bg_main.svg")
    readonly property url assetRegularKnobBackground: legacyAsset("knobs", "knob_bg_regular.svg")
    readonly property url assetSmallKnobBackground: legacyAsset("knobs", "knob_bg_small.svg")

    function legacyButton(fileName) {
        return legacyAsset("buttons", fileName);
    }

    function legacyRegionButton(regionButtonType, buttonSize) {
        return legacyButton("btn_" + regionButtonType + "_" + buttonSize + ".svg");
    }

    function legacySubRegionButton(buttonSize) {
        return legacyRegionButton(isClassic ? "elevated" : "embedded", buttonSize);
    }

    function legacyTopRegionButton(buttonSize) {
        return legacyRegionButton("embedded", buttonSize);
    }

    function legacyAsset(directory, fileName) {
        return Qt.resolvedUrl("../../LateNight/" + ColorScheme.name + "/" + directory + "/" + fileName);
    }

    function sharedImage(fileName) {
        return Qt.resolvedUrl("../../../qml/images/" + fileName);
    }
}
