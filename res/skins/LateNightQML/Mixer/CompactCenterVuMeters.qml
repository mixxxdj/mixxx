pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import "../Controls" as Controls

Item {
    id: root

    required property bool show4decks

    implicitHeight: show4decks ? 192 : 96
    implicitWidth: 34

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        MeterRow {
            Layout.fillWidth: true
            Layout.preferredHeight: 96
            leftDeckGroup: "[Channel1]"
            rightDeckGroup: "[Channel2]"
        }
        MeterRow {
            Layout.fillWidth: true
            Layout.preferredHeight: root.show4decks ? 96 : 0
            leftDeckGroup: "[Channel3]"
            rightDeckGroup: "[Channel4]"
            visible: root.show4decks
        }
    }

    component MeterRow: RowLayout {
        id: meterRow

        required property string leftDeckGroup
        required property string rightDeckGroup

        height: 96
        spacing: 1
        width: root.width

        Controls.ImageVuMeter {
            Layout.fillHeight: true
            Layout.preferredWidth: 8
            backgroundVariant: 1
            group: meterRow.leftDeckGroup
        }
        Controls.ImageVuMeter {
            Layout.fillHeight: true
            Layout.preferredWidth: 7
            backgroundVariant: 1
            group: "[Main]"
            levelKey: "vu_meter_left"
            peakKey: "peak_indicator_left"
        }
        Controls.ImageVuMeter {
            Layout.fillHeight: true
            Layout.preferredWidth: 7
            backgroundVariant: 1
            group: "[Main]"
            levelKey: "vu_meter_right"
            peakKey: "peak_indicator_right"
        }
        Controls.ImageVuMeter {
            Layout.fillHeight: true
            Layout.preferredWidth: 8
            backgroundVariant: 1
            group: meterRow.rightDeckGroup
        }
    }
}
