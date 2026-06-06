pragma Singleton
import QtQuick
import "."

QtObject {
    readonly property color backgroundColor: "#1e1e1e"
    readonly property color buttonActiveColor: white
    readonly property color buttonNormalColor: "#696969"
    readonly property color buttonPressedColor: white
    readonly property color darkGray: "#0f0f0f"
    readonly property color accentColor: ColorScheme.accentColor
    readonly property color libraryPanelSplitterBackground: "#1e1e1e"
    readonly property color libraryPanelSplitterHandle: "#5f5f5f"
    readonly property color libraryPanelSplitterHandleActive: "#7a7a7a"
    readonly property color textColor: white
    readonly property color toolbarActiveColor: white
    readonly property color toolbarBackgroundColor: "#242424"
    readonly property int toolbarButtonHeight: 26
    readonly property int toolbarButtonWidth: 52
    readonly property color white: "#D9D9D9"

    readonly property url assetDeckBeatjumpLeftButton: legacyAsset("buttons", "btn__beatjump_left.svg")
    readonly property url assetDeckBeatjumpRightButton: legacyAsset("buttons", "btn__beatjump_right.svg")
    readonly property url assetDeckCoverDefault: legacyAsset("style", "cover_default.svg")
    readonly property url assetDeckCueButton: legacyAsset("buttons", "btn__cue_deck.svg")
    readonly property url assetDeckKeylockButton: legacyAsset("buttons", "btn__keylock.svg")
    readonly property url assetDeckLoopButton: legacyAsset("buttons", "btn__loop.svg")
    readonly property url assetDeckPlayButton: legacyAsset("buttons", "btn__play_deck.svg")
    readonly property url assetDeckRateSliderBackground: legacyAsset("sliders", "slider_pitch_deck.svg")
    readonly property url assetDeckRateSliderHandle: legacyAsset("sliders", "knob_pitch_deck.svg")
    readonly property url assetDeckReverseButton: legacyAsset("buttons", "btn__reverse.svg")
    readonly property url assetDeckSpinnyBackground: legacyAsset("style", "spinny_bg.svg")
    readonly property url assetDeckSpinnyGhostIndicator: legacyAsset("style", "spinny_indicator_ghost.svg")
    readonly property url assetDeckSpinnyIndicator: legacyAsset("style", "spinny_indicator.svg")
    readonly property url assetDeckSpinnyMask12: legacyAsset("style", "spinny_mask_12.svg")
    readonly property url assetDeckSpinnyMask34: legacyAsset("style", "spinny_mask_34.svg")
    readonly property url assetDeckSyncButton: legacyAsset("buttons", "btn__sync_deck.svg")
    readonly property url assetDeckVolumeSliderBackground: legacyAsset("sliders", "slider_volume_deck.svg")
    readonly property url assetDeckVolumeSliderHandle: legacyAsset("sliders", "knob_volume_deck.svg")
    readonly property url assetFxKnobBackground: legacyAsset("knobs", "knob_bg_fx.svg")
    readonly property url assetMainKnobBackground: legacyAsset("knobs", "knob_bg_main.svg")
    readonly property url assetRegularKnobBackground: legacyAsset("knobs", "knob_bg_regular.svg")
    readonly property url assetSmallKnobBackground: legacyAsset("knobs", "knob_bg_small.svg")

    function legacyAsset(directory, fileName) {
        return Qt.resolvedUrl("../../LateNight/" + ColorScheme.name + "/" + directory + "/" + fileName);
    }

    function sharedImage(fileName) {
        return Qt.resolvedUrl("../../../qml/images/" + fileName);
    }
}
