pragma ComponentBehavior: Bound

import QtQuick
import "../Controls" as Controls
import "../LateNightTheme"

Item {
    id: root

    required property bool show4decks

    clip: true
    implicitHeight: show4decks ? 192 : 96
    implicitWidth: LateNightTheme.compactVuSlotWidth
    readonly property int deckGroupWidth: LateNightTheme.compactVuDeckGroupWidth
    readonly property int mainGroupWidth: LateNightTheme.compactVuMainGroupWidth
    readonly property real rowHeight: (height - (show4decks ? LateNightTheme.deckRowGutter : 0)) / (show4decks ? 2 : 1)

    MeterRow {
        height: root.rowHeight
        leftDeckGroup: "[Channel1]"
        rightDeckGroup: "[Channel2]"
        width: parent.width
        y: 0
    }
    MeterRow {
        height: root.rowHeight
        leftDeckGroup: "[Channel3]"
        rightDeckGroup: "[Channel4]"
        visible: root.show4decks
        width: parent.width
        y: root.rowHeight + LateNightTheme.deckRowGutter
    }

    component MeterRow: Item {
        id: meterRow

        required property string leftDeckGroup
        required property string rightDeckGroup

        clip: true
        implicitHeight: 96
        implicitWidth: root.implicitWidth

        Item {
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.top: parent.top
            width: root.deckGroupWidth

            MeterPanel {
                anchors.fill: parent
                showLeftBorder: LateNightTheme.isClassic
            }
            Controls.ImageVuMeter {
                anchors.left: parent.left
                anchors.leftMargin: LateNightTheme.isClassic ? 6 : 5
                anchors.right: parent.right
                anchors.rightMargin: 5
                anchors.verticalCenter: parent.verticalCenter
                backgroundVariant: 1
                group: meterRow.leftDeckGroup
                height: LateNightTheme.compactVuMeterHeight
            }
        }
        Item {
            anchors.bottom: parent.bottom
            anchors.top: parent.top
            anchors.horizontalCenter: parent.horizontalCenter
            width: root.mainGroupWidth

            MeterPanel {
                anchors.left: parent.left
                anchors.leftMargin: 4
                anchors.right: parent.right
                anchors.rightMargin: 4
                anchors.top: parent.top
                anchors.bottom: parent.bottom
            }
            Controls.ImageVuMeter {
                anchors.verticalCenter: parent.verticalCenter
                backgroundVariant: 1
                drawGroove: false
                group: "[Main]"
                height: LateNightTheme.compactVuMeterHeight
                levelKey: "vu_meter_left"
                peakKey: "peak_indicator_left"
                width: 6
                x: 10
            }
            Controls.ImageVuMeter {
                anchors.verticalCenter: parent.verticalCenter
                backgroundVariant: 1
                drawGroove: false
                group: "[Main]"
                height: LateNightTheme.compactVuMeterHeight
                levelKey: "vu_meter_right"
                peakKey: "peak_indicator_right"
                width: 6
                x: 16
            }
        }
        Item {
            anchors.bottom: parent.bottom
            anchors.right: parent.right
            anchors.top: parent.top
            width: root.deckGroupWidth

            MeterPanel {
                anchors.fill: parent
                showRightBorder: LateNightTheme.isClassic
            }
            Controls.ImageVuMeter {
                anchors.left: parent.left
                anchors.leftMargin: 5
                anchors.right: parent.right
                anchors.rightMargin: LateNightTheme.isClassic ? 6 : 5
                anchors.verticalCenter: parent.verticalCenter
                backgroundVariant: 1
                group: meterRow.rightDeckGroup
                height: LateNightTheme.compactVuMeterHeight
            }
        }
    }

    component MeterPanel: Rectangle {
        property bool showLeftBorder: true
        property bool showRightBorder: true

        color: LateNightTheme.compactVuPanelColor

        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            color: LateNightTheme.compactVuPanelBorderColor
            height: 1
            z: 2
        }
        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            color: LateNightTheme.compactVuPanelBorderBottomColor
            height: 1
            z: 2
        }
        Rectangle {
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            color: LateNightTheme.compactVuPanelBorderLeftColor
            visible: parent.showLeftBorder
            width: 1
            z: 1
        }
        Rectangle {
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            color: LateNightTheme.compactVuPanelBorderRightColor
            visible: parent.showRightBorder
            width: 1
            z: 1
        }
    }
}
