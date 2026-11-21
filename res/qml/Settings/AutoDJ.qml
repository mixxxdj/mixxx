import QtQuick
import QtQuick.Layouts
import Mixxx 1.0 as Mixxx
import "." as Setting
import "../Theme"

Category {
    id: root

    property bool dirty: false
    label: qsTr("Auto DJ")

    function load() {
        startOnLaunch.selected = Mixxx.Config.autoDJStartOnLaunch ? "on" : "off";
        root.dirty = false;
    }

    Component.onCompleted: root.load()

    ColumnLayout {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 14
        spacing: 10

        RowLayout {
            Layout.fillWidth: true

            Mixxx.SettingParameter {
                Layout.fillWidth: true
                label: qsTr("Start Auto DJ when Mixxx launches")

                Text {
                    anchors.fill: parent
                    color: Theme.white
                    font.pixelSize: 14
                    text: parent.label
                    verticalAlignment: Text.AlignVCenter
                }
            }
            Setting.RatioChoice {
                id: startOnLaunch

                options: ["on", "off"]
                onSelectedChanged: root.dirty = true
            }
        }
        Text {
            Layout.fillWidth: true
            color: Theme.white
            text: qsTr("Auto DJ may begin playing tracks immediately on startup.")
            wrapMode: Text.WordWrap
        }
    }

    Item {
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        height: 20

        Setting.FormButton {
            anchors.left: parent.left
            activeColor: "#999999"
            backgroundColor: "#7D3B3B"
            text: qsTr("Reset")
            onPressed: startOnLaunch.selected = "off"
        }
        Row {
            anchors.right: parent.right
            spacing: 10

            Setting.FormButton {
                activeColor: "#999999"
                backgroundColor: "#3F3F3F"
                text: qsTr("Cancel")
                visible: root.dirty
                onPressed: root.load()
            }
            Setting.FormButton {
                activeColor: "#999999"
                backgroundColor: root.dirty ? "#3a60be" : "#3F3F3F"
                enabled: root.dirty
                opacity: enabled ? 1.0 : 0.5
                text: qsTr("Save")
                onPressed: {
                    Mixxx.Config.autoDJStartOnLaunch = startOnLaunch.selected === "on";
                    root.load();
                }
            }
        }
    }
}
