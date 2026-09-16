pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import "../Controls" as Controls
import "../LateNightTheme"

Item {
    id: root

    required property bool show4decks

    implicitHeight: show4decks ? 192 : 96
    implicitWidth: LateNightTheme.compactVuSlotWidth
    readonly property int deckGroupWidth: LateNightTheme.compactVuDeckGroupWidth
    readonly property int mainGroupWidth: LateNightTheme.compactVuMainGroupWidth

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        MeterRow {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredHeight: 1
            leftDeckGroup: "[Channel1]"
            rightDeckGroup: "[Channel2]"
        }
        MeterRow {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredHeight: 1
            leftDeckGroup: "[Channel3]"
            rightDeckGroup: "[Channel4]"
            visible: root.show4decks
        }
    }

    component MeterRow: Item {
        id: meterRow

        required property string leftDeckGroup
        required property string rightDeckGroup

        Layout.fillHeight: true
        Layout.fillWidth: true
        implicitHeight: 96
        implicitWidth: root.implicitWidth

        Item {
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.top: parent.top
            width: root.deckGroupWidth

            Rectangle {
                anchors.fill: parent
                color: LateNightTheme.compactVuPanelColor
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

            Rectangle {
                anchors.left: parent.left
                anchors.leftMargin: 4
                anchors.right: parent.right
                anchors.rightMargin: 4
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                color: LateNightTheme.compactVuPanelColor
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

            Rectangle {
                anchors.fill: parent
                color: LateNightTheme.compactVuPanelColor
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
}
