import Mixxx 1.0 as Mixxx
import QtQuick
import "../LateNightTheme"

Item {
    id: root

    property int backgroundVariant: 0
    readonly property bool clipping: peakIndicatorControl.value > 0
    property bool drawGroove: true
    required property string group
    property real heldPeak: 0
    property double lastDecayTime: 0
    readonly property real level: Math.max(0, Math.min(1, levelControl.value))
    readonly property int levelHeight: root.micAux ? 41 : 81
    property string levelKey: "vu_meter"
    property bool micAux: false
    readonly property int peakFallTime: root.group === "[Main]" ? 50 : 10
    readonly property int peakHoldSize: root.group === "[Main]" ? 5 : 4
    property string peakKey: "peak_indicator"
    readonly property string variantName: backgroundVariant === 0 ? "dark" : "light"

    clip: true
    implicitHeight: root.micAux ? 54 : 96
    implicitWidth: 8

    onLevelChanged: {
        if (root.level === 0) {
            root.heldPeak = 0;
            peakHoldTimer.stop();
        } else if (root.level > root.heldPeak) {
            root.heldPeak = root.level;
            peakHoldTimer.restart();
        }
    }

    Timer {
        id: peakHoldTimer

        interval: 500
    }
    Timer {
        interval: 10
        repeat: true
        running: root.visible && !peakHoldTimer.running && root.heldPeak > 0

        onRunningChanged: if (running)
            root.lastDecayTime = Date.now()
        onTriggered: {
            const now = Date.now();
            root.heldPeak = Math.max(0, root.heldPeak - 2 * (now - root.lastDecayTime) / (root.peakFallTime * root.levelHeight));
            root.lastDecayTime = now;
        }
    }
    Rectangle {
        anchors.horizontalCenter: parent.horizontalCenter
        color: "#040404"
        height: parent.height
        visible: root.drawGroove
        width: 8
        y: 0
    }
    Image {
        anchors.horizontalCenter: parent.horizontalCenter
        fillMode: Image.PreserveAspectFit
        height: root.micAux ? 9 : 11
        smooth: false
        source: root.micAux ? LateNightTheme.assetMicAuxVuClippingBackground : LateNightTheme.mixerVuClipBackground(root.variantName)
        width: 6
        y: 1
    }
    Image {
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 1
        anchors.horizontalCenter: parent.horizontalCenter
        fillMode: Image.PreserveAspectFit
        height: root.levelHeight
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
        y: 1
    }
    Item {
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 1
        anchors.horizontalCenter: parent.horizontalCenter
        clip: true
        height: root.levelHeight * root.level
        width: 6

        Image {
            id: levelImage

            anchors.bottom: parent.bottom
            anchors.horizontalCenter: parent.horizontalCenter
            fillMode: Image.PreserveAspectFit
            height: root.levelHeight
            smooth: false
            source: root.micAux ? LateNightTheme.assetMicAuxVuLevelActive : LateNightTheme.lateNightAsset("style", "vu_deck_level_active.png")
            width: 6
        }
    }
    Item {
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 1 + root.levelHeight * root.heldPeak - height
        anchors.horizontalCenter: parent.horizontalCenter
        clip: true
        height: Math.min(root.peakHoldSize, root.levelHeight * root.heldPeak)
        visible: root.heldPeak > root.level
        width: 6

        Image {
            anchors.horizontalCenter: parent.horizontalCenter
            height: root.levelHeight
            source: levelImage.source
            sourceSize: Qt.size(6, root.levelHeight)
            width: 6
            y: -(root.levelHeight * (1 - root.heldPeak))
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
