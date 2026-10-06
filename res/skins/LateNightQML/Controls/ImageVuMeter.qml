import Mixxx 1.0 as Mixxx
import QtQuick
import "../LateNightTheme"

Item {
    id: root

    property int backgroundVariant: 0
    property real clipThreshold: 0.98
    readonly property bool clipping: peakIndicatorControl.value > 0 || levelControl.value >= clipThreshold
    property bool drawGroove: true
    required property string group
    property bool micAux: false
    readonly property real level: Math.max(0, Math.min(1, levelControl.value))
    property string levelKey: "vu_meter"
    property string peakKey: "peak_indicator"
    readonly property string variantName: backgroundVariant === 0 ? "dark" : "light"

    clip: true
    implicitHeight: root.micAux ? 55 : 96
    implicitWidth: 8

    Rectangle {
        anchors.horizontalCenter: parent.horizontalCenter
        color: "#040404"
        height: root.micAux ? 50 : parent.height
        visible: root.drawGroove
        width: 8
        y: root.micAux ? 2 : 0
    }
    Image {
        anchors.horizontalCenter: parent.horizontalCenter
        fillMode: Image.PreserveAspectFit
        height: root.micAux ? 9 : 11
        smooth: false
        source: root.micAux ? LateNightTheme.assetMicAuxVuClippingBackground : LateNightTheme.mixerVuClipBackground(root.variantName)
        width: 6
        y: root.micAux ? 2 : 1
    }
    Image {
        anchors.bottom: parent.bottom
        anchors.bottomMargin: root.micAux ? 3 : 1
        anchors.horizontalCenter: parent.horizontalCenter
        fillMode: Image.PreserveAspectFit
        height: root.micAux ? 41 : 81
        smooth: false
        source: root.micAux ? LateNightTheme.assetMicAuxVuLevelBackground : LateNightTheme.mixerVuLevelBackground(root.variantName)
        width: 6
    }
    Image {
        anchors.horizontalCenter: parent.horizontalCenter
        fillMode: Image.PreserveAspectFit
        height: root.micAux ? 9 : 11
        opacity: root.clipping ? 1 : 0
        smooth: false
        source: root.micAux ? LateNightTheme.assetMicAuxVuClippingActive : LateNightTheme.lateNightAsset("style", "vu_deck_clipping_active.png")
        width: 6
        y: root.micAux ? 2 : 1
    }
    Item {
        anchors.bottom: parent.bottom
        anchors.bottomMargin: root.micAux ? 3 : 1
        anchors.horizontalCenter: parent.horizontalCenter
        clip: true
        height: (root.micAux ? 41 : 81) * root.level
        width: 6

        Image {
            anchors.bottom: parent.bottom
            anchors.horizontalCenter: parent.horizontalCenter
            fillMode: Image.PreserveAspectFit
            height: root.micAux ? 41 : 81
            smooth: false
            source: root.micAux ? LateNightTheme.assetMicAuxVuLevelActive : LateNightTheme.lateNightAsset("style", "vu_deck_level_active.png")
            width: 6
        }
    }
    Mixxx.ControlProxy {
        id: levelControl

        group: root.group
        key: root.levelKey
    }
    Mixxx.ControlProxy {
        id: peakIndicatorControl

        group: root.group
        key: root.peakKey
    }
}
