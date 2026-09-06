import QtQuick
import "../Controls" as LateNightControls
import "../LateNightTheme"

Item {
    id: root

    readonly property int fourDeckChannelHeight: root.showEqKnobs ? (root.showXfader ? 359 : 344) : (root.showXfader ? 197 : 182)
    required property var groups
    property bool show4decks: false
    property bool showEqKillButtons: true
    property bool showEqKnobs: true
    property bool showXfader: true

    implicitHeight: root.show4decks ? (root.showXfader ? 405 : root.fourDeckChannelHeight + 8) : (root.showXfader ? 203 : (root.showEqKnobs ? 182 : 164))
    implicitWidth: root.show4decks ? 278 : (root.showEqKnobs ? (root.showEqKillButtons ? 278 : 242) : (root.showXfader ? 163 : 151))

    Item {
        anchors.horizontalCenter: parent.horizontalCenter
        height: parent.height
        visible: !root.show4decks
        width: 278
        y: root.showXfader ? 0 : Math.max(0, (parent.height - (root.showEqKnobs ? 174 : 156)) / 2)

        MixerChannel2Deck {
            anchors.right: parent.horizontalCenter
            anchors.rightMargin: 25
            group: root.groups[0]
            showEqKillButtons: root.showEqKillButtons
            showEqKnobs: root.showEqKnobs
            showXfader: root.showXfader
        }
        MixerChannel2Deck {
            anchors.left: parent.horizontalCenter
            anchors.leftMargin: 27
            group: root.groups[1]
            mirror: true
            showEqKillButtons: root.showEqKillButtons
            showEqKnobs: root.showEqKnobs
            showXfader: root.showXfader
        }
        Row {
            spacing: 3
            x: 140 - width / 2
            y: 17

            PflButton {
                group: root.groups[0]
            }
            PflButton {
                group: root.groups[1]
            }
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
            x: 118
            y: 55
        }
        Rectangle {
            color: "#040404"
            height: 96
            width: 14
            x: 133
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
            x: 134
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
            x: 140
            y: 55
        }
        LateNightControls.ImageVuMeter {
            group: root.groups[1]
            height: 96
            width: 8
            x: 154
            y: 55
        }
        Crossfader {
            anchors.horizontalCenter: parent.horizontalCenter
            compact: !root.showEqKnobs
            groups: [root.groups[0], root.groups[1]]
            showAssignments: false
            visible: root.showXfader
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
                showXfader: root.showXfader
            }
            MixerChannel4Deck {
                group: root.groups[0]
                leftStyle: "default"
                rightStyle: "warning"
                showEqKillButtons: root.showEqKillButtons
                showEqKnobs: root.showEqKnobs
                showXfader: root.showXfader
            }
            MixerChannel4Deck {
                group: root.groups[1]
                leftStyle: "warning"
                rightStyle: "default"
                showEqKillButtons: root.showEqKillButtons
                showEqKnobs: root.showEqKnobs
                showXfader: root.showXfader
            }
            MixerChannel4Deck {
                group: root.groups[3]
                leftStyle: "warning"
                rightStyle: "default"
                showEqKillButtons: root.showEqKillButtons
                showEqKnobs: root.showEqKnobs
                showXfader: root.showXfader
            }
        }
        Crossfader {
            anchors.horizontalCenter: parent.horizontalCenter
            groups: root.groups
            showAssignments: false
            visible: root.showXfader
            y: root.fourDeckChannelHeight + Math.max(0, (parent.height - root.fourDeckChannelHeight - height) / 2)
        }
    }
}
