import "../Deck" as DeckControls
import "../LateNightTheme"
import Mixxx 1.0 as Mixxx
import QtQuick

DeckControls.LateNightIconButton {
    id: root

    required property string controlKey
    readonly property bool expanded: expandControl.value > 0

    activeColor: LateNightTheme.samplerColor
    backgroundSource: LateNightTheme.lateNightSubRegionButton("square")
    iconSource: root.expanded ? LateNightTheme.assetSamplerCollapseButton : LateNightTheme.assetSamplerExpandButton
    implicitHeight: 26
    implicitWidth: 20
    stretchIcon: true

    MouseArea {
        anchors.fill: parent
        cursorShape: Qt.PointingHandCursor

        onClicked: expandControl.value = root.expanded ? 0 : 1
    }
    Mixxx.ControlProxy {
        id: expandControl

        group: "[LateNight]"
        key: root.controlKey
    }
}
