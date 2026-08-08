import QtQuick
import "../../../qml" as Skin
import "../LateNightTheme"

Skin.ControlFader {
    id: root

    property int backgroundMargin: 0
    property alias backgroundSource: backgroundImage.source
    property bool centeredBar: false
    property real centeredBarAxis: height / 2
    property alias handleSource: handleImage.source
    property color valueLineColor: LateNightTheme.mixerSliderBarColor

    bar: !centeredBar
    barColor: valueLineColor
    barMargin: 8
    barWidth: 2
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
        height: implicitHeight
        width: implicitWidth
        x: root.horizontal ? root.visualPosition * (root.width - width) : (root.width - width) / 2
        y: root.vertical ? root.visualPosition * (root.height - height) : (root.height - height) / 2
    }

    Rectangle {
        readonly property real handleCenter: handleImage.x + handleImage.width / 2
        readonly property real start: root.width / 2

        color: root.valueLineColor
        height: root.barWidth
        radius: height / 2
        visible: root.centeredBar && root.horizontal && width > 0
        width: Math.abs(handleCenter - start)
        x: Math.min(start, handleCenter)
        y: root.centeredBarAxis - height / 2
        z: 1
    }
}
