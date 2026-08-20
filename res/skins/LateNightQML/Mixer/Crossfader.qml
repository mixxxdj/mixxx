import Mixxx 1.0 as Mixxx
import QtQuick
import "../Controls" as LateNightControls
import "../LateNightTheme"

Item {
    id: root

    property bool compact: false
    required property var groups
    property bool showAssignments: true

    implicitHeight: 40
    implicitWidth: compact ? 85 : 115

    LateNightControls.Fader {
        anchors.centerIn: parent
        backgroundSource: root.compact ? LateNightTheme.assetMixerCrossfaderSmallBackground : LateNightTheme.assetMixerCrossfaderBackground
        bar.axis: 19
        bar.margin: 7
        bar.start: 0.5
        group: "[Master]"
        handleHeight: 40
        handleSource: LateNightTheme.assetMixerCrossfaderHandle
        handleWidth: 19
        height: 40
        key: "crossfader"
        orientation: Qt.Horizontal
        width: root.compact ? 85 : 115
    }
    Row {
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        opacity: root.showAssignments ? 1 : 0
        spacing: 0

        Repeater {
            model: root.groups.length

            CrossfaderAssignButton {
                required property int index

                group: root.groups[index]
                leftStyle: index < 2 ? "default" : "warning"
                rightStyle: index < 2 ? "warning" : "default"
            }
        }
    }
}
