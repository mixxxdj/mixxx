import "." as Skin
import Mixxx 1.0 as Mixxx
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "Deck" as DeckComponent
import "Theme"

Rectangle {
    id: root

    required property string group
    required property string label

    color: Theme.embeddedBackgroundColor
    radius: 5

    Mixxx.ControlProxy {
        id: bpmControl

        group: root.group
        key: "bpm"
    }
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 6
        spacing: 4

        RowLayout {
            Layout.fillWidth: true
            Layout.maximumHeight: 22
            Layout.minimumHeight: 22
            Layout.preferredHeight: 22

            Label {
                Layout.fillWidth: true
                color: Theme.textColor
                font.bold: true
                font.pixelSize: 16
                text: root.label
            }
            Label {
                color: Theme.textColor
                font.pixelSize: 14
                text: Number(bpmControl.value).toFixed(2) + " BPM"
            }
        }
        DeckComponent.InfoBar {
            Layout.fillWidth: true
            Layout.maximumHeight: 34
            Layout.minimumHeight: 34
            Layout.preferredHeight: 34
            group: root.group
            minimized: true
            rightColumnWidth: 70
        }
        Skin.WaveformDisplay {
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.minimumHeight: 70
            group: root.group
        }
        RowLayout {
            Layout.fillWidth: true
            Layout.maximumHeight: 40
            Layout.minimumHeight: 40
            Layout.preferredHeight: 40
            spacing: 6

            DeckComponent.PlayButton {
                Layout.fillHeight: true
                Layout.maximumWidth: 72
                Layout.minimumWidth: 72
                Layout.preferredWidth: 72
                group: root.group
                minimized: true
            }
            DeckComponent.CueButton {
                Layout.fillHeight: true
                Layout.maximumWidth: 72
                Layout.minimumWidth: 72
                Layout.preferredWidth: 72
                group: root.group
                minimized: true
            }
            Item {
                Layout.fillWidth: true
            }
        }
    }
}
