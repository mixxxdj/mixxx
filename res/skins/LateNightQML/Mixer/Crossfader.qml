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
        barMargin: 7
        barStart: 0.5
        centeredBar: true
        centeredBarAxis: 19
        group: "[Master]"
        handleSource: LateNightTheme.assetMixerCrossfaderHandle
        height: implicitHeight
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

            Row {
                property string assignmentGroup: root.groups[index]
                required property int index

                spacing: 0

                CrossfaderAssignButton {
                    activeStyle: index < 2 ? "default" : "warning"
                    group: parent.assignmentGroup
                    label: "left"
                    side: 0
                }
                CrossfaderAssignButton {
                    activeStyle: "warning"
                    group: parent.assignmentGroup
                    label: "mid"
                    side: 1
                }
                CrossfaderAssignButton {
                    activeStyle: index < 2 ? "default" : "warning"
                    group: parent.assignmentGroup
                    label: "right"
                    side: 2
                }
            }
        }
    }
}
