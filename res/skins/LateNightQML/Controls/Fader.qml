import QtQuick
import "../../../qml" as Skin
import "../LateNightTheme"

Skin.ControlFader {
    id: root

    property int backgroundMargin: 0
    property alias backgroundSource: backgroundImage.source
    property real handleHeight: 0
    property alias handleSource: handleImage.source
    property real handleWidth: 0
    property color valueLineColor: LateNightTheme.mixerSliderBarColor

    bar.color: valueLineColor
    bar.enabled: true
    bar.margin: 8
    bar.width: 2
    implicitHeight: backgroundImage.implicitHeight + (backgroundMargin * 2)
    implicitWidth: backgroundImage.implicitWidth + (backgroundMargin * 2)
    showDefaultHandle: false

    background: Image {
        id: backgroundImage

        anchors.fill: parent
        anchors.margins: root.backgroundMargin
        fillMode: Image.PreserveAspectFit
    }
    handle: Image {
        id: handleImage

        fillMode: Image.PreserveAspectFit
        height: root.handleHeight > 0 ? root.handleHeight : implicitHeight
        width: root.handleWidth > 0 ? root.handleWidth : implicitWidth
        x: root.horizontal ? Math.round(root.visualPosition * (root.width - width)) : Math.round((root.width - width) / 2)
        y: root.vertical ? Math.round(root.visualPosition * (root.height - height)) : Math.round((root.height - height) / 2)
    }
}
