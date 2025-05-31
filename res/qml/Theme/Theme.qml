import QtQuick 2.12
pragma Singleton

QtObject {
    property color accentColor: "#3a60be"
    property color backgroundColor: "#1e1e20"
    property color blue: "#01dcfc"
    property color bpmSliderBarColor: green
    property color buttonNormalColor: midGray
    property color crossfaderBarColor: red
    property color crossfaderOrientationColor: lightGray
    property color darkGray: "#0f0f0f"
    property color darkGray2: "#2e2e2e"
    property color darkGray3: "#3F3F3F"
    property color deckActiveColor: green
    property color deckBackgroundColor: darkGray
    property color deckLineColor: darkGray2
    property color deckTextColor: lightGray2
    property color effectColor: yellow
    property color effectUnitColor: red
    property color embeddedBackgroundColor: "#a0000000"
    property color eqFxColor: red
    property color eqHighColor: white
    property color eqLowColor: white
    property color eqMidColor: white
    property color gainKnobColor: blue
    property color green: "#85c85b"
    property color knobBackgroundColor: "#262626"
    property color lightGray: "#747474"
    property color lightGray2: "#b0b0b0"
    property color midGray: "#696969"
    property color pflActiveButtonColor: blue
    property color red: "#ea2a4e"
    property color samplerColor: blue
    property color textColor: lightGray2
    property color toolbarActiveColor: white
    property color toolbarBackgroundColor: darkGray2
    property color volumeSliderBarColor: blue
    property color warningColor: "#7D3B3B"
    property color waveformBeatColor: lightGray
    property color waveformCursorColor: white
    property color waveformMarkerDefault: '#ff7a01'
    property color waveformMarkerIntroOutroColor: '#2c5c9a'
    property color waveformMarkerLabel: Qt.rgba(255, 255, 255, 0.8)
    property color waveformMarkerLoopColor: '#00b400'
    property color waveformMarkerLoopColorDisabled: '#FFFFFF'
    property color waveformPostrollColor: midGray
    property color waveformPrerollColor: midGray
    property color white: "#D9D9D9"
    property color yellow: "#fca001"
    property int buttonFontPixelSize: 10
    property int textFontPixelSize: 14
    property string fontFamily: "Open Sans"
    property string imgBpmSliderBackground: "images/slider_bpm.svg"
    property string imgButton: "images/button.svg"
    property string imgButtonPressed: "images/button_pressed.svg"
    property string imgCrossfaderBackground: "images/slider_crossfader.svg"
    property string imgCrossfaderHandle: "images/slider_handle_crossfader.svg"
    property string imgKnob: "images/knob.svg"
    property string imgKnobMini: "images/miniknob.svg"
    property string imgKnobMiniShadow: "images/miniknob_shadow.svg"
    property string imgKnobShadow: "images/knob_shadow.svg"
    property string imgMicDuckingSlider: "images/slider_micducking.svg"
    property string imgMicDuckingSliderHandle: "images/slider_handle_micducking.svg"
    property string imgPopupBackground: imgButton
    property string imgSectionBackground: "images/section.svg"
    property string imgSliderHandle: "images/slider_handle.svg"
    property string imgVolumeSliderBackground: "images/slider_volume.svg"
}
