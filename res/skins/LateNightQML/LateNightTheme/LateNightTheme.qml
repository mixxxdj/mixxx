pragma Singleton
import QtQuick
import "."

QtObject {
    readonly property color backgroundColor: "#1e1e1e"
    readonly property color buttonActiveColor: white
    readonly property color buttonNormalColor: "#696969"
    readonly property color buttonPressedColor: white
    readonly property color darkGray: "#0f0f0f"
    readonly property color libraryPanelSplitterBackground: "#1e1e1e"
    readonly property color libraryPanelSplitterHandle: "#5f5f5f"
    readonly property color libraryPanelSplitterHandleActive: "#7a7a7a"
    readonly property color textColor: white
    readonly property color toolbarActiveColor: white
    readonly property color toolbarBackgroundColor: "#242424"
    readonly property int toolbarButtonHeight: 26
    readonly property int toolbarButtonWidth: 52
    readonly property color white: "#D9D9D9"

    readonly property url assetDeckBeatjumpLeftButton: buttonAsset("btn__beatjump_left.svg")
    readonly property url assetDeckBeatjumpRightButton: buttonAsset("btn__beatjump_right.svg")
    readonly property url assetDeckCoverDefault: styleAsset("cover_default.svg")
    readonly property url assetDeckCueButton: buttonAsset("btn__cue_deck.svg")
    readonly property url assetDeckKeylockButton: buttonAsset("btn__keylock.svg")
    readonly property url assetDeckLoopButton: buttonAsset("btn__loop.svg")
    readonly property url assetDeckPlayButton: buttonAsset("btn__play_deck.svg")
    readonly property url assetDeckRateSliderBackground: sliderAsset("slider_pitch_deck.svg")
    readonly property url assetDeckRateSliderHandle: sliderAsset("knob_pitch_deck.svg")
    readonly property url assetDeckReverseButton: buttonAsset("btn__reverse.svg")
    readonly property url assetDeckSpinnyBackground: styleAsset("spinny_bg.svg")
    readonly property url assetDeckSpinnyGhostIndicator: styleAsset("spinny_indicator_ghost.svg")
    readonly property url assetDeckSpinnyIndicator: styleAsset("spinny_indicator.svg")
    readonly property url assetDeckSpinnyMask12: styleAsset("spinny_mask_12.svg")
    readonly property url assetDeckSpinnyMask34: styleAsset("spinny_mask_34.svg")
    readonly property url assetDeckSyncButton: buttonAsset("btn__sync_deck.svg")
    readonly property url assetDeckVolumeSliderBackground: sliderAsset("slider_volume_deck.svg")
    readonly property url assetDeckVolumeSliderHandle: sliderAsset("knob_volume_deck.svg")
    readonly property url assetFxKnobBackground: knobAsset("knob_bg_fx.svg")
    readonly property url assetMainKnobBackground: knobAsset("knob_bg_main.svg")
    readonly property url assetRegularKnobBackground: knobAsset("knob_bg_regular.svg")
    readonly property url assetSmallKnobBackground: knobAsset("knob_bg_small.svg")

    function colorSchemeAsset(kind, fileName) {
        return Qt.resolvedUrl("../../LateNight/" + ColorScheme.name + "/" + kind + "/" + fileName);
    }

    function buttonAsset(fileName) {
        return colorSchemeAsset("buttons", fileName);
    }

    function knobAsset(fileName) {
        return colorSchemeAsset("knobs", fileName);
    }

    function sliderAsset(fileName) {
        return colorSchemeAsset("sliders", fileName);
    }

    function styleAsset(fileName) {
        return colorSchemeAsset("style", fileName);
    }

    function sharedImage(fileName) {
        return Qt.resolvedUrl("../../../qml/images/" + fileName);
    }
}
