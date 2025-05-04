import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Dialogs
import Qt.labs.qmlmodels
import Qt5Compat.GraphicalEffects
import Mixxx 1.0 as Mixxx
import ".." as Skin
import "../Theme"

Mixxx.SettingGroup {
    id: root
    label: "Controllers"
    property var selectedController: null
    property var manager: Mixxx.ControllerManager
    property bool dirty: false
    Component.onCompleted: {
        if (manager.knownDevices) {
        //     console.log("knownDevices", JSON.stringify(manager.knownDevices))
        //     console.log("unknownDevices", JSON.stringify(manager.unknownDevices))
            // console.log("settingTree", JSON.stringify(manager.knownDevices[0].availableMappings[0]))
        //     console.log("settingTree", JSON.stringify(manager.knownDevices[0].availableMappings[0].getSettingTree(Mixxx.Config)))
        }
    }
    ColumnLayout {
        anchors.leftMargin: root.selectedController ? 10 : 20
        anchors.rightMargin: 14
        anchors.fill: parent
        spacing: root.selectedController ? 18 : 0
        GridLayout {
            columns: 2
            columnSpacing: 24
            Repeater {
                id: controllers
                model: {
                    if (root.selectedController) {
                        if (root.selectedController.hasSettings)
                            return [root.selectedController]
                        else if (manager.knownDevices.indexOf(root.selectedController) == -1)
                            return [...manager.knownDevices, root.selectedController]
                    }
                    return manager.knownDevices
                }
                delegate: ControllerCard {
                    id: controller
                    focused: knownDevice && root.selectedController == modelData
                    knownDevice: manager.knownDevices.indexOf(modelData) > -1
                    opacity: root.selectedController && !root.selectedController.knownDevice && root.selectedController != modelData ? 0.2 : 1
                    onSelect: {
                        modelData.hasSettings = controller.selectedMapping?.hasSettings
                        root.selectedController = modelData
                    }
                    onDirtyChanged: {
                        root.dirty |= dirty
                        console.log("dirty", root.dirty, dirty)
                    }
                    onSettingsChanged: {
                        if (!controller.settings && controller.focused) {
                            root.selectedController = null
                        }
                    }
                }
            }
            Item {
                visible: root.selectedController == null || !root.selectedController.hasSettings
                Layout.fillWidth: true
                Layout.fillHeight: true
            }
        }
        ListView {
            Layout.fillWidth: true
            implicitHeight: contentHeight
            visible: root.selectedController == null

            header: Rectangle {
                // implicitWidth: 120
                implicitHeight: 32
                width: ListView.view.width
                color: "#161616"
                Text {
                    color: '#FFFFFF'
                    anchors.fill: parent
                    anchors.margins: 10
                    anchors.leftMargin: 20
                    anchors.rightMargin: 20
                    text: 'Other detected devices'
                    font.pixelSize: 16
                    font.weight: Font.DemiBold
                }
            }

            model: manager.unknownDevices
            delegate: Rectangle {
                height: 32
                width: ListView.view.width
                required property int index
                required property var modelData
                color: index % 2 == 0 ? '#0C0C0C' : '#272727'
                MouseArea {
                    id: unknownDeviceRow
                    hoverEnabled: true
                    anchors.fill: parent
                }
                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 6
                    Rectangle {
                        color: '#393939'
                        Layout.preferredWidth: 35
                        Layout.preferredHeight: 18
                        radius: 4
                        Text {
                            anchors.centerIn: parent
                            text: {
                                switch (modelData.type) {
                                    case Mixxx.ControllerDevice.Type.MIDI:
                                        return "MIDI";
                                    case Mixxx.ControllerDevice.Type.HID:
                                        return "HID";
                                    case Mixxx.ControllerDevice.Type.BULK:
                                        return "BULK";
                                }
                                return "UNKN";
                            }
                            color: '#FFFFFF'
                            font.pixelSize: 14
                            font.weight: Font.Medium
                        }
                    }
                    Text {
                        Layout.fillWidth: true
                        text: modelData.name
                        color: '#FFFFFF'
                        font.pixelSize: 14
                        font.weight: Font.Medium
                    }
                    Skin.FormButton {
                        visible: unknownDeviceRow.containsMouse || hovered
                        text: "Create device"
                        activeColor: "#999999"
                        onPressed: {
                            root.selectedController = modelData
                            console.log(root.selectedController)
                        }
                    }
                }
            }
        }
        RowLayout {
            visible: root.dirty || !!root.selectedController?.hasSettings || (root.selectedController && !root.selectedController.knownDevice)
            Layout.leftMargin: 14
            Layout.topMargin: 18
            Layout.rightMargin: 14
            Skin.FormButton {
                visible: !!root.selectedController?.hasSettings
                text: "Reset"
                opacity: enabled ? 1.0 : 0.5
                backgroundColor: "#7D3B3B"
                activeColor: "#999999"
                onPressed: {
                    root.load()
                }
            }
            Item {
                Layout.fillWidth: true
            }
            Text {
                Layout.alignment: Qt.AlignVCenter
                Layout.rightMargin: 16
                id: errorMessage
                text: ""
                color: "#7D3B3B"
            }
            Skin.FormButton {
                text: "Cancel"
                opacity: enabled ? 1.0 : 0.5
                backgroundColor: "#3F3F3F"
                activeColor: "#999999"
                onPressed: {
                    root.selectedController = null
                    // errorMessage.text = ""
                    // root.save()
                }
            }
            Skin.FormButton {
                text: "Save"
                opacity: enabled ? 1.0 : 0.5
                backgroundColor: root.hasChanges ? "#3a60be" : "#3F3F3F"
                activeColor: "#999999"
                onPressed: {
                    errorMessage.text = ""
                    root.save()
                }
            }
        }
    }

    function save() {
    }
}
