import QtQuick 2.12
pragma Singleton

QtObject {
    property color white: "#e3d7fb"
    property color yellow: "#fca001"
    property color red: "#ea2a4e"
    property color blue: "#01dcfc"
    property color green: "#85c85b"
    property color lightGray: "#747474"
    property color lightGray2: "#b0b0b0"
    property color midGray: "#696969"
    property color darkGray: "#0f0f0f"
    property color darkGray2: "#2e2e2e"
    property color eqHighColor: white
    property color eqMidColor: white
    property color eqLowColor: white
    property color eqFxColor: red
    property color effectColor: yellow
    property color effectUnitColor: red
    property color bpmSliderBarColor: green
    property color volumeSliderBarColor: blue
    property color gainKnobColor: blue
    property color samplerColor: blue
    property color crossfaderOrientationColor: lightGray
    property color crossfaderBarColor: red
    property color toolbarBackgroundColor: darkGray2
    property color pflActiveButtonColor: blue
    property color backgroundColor: "#1e1e20"
    property color deckActiveColor: green
    property color deckBackgroundColor: darkGray
    property color knobBackgroundColor: "#262626"
    property color deckLineColor: darkGray2
    property color deckTextColor: lightGray2
    property color embeddedBackgroundColor: "#a0000000"
    property color buttonNormalColor: midGray
    property color textColor: lightGray2
    property color toolbarActiveColor: white
    property string fontFamily: "Open Sans"
    property int textFontPixelSize: 14
    property int buttonFontPixelSize: 10
    property string imgButton: "images/button.svg"
    property string imgButtonPressed: "images/button_pressed.svg"
    property string imgSliderHandle: "images/slider_handle.svg"
    property string imgBpmSliderBackground: "images/slider_bpm.svg"
    property string imgVolumeSliderBackground: "images/slider_volume.svg"
    property string imgCrossfaderHandle: "images/slider_handle_crossfader.svg"
    property string imgCrossfaderBackground: "images/slider_crossfader.svg"
    property string imgMicDuckingSliderHandle: "images/slider_handle_micducking.svg"
    property string imgMicDuckingSlider: "images/slider_micducking.svg"
    property string imgPopupBackground: imgButton
    property string imgKnob: "images/knob.svg"
    property string imgKnobShadow: "images/knob_shadow.svg"
    property string imgKnobMini: "images/miniknob.svg"
    property string imgKnobMiniShadow: "images/miniknob_shadow.svg"
    property string imgSectionBackground: "images/section.svg"
}
