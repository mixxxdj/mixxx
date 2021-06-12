import "." as Skin
import QtQuick 2.12
import QtQuick.Controls 2.12
import "Theme"

Item {
    id: root

    property real crossfaderWidth // required

    implicitHeight: crossfaderSlider.height + 5

    Item {
        id: effectUnitLeftPlaceholder

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.bottom: parent.bottom
    }

    Skin.ControlSlider {
        id: crossfaderSlider

        orientation: Qt.Horizontal
        anchors.centerIn: parent
        width: root.crossfaderWidth
        group: "[Master]"
        key: "crossfader"
        barColor: Theme.crossfaderBarColor
        barStart: 0.5
        fg: Theme.imgCrossfaderHandle
        bg: Theme.imgCrossfaderBackground
    }

    Item {
        id: effectUnitRightPlaceholder

        anchors.top: parent.top
        anchors.right: parent.right
        anchors.bottom: parent.bottom
    }

}
