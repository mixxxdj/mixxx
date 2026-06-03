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
