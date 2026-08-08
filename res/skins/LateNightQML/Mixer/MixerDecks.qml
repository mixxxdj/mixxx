import QtQuick
import "../Controls" as LateNightControls
import "../LateNightTheme"

Item {
    id: root

    required property var groups
    property bool show4decks: false
    property bool showEqKillButtons: true
    property bool showEqKnobs: true
    property bool showXfader: true

    implicitHeight: show4decks ? 405 : 203
    implicitWidth: show4decks ? (showEqKillButtons ? 278 : 206) : (showEqKillButtons ? 278 : 242)

    Item {
        anchors.horizontalCenter: parent.horizontalCenter
        height: parent.height
        visible: !root.show4decks
        width: 278

        MixerChannel2Deck {
            anchors.right: parent.horizontalCenter
            anchors.rightMargin: 25
            group: root.groups[0]
            showEqKillButtons: root.showEqKillButtons
            showEqKnobs: root.showEqKnobs
        }
        MixerChannel2Deck {
            anchors.left: parent.horizontalCenter
            anchors.leftMargin: 27
            group: root.groups[1]
            mirror: true
            showEqKillButtons: root.showEqKillButtons
            showEqKnobs: root.showEqKnobs
        }
        PflButton {
            group: root.groups[0]
            x: 110
            y: 17
        }
        PflButton {
            group: root.groups[1]
            x: 139
            y: 17
        }
        LateNightControls.Knob {
            backgroundSource: LateNightTheme.assetRegularKnobBackground
            displayArc: true
            displayArcColor: LateNightTheme.mixerArcGainColor
            displayArcRadius: LateNightTheme.mixerArcRadiusBig
            displayArcStart: LateNightControls.Knob.ArcStart.Center
            displayArcWidth: LateNightTheme.mixerArcWidth
            group: root.groups[0]
            height: 34
            indicatorColor: "orange"
            indicatorKind: "regular"
            key: "pregain"
            width: 40
            x: 70
            y: 10
        }
        LateNightControls.Knob {
            backgroundSource: LateNightTheme.assetRegularKnobBackground
            displayArc: true
            displayArcColor: LateNightTheme.mixerArcGainColor
            displayArcRadius: LateNightTheme.mixerArcRadiusBig
            displayArcStart: LateNightControls.Knob.ArcStart.Center
            displayArcWidth: LateNightTheme.mixerArcWidth
            group: root.groups[1]
            height: 34
            indicatorColor: "orange"
            indicatorKind: "regular"
            key: "pregain"
            width: 40
            x: 170
            y: 10
        }
        LateNightControls.ImageVuMeter {
            group: root.groups[0]
            height: 96
            width: 8
            x: 115
            y: 55
        }
        Rectangle {
            color: "#040404"
            height: 96
            width: 14
            x: 130
            y: 55
        }
        LateNightControls.ImageVuMeter {
            backgroundVariant: 0
            drawGroove: false
            group: "[Main]"
            height: 96
            levelKey: "vu_meter_left"
            peakKey: "peak_indicator_left"
            width: 6
            x: 131
            y: 55
        }
        LateNightControls.ImageVuMeter {
            backgroundVariant: 0
            drawGroove: false
            group: "[Main]"
            height: 96
            levelKey: "vu_meter_right"
            peakKey: "peak_indicator_right"
            width: 6
            x: 137
            y: 55
        }
        LateNightControls.ImageVuMeter {
            group: root.groups[1]
            height: 96
            width: 8
            x: 151
            y: 55
        }
        Crossfader {
            compact: !root.showEqKnobs
            groups: [root.groups[0], root.groups[1]]
            showAssignments: false
            visible: root.showXfader
            x: root.showEqKnobs ? 82 : 90
            y: 155
        }
    }
    Item {
        anchors.fill: parent
        visible: root.show4decks

        Row {
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 2

            MixerChannel4Deck {
                group: root.groups[2]
                leftStyle: "default"
                rightStyle: "warning"
                showEqKillButtons: root.showEqKillButtons
                showEqKnobs: root.showEqKnobs
            }
            MixerChannel4Deck {
                group: root.groups[0]
                leftStyle: "default"
                rightStyle: "warning"
                showEqKillButtons: root.showEqKillButtons
                showEqKnobs: root.showEqKnobs
            }
            MixerChannel4Deck {
                group: root.groups[1]
                leftStyle: "warning"
                rightStyle: "default"
                showEqKillButtons: root.showEqKillButtons
                showEqKnobs: root.showEqKnobs
            }
            MixerChannel4Deck {
                group: root.groups[3]
                leftStyle: "warning"
                rightStyle: "default"
                showEqKillButtons: root.showEqKillButtons
                showEqKnobs: root.showEqKnobs
            }
        }
        Crossfader {
            anchors.horizontalCenter: parent.horizontalCenter
            groups: root.groups
            showAssignments: false
            visible: root.showXfader
            y: 365
        }
    }
}
